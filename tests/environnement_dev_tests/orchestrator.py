import argparse
import json
import os
import platform
import shutil
import subprocess
import sys
import time
import uuid
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parents[2]
DEV_ENV_DIR = Path(__file__).resolve().parent


def main(argv=None):
    parser = argparse.ArgumentParser(description="Development environment integration tests")
    parser.add_argument("--family", action="append", choices=("common", "linux", "window", "vscode"))
    parser.add_argument("--native-only", action="store_true", help="Explicitly exclude Docker and VS Code scenarios")
    parser.add_argument("--results-dir", type=Path, default=ROOT_DIR / "build" / "dev-env-results")
    parser.add_argument("--family-timeout", type=float, default=1800)
    args = parser.parse_args(argv)
    if args.family_timeout <= 0:
        parser.error("--family-timeout must be positive")
    families = args.family or ["common", "linux" if sys.platform == "linux" else "window", "vscode"]
    families = list(dict.fromkeys(families))
    run_id = uuid.uuid4().hex
    output = args.results_dir.resolve() / run_id
    output.mkdir(parents=True)
    env = dict(os.environ, PRYSMA_TEST_RUN_ID=run_id, PRYSMA_TEST_RESULTS_DIR=str(output), PYTHONUNBUFFERED="1")
    docker_path = shutil.which("docker")
    docker = os.path.abspath(docker_path) if docker_path else "docker"
    report = {"system": platform.platform(), "python": sys.version,
              "selection": families, "native_only": args.native_only, "families": []}
    for command in ([sys.executable, "-m", "pytest", "--version"], [docker, "version"],
                    [docker, "image", "inspect", env.get("PRYSMA_TEST_IMAGE", "prysma-debug")]):
        try:
            result = subprocess.run(command, capture_output=True, text=True, timeout=20)
            report[" ".join(command)] = {"exit_code": result.returncode, "stdout": result.stdout, "stderr": result.stderr}
        except (OSError, subprocess.TimeoutExpired) as error:
            report[" ".join(command)] = str(error)
    failed = False
    try:
        for family in families:
            if family == "linux" and sys.platform != "linux" or family == "window" and sys.platform != "win32":
                report["families"].append({"name": family, "status": "not applicable"})
                continue
            if args.native_only and family == "vscode":
                report["families"].append({"name": family, "status": "not selected"})
                continue
            command = [sys.executable, "-m", "pytest", str(DEV_ENV_DIR / family), "-v", "-ra",
                       "--junitxml", str(output / f"{family}.xml"), "-o", "junit_logging=all"]
            if args.native_only:
                command += ["-m", "not docker"]
            print(f"Running {family}; log: {output / (family + '.log')}", flush=True)
            started = time.monotonic()
            with (output / f"{family}.log").open("w", encoding="utf-8") as log:
                process = subprocess.Popen(command, cwd=ROOT_DIR, env=env, stdout=log,
                                           stderr=subprocess.STDOUT, start_new_session=os.name != "nt")
                try:
                    code = process.wait(timeout=args.family_timeout)
                except subprocess.TimeoutExpired:
                    if os.name == "nt":
                        subprocess.run(["taskkill", "/PID", str(process.pid), "/T", "/F"], timeout=20)
                    else:
                        import signal
                        os.killpg(process.pid, signal.SIGTERM)
                        try:
                            process.wait(timeout=10)
                        except subprocess.TimeoutExpired:
                            os.killpg(process.pid, signal.SIGKILL)
                    process.wait()
                    code = 124
            print((output / f"{family}.log").read_text(encoding="utf-8"), flush=True)
            success = code == 0 or code == 5 and args.native_only
            failed |= not success
            report["families"].append({"name": family, "exit_code": code,
                                       "duration": time.monotonic() - started,
                                       "status": "passed" if code == 0 else "not selected" if success else "failed"})
    finally:
        if docker_path and not args.native_only:
            for resource in ("container", "volume"):
                try:
                    command = [docker, "ps", "-aq"] if resource == "container" else [docker, "volume", "ls", "-q"]
                    result = subprocess.run([*command, "--filter", f"label=prysma.test={run_id}"],
                                            capture_output=True, text=True, timeout=15)
                    for name in result.stdout.split() if result.returncode == 0 else []:
                        removed = subprocess.run([docker, resource, "rm", "-f", name], timeout=15)
                        failed |= removed.returncode != 0
                except (OSError, subprocess.TimeoutExpired) as error:
                    print(f"Cleanup unavailable: {error}", file=sys.stderr)
                    failed = True
        (output / "summary.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Results: {output}")
    return int(failed)


if __name__ == "__main__":
    sys.exit(main())
