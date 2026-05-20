import json
import os
import requests

## This is where Discord notifications are managed.
## A Discord bot is set up to notify developers in case of regressions.

class DiscordBotNotifier:
    
    def __init__(self, root_dir):
        config_path = os.environ.get("PRYSMA_CONFIG_PATH", os.path.join(root_dir, "dashboard", "dashboard_config.json"))
        config_path = os.path.expanduser(os.path.expandvars(config_path))
        try:
            with open(config_path, "r") as f:
                self.config = json.load(f)
        except Exception:
            self.config = {}

    def check_and_notify(self, test_name, current_metrics, baseline_metrics, last_run_metrics):
        # Enabled or not?
        if not self.config.get("notifications", {}).get("enabled", False) or not baseline_metrics:
            return

        regressions = []
        thresholds = self.config.get("thresholds", {})
        
        for metric, current_val in current_metrics.items():
            base_val = baseline_metrics.get(metric)
            if not base_val or base_val <= 0:
                continue
                
            diff_pct = ((current_val - base_val) / base_val) * 100
            limit = thresholds.get(metric.replace("-", "_").lower(), 5.0)
            
            if diff_pct > limit:
                # If the old run already exceeded the threshold, we avoid spamming
                if last_run_metrics and last_run_metrics.get(metric, 0) > 0:
                    last_diff = ((last_run_metrics[metric] - base_val) / base_val) * 100
                    if last_diff > limit:
                        continue
                        
                regressions.append(f"**{metric}** : {current_val} (+{diff_pct:.2f}% vs median / Threshold : {limit}%)")

        if regressions:
            # Send to Discord
            bot_cfg = self.config.get("notifications", {}).get("discord_bot", {})
            bot_token = bot_cfg.get("bot_token")
            channel_id = bot_cfg.get("channel_id")
            
            if not bot_token or not channel_id or "TON_" in bot_token or "YOUR_" in bot_token:
                print("Error: Discord token or channel ID not configured.")
                return

            try:
                url = f"https://discord.com/api/v10/channels/{channel_id}/messages"
                payload = {
                    "content": f"**Regression detected (Prysma)**\n"
                               f"**Benchmark:** {test_name}\n\n"
                               f"**Threshold breaches:**\n" + 
                               "\n".join(regressions) + 
                               f"\n\nCheck the dashboard for more details."
                }
                response = requests.post(url, headers={
                    "Authorization": f"Bot {bot_token}",
                    "Content-Type": "application/json"
                }, json=payload, timeout=5)
                if response.status_code in [200, 204]:
                    print(f"--> Discord notification sent for {test_name}")
                else:
                    print(f"--> Discord error (HTTP {response.status_code})")
            except Exception as e:
                print(f"--> Error sending to Discord: {e}")
