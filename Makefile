all:
	gcc -Wall cfuse/src/cfuse.c cfuse/src/gd_interface.c cfuse/src/map.c `pkg-config fuse3 --cflags --libs` -lcurl -o cfuse_bin

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
	fusermount -u $(PWD)/mount_point
	gcc -Wall cfuse/src/cfuse.c cfuse/src/gd_interface.c cfuse/src/map.c `pkg-config fuse3 --cflags --libs` -lcurl -o cfuse_bin
	./cfuse_bin $(PWD)/mount_point
	./test/main

