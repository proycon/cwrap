all:
	gcc cwrap.c -o cwrap -I/usr/include/ -L/usr/lib/ -lmicrohttpd
