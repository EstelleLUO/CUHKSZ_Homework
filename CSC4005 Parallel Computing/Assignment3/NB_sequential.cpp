/* Sequential N-body Simulation Program*/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <cmath>
#include <sys/time.h>
#define G 6//(6.67384*pow(10,-11)) //Gravity Constant
#define X_RESN 800
#define Y_RESN 800
#define timeInterval 0.02
#define theta 0.1
#define compute_time 2000
/*
#define sumPOW(x,y) (pow(x,2)+pow(y,2))
#define aG(x,y,m) (G*m/sqrt(sumPOW(x,y)))
#define SIN(x,y) (y/sqrt(sumPOW(x,y))) 
#define COS(x,y) (x/sqrt(sumPOW(x,y))) 
#define aGx(x,y,m) (aG(x,y,m)*SIN(x,y))
#define aGy(x,y,m) (aG(x,y,m)*COS(x,y))
#define v(v0,a,t) (v0+a*t)
#define MASS 1
*/
#define particle_num 500 //Number of particles
using namespace std;

/* Lower and uppter bound for paticle intial position */
int X_L = 3*X_RESN / 8.0;     
int X_U = 5*X_RESN / 8.0;    
int Y_L = 3*Y_RESN / 8.0;   
int Y_U = 5*Y_RESN / 8.0;     

/* Lower and upper bound for particle movement*/
int X_L_M = 2*X_RESN / 8.0;   
int X_U_M = 6*X_RESN / 8.0;   
int Y_L_M = 2*Y_RESN / 8.0;   
int Y_U_M = 6*Y_RESN / 8.0;  

/* Declaration */
struct particle;


/* 
* N-body Construction
* This holds information for a single particle,
* including position, velocity and mass.*/

typedef struct particle{
  double x_pos, y_pos;
  double x_v, y_v;
  double x_f, y_f; //Gravitational forces that apply against this particle
  double mass;
  int index;
}particle_t; 

/* Global variable Declaration */
particle_t particle[particle_num]; 

void initialization();
void update_force();
void update_position();

void initialization(){
    srand(time(0) + rand());
    for (int i = 0; i < particle_num; i++) {
        srand(time(0) + rand());
        particle[i].mass = rand() % (1000 - 900) + 900; //random number in [50, 100) 
        srand(time(0) + rand());
        particle[i].x_v = rand() % 20 - 10; //set initial speed to 0
        srand(time(0) + rand());
        particle[i].y_v = rand() % 20 - 10;
        particle[i].x_f = 0;  //set initial force to 0
        particle[i].y_f = 0;
        srand(time(0) + rand());
        particle[i].x_pos = rand() % (X_U - X_L) + X_L;  //random x coordination 
        srand(time(0) + rand());
        particle[i].y_pos = rand() % (Y_U - Y_L) + Y_L; //random y coordination 

    }
}


void update_force() {
    for (int i = 0; i < particle_num; i++) {
        
        particle[i].x_f = 0;
        particle[i].y_f = 0;

        for (int j = 0; j < particle_num; j++) {
            if (j != i) {

                double ax = particle[i].x_pos;
                double bx = particle[j].x_pos;
                double ay = particle[i].y_pos;
                double by = particle[j].y_pos;
                double am = particle[i].mass;
                double bm = particle[j].mass;

                if (ax != bx || ay != by) {// Distance between two particles
                    double r = sqrt(pow((ax - bx), 2) + pow((ay - by), 2)); 
                    
                    double f_x = G*am*bm*(bx-ax) / pow(r, 3);     
                    double f_y = G*am*bm*(by-ay) / pow(r, 3);  
                    
                    if (r > 5) {
                        particle[i].x_f += f_x;
                        particle[i].y_f += f_y;

                    }
                    else {
                        double temp = particle[i].x_v;
                        particle[i].x_v = -(bm/am) * particle[j].x_v;
                        particle[j].x_v = -(bm/am) * temp;
                        temp = particle[i].y_v;
                        particle[i].y_v = -(bm/am) * particle[j].y_v;
                        particle[j].y_v = -(bm/am) * temp;

                    }
                }
            }  
        }
    }
}


void update_position() {
    for (int i = 0; i < particle_num; i++) {

        double m = particle[i].mass;
        double a_x = particle[i].x_f / m;
        double a_y = particle[i].y_f / m;
        double init_v_x = particle[i].x_v;
        double init_v_y = particle[i].y_v;

        double move_x = init_v_x*timeInterval + 0.5*a_x*pow(timeInterval, 2);
        double move_y = init_v_y*timeInterval + 0.5*a_y*pow(timeInterval, 2);

        particle[i].x_pos += move_x;
        particle[i].y_pos += move_y;

        if (particle[i].x_pos < X_L_M || particle[i].x_pos > X_U_M) {
            particle[i].x_v = -particle[i].x_v;
        }

        if (particle[i].y_pos < Y_L_M || particle[i].y_pos > Y_U_M) {
            particle[i].y_v = -particle[i].y_v;
        }

    }
}

/* Global Xwindow Parameters */
Window win; //Initialize a window
unsigned int width, height, //Window size
                x, y, //Window position
                border_width, //Border width in pixels
                display_width, display_height, //Size of screen
                screen; //Which screen
char *window_name="N-body Simulation", *display_name=NULL;
bool isEnable=true; //Decides whether the Xwindow will be displayed or not

/* Create Graph */
GC gc;
XGCValues values;
Display *display;
XSizeHints size_hints;
Pixmap pm;
long valuemask=0;

void XWindow_Init(){

    /* connect to Xserver */
    if ((display = XOpenDisplay (display_name)) == NULL) {
        fprintf (stderr, "drawon: cannot connect to X server %s\n",
        XDisplayName (display_name));
        exit (-1);
    }
    
    /* get screen size */
    screen = DefaultScreen (display);
    display_width = DisplayWidth (display, screen);
    display_height = DisplayHeight (display, screen);

    /* set window size */
    width = X_RESN;
    height = Y_RESN;

    /* set window position */
    x = 10;
    y = 10;

    /* create opaque window */
    border_width = 1;
    win = XCreateSimpleWindow (display, RootWindow (display, screen),
                            x, y, width, height, border_width, 
                            BlackPixel (display, screen), WhitePixel (display, screen));


	XSelectInput(display,win,ExposureMask|KeyPressMask);
    /* map the window */
    XMapWindow (display, win);

    /* create graphics context */
    gc = DefaultGC(display,screen);
	pm = XCreatePixmap(display,win,X_RESN,Y_RESN,DefaultDepth(display,screen));
	XFillRectangle(display,pm,gc,0,0,X_RESN,Y_RESN);
	XSetForeground(display,gc,WhitePixel(display,screen));

}

void XFinish(){
    XFreePixmap(display,pm);
	XCloseDisplay(display);
}

int main(int argc, char **argv){
    double current_time=0;
    if(isEnable) XWindow_Init();

    initialization();

    while (current_time < compute_time){
        if (isEnable){
            XSetForeground(display,gc,0);
		    XFillRectangle(display,pm,gc,0,0,X_RESN,Y_RESN);
        }
        update_force();
        update_position();
		
        if (isEnable){
            XSetForeground(display,gc,WhitePixel (display, screen));

            for (int i = 0; i < particle_num; i++) {
                XDrawPoint (display, pm, gc, (int)particle[i].y_pos, (int)particle[i].x_pos);
            }
            XCopyArea(display,pm,win,gc,0,0,X_RESN,Y_RESN,0,0);
        } 
        current_time += timeInterval;
    } 

    if (isEnable) XFinish();
    return 0;
}