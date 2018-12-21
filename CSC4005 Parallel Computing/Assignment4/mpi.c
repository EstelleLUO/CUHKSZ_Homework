#include "mpi.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#define WIDTH 200
#define HEIGHT 200
#define FIREPLACE_WIDTH 40
#define MASTER 0
#define TRUE 1
#define FALSE 0
#define MAX_ITERATION 1000
#define FRAME_INTERVAL 20
#define X_REFRESH_RATE 1000

#define ROOM_TEMP 20
#define FIRE_TEMP 100
#define COLOR_RATIO (float)(65535.0 / 255.0)
#define isEnable 1 //1 indicates showing the window while 0 is not
#define EPSILON 1

Window window;
XSizeHints size_hints;
Display *display;
char *window_name = "Temperature Simulation", *display_name = NULL;

int screen, border_width;
XColor yellow, green, black, white;
XColor temperature[256];
Colormap default_cmap  ;
XEvent myevent;
GC gc, startGC, pauseGC, exitGC;
char colorTable[] = "0 0 150\n0 0 255\n0 75 255\n0 150 255\n0 200 255\n50 255 255\n150 255 255\n200 255 255\n255 255 150\n255 255 50\n255 200 0\n255 150 100\n255 150 50\n255 100 0\n230 0 0\n150 0 0\n100 0 0\n50 0 0";

int dx[4] = {0, -1, 0, 1};
int dy[4] = {1, 0, -1, 0};

void init_color() {
    default_cmap   = DefaultColormap(display, screen);

    char *color_num;
    /* Initialize the colors*/
    int idx = 3, count = 0;
    for(color_num = strtok(colorTable, " \n"); color_num != NULL; color_num = strtok(NULL, " \n")) {
        switch(count){
            case 0: temperature[idx].red = (int)(atoi(color_num) * COLOR_RATIO); break;
            case 1: temperature[idx].green = (int)(atoi(color_num) * COLOR_RATIO); break;
            case 2: temperature[idx].blue = (int)(atoi(color_num) * COLOR_RATIO); break;
        }
        count++;

        if(count == 3) {
            XAllocColor(display, default_cmap , &temperature[idx]); 
            idx++; 
            count = 0;
        }
    }
}

void init_window() {
    XSetWindowAttributes attr[1]; 

    display = XOpenDisplay(display_name);

    /* connect to Xserver */
    if (  (display = XOpenDisplay (display_name)) == NULL ) {
        fprintf (stderr, "drawon: cannot connect to X server %s\n",
                            XDisplayName (display_name) );
    exit (-1);
    }
    /* get screen size */
    screen = DefaultScreen(display);

    init_color();
    
    size_hints.x = 0;
    size_hints.y = 0;
    size_hints.width = WIDTH;
    size_hints.height = HEIGHT;
    size_hints.flags = USPosition | USSize;
    border_width = 0;

    window = XCreateSimpleWindow(display, RootWindow (display, screen), WIDTH, HEIGHT,
                                            WIDTH,HEIGHT,border_width, BlackPixel (display, screen), WhitePixel (display, screen));
    XSetStandardProperties(display, window, "", NULL, None, NULL, 0, &size_hints);
    
    gc = XCreateGC(display, window, 0, 0);

    XMapWindow (display, window);
    XSync(display, 0);
}
/* Use a map to record the temperature in the room*/
void init_map(float map[WIDTH][HEIGHT]) {
    /* Set the room temperature*/
    for(int i = 0; i < WIDTH; i++)
        for(int j = 0; j < HEIGHT; j++)
            map[i][j] = 15.0;

    /* Set the wall temperature*/
    for(int i = 0; i < WIDTH; i++) {
        map[i][0] = ROOM_TEMP;
        map[i][HEIGHT-1] = ROOM_TEMP;
    }
    
    for (int j=0;j<HEIGHT;j++){
        map[0][j] = ROOM_TEMP;
        map[WIDTH-1][j] = ROOM_TEMP;
    }

    /* Set the position for the fireplace*/
    for(int i = WIDTH/2-FIREPLACE_WIDTH/2; i < WIDTH/2+FIREPLACE_WIDTH/2; i++) map[i][0] = FIRE_TEMP;
}

void draw_map(float map[WIDTH][HEIGHT]) {
    for(int i = 0; i < WIDTH; i++)
        for(int j = 0; j < HEIGHT; j++) {
            XSetForeground(display, gc, temperature[(int)(map[i][j]/5.0)].pixel);
            XDrawPoint(display, window, gc, i, j);
        }
}

int main(int argc, char* argv[]) {
    int rank, rank_size;
    float map[WIDTH][HEIGHT], mid[WIDTH][HEIGHT];
    int iteration = MAX_ITERATION;
    struct timespec start, finish;
    double execution_time = 0;
    int size = 0;
    /* Record the error in the whole map*/
    float error=0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &rank_size);
    /* Decides the size of field distributed to each processor*/
    size = WIDTH * HEIGHT / rank_size;

    if (rank == MASTER) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        if (isEnable) init_window();
        /* Initialize three maps*/
        init_map(map);
        memcpy(mid, map, sizeof(map));
    }

    while (iteration != 0) {
        if (rank == MASTER) {
            MPI_Bcast(map, WIDTH * HEIGHT, MPI_FLOAT, MASTER, MPI_COMM_WORLD);
            MPI_Scatter(map, size, MPI_FLOAT, mid, size, MPI_FLOAT, MASTER, MPI_COMM_WORLD);
            for (int i = 0; i < size; i++) {
                int part = rank * size + i;
                int part_x = part / WIDTH, part_y = part % HEIGHT;
                int mid_x = i / WIDTH, mid_y = i % HEIGHT;
                /* On the boundary*/
                if (part_x == 0 || part_x == WIDTH - 1 || part_y == 0 || part_y == HEIGHT - 1) continue;
                /* Calculate the mid point's temperature*/
                mid[mid_x][mid_y] = 0;
                for (int k = 0; k < 4; k++) mid[mid_x][mid_y] += map[part_x + dx[k]][part_y + dy[k]];
                mid[mid_x][mid_y] /= 4;
            }
            MPI_Gather(mid, size, MPI_FLOAT, map, size, MPI_FLOAT, MASTER, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);

            iteration--;
            if (isEnable) {
                draw_map(map);
                XFlush(display);
                usleep(30000);
            }

        } else {
            /* Slave Process*/
            MPI_Bcast(map, WIDTH * HEIGHT, MPI_FLOAT, MASTER, MPI_COMM_WORLD);
            MPI_Scatter(map, size, MPI_FLOAT, mid, size, MPI_FLOAT, MASTER, MPI_COMM_WORLD);
            for (int i = 0; i < size; i++) {
                int part = rank * size + i;
                int part_x = part / WIDTH, part_y = part % HEIGHT;
                int mid_x = i / WIDTH, mid_y = i % HEIGHT;
                /* On the boundary */
                if (part_x == 0 || part_x == WIDTH - 1 || part_y == 0 || part_y == HEIGHT - 1) continue;
                /* Calculate the mid point's temperature*/
                mid[mid_x][mid_y] = 0;
                for (int k = 0; k < 4; k++) mid[mid_x][mid_y] += map[part_x + dx[k]][part_y + dy[k]];
                mid[mid_x][mid_y] /= 4;
            }
            MPI_Gather(mid, size, MPI_FLOAT, map, size, MPI_FLOAT, MASTER, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }
    if (rank ==MASTER){
        clock_gettime(CLOCK_MONOTONIC, &finish); 
        execution_time = (double)(finish.tv_sec-start.tv_sec) + (double)(finish.tv_nsec - start.tv_nsec)/1000000000;
        printf("Excution time is: %ld\n", execution_time);
    }

    MPI_Finalize();
    return 0;
}
