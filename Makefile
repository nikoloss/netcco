obj-m += netcco.o

all: netccoctl
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm netccoctl
	
netccoctl:	netccoctl.c nc_def.h