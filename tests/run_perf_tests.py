import os
import sys
import json
import subprocess
from perf_framework.builder import ProjectBuilder
from perf_framework.profiler import Profiler

def get_current_commit():
    try:
        return subprocess.check_output(["git", "rev-parse", "--short", "HEAD"], text=True).strip()
    except Exception:
        return "unknown"

def compile_and_get_tests(root_dir):
    builder = ProjectBuilder(root_dir)
    try:
        exe = builder.build_all()
    except Exception as e:
        print(f"Compilation failed: {e}")
        sys.exit(1)
        
    test_cases = builder.discover_tests()
    print(f"[{len(test_cases)} benchmarks found]")
    return exe, test_cases

def main():
    root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    exe, test_cases = compile_and_get_tests(root_dir)
    
    profiler = Profiler(root_dir, exe)
    results = {
        "commit_hash": get_current_commit(),
        "benchmarks": {}
    }
    
    print("\nExecuting benchmarks...")
    for test_name in test_cases:
        metrics = profiler.run_benchmark(test_name)
        if not metrics:
            continue
        peak_rss = profiler.measure_peak_rss(test_name)
        metrics["peak_rss"] = peak_rss
        results["benchmarks"][test_name] = metrics
        
    output_path = os.path.join(root_dir, "perf_run_data.json")
    with open(output_path, "w") as f:
        json.dump(results, f, indent=4)
        
    print(f"\nMetrics captured and written to: {output_path}")

if __name__ == "__main__":
    main()