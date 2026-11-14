"""Small synchronous Debug Adapter Protocol client for Prysma integration tests.

Uses DAP's Content-Length framing and queues unrelated events/responses so
waiting for one response does not silently discard another message.
This is test infrastructure, not a production debugger implementation.
"""

from __future__ import annotations

import json
import socket
import time
from typing import Any, Callable

Message = dict[str, Any]


class DAPClient:
    MAX_BODY_BYTES = 8 * 1024 * 1024

    def __init__(self, host: str = "127.0.0.1", port: int = 5678, timeout: float = 15.0):
        self.host = host
        self.port = port
        self.default_timeout = timeout
        self.sock: socket.socket | None = None
        self.seq = 1
        self._buffer = b""
        self._responses: dict[int, Message] = {}
        self._events: list[Message] = []

    def _disconnect_transport(self) -> None:
        """Close transport without discarding messages already received."""
        sock, self.sock = self.sock, None
        if sock is not None:
            try:
                sock.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            sock.close()

    def close(self) -> None:
        """Explicitly finish a session and discard its queued messages."""
        self._disconnect_transport()
        self._buffer = b""
        self._responses.clear()
        self._events.clear()

    def connect_and_initialize(
        self, arguments: Message | None = None, timeout: float = 15.0
    ) -> Message | None:
        """Retry until a real DAP initialize response is received or deadline expires."""
        defaults: Message = {
            "clientID": "prysma-integration-test",
            "adapterID": "debugpy",
            "pathFormat": "path",
            "linesStartAt1": True,
            "columnsStartAt1": True,
        }
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            self.close()
            remaining = deadline - time.monotonic()
            try:
                sock = socket.create_connection(
                    (self.host, self.port), timeout=min(2.0, remaining)
                )
                self.sock = sock
                request_id = self.send_request("initialize", arguments or defaults)
                response = self.wait_for_response(
                    request_id, timeout=min(4.0, max(0.1, deadline - time.monotonic()))
                )
                if response is not None and response.get("success") is True:
                    return response
            except (OSError, ValueError):
                pass
            if time.monotonic() < deadline:
                time.sleep(min(0.2, deadline - time.monotonic()))
        self.close()
        return None

    def send_request(self, command: str, arguments: Message | None = None) -> int:
        if self.sock is None:
            raise ConnectionError("DAP client is not connected")
        request_id = self.seq
        self.seq += 1
        body = json.dumps({"seq": request_id, "type": "request", "command": command,
                           "arguments": arguments or {}}).encode("utf-8")
        self.sock.settimeout(self.default_timeout)
        self.sock.sendall(f"Content-Length: {len(body)}\r\n\r\n".encode("ascii") + body)
        return request_id

    def _receive(self, deadline: float) -> Message:
        while True:
            header_end = self._buffer.find(b"\r\n\r\n")
            if header_end > 8192:
                raise ValueError("DAP header exceeds 8192 bytes")
            if header_end >= 0:
                headers = self._buffer[:header_end].decode("ascii").split("\r\n")
                lengths = [int(line.split(":", 1)[1]) for line in headers
                           if line.lower().startswith("content-length:")]
                if len(lengths) != 1 or not 0 < lengths[0] <= self.MAX_BODY_BYTES:
                    raise ValueError("Invalid DAP Content-Length")
                end = header_end + 4 + lengths[0]
                if len(self._buffer) >= end:
                    message = json.loads(self._buffer[header_end + 4:end])
                    self._buffer = self._buffer[end:]
                    return message
            elif len(self._buffer) > 8192:
                raise ValueError("DAP header exceeds 8192 bytes")
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise TimeoutError("DAP response deadline exceeded")
            if self.sock is None:
                raise ConnectionError("DAP connection is closed")
            self.sock.settimeout(remaining)
            data = self.sock.recv(65536)
            if not data:
                self._disconnect_transport()
                raise ConnectionError("DAP peer closed the connection")
            self._buffer += data

    def _wait(self, predicate: Callable[[Message], bool], timeout: float) -> Message | None:
        deadline = time.monotonic() + timeout
        while True:
            try:
                message = self._receive(deadline)
            except TimeoutError:
                return None
            if predicate(message):
                return message
            if message.get("type") == "response":
                self._responses[message["request_seq"]] = message
            elif message.get("type") == "event":
                self._events.append(message)
            elif message.get("type") == "request":
                raise ValueError(f"Unsupported reverse DAP request: {message}")

    def wait_for_response(self, request_id: int, timeout: float | None = None) -> Message | None:
        if request_id in self._responses:
            return self._responses.pop(request_id)
        return self._wait(lambda message: message.get("type") == "response"
                          and message.get("request_seq") == request_id,
                          self.default_timeout if timeout is None else timeout)

    def wait_for_event(self, event: str, timeout: float | None = None) -> Message | None:
        for index, message in enumerate(self._events):
            if message.get("event") == event:
                return self._events.pop(index)
        return self._wait(lambda message: message.get("type") == "event"
                          and message.get("event") == event,
                          self.default_timeout if timeout is None else timeout)

    def request(self, command: str, arguments: Message | None = None) -> Message:
        response = self.wait_for_response(self.send_request(command, arguments))
        assert response is not None and response.get("success"), f"DAP {command}: {response}"
        return response.get("body", {})

