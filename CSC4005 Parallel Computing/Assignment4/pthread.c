#include "const.h"
#include "models.h"
#include "display.h"
#include <pthread.h>
#include <stdio.h>

#define legal(x, n) ( (x)>=0 && (x)<(n) )
#define THREAD_NUM 		4
#define WIDTH 			100
#define HEIGHT 			100
#define EPSILON 		1
#define MAX_ITERATION 	1000
#define SCALE 			1.5
#define SCALE_TIME 		8

#define isEnable 1 //1 indicates showing the window while 0 is not

int iteration;
TemperatureField *field;
TemperatureField *midField;

/* Global Variables for Pthread*/
pthread_t threadPool[THREAD_NUM];
pthread_mutex_t subThreadStart[THREAD_NUM],subThreadEnd[THREAD_NUM];
int pid[THREAD_NUM];
double error[THREAD_NUM];

int dx[4] = {0, -1, 0, 1};
int dy[4] = {1, 0, -1, 0};

int x, y, iter_counter;

void* thread_func(void* data)
{   
    int pid = *((int*)data);

	pthread_mutex_init(&subThreadStart[pid], NULL);
	pthread_mutex_init(&subThreadEnd[pid], NULL);
	pthread_mutex_lock(&subThreadStart[pid]);
	pthread_mutex_lock(&subThreadEnd[pid]);

    while (True)
    {
	    pthread_mutex_lock(&subThreadStart[pid]);
		/* Work distribution*/
		int blockSize, lineStart, lineEnd;
		if (pid != THREAD_NUM-1){
			/* Task assignment for the pthreads except the last one*/
			blockSize = (field->x-field->x%THREAD_NUM)/THREAD_NUM;
			lineStart = blockSize*pid;
			lineEnd = blockSize*(pid+1);
		}
		else {
			/* Task assignment for the pthreads except the last thread*/
			blockSize = field->x/THREAD_NUM + (field->x%THREAD_NUM);
			lineStart = field->x-blockSize;
			lineEnd = field->x;
		}

		error[pid]=0;

	    int i, j, d;
	    for (i=lineStart; i<lineEnd; i++){
			for (j=0; j<field->y; j++){
				midField->t[i][j] = 0;
				for (d=0; d<4; d++)
					if ( legal(i+dx[d], field->x) && legal(j+dy[d], field->y) )
						midField->t[i][j] += field->t[i+dx[d]][j+dy[d]];
					else midField->t[i][j] += ROOM_TEMP; //For the points on the corners
				midField->t[i][j] /= 4;
				/* Calculate the change in the whole field except the walls*/
				if (i!=0 ||i!=field->x-1 || j!=0 || j!=field->y-1) 
					error[pid] += fabs(midField->t[i][j] - field->t[i][j]);
			}
		}
	    pthread_mutex_unlock(&subThreadEnd[pid]);
    }
    pthread_exit(NULL);
}

void temperature_iterate()
{
	iter_counter++;
	/* Redraw the field with new Fireplace position*/
	refreshField(field, 0, 0, field->x, field->y, field->x, field->y);
	int i;
	for (i=0; i<THREAD_NUM; i++){
		pthread_mutex_unlock(&subThreadStart[i]);
		pthread_mutex_lock(&subThreadEnd[i]);
	}
}

int main(int argc, char **argv)
{
    struct timespec start, finish;
    clock_gettime(CLOCK_MONOTONIC, &start);

	x =WIDTH;
	y = HEIGHT;
	iteration = MAX_ITERATION;

	/* Field Initialization*/
    field = malloc(sizeof(TemperatureField));
    midField = malloc(sizeof(TemperatureField));
    field->x = x;
    field->y = y;


	/* Xwindow Initialization*/
    if (isEnable)XWindow_Init(field);

	int i;
    for (i=0; i<THREAD_NUM; ++i)
    {
		pid[i] = i;
		pthread_create(&threadPool[i], NULL, thread_func, &pid[i]);
    }

    int iter, incre_counter;
	int X_Size[SCALE_TIME],Y_Size[SCALE_TIME];
	
	/* Final size of rescaling*/
    X_Size[SCALE_TIME-1] = x;
    Y_Size[SCALE_TIME-1] = y;

	/* Downsize the map*/
    for (incre_counter=SCALE_TIME-2; incre_counter>=0; incre_counter--)
    {
		X_Size[incre_counter] = X_Size[incre_counter+1] / SCALE;
		Y_Size[incre_counter] = Y_Size[incre_counter+1] / SCALE;
    }

	/* Draw the map from the smallest one*/
    for (incre_counter=0; incre_counter<SCALE_TIME; incre_counter++){
		/* Initialize every map*/
		int startPos_X = (incre_counter>0)?X_Size[incre_counter-1]:0;
		int startPos_Y = (incre_counter>0)?Y_Size[incre_counter-1]:0;
		newField(field, X_Size[incre_counter], Y_Size[incre_counter], startPos_X, startPos_Y);
		newField(midField, X_Size[incre_counter], Y_Size[incre_counter], startPos_X, startPos_Y);	  
		
		/* For the first one*/
		if (incre_counter ==0) initField(field);
		if (isEnable) XResize(field);

		for (iter=0; iter<iteration; iter++){
			temperature_iterate();
			double sumError;
			int i;
			for (i=0;i<THREAD_NUM;i++) sumError +=error[i];
			if (sumError<EPSILON) break;

			TemperatureField *tempField;
			tempField = field;
			field = midField;
			midField = tempField;

			if (isEnable){
				clock_gettime(CLOCK_MONOTONIC, &finish);
				/* Refresh the map */
				if ((long long)(finish.tv_sec-start.tv_sec)*1000000000 + finish.tv_nsec - start.tv_nsec > FRAME_INTERVAL*1000000)
				{
					clock_gettime(CLOCK_MONOTONIC, &start);
					XRedraw(field);
				}
			}
			
		}
    }

	if (isEnable){
		XRedraw(field);
		usleep(3000000);
	}

    deleteField(field);
    deleteField(midField);

    printf("Finished in %d iterations.\n", iter_counter);
    clock_gettime(CLOCK_MONOTONIC, &finish);
    printf("The execution time is: %lf\n", (double)(finish.tv_sec-start.tv_sec) + (double)(finish.tv_nsec - start.tv_nsec)/1000000000);
    pthread_exit(NULL);
    return 0;
}
