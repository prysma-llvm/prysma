import sqlite3
import subprocess
from datetime import datetime

## Strictly private to the server
## Allows saving performance metrics to a database
## Allows retrieving the baseline and the last run
## Allows retrieving the commit hash
## It is the server database.

class PerfDatabase:
    
    def __init__(self, db_path):
        self.db_path = db_path
        self._init_db()

    def _init_db(self):
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Migration if columns are missing
        cursor.execute("PRAGMA table_info(perf_history)")
        columns = [row[1] for row in cursor.fetchall()]
        if columns:
            expected_columns = {
                "commit_hash": "TEXT",
                "timestamp": "TEXT",
                "benchmark_name": "TEXT",
                "cycles": "INTEGER",
                "instructions": "INTEGER",
                "l1_dcache_misses": "INTEGER",
                "l2_cache_misses": "INTEGER",
                "l3_cache_misses": "INTEGER",
                "ram_accesses": "INTEGER",
                "branch_misses": "INTEGER",
                "peak_rss": "INTEGER"
            }
            for col, col_type in expected_columns.items():
                if col not in columns:
                    cursor.execute(f"ALTER TABLE perf_history ADD COLUMN {col} {col_type}")

        cursor.execute("""
            CREATE TABLE IF NOT EXISTS perf_history (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                commit_hash TEXT,
                timestamp TEXT,
                benchmark_name TEXT,
                cycles INTEGER,
                instructions INTEGER,
                l1_dcache_misses INTEGER,
                l2_cache_misses INTEGER,
                l3_cache_misses INTEGER,
                ram_accesses INTEGER,
                branch_misses INTEGER,
                peak_rss INTEGER
            )
        """)
        conn.commit()
        conn.close()

    def get_current_commit(self):
        try:
            return subprocess.check_output(["git", "rev-parse", "--short", "HEAD"], text=True).strip()
        except Exception:
            return "unknown"

    def get_baseline(self, test_name):
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        cursor.execute("""
            SELECT cycles, instructions, l1_dcache_misses, l2_cache_misses, 
                   l3_cache_misses, ram_accesses, branch_misses, peak_rss
            FROM perf_history 
            WHERE benchmark_name = ? 
            ORDER BY id DESC LIMIT 5
        """, (test_name,))
        rows = cursor.fetchall()
        conn.close()
        
        if not rows:
            return None, None
            
        # We calculate the medians on the last 5 runs
        import statistics
        keys = ["cycles", "instructions", "L1-dcache-load-misses", "L2-cache-misses", "L3-cache-misses", "RAM-accesses", "branch-misses", "peak_rss"]
        median_baseline = {keys[i]: statistics.median([r[i] for r in rows]) for i in range(len(keys))}
        last_run = {keys[i]: rows[0][i] for i in range(len(keys))}
        return median_baseline, last_run

    def insert_metrics(self, test_name, metrics, peak_rss, commit_hash=None):
        if commit_hash is None:
            commit_hash = self.get_current_commit()
        timestamp = datetime.utcnow().isoformat() + "Z"
        
        instructions = metrics.get("instructions", 0)
        l2_misses = metrics.get("L2-cache-misses", 0)
        l3_misses = metrics.get("L3-cache-misses", metrics.get("LLC-misses", 0))
        ram_accesses = metrics.get("RAM-accesses", 0) or l3_misses

        if not metrics.get("cycles") or not instructions:
            raise ValueError(f"Error: missing critical performance metrics (cycles or instructions) for {test_name}")

        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        cursor.execute("""
            INSERT INTO perf_history 
            (commit_hash, timestamp, benchmark_name, cycles, instructions, 
             l1_dcache_misses, l2_cache_misses, l3_cache_misses, ram_accesses, 
              branch_misses, peak_rss)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            commit_hash, timestamp, test_name,
            metrics.get("cycles", 0),
            instructions,
            metrics.get("L1-dcache-load-misses", 0),
            l2_misses,
            l3_misses,
            ram_accesses,
            metrics.get("branch-misses", 0) or metrics.get("branch_misses", 0),
            peak_rss
        ))
        conn.commit()
        conn.close()
