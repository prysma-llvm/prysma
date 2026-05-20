import streamlit as st
import pandas as pd
import numpy as np
import sqlite3
import os
import plotly.express as px
import plotly.graph_objects as go
import json

def load_css(app_dir):
    # Load custom dark style from CSS file
    css_path = os.path.join(app_dir, "style.css")
    if os.path.exists(css_path):
        with open(css_path, "r") as f:
            st.markdown(f"<style>{f.read()}</style>", unsafe_allow_html=True)

def load_config(app_dir):
    # Load regression thresholds
    config_path = os.path.join(app_dir, "dashboard_config.json")
    try:
        with open(config_path, "r") as f:
            return json.load(f)
    except Exception:
        return {
            "thresholds": {
                "instructions": 0.5, "cycles": 5.0, "CPI": 4.0,
                "l1_dcache_misses": 10.0, "l2_cache_misses": 10.0,
                "l3_cache_misses": 10.0, "ram_accesses": 10.0,
                "branch_misses": 5.0, "peak_rss": 5.0
            }
        }

def check_db_has_data(db_path):
    if not os.path.exists(db_path):
        return False
    conn = sqlite3.connect(db_path)
    try:
        cursor = conn.cursor()
        cursor.execute("SELECT COUNT(*) FROM perf_history")
        row = cursor.fetchone()
        return row and row[0] > 0
    except Exception:
        return False
    finally:
        conn.close()

def get_db_date_range(db_path):
    conn = sqlite3.connect(db_path)
    try:
        cursor = conn.cursor()
        cursor.execute("SELECT MIN(timestamp), MAX(timestamp) FROM perf_history")
        row = cursor.fetchone()
        if row and row[0] and row[1]:
            min_ts = pd.to_datetime(row[0]).tz_localize(None)
            max_ts = pd.to_datetime(row[1]).tz_localize(None)
            return min_ts, max_ts
    except Exception:
        pass
    finally:
        conn.close()
    now = pd.Timestamp.now()
    return now - pd.Timedelta(days=30), now

def load_data(db_path, start_dt=None, end_dt=None, limit=500):
    # Retrieve filtered history from sqlite
    conn = sqlite3.connect(db_path)
    try:
        query = "SELECT * FROM perf_history"
        params = []
        conditions = []
        if start_dt and end_dt:
            conditions.append("timestamp BETWEEN ? AND ?")
            params.append(start_dt.strftime("%Y-%m-%dT%H:%M:%SZ"))
            params.append(end_dt.strftime("%Y-%m-%dT%H:%M:%SZ"))
            
        if conditions:
            query += " WHERE " + " AND ".join(conditions)
            
        query += " ORDER BY timestamp DESC"
        
        if limit:
            query += f" LIMIT {int(limit)}"
            
        df = pd.read_sql_query(query, conn, params=params)
        if not df.empty:
            df['datetime'] = pd.to_datetime(df['timestamp'], errors='coerce').dt.tz_localize(None)
            df = df[df['datetime'].notna()].copy()
        return df
    except Exception:
        return pd.DataFrame()
    finally:
        conn.close()

def render_sidebar(threshs):
    # Display configured thresholds on the sidebar
    with st.sidebar:
        st.markdown("### Regression Thresholds")
        st.info("Configured via dashboard_config.json")
        for key, val in threshs.items():
            st.write(f"- **{key.replace('_', ' ').capitalize()}** : `{val}%`")

def get_threshold_map(threshs):
    # Associate each metric with its red and yellow threshold
    return {
        "cycles": (threshs.get("cycles", 5.0), threshs.get("cycles", 5.0) * 0.6),
        "instructions": (threshs.get("instructions", 0.5), threshs.get("instructions", 0.5) * 0.4),
        "CPI": (threshs.get("CPI", 4.0), threshs.get("CPI", 4.0) * 0.5),
        "l1_dcache_misses": (threshs.get("l1_dcache_misses", 10.0), threshs.get("l1_dcache_misses", 10.0) * 0.5),
        "l2_cache_misses": (threshs.get("l2_cache_misses", 10.0), threshs.get("l2_cache_misses", 10.0) * 0.5),
        "l3_cache_misses": (threshs.get("l3_cache_misses", 10.0), threshs.get("l3_cache_misses", 10.0) * 0.5),
        "ram_accesses": (threshs.get("ram_accesses", 10.0), threshs.get("ram_accesses", 10.0) * 0.5),
        "branch_misses": (threshs.get("branch_misses", 5.0), threshs.get("branch_misses", 5.0) * 0.4),
        "peak_rss": (threshs.get("peak_rss", 5.0), threshs.get("peak_rss", 5.0) * 0.5)
    }

def render_date_inputs(min_dt, max_dt):
    st.markdown("### Temporal Filters")
    c1, c2, c3, c4 = st.columns([1.5, 1, 1.5, 1])
    start_date = c1.date_input("Start", value=min_dt.date())
    start_time = c2.time_input("Start time", value=pd.Timestamp("00:00:00").time())
    end_date = c3.date_input("End", value=max_dt.date())
    end_time = c4.time_input("End time", value=pd.Timestamp("23:59:59").time())

    start_dt = pd.to_datetime(f"{start_date} {start_time}")
    end_dt = pd.to_datetime(f"{end_date} {end_time}")
    return start_dt, end_dt

def compute_benchmarks(df, search_query, selected_metric, selected_alert, threshold_map):
    # Calculate variations and prepare the list of benchmarks to show
    benchmarks_to_show = []
    
    # Efficiency (CPI) calculated on the fly
    df['CPI'] = np.where(df['instructions'] > 0, df['cycles'] / df['instructions'], 0)
    
    for benchmark in df['benchmark_name'].unique():
        if search_query and search_query.lower() not in benchmark.lower():
            continue
            
        bench_df = df[df['benchmark_name'] == benchmark].copy()
        chart_df = bench_df.sort_values(by="timestamp", ascending=True)
        
        diff_val = 0.0
        status = "Green"
        
        if len(chart_df) >= 2:
            latest_val = chart_df.iloc[-1][selected_metric]
            median_val = chart_df.iloc[:-1].tail(5)[selected_metric].median()
            diff_val = ((latest_val - median_val) / median_val) * 100 if median_val > 0 else 0
            
            red_t, yellow_t = threshold_map.get(selected_metric, (5.0, 2.5))
            if diff_val >= red_t:
                status = "Red"
            elif diff_val >= yellow_t:
                status = "Yellow"
                
        # Alert filter
        if selected_alert == "All":
            pass
        elif selected_alert == "Stable (Green)" and status != "Green":
            continue
        elif selected_alert == "Warning (Yellow)" and status != "Yellow":
            continue
        elif selected_alert == "Regression (Red)" and status != "Red":
            continue
            
        benchmarks_to_show.append({
            "name": benchmark,
            "status": status,
            "diff_val": diff_val,
            "chart_df": chart_df,
            "bench_df": bench_df
        })
        
    return benchmarks_to_show

def draw_chart(chart_df, metric_col, metric_label, search_commit, benchmark):
    # Plot metric evolution with Plotly
    fig = px.line(
        chart_df, x="commit_hash", y=metric_col, markers=True,
        labels={"commit_hash": "Commit", metric_col: metric_label},
        title=f"Evolution: {metric_label}"
    )
    fig.update_layout(
        height=280, margin=dict(l=20, r=20, t=40, b=20),
        paper_bgcolor="rgba(0,0,0,0)", plot_bgcolor="rgba(0,0,0,0)",
        font_color="#c9d1d9",
        xaxis=dict(showgrid=True, gridcolor="rgba(255,255,255,0.05)"),
        yaxis=dict(showgrid=True, gridcolor="rgba(255,255,255,0.05)")
    )
    fig.update_traces(line_color="#58a6ff", marker=dict(size=8, color="#ff7b72"))
    
    if search_commit.strip():
        high_df = chart_df[chart_df["commit_hash"].str.contains(search_commit.strip(), case=False, na=False)]
        if not high_df.empty:
            fig.add_trace(go.Scatter(
                x=high_df["commit_hash"], y=high_df[metric_col], mode="markers",
                marker=dict(color="#FFD700", size=15, symbol="star", line=dict(color="#8a2be2", width=2)),
                name=f"Search: {search_commit.strip()}", hovertext=high_df["commit_hash"]
            ))
            
    st.plotly_chart(fig, use_container_width=True, key=f"chart_{benchmark}")

def render_table(bench_df, search_commit):
    # Display runs table
    records = bench_df[[
        "commit_hash", "timestamp", "cycles", "instructions", "CPI", 
        "l1_dcache_misses", "l2_cache_misses", "l3_cache_misses", "ram_accesses", 
        "branch_misses", "peak_rss"
    ]].sort_values(by="timestamp", ascending=False)
    
    def highlight_row(row):
        match = search_commit.strip() and search_commit.strip().lower() in str(row.get("commit_hash", "")).lower()
        return ["background-color: rgba(138, 43, 226, 0.45); color: #ffffff; font-weight: bold;" if match else "" for _ in row]
    
    st.dataframe(
        records.style.apply(highlight_row, axis=1),
        column_config={
            "commit_hash": st.column_config.TextColumn("Commit"),
            "timestamp": st.column_config.TextColumn("Timestamp"),
            "cycles": st.column_config.NumberColumn("CPU Cycles", format="%d"),
            "instructions": st.column_config.NumberColumn("Instructions", format="%d"),
            "CPI": st.column_config.NumberColumn("CPI", format="%.4f"),
            "l1_dcache_misses": st.column_config.NumberColumn("L1 Misses", format="%d"),
            "l2_cache_misses": st.column_config.NumberColumn("L2 Misses", format="%d"),
            "l3_cache_misses": st.column_config.NumberColumn("L3 Misses", format="%d"),
            "ram_accesses": st.column_config.NumberColumn("RAM Accesses", format="%d"),
            "branch_misses": st.column_config.NumberColumn("Branch Misses", format="%d"),
            "peak_rss": st.column_config.NumberColumn("Peak RSS (KB)", format="%d")
        },
        height=280, use_container_width=True
    )

def render_benchmark(item, metric_col, metric_label, threshold_map, search_commit):
    # Manage display of an individual benchmark
    name = item["name"]
    status = item["status"]
    diff_val = item["diff_val"]
    chart_df = item["chart_df"]
    bench_df = item["bench_df"]
    
    st.markdown(f"### {name}")

    if len(chart_df) >= 2:
        red_thresh, yellow_thresh = threshold_map.get(metric_col, (5.0, 2.5))
        if status == "Red":
            st.error(f"REGRESSION: {metric_label} (+{diff_val:.2f}% vs median baseline, threshold: {red_thresh}%)")
        elif status == "Yellow":
            st.warning(f"WARNING: {metric_label} (+{diff_val:.2f}% vs median baseline, yellow threshold: {yellow_thresh:.2f}%)")
        else:
            st.success(f"STABLE: {metric_label} ({diff_val:+.2f}% vs median baseline)")
    else:
        st.info("First measurement (no history).")

    col_chart, col_table = st.columns([1, 1])
    with col_chart:
        draw_chart(chart_df, metric_col, metric_label, search_commit, name)
    with col_table:
        render_table(bench_df, search_commit)
    st.markdown("---")

def main():
    app_dir = os.path.dirname(os.path.abspath(__file__))
    root_dir = os.path.dirname(app_dir)
    
    load_css(app_dir)
    st.title("Prysma Performance Dashboard")
    
    # Load config and DB
    config = load_config(app_dir)
    db_path = os.path.join(root_dir, "prysma_perf.db")
    
    if not check_db_has_data(db_path):
        st.warning("No metrics registered.")
        st.stop()
        
    threshs = config.get("thresholds", {})
    render_sidebar(threshs)
    threshold_map = get_threshold_map(threshs)
    
    # Filters
    min_dt, max_dt = get_db_date_range(db_path)
    start_dt, end_dt = render_date_inputs(min_dt, max_dt)
    
    df = load_data(db_path, start_dt, end_dt)
    if df.empty:
        st.warning("No data in this period.")
        st.stop()
        
    f1, f2, f3, f4 = st.columns([2, 1.5, 1.5, 1.5])
    search_query = f1.text_input("Search a test", placeholder="Test name...")
    search_commit = f2.text_input("Filter by commit", placeholder="Commit hash...")

    metrics_list = {
        "CPU Cycles": "cycles", "Instructions": "instructions", "CPI (Efficiency)": "CPI",
        "L1 Cache Misses": "l1_dcache_misses", "L2 Cache Misses": "l2_cache_misses",
        "L3 Cache Misses": "l3_cache_misses", "RAM Accesses": "ram_accesses",
        "Branch Misses": "branch_misses", "Peak RSS (Memory)": "peak_rss"
    }
    selected_metric_label = f3.selectbox("Metric to plot", list(metrics_list.keys()))
    metric_col = metrics_list[selected_metric_label]
    alert_filter = f4.selectbox("Filter by status", ["All", "Stable (Green)", "Warning (Yellow)", "Regression (Red)"])

    # Calculate
    benchmarks_to_show = compute_benchmarks(df, search_query, metric_col, alert_filter, threshold_map)
    
    if not benchmarks_to_show:
        st.info("No results for these filters.")
        return
        
    if 'limit' not in st.session_state:
        st.session_state.limit = 10

    # Display
    for item in benchmarks_to_show[:st.session_state.limit]:
        render_benchmark(item, metric_col, selected_metric_label, threshold_map, search_commit)

    # Pagination buttons
    if len(benchmarks_to_show) > 10:
        c_left, c_mid1, c_mid2, c_right = st.columns([3, 1, 1, 3])
        if len(benchmarks_to_show) > st.session_state.limit:
            if c_mid1.button("See more", use_container_width=True):
                st.session_state.limit += 10
                st.rerun()
        if st.session_state.limit > 10:
            if c_mid2.button("Reset", use_container_width=True):
                st.session_state.limit = 10
                st.rerun()

if __name__ == "__main__":
    main()
