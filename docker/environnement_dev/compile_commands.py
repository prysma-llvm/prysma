import json
import shlex
import sys
from pathlib import Path


def adapt(root, host_root=None):
    root = root.resolve()
    host_root = host_root or root.as_posix()
    mappings = {"/workspace": host_root,
                "/usr/lib/llvm-18/include": host_root + "/.clangd-includes"}

    def path(value):
        for old, new in mappings.items():
            if value == old or value.startswith(old + "/"):
                return new + value[len(old):]
        return value

    entries = {}
    for database in (root / "build/compile_commands.json", root / "tests/unit_tests/build/compile_commands.json"):
        if not database.exists():
            continue
        data = json.loads(database.read_text(encoding="utf-8"))
        if not isinstance(data, list):
            raise ValueError(f"Expected a JSON array: {database}")
        for entry in data:
            for key in ("directory", "file", "output"):
                if key in entry:
                    entry[key] = path(entry[key])
            if "command" in entry:
                entry["arguments"] = shlex.split(entry.pop("command"))
            arguments = entry["arguments"]
            for index, argument in enumerate(arguments):
                if argument.startswith("-I"):
                    arguments[index] = "-I" + path(argument[2:])
                else:
                    arguments[index] = path(argument)
            entries[entry["file"]] = entry
        temporary = database.with_suffix(".json.tmp")
        temporary.write_text(json.dumps(data, indent=2), encoding="utf-8")
        temporary.replace(database)
    if entries:
        database = root / "compile_commands.json"
        temporary = database.with_suffix(".json.tmp")
        temporary.write_text(json.dumps(list(entries.values()), indent=2), encoding="utf-8")
        temporary.replace(database)


if __name__ == "__main__":
    try:
        adapt(Path(sys.argv[1]), *sys.argv[2:])
    except (OSError, ValueError, KeyError, TypeError) as error:
        sys.exit(f"ERROR: Cannot adapt compile_commands.json: {error}")
