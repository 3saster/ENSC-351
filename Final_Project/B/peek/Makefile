filename = peek

obj-m += $(filename).o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

install:
	@-sudo rm /dev/$(filename) > /dev/null 2>&1 || true
	@-sudo rmmod $(filename) > /dev/null 2>&1 || true
	@sudo dmesg -C
	@sudo insmod $(filename).ko

test:
	@-sudo rm /dev/$(filename) > /dev/null 2>&1 || true
	@sudo mknod /dev/$(filename) c $(shell cat /proc/devices | grep $(filename) | awk '{print $$1;}') 0
	@head -c 2 /dev/$(filename) || true
	@head -c 1 /dev/$(filename) || true
	@printf 'ï¿½ï¿½ï¿' > /dev/$(filename) || true
	@head -c 1 /dev/$(filename) || true
	@dmesg

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

remove:
	@-sudo rm /dev/$(filename) > /dev/null 2>&1 || true
	@sudo rmmod $(filename).ko || true
