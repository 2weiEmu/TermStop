/*
* Thank you:
* https://stackoverflow.com/questions/1798511/how-to-avoid-pressing-enter-with-getchar-for-reading-a-single-character-only
*
*
*  TODO: add option to write Splits to file instead of stdout
*  TODO: add ability to name splits
*/

#define TIMER_SLEEP 90000


// LIBS
#include <stdio.h>
#include <termios.h> //termios, TCSANOW, ECHO, ICANON
#include <unistd.h> //STDIN_FILENO
#include <sys/time.h>
#include <pthread.h>

// VARS
enum COMMAND {
    SPLIT = '\n',
    START = 's',
    QUIT = 'q',
    HALT = 'h'
};



// global variable to share between the threads
int user_command = START;   
char format_time[16];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void* collect_user_input(void* vargp) {

    int c;

    while (user_command != HALT) {

	c = getchar();

	switch (c) {

	    case SPLIT:
		
		printf("\033[AThis is a split. Split info:\n\n");
	    case START: 
	    case QUIT:
	    case HALT:

		pthread_mutex_lock(&mutex);
		user_command = c;
		pthread_mutex_unlock(&mutex);

		break;

	    default:
		break;
	}
    }

    return NULL;

}

void format_timestamp(char format_time[16], suseconds_t diff_us) {
    // TODO:
    
    int milli = (diff_us / 10000) % 100;
    int sec = (diff_us / 1000000) % 60;
    int min = (diff_us / 1000000 / 60) % 60;
    int hour = ((diff_us / 1000000) / 60 / 60) % 60;

    sprintf(format_time, "%.2dh-%.2dm-%.2ds-%.2dms", hour, min, sec, milli);
    
}

// MAIN
int main(int argc, char** argv) {

    static struct termios oldt, newt;
    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    // removes ICANON so that terminal input is not buffered
    newt.c_lflag &= ~(ICANON | ECHO);  // makes sure also that ECHO does not imemdiately give me the char twice
    // set new attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // creating user input collection thread
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, collect_user_input, NULL);


    struct timeval before, intermediate;
    time_t diff_s;
    suseconds_t diff_us;

    gettimeofday(&before, NULL);


    // XXh-XXm-XXs-XXms 16 chars.


    while(user_command != HALT) {

	gettimeofday(&intermediate, NULL);
	diff_s = intermediate.tv_sec - before.tv_sec;
	diff_us = ((diff_s * 1000000) + intermediate.tv_usec) - before.tv_usec;

	pthread_mutex_lock(&mutex);
	format_timestamp(format_time, diff_us);
	pthread_mutex_unlock(&mutex);
	

	usleep(TIMER_SLEEP); 
	printf("\033[AStopwatch: %s\n", format_time);

	//printf("%c", c);
    }

    pthread_join(thread_id, NULL); // waiting for the input thread to finish

    printf("\n"); // purely for formatting
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt); // restore terminal settings

    return 0;
}
