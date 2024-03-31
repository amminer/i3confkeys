cc = gcc
cflags = -lX11 -lxkbcommon-x11 -lxkbcommon
exe = i3confkeys

main:
	$(cc) main.c -o $(exe) $(cflags)

run: main
	./$(exe)

