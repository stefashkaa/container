# Create Container

## Goal

1. Create own container
    * create child process
    * isolate it using namespaces
    * setup guest-host connection
    * create mountpoint
    * use cgroups for qouting
2. Benchmark [Own container, host machine, LXC, Docker]

## Run

1. Clone this repo:
`git clone https://github.com/stefashkaa/container.git`
2. Go to repo directory:
`cd container`
3. Compile program via GCC:
`gcc container.c`
4. Before using *cgroups* you should run the following commands:
```
sudo mkdir /sys/fs/cgroup/cpu/demo
echo 50000 > /sys/fs/cgroup/cpu/demo/cpu.cfs_quota_us
echo 100000 > /sys/fs/cgroup/cpu/demo/cpu.cfs_period_us
```
5. Run the command:
`sudo ./a.out`
6. Enjoy !
