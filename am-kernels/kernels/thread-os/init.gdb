define hook-quit
  kill
end

set pagination off
target remote localhost:1234
file build/thread-os-x86_64-qemu.elf
break main
layout src
continue