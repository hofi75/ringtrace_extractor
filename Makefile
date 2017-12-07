
all: rt_core rt_core2
install: rt_core
	install -m 0755 rt_core /usr/local/bin

rt_core: rt_core.c
	gcc -g -o rt_core rt_core.c

rt_core2: rt_core2.c
	gcc -g -o rt_core2 rt_core2.c

test32: test.c
	gcc -g -m32 -o test32 test.c

test64: test.c
	gcc -g -o test64 test.c
