#include <stdio.h>
#include <fcntl.h>
#include <linux/types.h>

#define XK_XKB_KEYS
#define XK_LATIN1

#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/extensions/XTest.h>
#include <assert.h>

#define NIL (0)

#define JS_EVENT_BUTTON 0x01
#define JS_EVENT_AXIS   0x02
#define JS_EVENT_INIT   0x80

struct js_event {
	__u32 time;
	__s16 value;
	__u8 type;
	__u8 number;
};

int main() {
	Display *dpy;
	int fd, i;
	struct js_event ev[64];

	dpy = XOpenDisplay(NIL);
	assert(dpy);
	fd = open("/dev/input/js0", O_RDONLY);

	while (1) {
		size_t rb = read(fd, ev, sizeof(ev));
		if (rb < (int)sizeof(struct js_event)) {
			perror("short read");
			return 1;
		}

		for (i = 0; i < (int)(rb / sizeof(struct js_event)); i++) {
			printf("time: %x / value: %d / type: %x / number: %x\n", ev[i].time, ev[i].value, ev[i].type, ev[i].number);
			if (ev[i].type == JS_EVENT_BUTTON && ev[i].number == 0x1) {
				XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_a), ev[i].value, 0);
				XFlush(dpy);
			}
			if (ev[i].type == JS_EVENT_AXIS && ev[i].number == 0) {
				XTestFakeRelativeMotionEvent(dpy, (int)(0.001 * ev[i].value), 0, 0);
				XFlush(dpy);
			}
		}

	}

	close(fd);
	return 0;
}
