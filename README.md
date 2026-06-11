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

sudo modprobe -r sfc
sudo modprobe sfc
sudo ip netns delete ns_port2
sudo ip netns add ns_port2
sudo ip link set enp1s0f1 netns ns_port2
sudo ip netns exec ns_port2 ip addr add 10.99.99.2/24 dev enp1s0f1
sudo ip netns exec ns_port2 ip link set enp1s0f1 up
sudo ip netns exec ns_port2 ip link set lo up
sudo ip addr add 10.99.99.1/24 dev enp1s0f0np0
sudo ip link set enp1s0f0np0 up
sudo ip netns exec ns_port2 build/release/receive