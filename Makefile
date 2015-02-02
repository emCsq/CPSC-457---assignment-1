obj-m += checksum.o

all: itrace checksum assemble

itrace: itrace.c
	gcc -o  itrace itrace.c -ludis86

checksum: checksum.c
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install

assemble: hello.asm
	nasm -f elf32 -g hello.asm
	ld hello.o -o hello

cleanAll: cleaniTrace cleanChecksum cleanHello

cleaniTrace: itrace.c
	rm -r itrace

cleanChecksum:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

cleanHello:
	rm -r hello
