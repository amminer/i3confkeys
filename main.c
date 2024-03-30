#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>


enum qwerty {  // TODO
	Esc = 66
};


unsigned long get_active_window_id(Display* display) {
	Atom type_return;
	int format_return;
	ulong nitems_return;
	ulong bytes_left;
	unsigned char *data;
	unsigned long active_window_id;

	Window root = DefaultRootWindow(display);
	Atom property = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
	XGetWindowProperty(
		display,	// which display
		root,		// which window
		property,	// property name
		0,		// offset into property
		1,		// num atoms to retrieve
		False,		// whether to delete property
		XA_WINDOW,	// type of property to get
 		&type_return,
		&format_return,
		&nitems_return,
		&bytes_left,
		&data
	);

	//(return_value == Success) && nitems_return // TODO err handling
	Window active_window = ((Window*) data)[0];
	active_window_id = *(ulong*)data;  // TODO different type based on format_return?
	printf("retrieved %d items of type %d (requested %d) with %d bytes left to read: 0x%lx\n",
	       nitems_return, type_return, XA_WINDOW, bytes_left, active_window_id);
	XFree(data);
	return active_window_id;
}


int main() {
	Display *display;
	XEvent event, previous_event;
	unsigned long active_window_id;
	KeyCode keycode;
	KeySym keysym;
	char *keyname;

	display = XOpenDisplay(NULL);
	if (display == NULL) {
		fprintf(stderr, "Unable to open X display\n");
		exit(1);
	}

	active_window_id = get_active_window_id(display);

	// steal the keyboard from the terminal
	XGrabKeyboard(
		display,
		active_window_id,
		False,			// report events with respect to grabber
		GrabModeAsync,		// mouse pointer events
		GrabModeAsync,		// keyboard events
		CurrentTime
	);

	// tell x what events we want
	XSelectInput(display, active_window_id, KeyPressMask|KeyReleaseMask);

	// event loop
	while (1) {
		XNextEvent(display, &event);
		if (event.type == KeyPress) {
			keycode = event.xkey.keycode;
			keysym = XkbKeycodeToKeysym(display, keycode, 0, 0);
			keyname = XKeysymToString(keysym);
			printf("Key down: %d, %s\n", keycode, keyname);
		} else if (event.type == KeyRelease) {
			keycode = event.xkey.keycode;
			keysym = XkbKeycodeToKeysym(display, keycode, 0, 0);
			keyname = XKeysymToString(keysym);
			printf("Key up: %d, %s\n", keycode, keyname);
		}
		// hit escape twice to finish up
		if (keycode == Esc
		&& previous_event.xkey.keycode == Esc
		&& previous_event.type == KeyRelease) {
			// TODO finalize output string using keynames
			break;  // TODO maybe use ctrl+c instead
		}
		previous_event = event;
	}
	// TODO clipboard logic

	/*
	When I press alt, press prtscr, release prtscr, and release alt, I get this:

	Key down: 64, Alt_L
	Key up: 64, Alt_L
	Key down: 64, Alt_L
	Key down: 107, Print
	Key up: 107, Print
	Key up: 64, Alt_L

	I was hoping for Sys_Req. It looks like xlib is lower level than I was
	hoping/expecting. Going to try something else, but wanted to keep this commit
	around for future reference.
	*/

	return 0;
}

