#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>

enum PType {End, Mid};
typedef struct {int x; int y; int type;} point;

Display *display;
GC gc;
Window window_id;


draw_line(point a, point b, float t) {
    XDrawLine(display, window_id, gc, a.x, a.y, b.x, b.y);
}

draw_point(point a) {
    static int diameter = 5;
    XFillArc(display, window_id, gc,
        a.x, a.y, diameter, diameter, 360*64, 360*64);
}

point inter_point(point a, point b, float t) {
    point result = {a.x+t*(b.x-a.x), a.y + t*(b.y-a.y), Mid};
    return result;
}

point bezier(point *points, float t) {
    if(points[0].type == End) {   
        draw_point(points[0]); 
        return points[0];
    }
    int i;
    for(i = 0; points[i+1].type != End; i++) {
        draw_line(points[i], points[i+1], t);
        points[i] = inter_point(points[i], points[i+1], t);
    }
    points[i].type = End;
    return bezier(points, t);
}

long xscreen_window() {
    char *xscreen_id = getenv("XSCREENSAVER_WINDOW");
    if(!xscreen_id) {
        printf("Couldn't find window!\n");
        exit(-1);
    }
    return strtol(xscreen_id, 0, 16);
}

void init_points(point *points, int count) {
    int i;
    long dummy_id;
    int width, height, dummy;
    XGetGeometry(display, window_id, &dummy_id,
              &dummy, &dummy, &width, &height, &dummy, &dummy);

    for(i = 0; i < count; i++) {
        points[i].x = random() % width;
        points[i].y = random() % height;
        points[i].type = Mid;
    }
    points[i].type = End;
}

void draw_lines(point *points) {
    int i;
    for(i = 0; points[i+1].type != End; i++) {
        draw_line(points[i], points[i+1], 0);
    }
}

#define MAX_POINTS 8
#define MAX_TIME 1000
#define SHORT_SLEEP 10*1000
#define FINISH_SLEEP 1000*1000

int main() {
    printf("Welcome!\n");
    display = XOpenDisplay(getenv("DISPLAY"));
    window_id = xscreen_window();
    gc = XCreateGC(display, window_id, 0, NULL);
    int i;
    point points[MAX_POINTS];
    point sequence[MAX_TIME];

    XSetForeground(display, gc, XWhitePixel(display, 0));
    while(1) {
        init_points(points, MAX_POINTS); 
        point temp_points[MAX_POINTS];
        for(i = 0; i < MAX_TIME; i += 1) {
            float t = i / (float) MAX_TIME;
            XClearWindow(display, window_id);
            memcpy(temp_points, points, MAX_POINTS*sizeof(point));   
            sequence[i] = bezier(temp_points, t);
            draw_lines(sequence);
            sequence[i].type = Mid;
            XFlush(display);
            usleep(SHORT_SLEEP);
        }
        XClearWindow(display, window_id);
        sequence[i].type = End;
        draw_lines(sequence);
        XFlush(display);
        usleep(FINISH_SLEEP);
    }
}
