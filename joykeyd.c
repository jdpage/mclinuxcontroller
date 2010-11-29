#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <linux/types.h>

#define XK_XKB_KEYS
#define XK_LATIN1
#define XK_MISCELLANY

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

struct vector {
	int x;
	int y;
};

int tdiff(struct timeval a, struct timeval b) {
	int microseconds;
	microseconds = (a.tv_sec - b.tv_sec) * 1000000;
	microseconds += a.tv_usec - b.tv_usec;
	return microseconds / 1000;
}

int main(int argc, char **argv) {
	Display *dpy;
	int fd, i, flags;
	unsigned int last_dkey, last_slot = 0, last_6, last_7;
	struct js_event ev[64];
	struct vector mouse;
	struct timeval t, last_t;

	gettimeofday(&last_t, NULL);

	mouse.x = 0; /* no movement */
	mouse.y = 0;

	dpy = XOpenDisplay(NIL);
	assert(dpy);
	fd = open("/dev/input/js0", O_RDONLY);

	if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
		flags = 0;
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	while (1) {
		size_t rb = read(fd, ev, sizeof(ev));
		if (rb < 0) {
			if (errno == EAGAIN)
				goto domouse;
		}
		if (rb < (int)sizeof(struct js_event)) {
			perror("short read");
			return 1;
		}

		for (i = 0; i < (int)(rb / sizeof(struct js_event)); i++) {
			if (ev[i].type == JS_EVENT_BUTTON) {
				unsigned int k, mode = 0;
				switch (ev[i].number) {
					case 0:
						k = XK_i;
						break;
					case 1:
						k = XK_Escape;
						break;
					case 3:
						k = XK_space;
						break;
					case 2:
						k = XK_Shift_L;
						break;
					case 6:
						if (ev[i].value) {
							last_slot = (last_slot + 8) % 9;
							k = XK_1 + last_slot;
							last_6 = k;
						} else {
							k = last_6;
						}
						break;
					case 7:
						if (ev[i].value) {
							last_slot = (last_slot + 1) % 9;
							k = XK_1 + last_slot;
							last_7 = k;
						} else {
							k = last_7;
						}
						break;
					case 8:
						k = XK_t;
						break;
					case 9:
						k = XK_f;
						break;
					case 4:
						k = 3;
						mode = 1;
						break;
					case 5:
						k = 1;
						mode = 1;
						break;
				}
				if (!mode) {
					XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, k), ev[i].value, 0);
				} else {
					XTestFakeButtonEvent(dpy, k, ev[i].value, 0);
				}
			} else if (ev[i].type == JS_EVENT_AXIS) {
				unsigned int k, mode = 0;
				switch (ev[i].number) {
					case 2:
						mouse.x = ev[i].value;
						break;
					case 3:
						mouse.y = ev[i].value;
						break;
					case 4:
						if (ev[i].value < 0)
							k = XK_a;
						else if (ev[i].value > 0)
							k = XK_d;
						mode = 1;
						break;
					case 5:
						if (ev[i].value < 0)
							k = XK_w;
						else if (ev[i].value > 0)
							k = XK_s;
						mode = 1;
						break;
				}
				if (mode) {
					if (ev[i].value == 0) {
						XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, last_dkey), 0, 0);
					} else {
						XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, k), 1, 0);
						last_dkey = k;
					}
				}
			}
		}
domouse:
		gettimeofday(&t, NULL);
		if (tdiff(t, last_t) > 16) {
			XTestFakeRelativeMotionEvent((Display *)dpy, mouse.x / 0xfff, mouse.y / 0xfff, 0);
			XFlush((Display *)dpy);
			last_t = t;
		}
	}

	close(fd);
	return 0;
}
