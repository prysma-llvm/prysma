import os
import json
import shutil
import glob
import subprocess
import xml.etree.ElementTree as ET

class Profiler:
    
    def __init__(self, root_dir, exe):
        self.root_dir = root_dir
        self.exe = exe
        self.perf_bin = self._find_perf()
        self.perf_events = "cycles,instructions,L1-dcache-load-misses,LLC-misses,branches,branch-misses"

    def _find_perf(self):
        candidates = sorted(glob.glob("/usr/lib/linux-tools/*/perf"), reverse=True)
        for c in candidates:
            if os.access(c, os.X_OK): return c
        return shutil.which("perf")

    def run_benchmark(self, test_name):
        print(f"\n---> Profiling: {test_name}")
        
        if not self.perf_bin:
            raise RuntimeError("Error: 'perf' binary not found. Real hardware measurements are required.")

        safe_name = "".join(c if c.isalnum() else "_" for c in test_name).strip("_")
        json_output = os.path.join(self.root_dir, f"perf_{safe_name}.json")
        cmd = [self.perf_bin, "stat", "-D", "-1", "-j", "-o", json_output, "-e", self.perf_events, self.exe, test_name]

        # Try running perf normally, otherwise with sudo (forcing standard C locale for valid JSON floats)
        run_env = dict(os.environ, LC_ALL="C")
        ret = subprocess.run(cmd, env=run_env, cwd=self.root_dir, capture_output=True, text=True)
        if ret.returncode != 0 or not os.path.exists(json_output):
            print("Failed without privileges, retrying with sudo...")
            ret = subprocess.run(["sudo", "env", "LC_ALL=C"] + cmd, env=run_env, cwd=self.root_dir, capture_output=True, text=True)

        metrics = {}
        if os.path.exists(json_output):
            try:
                with open(json_output, "r") as f:
                    for line in f:
                        data = json.loads(line)
                        if "event" in data and "counter-value" in data:
                            event = data["event"]
                            name = event.split('/')[-2] if '/' in event else event
                            val_str = data["counter-value"].replace(",", ".").strip()
                            # Check if the counter-value is numeric (ignore "<not counted>")
                            if val_str.replace(".", "", 1).isdigit():
                                metrics[name] = metrics.get(name, 0) + int(float(val_str))
                os.remove(json_output)
                
                # Normalize values to a single iteration (since we run a loop of 1000 iterations in C++)
                for key in metrics:
                    metrics[key] = int(round(metrics[key] / 1000.0))
            except Exception as e:
                raise RuntimeError(f"Error parsing perf output: {e}")

        if not metrics or not metrics.get("cycles"):
            raise RuntimeError(f"Error: perf stat failed to retrieve hardware performance counters for {test_name}. Check permissions or execution environment.")
            
        return metrics

    def measure_peak_rss(self, test_name):
        # Use /usr/bin/time -f %M to get maximum memory usage
        try:
            res = subprocess.run(["/usr/bin/time", "-f", "%M", self.exe, test_name], cwd=self.root_dir, capture_output=True, text=True)
            for line in reversed(res.stderr.strip().splitlines()):
                parts = line.strip().split()
                if parts and parts[-1].isdigit():
                    return int(parts[-1])
        except Exception as e:
            raise RuntimeError(f"Error executing /usr/bin/time to get peak RSS: {e}")

        raise RuntimeError(f"Error: /usr/bin/time failed to measure peak RSS for {test_name}.")
