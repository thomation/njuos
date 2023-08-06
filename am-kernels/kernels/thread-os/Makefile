NAME = thread-os
SRCS = thread-os.c
include $(AM_HOME)/Makefile

debug:
	qemu-system-x86_64 -s -S build/thread-os-x86_64-qemu &
	gdb -x init.gdb