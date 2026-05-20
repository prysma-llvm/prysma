import os
import sys
import json
from perf_framework.database import PerfDatabase
from perf_framework.notifier import DiscordBotNotifier

## We separate this file to offer the minimum privilege to the user.
## Indeed, the user who does a push or a pull request does not need
## to manipulate the database.
## This script will have a private version updated by a server administrator,
## and thus, that is why we have a strict separation of responsibilities.
## We have the generation of metrics in the form of a json file, and then as you can see
## this file allows saving the data to the database.

def main():
    # 
    root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    
    # The database path can be configured by an environment variable
    db_path = os.environ.get("PRYSMA_DB_PATH", os.path.join(root_dir, "prysma_perf.db"))
    json_path = os.path.join(root_dir, "perf_run_data.json")
    
    if not os.path.exists(json_path):
        print(f"Error: the results file {json_path} is missing.")
        sys.exit(1)
        
    try:
        with open(json_path, "r") as f:
            data = json.load(f)
    except Exception as e:
        print(f"Error reading JSON file: {e}")
        sys.exit(1)
        
    commit_hash = data.get("commit_hash", "unknown")
    benchmarks = data.get("benchmarks", {})
    
    db = PerfDatabase(db_path)
    notifier = DiscordBotNotifier(root_dir)
    
    print(f"Importing results for commit: {commit_hash}")
    
    for test_name, metrics in benchmarks.items():
        if not isinstance(metrics, dict):
            continue
            
        peak_rss = metrics.get("peak_rss", 0)
        
        baseline, last_run = db.get_baseline(test_name)
        
        notifier.check_and_notify(test_name, metrics, baseline, last_run)
        
        db.insert_metrics(test_name, metrics, peak_rss, commit_hash=commit_hash)
        
    print(f"Database updated: {db_path}")

if __name__ == "__main__":
    main()
