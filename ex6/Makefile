all: vm vm_d

vm: bef_vm.c
	gcc -O3 -Wall bef_vm.c -o vm

vm_d: bef_vm.c
	gcc -Wall -g -DDEBUG bef_vm.c -o vm_d

clean:
	rm vm vm_d
	rm -r vm_d.dSYM
