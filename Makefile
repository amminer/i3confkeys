main:
	gcc main.c -o ./i3confkeys -lX11 -lxkbcommon-x11 -lxkbcommon
run: main
	./i3confkeys


