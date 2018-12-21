#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <ncurses.h>


int main(int argc, char *argv[]){
	char ch;
	printf("enter a char: ");
	ch=getch();
	// printf("The entered char is: %c",ch);
	return 0;
}

