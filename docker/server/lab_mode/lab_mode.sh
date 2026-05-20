#!/bin/bash
## Note: This script runs on a Debian 12 OptiPlex server.
## My personal server setup is entirely based on Docker containers.
## If you run other system services, you may need to adapt the script.
## It is generic for users who only use Docker as their personal server.
## Note: It will temporarily interrupt all your services. This is not an issue
## if those services are personal and not used by anyone else.
## Everything is disabled by default to avoid noise from other processes.


set -e

SELF_ID=$(hostname)

host_exec() {
  if [ -f /proc/1/ns/mnt ]; then
    nsenter -t 1 -m -u -i -n -p "$@"
  else
    "$@"
  fi
}

case "$1" in
  enable)
    echo "Activating laboratory mode..."
    
    # 1. Save current host configurations
    host_exec sh -c 'cat /proc/sys/kernel/randomize_va_space > /tmp/lab_aslr' 2>/dev/null || true
    host_exec sh -c 'cat /proc/sys/kernel/perf_event_paranoid > /tmp/lab_perf_paranoid' 2>/dev/null || true
    host_exec sh -c 'cat /proc/sys/kernel/kptr_restrict > /tmp/lab_kptr' 2>/dev/null || true
    host_exec sh -c 'cat /sys/devices/system/cpu/intel_pstate/no_turbo > /tmp/lab_turbo' 2>/dev/null || true
    for cpu in 1 2 3; do
      host_exec sh -c "cat /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_governor > /tmp/lab_gov_cpu$cpu" 2>/dev/null || true
      host_exec sh -c "cat /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_min_freq > /tmp/lab_min_cpu$cpu" 2>/dev/null || true
      host_exec sh -c "cat /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_max_freq > /tmp/lab_max_cpu$cpu" 2>/dev/null || true
      host_exec sh -c "cat /sys/devices/system/cpu/cpu$cpu/cpufreq/energy_performance_preference > /tmp/lab_epp_cpu$cpu" 2>/dev/null || true
      for state in {1..9}; do
        if host_exec [ -f /sys/devices/system/cpu/cpu$cpu/cpuidle/state$state/disable ]; then
          host_exec sh -c "cat /sys/devices/system/cpu/cpu$cpu/cpuidle/state$state/disable > /tmp/lab_cstate_cpu${cpu}_state${state}" 2>/dev/null || true
        fi
      done
    done

    # 2. Stop host systemd monitor and running containers
    host_exec systemctl stop server-monitor 2>/dev/null || echo "Warning: server-monitor not stopped"
    
    host_exec rm -f /tmp/lab_stopped_containers
    running_containers=$(docker ps -q)
    for container in $running_containers; do
      if [[ "$container" != *"$SELF_ID"* ]]; then
        name=$(docker inspect --format '{{.Name}}' "$container" | sed 's/\///')
        echo "Stopping container: $name"
        docker stop "$container"
        host_exec sh -c "echo '$name' >> /tmp/lab_stopped_containers"
      fi
    done
    
    # 3. Apply CPU tuning to cores 1-3
    echo "Configuring CPU stability (cores 1-3)..."
    host_exec sh -c 'echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo' 2>/dev/null || true
    
    for cpu in 1 2 3; do
      host_exec sh -c "echo performance > /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_governor" 2>/dev/null || true
      MAX_FREQ=$(host_exec cat /sys/devices/system/cpu/cpu$cpu/cpufreq/cpuinfo_max_freq 2>/dev/null)
      if [ ! -z "$MAX_FREQ" ]; then
        host_exec sh -c "echo $MAX_FREQ > /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_min_freq" 2>/dev/null || true
        host_exec sh -c "echo $MAX_FREQ > /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_max_freq" 2>/dev/null || true
      fi
      host_exec sh -c "echo performance > /sys/devices/system/cpu/cpu$cpu/cpufreq/energy_performance_preference" 2>/dev/null || true
      for state in {1..9}; do
        if host_exec [ -f /sys/devices/system/cpu/cpu$cpu/cpuidle/state$state/disable ]; then
          host_exec sh -c "echo 1 > /sys/devices/system/cpu/cpu$cpu/cpuidle/state$state/disable" 2>/dev/null || true
        fi
      done
    done

    # 4. Temporarily disable ASLR and unlock perf PMU
    echo "Temporarily disabling ASLR and unlocking perf PMU..."
    host_exec sh -c 'echo 0 > /proc/sys/kernel/randomize_va_space'
    host_exec sh -c 'echo -1 > /proc/sys/kernel/perf_event_paranoid'
    host_exec sh -c 'echo 0 > /proc/sys/kernel/kptr_restrict'

    # 5. Clear system caches
    echo "Clearing system caches..."
    host_exec sh -c 'sync && echo 3 > /proc/sys/vm/drop_caches'

    echo "Laboratory mode active"
    ;;

  disable)
    echo "Disabling laboratory mode..."
    
    # 1. Restore ASLR and perf settings
    if host_exec [ -f /tmp/lab_aslr ]; then
      VAL=$(host_exec cat /tmp/lab_aslr)
      host_exec sh -c "echo $VAL > /proc/sys/kernel/randomize_va_space"
      host_exec rm -f /tmp/lab_aslr
    else
      host_exec sh -c "echo 2 > /proc/sys/kernel/randomize_va_space"
    fi
    
    if host_exec [ -f /tmp/lab_perf_paranoid ]; then
      VAL=$(host_exec cat /tmp/lab_perf_paranoid)
      host_exec sh -c "echo $VAL > /proc/sys/kernel/perf_event_paranoid"
      host_exec rm -f /tmp/lab_perf_paranoid
    else
      host_exec sh -c "echo 3 > /proc/sys/kernel/perf_event_paranoid"
    fi

    if host_exec [ -f /tmp/lab_kptr ]; then
      VAL=$(host_exec cat /tmp/lab_kptr)
      host_exec sh -c "echo $VAL > /proc/sys/kernel/kptr_restrict"
      host_exec rm -f /tmp/lab_kptr
    else
      host_exec sh -c "echo 1 > /proc/sys/kernel/kptr_restrict"
    fi
    
    # 2. Restore Turbo Boost
    if host_exec [ -f /tmp/lab_turbo ]; then
      VAL=$(host_exec cat /tmp/lab_turbo)
      host_exec sh -c "echo $VAL > /sys/devices/system/cpu/intel_pstate/no_turbo" 2>/dev/null || true
      host_exec rm -f /tmp/lab_turbo
    else
      host_exec sh -c "echo 0 > /sys/devices/system/cpu/intel_pstate/no_turbo" 2>/dev/null || true
    fi
    
    # 3. Restore CPU scaling governors and frequencies for cores 1-3
    for cpu in 1 2 3; do
      if host_exec [ -f /tmp/lab_gov_cpu$cpu ]; then
        VAL=$(host_exec cat /tmp/lab_gov_cpu$cpu)
        host_exec sh -c "echo $VAL > /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_governor" 2>/dev/null || true
        host_exec rm -f /tmp/lab_gov_cpu$cpu
      fi
      if host_exec [ -f /tmp/lab_min_cpu$cpu ]; then
        VAL=$(host_exec cat /tmp/lab_min_cpu$cpu)
        host_exec sh -c "echo $VAL > /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_min_freq" 2>/dev/null || true
        host_exec rm -f /tmp/lab_min_cpu$cpu
      fi
      if host_exec [ -f /tmp/lab_max_cpu$cpu ]; then
        VAL=$(host_exec cat /tmp/lab_max_cpu$cpu)
        host_exec sh -c "echo $VAL > /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_max_freq" 2>/dev/null || true
        host_exec rm -f /tmp/lab_max_cpu$cpu
      fi
      if host_exec [ -f /tmp/lab_epp_cpu$cpu ]; then
        VAL=$(host_exec cat /tmp/lab_epp_cpu$cpu)
        host_exec sh -c "echo $VAL > /sys/devices/system/cpu/cpu$cpu/cpufreq/energy_performance_preference" 2>/dev/null || true
        host_exec rm -f /tmp/lab_epp_cpu$cpu
      fi
      for state in {1..9}; do
        if host_exec [ -f /tmp/lab_cstate_cpu${cpu}_state${state} ]; then
          VAL=$(host_exec cat /tmp/lab_cstate_cpu${cpu}_state${state})
          host_exec sh -c "echo $VAL > /sys/devices/system/cpu/cpu$cpu/cpuidle/state$state/disable" 2>/dev/null || true
          host_exec rm -f /tmp/lab_cstate_cpu${cpu}_state${state}
        fi
      done
    done
    
    # 4. Restart previously stopped containers
    if host_exec [ -f /tmp/lab_stopped_containers ]; then
      stopped_containers=$(host_exec cat /tmp/lab_stopped_containers)
      for name in $stopped_containers; do
        if [ ! -z "$name" ]; then
          echo "Starting $name..."
          docker start "$name" 2>/dev/null || echo "Warning: failed to start $name"
        fi
      done
      host_exec rm -f /tmp/lab_stopped_containers
    fi
    
    host_exec systemctl start server-monitor 2>/dev/null || echo "Warning: server-monitor not started"
    
    echo "Laboratory mode disabled"
    ;;

  *)
    echo "Usage: $0 {enable|disable}"
    exit 1
    ;;
esac
