all:
	gcc -Wall cfuse/cfuse.c `pkg-config fuse3 --cflags --libs` -o cfuse
