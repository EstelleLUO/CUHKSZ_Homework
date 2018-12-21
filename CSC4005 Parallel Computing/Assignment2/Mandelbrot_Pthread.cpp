#include "pthread.h"
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <vector>
#include <malloc.h>
#include <time.h>
#include <iostream>
#define ROOT 0
#define X_RESN 800
#define Y_RESN 800
#define MAX_ITERATION 10000
#define MAX_THREADS 1000
#define NUM_THREADS 10
using namespace std;

/* Global Xwindow Parameters */
double minX, minY;
double maxX, maxY; 
int x,y,width, height, border_width;
bool isEnable = false;
Display *display;
Window window;      //initialization for a window
char *window_name = "Mandelbrot Set", *display_name = NULL;
int screen;         //which screen 
/* create graph */
GC gc;
XGCValues values;
long valuemask = 0;

int tasks[MAX_THREADS];
int taskRemaining;
int nThreads;
pthread_mutex_t	mutexJob, mutexX;


typedef struct complextype
{
	double real, imag;
} Compl;

struct Pixel {
    int i, j, iterations;
    Pixel(){};
    Pixel(int _i, int _j, int _repeat) {
        i = _i, j = _j, iterations = _repeat;
    }
};

void XWindow_Init(){
    if((display = XOpenDisplay (display_name)) == NULL) {
        fprintf(stderr, "drawon: cannot connect to X server %s\n",XDisplayName (display_name));
        exit(-1);
    }

    /* get the screen size */
    screen = DefaultScreen(display);

    /* create window */
    window = XCreateSimpleWindow(display, 
        RootWindow(display, screen), x, y, 
            width, height, border_width, 
                BlackPixel(display, screen), 
                    WhitePixel(display, screen));
    
    gc = XCreateGC(display, window, valuemask, &values); 
    XSetForeground (display, gc, BlackPixel (display, screen));
    XSetBackground(display, gc, 0X0000FF00);
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);
    
    /* map the window */
    XMapWindow(display, window);
    XSync(display, 0);
}

void* Mandelbrot_calc(void *t){
    /* Check load balance 
    struct timespec start, finish;
    clock_gettime(CLOCK_MONOTONIC, &start);
    */
    int pid,j,k;
    /* Mandlebrot variables */
    Compl z, c;
    float lengthsq, temp;
    pid = *((int*)t);

    while (true){
        for (j=0;j<Y_RESN;j++){
            z.real = z.imag = 0.0;
            double scaleX = width / (maxX - minX);
            double scaleY = height / (maxY - minY);

            c.imag = ((double)pid + scaleX * minX) / scaleX;             
            c.real = ((double)j + scaleY * minY) / scaleY; 
            k = 0;
            lengthsq=0.0;
            while (k<MAX_ITERATION && lengthsq<4.0){
                temp = z.real*z.real - z.imag*z.imag + c.real;
                z.imag = 2.0*z.real*z.imag + c.imag;
                z.real = temp;
                lengthsq = z.real*z.real+z.imag*z.imag;
                k++;
            }
            /* Draw Points */
            if (isEnable){
                pthread_mutex_lock(&mutexX);
                XSetForeground (display, gc, 1024 * 1024 * (k % 256));	
                XDrawPoint(display,window,gc,j,pid);
                pthread_mutex_unlock(&mutexX);
            }
        }
        pthread_mutex_lock(&mutexJob);
        if (taskRemaining<0){
            /*
            clock_gettime(CLOCK_MONOTONIC, &finish);
            cout<<"Execution Time: "<<finish.tv_sec-start.tv_sec + (double)(finish.tv_nsec - start.tv_nsec)/1000000000.0<<endl;
            */
            pthread_mutex_unlock(&mutexJob);
            pthread_exit(NULL);
        }
        else pid=taskRemaining--;
        pthread_mutex_unlock(&mutexJob);
    }
}

int main(int argc, char **argv)
{
	struct timespec start, finish;
	clock_gettime(CLOCK_MONOTONIC, &start);
    /* Set the position of Mandelbrot Set*/
    minX=minY=-2;
    maxX=maxY=2;

    string in = argv[1];
    if (in == "enable"){
        isEnable = true;
        /* set window size */
        width=X_RESN;
        height=Y_RESN;

        /* set window position */
        x = 0;
        y = 0;

        /* border width in pixels */
        border_width = 0;
    }
    else isEnable = false;
    nThreads=NUM_THREADS;
    nThreads=min(nThreads,X_RESN);
    /* connect to Xserver */
    if (isEnable) XWindow_Init();

    /* Pthread Initialization*/
 	pthread_mutex_init(&mutexJob, NULL);	
	pthread_mutex_init(&mutexX, NULL);	
    pthread_t *threads=(pthread_t*)malloc(sizeof(pthread_t)*nThreads);

    /* Pthread Creation */
    taskRemaining=X_RESN-nThreads;
    for (int i=0;i<nThreads;i++){
        tasks[i]=X_RESN-i-1;
        pthread_create(&threads[i],NULL,Mandelbrot_calc,(void*)&tasks[i]);
    }

    for (int i=0;i<nThreads;++i){
        pthread_join(threads[i],NULL);
    }

    if (isEnable){
        XFlush(display);
    } 
    clock_gettime(CLOCK_MONOTONIC, &finish);
	cout << "Execution Time: "<<finish.tv_sec-start.tv_sec + (double)(finish.tv_nsec - start.tv_nsec)/1000000000.0<<endl;
    sleep(50);
    pthread_exit(NULL);
	return 0;
}