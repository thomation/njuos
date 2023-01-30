# njuos
南大操作系统设计与实现的作业

# env
* win11, wls2, qemu-system-x86_64 7.0
* 直接在win11下显示gtk结果：make ARCH=native run
* 使用qemu: make ARCH=x86_64-qemu run，注意不能make后再启动qemu，那样键盘不好用，也不能再vscode里操作，没有sdl