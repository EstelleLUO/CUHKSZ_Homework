/* Parallel n-Body simulation using Pthread */
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define THREADNUM 4         /* number of threads*/
#define particle_num 500              /* number of particles */
#define G 6 //(6.67384*pow(10,-11)) //Gravity Constant
#define timeInterval 0.002        /* time slot of one movement */
#define compute_time 5000        /* total simulation step */
#define X_RESN 800            /* X resolution */
#define Y_RESN 800            /* Y resolution */
#define THETA 0.5 //Define the accuracy of the tree. If theta is 1, then is the same as Brute-Force Algorithm

/* Lower and uppter bound for paticle intial position */
int X_L = 3*X_RESN / X_RESN*100;     
int X_U = 5*X_RESN / X_RESN*100;    
int Y_L = 3*Y_RESN / Y_RESN*100;   
int Y_U = 5*Y_RESN / Y_RESN*100;     

/* Lower and upper bound for particle movement*/
int X_L_M = 2*X_RESN / X_RESN*100;   
int X_U_M = 6*X_RESN / X_RESN*100;   
int Y_L_M = 2*Y_RESN / Y_RESN*100;   
int Y_U_M = 6*Y_RESN / Y_RESN*100;  

/* 
* N-body Construction
* This holds information for a single particle,
* including position, velocity and mass.
*/
struct particle;
typedef struct particle{
  double x, y;
  double vx, vy;
  double fx, fy; //Gravitational forces that apply against this particle
  double m;
  int valid; //Mark if in the region of a quadtree has a particle or not
}particle_t; 


/* 
* Barnes-Hut Quadtree Construction
* This holds the information for the quad tree, 
* including number of particles in this nodes and its sub-nodes
* mass of the node (sum of its particles mass) and center of the mass
*/
enum direction{NW, NE, SW, SE};

struct node;
typedef struct node {
    double totalmass;
    double centerx, centery;
    double xmin, xmax;
    double ymin, ymax;
    double xmid, ymid;
    particle_t * children;
    struct node * NW;
    struct node * NE;
    struct node * SW;
    struct node * SE;
}node_t;

struct thread;
typedef struct thread{
    int id;
    node_t * root;
}thread_t;

enum direction Get_Children(double x, double y, double xmin, double xmax, double ymin, double ymax);
void Update_Center_Mass(node_t * nodep, particle_t * children);
node_t *Create_Node(particle_t * children, double xmin, double xmax, double ymin, double ymax);
void Insert_Body(particle_t * insbody, node_t * nodep);
void Calculate_force(node_t * nodep, particle_t * children);
void Destroy_Tree(node_t * nodep);

enum direction Get_Children(double x, double y, double xmin, double xmax, double ymin, double ymax) {
    double midx, midy;
    midx = 0.5*(xmin + xmax);
    midy = 0.5*(ymin + ymax);
    if (x < midx && y<midy) return NW;
    if (x < midx && y>=midy) return SW;
    if (x >=midx && y<midy) return NE;
    if (x >=midx && y>=midy) return SE;
}

void Update_Center_Mass(node_t * nodep, particle_t * children) {
    nodep->totalmass += children->m;
    nodep->centerx = (nodep->totalmass*nodep->centerx + children->m*children->x) / (nodep->totalmass + children->m);
    nodep->centery = (nodep->totalmass*nodep->centery + children->m*children->y) / (nodep->totalmass + children->m);
}

node_t *Create_Node(particle_t * children, double xmin, double xmax, double ymin, double ymax) {

    node_t* nodep;
    nodep = (node_t*)malloc(sizeof(node_t));
    
    nodep->totalmass = children->m;
    nodep->centerx = children->x;
    nodep->centery = children->y;
    nodep->xmin = xmin;
    nodep->xmax = xmax;
    nodep->ymin = ymin;
    nodep->ymax = ymax;
    nodep->xmid = 0.5*(xmin + xmax);
    nodep->ymid = 0.5*(ymin + ymax);
    nodep->children = children;
    nodep->NW = NULL;
    nodep->NE = NULL;
    nodep->SW = NULL;
    nodep->SE = NULL;
    
    return nodep;
}

void Insert_Body(particle_t * insbody, node_t * nodep) {
    
    enum direction direc;
    direc = Get_Children(insbody->x, insbody->y, nodep->xmin, nodep->xmax, nodep->ymin, nodep->ymax);
    Update_Center_Mass(nodep, insbody);

    if (nodep->children != NULL) {
        if (nodep->children->x== insbody->x && nodep->children->y == insbody->y) {
            nodep->children->m += insbody->m;
            nodep->children->vx = (nodep->children->m*nodep->children->vx + insbody->m*insbody->vx) / (nodep->children->m + insbody->m);
            nodep->children->vy = (nodep->children->m*nodep->children->vy + insbody->m*insbody->vy) / (nodep->children->m + insbody->m);
            insbody->valid = 0;
        } else {
            enum direction newdirec;
            newdirec = Get_Children(nodep->children->x, nodep->children->y, nodep->xmin, nodep->xmax, nodep->ymin, nodep->ymax);

            if (newdirec == NW) nodep->NW = Create_Node(nodep->children, nodep->xmin, nodep->xmid, nodep->ymin, nodep->ymid);
            if (newdirec == NE) nodep->NE = Create_Node(nodep->children, nodep->xmid, nodep->xmax, nodep->ymin, nodep->ymid);
            if (newdirec == SW) nodep->SW = Create_Node(nodep->children, nodep->xmin, nodep->xmid, nodep->ymid, nodep->ymax);
            if (newdirec == SE) nodep->SE = Create_Node(nodep->children, nodep->xmid, nodep->xmax, nodep->ymid, nodep->ymax);

            nodep->children = NULL;
        }
    } 

    if (nodep->children == NULL) {
        if (direc == NW) {
            if (nodep->NW == NULL) {
                nodep->NW = Create_Node(insbody, nodep->xmin, nodep->xmid, nodep->ymin, nodep->ymid);
            } else {
                Insert_Body(insbody, nodep->NW);
            }
        } else if (direc == NE) {
            if (nodep->NE == NULL) {
                nodep->NE = Create_Node(insbody, nodep->xmid, nodep->xmax, nodep->ymin, nodep->ymid);
            } else {
                Insert_Body(insbody, nodep->NE);
            }
        } else if (direc == SW) {
            if (nodep->SW == NULL) {
                nodep->SW = Create_Node(insbody, nodep->xmin, nodep->xmid, nodep->ymid, nodep->ymax);
            } else {
                Insert_Body(insbody, nodep->SW);
            }
        } else if (direc == SE) {
            if (nodep->SE == NULL) {
                nodep->SE = Create_Node(insbody, nodep->xmid, nodep->xmax, nodep->ymid, nodep->ymax);
            } else {
                Insert_Body(insbody, nodep->SE);
            }
        }
    }
}

void Calculate_force(node_t * nodep, particle_t * target) {
    double dx, dy, r, fx, fy, d;

    dx = nodep->centerx - target->x;
    dy = nodep->centery - target->y;
    r = sqrt(pow(dx, 2) + pow(dy, 2));
    d = nodep->xmax - nodep->xmin;

    if (d/r < THETA  && target!= nodep->children) {

        fx = G*nodep->totalmass*target->m*dx / pow(r+3, 3);
        fy = G*nodep->totalmass*target->m*dy / pow(r+3, 3);

        target->fx += fx;
        target->fy += fy;

    } else {
        if (nodep->NW != NULL) Calculate_force(nodep->NW, target);
        if (nodep->NE != NULL) Calculate_force(nodep->NE, target);
        if (nodep->SW != NULL) Calculate_force(nodep->SW, target);
        if (nodep->SE != NULL) Calculate_force(nodep->SE, target);
    }
}

void Destroy_Tree(node_t * nodep) {
    if (nodep != NULL) {
        if (nodep->NW != NULL) Destroy_Tree(nodep->NW);
        if (nodep->NE != NULL) Destroy_Tree(nodep->NE);
        if (nodep->SW != NULL) Destroy_Tree(nodep->SW);
        if (nodep->SE != NULL) Destroy_Tree(nodep->SE);
        free(nodep);
    }
}

particle_t * particle;
int result[compute_time][particle_num][3] = {0};

void initialization();
void XWindow_Init();
void XFinish();

/* Thread Function */
static void *update_force(void *thread) {
    thread_t *pid;
    pid = (thread_t *) thread;

    for (int i = 0; i < particle_num; i++) {
        if (particle[i].valid == 1) {
            if (pid->id == i % THREADNUM) Calculate_force(pid->root, particle+i);
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


int main(int argc, char const *argv[])
{
    int current_time = 0;
    int i, j;

    struct timespec start_time, end_time;

    initialization();

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    while (current_time < compute_time){
        pthread_t threads[THREADNUM];
        thread_t td[THREADNUM]; //Contains the thread information

        node_t * root;
        double xmin = X_U_M, xmax = X_L_M, ymin = Y_U_M, ymax = Y_L_M;

        for (i = 0; i < particle_num; i++) {
            if (particle[i].valid == 1) {
                particle[i].fx = 0;
                particle[i].fy = 0;
                if (xmin>particle[i].x) xmin = particle[i].x;
                if (xmax<particle[i].x) xmax = particle[i].x;
                if (ymin>particle[i].y) ymin = particle[i].y;
                if (ymax<particle[i].y) ymax = particle[i].y;
            }
        }

        root = Create_Node(particle, xmin, xmax, ymin, ymax);

        for (i = 1; i < particle_num; i++) {
            if (particle[i].valid == 1) Insert_Body(particle+i, root);
        }

        for (i = 0; i < THREADNUM; i++) {
            td[i].id = i;
            td[i].root = root;
            pthread_create(&threads[i], NULL, update_force, (void *)&td[i]);
        }

        for (i = 0; i < THREADNUM; i++) pthread_join(threads[i], NULL);

        for (i = 0; i < particle_num; i++) {
            if (particle[i].valid == 1) {
                particle[i].vx += timeInterval*particle[i].fx / particle[i].m;
                particle[i].vy += timeInterval*particle[i].fy / particle[i].m;

                particle[i].x += timeInterval*particle[i].vx;
                particle[i].y += timeInterval*particle[i].vy;

                if (particle[i].x < X_L_M || particle[i].x > X_U_M) {
                    particle[i].vx = -particle[i].vx;
                }

                if (particle[i].y < Y_L_M || particle[i].y > Y_U_M) {
                    particle[i].vy = -particle[i].vy;
                }
                
                result[current_time][i][0] = particle[i].valid;
                result[current_time][i][1] = (int)particle[i].x;
                result[current_time][i][2] = (int)particle[i].y;
            }
        }

        current_time += 1;
        Destroy_Tree(root);

    }; //End of while loop

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    printf("Execution Time is: %d\n", (end_time.tv_sec - start_time.tv_sec));

   if (isEnable) XWindow_Init();

    for (i = 0; i < compute_time; i++) {
        XSetForeground(display,gc,0);
        XFillRectangle(display,pm,gc,0,0,X_RESN,Y_RESN);
        XSetForeground(display,gc,WhitePixel (display, screen));
        for (j = 0; j <particle_num; j++) {
            if (result[i][j][0] == 1)
                XDrawPoint (display, pm, gc, result[i][j][2], result[i][j][1]);
        }
        XCopyArea(display,pm,win,gc,0,0,X_RESN,Y_RESN,0,0);
    }

    free(particle);
    return 0;
}


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


void initialization() {

    int i;
    particle = (particle_t*)malloc(particle_num*sizeof(particle_t));
    for (i = 0; i <particle_num; i++) {
        particle[i].vx = 0;
        particle[i].vy = 0;
        particle[i].fx = 0;
        particle[i].fy = 0;
        particle[i].valid = 1;

        srand(time(0) + rand());
        particle[i].m = rand() % (1000 - 900) + 900;

        srand(time(0) + rand());
        particle[i].x = rand() % (X_U - X_L) + X_L;

        srand(time(0) + rand());
        particle[i].y = rand() % (Y_U - Y_L) + Y_L;

    }
}