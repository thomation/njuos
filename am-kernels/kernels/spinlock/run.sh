qemu-system-x86_64 -serial mon:stdio -machine accel=tcg -smp 2,sockets=2,cores=4,threads=1,maxcpus=8  -drive format=raw,file=./build/spinlock-x86_64-qemu