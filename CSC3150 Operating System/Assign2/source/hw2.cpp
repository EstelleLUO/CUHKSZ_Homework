#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <iostream>

#define ROW 10
#define COLUMN 50 
#define NUM_THREAD 10
using namespace std;
/* Function Declaration */

struct Node{
	int x , y; 
	Node( int _x , int _y ) : x( _x ) , y( _y ) {}; 
	Node(){} ; 
} frog ; 

/* Global */
char map[ROW+10][COLUMN] ; 
pthread_mutex_t map_mutex; // thread mutex  
int logs[ROW+10];
int speed[ROW+10]; 
bool isRunning = true;
bool isLose = false; 
bool isWin = false;  
string set_up="a";
int logs_length = 15; 
int logs_speed =0;
int moving_dir =0; //If moving_dir == 0, then even rows moving rightward; vice versa

// Determine a keyboard is hit or not. If yes, return 1. If not, return 0. 
int kbhit(void){
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);

	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);

	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);

	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}


void *logs_move( void *t ){
	int pid = *((int*)&t);

	/*  Move the logs  */
    while (isRunning){
		pthread_mutex_lock(&map_mutex);
        // To let the logs in each row moving leftward or rightward
        if (pid%2==moving_dir){
            logs[pid] +=1;
        } 
        else {
            logs[pid] -=1;
            if (logs[pid] <0) logs[pid]+=COLUMN;
        } 

        for (int j=0; j<COLUMN-1; j++ ) map[pid][j] = ' ' ;

        int position=logs[pid];   	
		for(int i = 0; i < logs_length; i++){
			map[pid][position % (COLUMN - 1) ] = '=' ; 
            position++;
		}
        /*  Check keyboard hits, to change frog's position or quit the game. */
        if (kbhit()){
            char dir=getchar();
			if (dir == 'w' || dir == 'W') frog.x-- ;
			if (dir == 'a' || dir == 'A') frog.y-- ;
			if (dir == 'd' || dir == 'D') frog.y++ ;
			if (dir == 's' || dir == 'S'){	
				if (frog.x < ROW) frog.x++ ;  
			}
			if (dir == 'q' || dir == 'Q')	isRunning = false ;
		}

        /*  Check game's status  */
        //If the frog falls into the river
		map[pid][COLUMN-1] = 0;   
        if (map[frog.x][frog.y] == ' '){	
			isRunning = false ; 
			isLose = true ; 
		}
		else if (map[frog.x][frog.y] ==0){	
			isRunning = false;
			isLose = true;  
		}

		else if (frog.y<0){	
			isRunning = false;
			isLose = true;  
		}

		else if(frog.x == 0){
			isRunning = false ;
			isWin = true ; 
        }
        

        /*  Print the map on the screen  */
        if (isRunning){
            //Let the frog moves with the log
            if (frog.x==pid && map[frog.x][frog.y]=='='){
				if (frog.x % 2 == moving_dir) frog.y++ ; 
				else frog.y-- ; 	
			}

            printf("\033[0;0H\033[2J");		
			usleep(1000);

            //Print out the boundary
            for (int j=0;j<COLUMN-1;j++) map[ROW][j] = map[0][j] = '|';

            //Print out the frog
            map[frog.x][frog.y] = '0';  
            //Print the map row by row
			for(int i = 0; i <= ROW; i++)	puts(map[i]);
        }
        pthread_mutex_unlock( &map_mutex ) ;
		usleep(speed[pid] * 1000);
    }
    pthread_exit(NULL) ; 
}

int main( int argc, char *argv[] ){
	//Direct the users to set up the game
	cout << "------------------------------------------------------------"<<endl;
	cout << "The following steps can help you to initilize the game!" << endl;
	cout << "------------------------------------------------------------"<<endl;
	cout << "Type skip if you wish to skip the process otherwise type other characters to continue setting up (if you type any input other than offered options, it will skip the set up process): ";
	cin >> set_up;
	if (set_up != "skip"){
		cout << "------------------------------------------------------------"<<endl;
		cout <<"The default value for logs' length is 15. "<<endl;
		cout << "Please enter the length <50 you wish the logs have (the inputs that are not int will be regarded as default): ";
		cin >> logs_length;
		if (cin.fail() || logs_length>=50) logs_length = 15;

		cout << "------------------------------------------------------------"<<endl;
		cout << "The default value for logs' speed is random."<<endl;
		cout << "Pleaes enter the speed you wish the logs have (the inputs that are not int will be regarded as random, type 0 if you wish to be random): ";
		cin >> logs_speed;
		if (cin.fail()) logs_speed=0;

		cout << "------------------------------------------------------------"<<endl;
		cout <<"The default moving direction for the logs on even rows is to the right while odd rows is to the left." << endl;
		cout << "Please enter the direction you wish the logs have (0 means unchange, while 1 means change the direction of even rows and odd rows, other input will be regarded as default): "<<endl;
		cin >> moving_dir;
		if (cin.fail() || (moving_dir!=0||1)) moving_dir=0;
	}


	// Initialize the river map and frog's starting position
	memset( map , 0, sizeof( map ) ) ;

	for(int i = 1; i < ROW; ++i ){	
		for(int j = 0; j < COLUMN - 1; ++j ) map[i][j] = ' ' ;  
		if (logs_speed == 0) speed[i] = rand()%100 + 30;
		else speed[i] =logs_speed;
	}	

	for(int j = 0; j < COLUMN - 1; ++j ) map[ROW][j] = map[0][j] = '|' ;


	frog = Node( ROW, (COLUMN-1) / 2 ) ; 
	map[frog.x][frog.y] = '0' ; 

	//Print the map into screen
	for(int i = 0; i <= ROW; ++i) puts(map[i]);


	/*  Create pthreads for wood move and frog control.  */
    pthread_t threads[NUM_THREAD];
    pthread_mutex_init( &map_mutex, NULL );

    for (int i=1;i<NUM_THREAD;i++) pthread_create(&threads[i], NULL, logs_move, (void*)i); 
    for (int i=1;i<NUM_THREAD;i++) pthread_join(threads[i],NULL); 
    printf("\033[0;0H\033[2J");
	
	/*  Display the output for user: win, lose or quit.  */
    usleep(1000);
    if (isLose) puts("You lose the game!!");
	else if (isWin) puts("You win the game!!");
	else puts("You exit the game.");

    pthread_mutex_destroy(&map_mutex);
    pthread_exit(NULL);
	return 0;

}
