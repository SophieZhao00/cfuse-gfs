all:
	gcc -Wall cfuse/cfuse.c cfuse/gd_interface.c cfuse/map.c `pkg-config fuse3 --cflags --libs` -lcurl -o cfuse_bin

mount:
	./cfuse_bin $(PWD)/mount_point

unmount:
	fusermount -u $(PWD)/mount_point
	
unmount-hard:
	sudo unmount -l $(PWD)/mount_point

clean:
	rm cache/*

test_build:
	gcc test/main.c -o test/main
	./test/main

exp:
	gcc -Wall test/exp.c -o test/exp
	./test/exp

