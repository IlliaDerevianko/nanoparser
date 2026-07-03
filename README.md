### Resources
**NASDAQ historical market feed** - https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/

### System setup
```bash
sudo ip netns add ns_port2
sudo ip link set enp1s0f1 netns ns_port2
sudo ip netns exec ns_port2 ip addr add 10.99.99.2/24 dev enp1s0f1
sudo ip netns exec ns_port2 ip link set enp1s0f1 up
sudo ip netns exec ns_port2 ip link set lo up
sudo ip addr add 10.99.99.1/24 dev enp1s0f0np0
sudo ip link set enp1s0f0np0 up
```

### System tuning
BIOS: disable SMT, disable frequency boost

Kernel: Ubuntu realtime
```bash
# In /etc/default/grub
GRUB_CMDLINE_LINUX_DEFAULT="quiet splash amd_pstate=passive mitigations=off nowatchdog kthread_cpus=0-4 nohz nohz_full=5 isolcpus=domain,5 rcu_nocbs=5 irqaffinity=0-4 rcu_nocb_poll nosoftlockup"
# And run
sudo update-grub
```
```bash
cat /proc/interrupts | grep enp1s0f1
for i in {98..103}; do
    echo 5 | sudo tee /proc/irq/$i/smp_affinity_list
done

echo performance | sudo tee /sys/devices/system/cpu/cpu5/cpufreq/energy_performance_preference
for f in /sys/bus/workqueue/devices/*/cpumask; do
    echo 1f | sudo tee $f
done
echo -1 | sudo tee /proc/sys/kernel/sched_rt_runtime_us
echo performance | sudo tee /sys/devices/system/cpu/cpu5/cpufreq/scaling_governor
for i in /sys/devices/system/cpu/cpu5/cpuidle/state*/disable; do
    echo 1 | sudo tee $i
done

sudo ip netns exec ns_port2 ethtool -C enp1s0f1 adaptive-rx off rx-usecs 0
sudo ip netns exec ns_port2 ethtool -G enp1s0f1 rx 4096
sudo ip netns exec ns_port2 ethtool -K enp1s0f1 gro off lro off
```
