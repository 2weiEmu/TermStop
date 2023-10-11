#include <stdio.h>
#include <termios.h> //termios, TCSANOW, ECHO, ICANON
#include <unistd.h> //STDIN_FILENO
#include <sys/time.h>


enum COMMAND {
	SPLIT = '\n',
	START = 's',
	QUIT = 'q',
	HALT = 'h'
};

int main(int argc, char** argv) {

	
	// Key bindings: [ENTER] = split. S = start, Q = quit, H = Stop / Halt
	// Thank you to the good man on stack overflow:
	// https://stackoverflow.com/questions/1798511/how-to-avoid-pressing-enter-with-getchar-for-reading-a-single-character-only
	// I am still new to this, so I kinda have to find out how this works

	// TODO: add option to write Splits to file instead of stdout
	// TODO: add ability to name splits
	
	// Get keyboard input stream from the user
    int c = START;   
    static struct termios oldt, newt;

	// Get current terminal settings
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;

	// removes ICANON so that terminal input is not buffered
	newt.c_lflag &= ~(ICANON | ECHO);  // makes sure also that ECHO does not imemdiately give me the char twice

	// set new attributes
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

	int msec_since;


	struct timeval before, t, end;
	time_t diff;

	gettimeofday(&before, NULL);


    while(c != HALT) {

		c = getchar();

		gettimeofday(&t, NULL);

		diff = t.tv_sec - before.tv_sec;
		printf("diff: %ld", diff);


		printf("time: %d\n", msec_since);
        //printf("%c", c);
	}

	printf("\n"); // purely for formatting

	// restore settings
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);


    return 0;
}
