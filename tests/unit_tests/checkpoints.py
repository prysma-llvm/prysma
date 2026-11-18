"""Parse assertions on Prysma print statements and validate the runtime trace."""

import json
import math
import re
from dataclasses import dataclass
from itertools import zip_longest
from pathlib import Path

MARKER = "@@CP|"
SOURCE = re.compile(
    r'^(?P<prefix>[ \t]*dec string\[\] trace_(?P<label>\w+) = ")'
    r'@@CP\|(?:(?P<number>[0-9]+)\|)?(?P<payload>[^"\n]*)(?P<suffix>";)',
    re.MULTILINE,
)
INCLUDE = re.compile(r'(?m)^[ \t]*include[ \t]+"([^"\n]+)"[ \t]*;')
EXPECT = re.compile(r'\bcall print\((?P<expression>[^;\n]+)\);[ \t]*// @expect[ \t]+(?P<data>.+)$')
COMPILE_ERROR = re.compile(r'// @compile-error[ \t]+(?P<data>.+)$')
ANSI_COLOR = re.compile(r"\x1b\[[0-9;]*m")


@dataclass(frozen=True)
class Checkpoint:
    file: str
    number: int | None
    line: int | None
    label: str

    def __str__(self):
        location = f"{self.file}:{self.line}" if self.line is not None else self.file
        return f"{location} [#{self.number} {self.label}]" if self.number is not None else location


class InvalidExpectation(ValueError):
    def __init__(self, point, message):
        self.point = point
        self.message = message
        super().__init__(f"{point}: {message}")


def print_statements(marker, prefix, expressions):
    statements = [
        f'dec string[] {marker} = "{prefix}";',
        "call backSlashN();",
        f"call print(ref {marker});",
    ]
    if len(expressions) > 1:
        statements.append(f'dec string[] {marker}_separator = "|";')
    for index, expression in enumerate(expressions):
        if index:
            statements.append(f"call print(ref {marker}_separator);")
        statements.append(f"call print({expression});")
    return " ".join(statements)


def render_assertion(number, expressions, events):
    if type(number) is not int or number < 1:
        raise ValueError(f"checkpoint ID must be a positive integer, got {number!r}")
    statements = print_statements(f"trace_cp_{number}", f"{MARKER}{number}|", expressions)
    return f"{statements} // @expect {json.dumps(events)}\n    call backSlashN();"


def checkpoint(number, *, trace_positions):
    """Assert where this checkpoint is reached. An empty list means never."""
    if isinstance(trace_positions, list):
        return render_assertion(number, (), [[position] for position in trace_positions])
    return render_assertion(number, (), [trace_positions])


def assert_eq(number, expected, expression, *, trace_positions):
    """Assert a scalar or a group of values, optionally across repeated visits."""
    multiple = not isinstance(expression, str)
    expressions = expression if multiple else (expression,)
    if not isinstance(expressions, (list, tuple)) or not expressions or not all(
        isinstance(item, str) and item.strip() for item in expressions
    ):
        raise ValueError(f"checkpoint {number}: give a Prysma expression or a list of expressions")
    if isinstance(trace_positions, list):
        if not isinstance(expected, list) or len(expected) != len(trace_positions):
            raise ValueError(f"checkpoint {number}: give one expected result per position in trace_positions")
        pairs = zip(trace_positions, expected)
    else:
        pairs = [(trace_positions, expected)]
    rows = []
    for position, values in pairs:
        values = values if multiple else [values]
        if not isinstance(values, (list, tuple)) or len(values) != len(expressions):
            raise ValueError(f"checkpoint {number}: give one expected value per expression")
        rows.append([position, *values])
    return render_assertion(number, expressions, rows[0] if len(rows) == 1 else rows)


def scan_source(source):
    number = 0
    pending = False
    for index, line in enumerate(source.splitlines(), start=1):
        marker = SOURCE.match(line)
        assertion = EXPECT.search(line) if not line.lstrip().startswith("//") else None
        if marker or (assertion and not pending):
            number += 1
        direct = bool(assertion and not (pending or marker))
        if marker:
            pending = True
        if assertion:
            pending = False
        yield index, line, number, marker, assertion, direct


def load_json(data, location):
    try:
        return json.loads(data)
    except ValueError as error:
        raise InvalidExpectation(location, f"invalid JSON: {error}") from error


def parse_events(data, point, direct):
    # A scalar is shorthand for a single value in source order.
    if not isinstance(data, list):
        data = [point.number, data]
    if not data:
        return []
    rows = data if isinstance(data[0], list) else [data]
    events = []
    previous = 0
    for row in rows:
        if not isinstance(row, list) or not row or type(row[0]) is not int or row[0] <= previous:
            raise InvalidExpectation(point, "expected strictly increasing positive event positions")
        position, *values = row
        if direct and len(values) != 1:
            raise InvalidExpectation(point, "an annotated print expects one value per execution")
        for value in values:
            if type(value) not in (bool, int, float, str) or (type(value) is float and not math.isfinite(value)):
                raise InvalidExpectation(point, "expected a boolean, finite number or string")
            if isinstance(value, str) and any(char in value for char in "|\r\n"):
                raise InvalidExpectation(point, "expected strings cannot contain pipe or newline characters")
        events.append((position, point, tuple(values)))
        previous = position
    return events


def read_checkpoints(source, filename):
    points = {}
    events = []
    implicit_prints = {}
    compile_error = None
    labels = set()
    pending = None
    for index, line, number, marker, assertion, direct in scan_source(source):
        location = Checkpoint(filename, None, index, "")
        if marker:
            if pending:
                raise InvalidExpectation(pending, "missing // @expect on the checkpoint print")
            label = marker["label"]
            if label in labels:
                raise InvalidExpectation(location, f"duplicate checkpoint label {label}")
            labels.add(label)
            if marker["number"] != str(number):
                raise InvalidExpectation(location, f"expected checkpoint ID {number}, found {marker['number'] or 'no ID'}")
            pending = Checkpoint(filename, number, index, label)
        if "// @expect" in line and not assertion:
            raise InvalidExpectation(location, "expected 'call print(...); // @expect <JSON>'")
        if assertion:
            if direct and line[:assertion.start()].strip():
                raise InvalidExpectation(location, "an annotated print must start its own line")
            label = "print" if direct else pending.label
            point = Checkpoint(filename, number, index, label)
            points[(filename, str(number))] = point
            events.extend(parse_events(load_json(assertion["data"], point), point, direct))
            if direct:
                implicit_prints[index] = (number, assertion["expression"])
            pending = None
        if "// @compile-error" in line:
            match = COMPILE_ERROR.search(line)
            if not match or compile_error:
                raise InvalidExpectation(location, "expected one // @compile-error with a JSON regex string")
            pattern = load_json(match["data"], location)
            if not isinstance(pattern, str) or not pattern:
                raise InvalidExpectation(location, "expected a nonempty diagnostic regex string")
            try:
                compile_error = (location, re.compile(pattern, re.I))
            except re.error as error:
                raise InvalidExpectation(location, f"invalid diagnostic regex: {error}") from error
    if pending:
        raise InvalidExpectation(pending, "missing // @expect on the checkpoint print")
    return points, events, implicit_prints, compile_error


def prepare_sources(source_root: Path, entry: Path, destination: Path):
    """Collect expectations from the source and instrument plain annotated prints."""
    source_root = source_root.resolve()
    sources = {}

    def visit(relative):
        if relative in sources:
            return
        source = (source_root / relative).read_text(encoding="utf-8")
        sources[relative] = source
        for included in INCLUDE.findall(source):
            include_path = (source_root / relative.parent / included).resolve()
            visit(include_path.relative_to(source_root))

    visit(entry)
    points = {}
    events = []
    compile_error = None
    for relative, source in sources.items():
        filename = relative.as_posix()
        source_points, source_events, implicit_prints, rejection = read_checkpoints(source, filename)
        points.update(source_points)
        events.extend(source_events)
        if rejection:
            if compile_error:
                raise InvalidExpectation(rejection[0], "duplicate compile-error assertion in this project")
            compile_error = rejection

        def prefix(number):
            return f"{MARKER}{number}|{filename}|" if len(sources) > 1 else f"{MARKER}{number}|"

        source = SOURCE.sub(
            lambda match: f"{match['prefix']}{prefix(match['number'])}{match['payload']}{match['suffix']}",
            source,
        )
        lines = source.splitlines(keepends=True)
        for index, (number, expression) in implicit_prints.items():
            line = lines[index - 1]
            indent = line[:len(line) - len(line.lstrip())]
            statements = print_statements(f"trace_print_{number}", prefix(number), (expression,))
            lines[index - 1] = f"{indent}{statements} call backSlashN();\n"
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text("".join(lines), encoding="utf-8")

    if not events and not compile_error:
        raise InvalidExpectation(Checkpoint(entry.as_posix(), None, None, ""), "no expected checkpoint events")
    if events and compile_error:
        raise InvalidExpectation(compile_error[0], "compile-error tests cannot expect runtime events")
    events.sort(key=lambda event: event[0])
    for ordinal, (position, point, _) in enumerate(events, start=1):
        if position != ordinal:
            raise InvalidExpectation(point, f"expected event position {ordinal}, found {position} (duplicate or gap)")
    return points, [(point, values) for _, point, values in events], compile_error, len(sources) > 1


def format_failure(point, expected, actual, error):
    return (
        "[FAIL] PRYSMA INTEGRATION TEST\n"
        f"File       : {point.file}\n"
        f"Checkpoint : {point.number or '-'}\n"
        f"Line       : {point.line or '-'}\n"
        f"Expected   : {expected}\n"
        f"Actual     : {actual}\n"
        f"Error      : {error}"
    )


def compare_trace(stdout, expected, points, entry, qualified):
    observed = []
    for line in ANSI_COLOR.sub("", stdout).splitlines():
        if not line.startswith(MARKER):
            continue
        data = line[len(MARKER):].split("|")
        if qualified and len(data) >= 3:
            number, file, *values = data
        elif not qualified and len(data) >= 2:
            number, *values = data
            file = entry
        else:
            observed.append((None, line, ()))
            continue
        observed.append((points.get((file, number)), f"{file} #{number}", tuple(values)))

    for want, got in zip_longest(expected, observed):
        if want is None:
            point, raw, _ = got
            return format_failure(point or Checkpoint(entry, None, None, ""), "end of trace",
                                  point or raw, "UNEXPECTED_CHECKPOINT")
        point, values = want
        if got is None:
            return format_failure(point, point, "end of trace", "MISSING_CHECKPOINT")
        actual_point, raw, actual_values = got
        if point != actual_point:
            return format_failure(point, point, actual_point or raw, "CHECKPOINT_MISMATCH")
        if not values and actual_values == ("",):
            actual_values = ()
        if len(values) != len(actual_values):
            return format_failure(point, json.dumps(values), json.dumps(actual_values), "VALUE_COUNT_MISMATCH")
        for value, actual in zip(values, actual_values):
            if not same_value(value, actual):
                return format_failure(point, json.dumps(value), actual, "VALUE_MISMATCH")
    return None


def same_value(expected, actual):
    if isinstance(expected, bool):
        return actual == str(expected).lower()
    if isinstance(expected, int):
        return actual == str(expected)
    if isinstance(expected, float):
        try:
            # The Prysma runtime prints six digits after the decimal point.
            return math.isclose(float(actual), expected, rel_tol=1e-7, abs_tol=5e-7)
        except ValueError:
            return False
    return actual == expected
