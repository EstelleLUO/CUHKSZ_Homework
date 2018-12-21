#include "mpi.h"
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <vector>
#define ROOT 0
#define X_RESN 1600
#define Y_RESN 1600
#define MAX_ITERATION 1000000
using namespace std;

/* Global Xwindow Parameters */
double minX, minY;
double maxX, maxY; 
int threads;
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


typedef struct complextype
{
	double real, imag;
} Compl;

/* Store the values of iterations of each pixel */
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
    XFlush(display);
}

int main(int argc, char **argv)
{
    /* Record Execution Time */
    
    double beginTime, endTime;
    beginTime=MPI_Wtime();


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
    
    /* MPI Initialization*/
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    /*
    for (int i=0;i<size;i++){
        if (rank==i){
            beginTime = MPI_Wtime();
        }
    }
*/
    threads=size;

    MPI_Status status;
    MPI_Request request[size];
   
	/* assign tasks */
    int numPerTask = width/(size);
    if (rank == size - 1) numPerTask += width % (size);
    
    /* connect to Xserver */
    if (isEnable && rank == 0) XWindow_Init();
    
    MPI_Barrier(MPI_COMM_WORLD); 

    if (rank == 0 && size > 1) {
        bool isReady[size];
        int thread = 1;
        bool isRunning; 
        bool task; 
        /* Gather tasks*/
        for (int i = 1; i < size; ++i)
            MPI_Irecv(&task, 1, MPI_CHAR, i, MPI_ANY_TAG, MPI_COMM_WORLD, &request[i]);

        for (int i = 0; i < width; ++i) {
            isRunning = false;
            while (!isRunning) {
                int flag;
                /* Check the completion of the task*/
                MPI_Test(&request[thread], &flag, &status);
                if (flag) {
                    MPI_Irecv(&task, 1, MPI_CHAR, thread, MPI_ANY_TAG, MPI_COMM_WORLD, &request[thread]); 
                    MPI_Send(&i, 1, MPI_INT, thread, 0, MPI_COMM_WORLD);
                    isRunning = true;
                } 
                thread = (thread + 1) % size;
                if (thread == 0)
                    thread = (thread + 1 ) % size;
            }
        }
        for (int i = 1; i < size; ++i) {
            int flag;
            while (!flag) MPI_Test(&request[i], &flag, &status);
            int stop = -1;
            MPI_Send(&stop, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    else {
        vector<Pixel> pixelArray;
        int curIndex = 0;
        int beginPos;
        if (rank!=size-1) beginPos=numPerTask * (rank);
        else beginPos=(rank) * (numPerTask - (width % (size)));
        
        /* when the slave is avaible*/
        bool isRunning = true;
        int i = width - 1;
        if (rank != 0) {
            MPI_Isend(&isRunning, 1, MPI_CHAR, ROOT, 0, MPI_COMM_WORLD,&request[rank]);
            MPI_Recv(&i, 1, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
        while (i != -1) {
            /* Calculation */
            for (int j = 0; j < height; j++) { 
                Compl z, c;
                double temp, lengthsq;
                int k;
                z.real = 0.0;
                z.imag = 0.0;
                
                double scaleX = width / (maxX - minX);
                double scaleY = height / (maxY - minY);
                
                c.real = ((double)i + scaleX * minX) / scaleX; 
                c.imag = ((double)j + scaleY * minY) / scaleY; 

                k = 0;
                lengthsq = 0.0;

                /* calculate the maximum iterations at each pixel*/
                while(k < MAX_ITERATION && lengthsq < 4.0) { 
                    temp = z.real*z.real - z.imag*z.imag + c.real;
                    z.imag = 2*z.real*z.imag + c.imag;
                    z.real = temp;
                    lengthsq = z.real*z.real + z.imag*z.imag; 
                    k++;
                }
                if (!isEnable) continue;
                pixelArray.push_back(Pixel(i, j, k));
            }
            if (size == 1)  i--;
            if (rank != ROOT) {
                MPI_Isend(&isRunning, 1, MPI_CHAR, ROOT, 0, MPI_COMM_WORLD,&request[rank]);
                MPI_Recv(&i, 1, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            }
        }
        /* handle the case when there's only one core*/
        if (size == 1 && isEnable) {
            int count=2;
            while (count>0){
                for (int k = 0; k < pixelArray.size(); ++k){
                    int i = pixelArray[k].i;
                    int j = pixelArray[k].j;
                    int iteration = pixelArray[k].iterations;
                    XSetForeground (display, gc, 1024 * 1024 * (iteration % 256));	
                    XDrawPoint (display, window, gc, i, j);
                }
                count--;
            }
        }
        /* handle the case where there are more than one cores*/
        else if (isEnable) { 
            Pixel *pixel = new Pixel[pixelArray.size()];
            for (int i = 0; i < pixelArray.size(); ++i) pixel[i] = pixelArray[i];
            curIndex = pixelArray.size();
            MPI_Send(&curIndex, 1, MPI_INT, ROOT,0,MPI_COMM_WORLD);       
            MPI_Send(pixel, curIndex * sizeof(Pixel), MPI_CHAR, ROOT,0,MPI_COMM_WORLD);
        } 
    }
    if (isEnable && rank == ROOT && size != 1) {
        vector<Pixel> v; 
        for (int threads = 1; threads < size; ++threads) {
            int pixelNum;
            MPI_Recv(&pixelNum, sizeof(int), MPI_INT, threads, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            Pixel pixel[pixelNum];
            MPI_Recv(pixel, pixelNum * sizeof(Pixel), MPI_CHAR, threads, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            for (int i = 0; i < pixelNum; ++i) v.push_back(pixel[i]);
        }
       /* draw points */
        vector<Pixel>::iterator it;
        int count=2;
        while (count>0){
          for (it = v.begin(); it != v.end(); ++it) {
              int i = it->i;
              int j = it->j;
              int iterations = it->iterations;
              XSetForeground (display, gc, 1024 * 1024 * (iterations % 256));	
              XDrawPoint (display, window, gc, i, j);
          }
          count--;
        }
    }
    /*
    for (int i=0;i<size;i++){
        if (rank==i){
            endTime = MPI_Wtime();
            cout<<"Execution Time:" <<endTime-beginTime<<endl;
        }
    }
*/
    MPI_Barrier(MPI_COMM_WORLD);

    endTime=MPI_Wtime();
    if (rank==0) cout << "Execution Time: "<<endTime-beginTime<<endl;

    MPI_Finalize();
    if (isEnable) sleep(30);
	return 0;
}