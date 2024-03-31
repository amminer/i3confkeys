#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>


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
	/* printf("retrieved %d items of type %d (requested %d) with %d bytes left to read: 0x%lx\n",
	       nitems_return, type_return, XA_WINDOW, bytes_left, active_window_id); */
	XFree(data);
	return active_window_id;
}


char* handle_keypress(Display *display, XEvent event) {
	XKeyEvent *e = (XKeyEvent *) &event;
	KeyCode keycode;
	KeySym keysym;
	char str[256 + 1];
	int nbytes = 0;
	char *ksname;

	keycode = event.xkey.keycode;
	keysym = XkbKeycodeToKeysym(display, keycode, 0, 0);
	nbytes = XLookupString(e, str, 256, &keysym, NULL);
	ksname = XKeysymToString(keysym);
	/* if (e->type == KeyPress) {
		printf("Key down: %d, %s\n", keycode, ksname);
	} else {
		printf("Key up: %d, %s\n", keycode, ksname);
	} */
	return ksname;
}


void teardown(Display* display){
	XCloseDisplay(display);
	exit(0);
}


struct output {
	char *keysym;
	struct output *next;
};


void initialize_output_element(struct output *last_output, char* last_ksname) {
	last_output->keysym = last_ksname;
	last_output->next = NULL;
}


struct output *append_output(struct output *last_output) {
	struct output *new_output = malloc(sizeof(struct output));
	last_output->next = new_output;
	return new_output;
}


int main() {
	Display *display;
	XEvent event;
	unsigned long active_window_id;
	char *last_ksname;
	struct output *out_head = malloc(sizeof(struct output));
	struct output *last_output = out_head;

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
		last_ksname = handle_keypress(display, event);
		initialize_output_element(last_output, last_ksname);
		if (event.xkey.keycode != Esc) {
			// TODO parse keycodes into a set
			// maybe a linkedlist isn't ideal for this but it was easy to implement for now
			last_output = append_output(last_output);
		} else {
			break;
		}  // TODO some other keycode or combo to quit
	}
	// dump output and free memory
	while (out_head != NULL) {
		printf("%s%s", out_head->keysym, out_head->next ? "+" : "");
		last_output = out_head;
		out_head= out_head->next;
		free(last_output);
	}

	// TODO get rid of beginning enter keyup if it's present - might not be,
	// depending on how we were launched and host speed.
	// Also get rid of terminator sequence if it's present - right now, just
	// an enter keydown, but could be something else later.
	printf("\n");

	// TODO insert keysym string at cursor - use clipboard?

	teardown(display);
}

