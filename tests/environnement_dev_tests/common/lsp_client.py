import json
import queue
import threading


class LSPClient:
    def __init__(self, process):
        self.process = process
        self.messages = queue.Queue()
        self.pending = []
        self.sequence = 0
        self.reader = threading.Thread(target=self._read, daemon=True)
        self.reader.start()

    def _read(self):
        try:
            while True:
                headers = {}
                while True:
                    line = self.process.stdout.readline()
                    if not line:
                        raise ConnectionError("Language server closed stdout")
                    if line == b"\r\n":
                        break
                    key, value = line.decode("ascii").split(":", 1)
                    headers[key.lower()] = value.strip()
                length = int(headers["content-length"])
                if not 0 < length <= 8 * 1024 * 1024:
                    raise ValueError("Invalid LSP Content-Length")
                body = self.process.stdout.read(length)
                if len(body) != length:
                    raise ConnectionError("Truncated LSP message")
                self.messages.put(json.loads(body))
        except Exception as error:
            self.messages.put(error)

    def send(self, method, params, request=False):
        message = {"jsonrpc": "2.0", "method": method, "params": params}
        if request:
            self.sequence += 1
            message["id"] = self.sequence
        body = json.dumps(message).encode("utf-8")
        self.process.stdin.write(f"Content-Length: {len(body)}\r\n\r\n".encode("ascii") + body)
        self.process.stdin.flush()
        return message.get("id")

    def wait(self, predicate, timeout=20):
        import time
        deadline = time.monotonic() + timeout
        for index, message in enumerate(self.pending):
            if predicate(message):
                return self.pending.pop(index)
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise TimeoutError("LSP response deadline exceeded")
            message = self.messages.get(timeout=remaining)
            if isinstance(message, Exception):
                raise message
            if predicate(message):
                return message
            self.pending.append(message)

    def request(self, method, params):
        request_id = self.send(method, params, request=True)
        reply = self.wait(lambda message: message.get("id") == request_id)
        assert "error" not in reply, reply
        return reply.get("result")
