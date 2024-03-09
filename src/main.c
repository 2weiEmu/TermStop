#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

/*
* Thank you:
* https://stackoverflow.com/questions/1798511/how-to-avoid-pressing-enter-with-getchar-for-reading-a-single-character-only
*/

enum COMMAND 
{
    SPLIT = '\n',
    START = 's',
    QUIT = 'q',
    HALT = 'h'
};

const char* VERSION = "0.1.0-alpha";
const char* USAGE = "termstop [-a -s -q -v] [-t us-sleep] [-f save-file] [-d delimiter]";

int NAMING_FLAG = 0;		    // -a flag
int TIMER_SLEEP_US = 45000;	    // -t flag
int USE_FILE_FLAG = 0;		    // -f flag
char FILE_OUTPUT_PATH[256] = "";    // -f flag
char DELIMITER = ',';		    // -d flag
int SILENCE_FLAG = 0;		    // -s flag
int CSV_QUOTE_FLAG = 1;		    // -q flag
int SHOW_VERSION = 0;		    // -v flag


// global variable to share between the threads
static struct termios oldt, newt;

typedef struct state 
{
    int split_count;
    int user_command;
    char format_time[28];
    pthread_mutex_t mutex;

    FILE* splits_csv;
} 
state;

void change_user_command(enum COMMAND state, struct state* context) 
{
    pthread_mutex_lock(&context->mutex);
    context->user_command = state;
    pthread_mutex_unlock(&context->mutex);
}

void* collect_user_input(void* state) 
{
    struct state* context = state;

    int c;

    while (context->user_command != HALT) 
    {
	char user_split_name[200] = { 0 };
	char* saved_time;

	c = getchar();

	switch (c) 
	{
	    case SPLIT:

		saved_time = context->format_time;
		context->split_count++;

		if (NAMING_FLAG) 
		{
		    // temporarily reset terminal parameters, to the split may be named
		    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restore terminal settings
		    pthread_mutex_lock(&context->mutex);
		    
		    // TODO: should quote characters be allowed? they work, but is it a pain for the person using the csv?
		    printf("Name the new split which occurred at the following time:\n\n");
		    fgets(user_split_name, 199, stdin);
		    // remove the \n from the end, because it also scans that
		    strtok(user_split_name, "\n"); // slightly weird / unusual / unsafe seeming way but it works

		    // scanf would be used for formatting a string (scan formatted), use gets to get raw input, it goes until EOF or newline
		    // one of the answers here makes sense https://stackoverflow.com/questions/1247989/how-do-you-allow-spaces-to-be-entered-using-scanf
		    // use fgets to get the input and sscanf to _evaluate_ it. (like the post above says)
		    // sscanf(user_split_name, "%s", user_split_name);
		    
		    // set the terminal back to the required settings and unlock the thread

		    // writing the splits to the file
		    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
		    pthread_mutex_unlock(&context->mutex);
		} 
		else 
		{
		    sprintf(user_split_name, "split-number-%d", context->split_count);
		}

		if (USE_FILE_FLAG) 
		{
		    pthread_mutex_lock(&context->mutex);
		    if (CSV_QUOTE_FLAG) 
		    {
			fprintf(context->splits_csv, "\"%d\"%c \"%s\"%c \"%s\"\n", context->split_count, DELIMITER, saved_time, DELIMITER, user_split_name);
		    } 
		    else 
		    {
			fprintf(context->splits_csv, "%d%c %s%c %s\n", context->split_count, DELIMITER, saved_time, DELIMITER, user_split_name);
		    }
		    pthread_mutex_unlock(&context->mutex);
		}

		if (!SILENCE_FLAG) 
		{
		    printf("\033[Asplit\t\t%s\t\tname\t\t%s\t\tcount\t\t%d\n\n", saved_time, user_split_name, context->split_count);
		}
		change_user_command(c, context);

		break;
	    case HALT:
		change_user_command(c, context);
		break;

	    default:
		break;
	}
    }

    return NULL;

}

void format_timestamp(char format_time[28], suseconds_t diff_us) 
{
    short milli = (diff_us / 10000) % 100;
    short sec = (diff_us / 1000000) % 60;
    short min = (diff_us / 1000000 / 60) % 60;
    short hour = ((diff_us / 1000000) / 60 / 60) % 60;

    sprintf(format_time, "%.2dh.%.2dm.%.2ds.%.2dms", hour, min, sec, milli);
}

// MAIN
int main(int argc, char** argv) 
{
    // parsing the flags
    opterr = 0;
    int c;

    while ((c = getopt(argc, argv, "avt:f:d:sq")) != -1) 
    {
	switch (c) 
	{
	    case 'a':
		NAMING_FLAG = 1;
		break;
	    case 't':
		// TODO: add error check
		TIMER_SLEEP_US = atoi(optarg);
		break;
	    case 'f':
		USE_FILE_FLAG = 1;
		strncpy(FILE_OUTPUT_PATH, optarg, 255);
		break;
	    case 'd':
		DELIMITER = optarg[0];
		break;
	    case 's':
		SILENCE_FLAG = 1;
		break;
	    case 'q':
		CSV_QUOTE_FLAG = 0;
		break;
	    case 'v':
		SHOW_VERSION = 1;
		break;
		
	    case '?': // TODO: better errors needed
		if (optopt == 't')
		    printf("No option for the t flag was given. Please give a time in microseconds.\n");
		else if (optopt == 'f')
		    printf("No file to save to was specified.\n");
		else if (optopt == 'd')
		    printf("No new single-character delimiter was given.\n");
		else 
		    printf("Unknown option.\nUsage:\n%s\n", USAGE);
		return 1;
	    default:
		abort();

	}

    }

    if (SHOW_VERSION) 
    {
	printf("%s\n", VERSION);
	return 0;
    }

    #ifdef DEBUG
	printf("NAMING_FLAG: %d\nTIMER_SLEEP_US: %d\nUSE_FILE_FLAG: %d\nFILE_OUTPUT_PATH: %s\nDELIMETER: %c\nSILENCE_FLAG: %d\nCSV_QUOTE_FLAG: %d\n",
	   NAMING_FLAG, TIMER_SLEEP_US, USE_FILE_FLAG, FILE_OUTPUT_PATH, DELIMITER, SILENCE_FLAG, CSV_QUOTE_FLAG);
    #endif

    state context;
    pthread_mutex_init(&context.mutex, NULL);
    context.split_count = 0;
    context.user_command = START;

    // setting up the terminal
    tcgetattr(STDIN_FILENO, &oldt); // Get current terminal settings
    newt = oldt;
    // removes ICANON so that terminal input is not buffered
    newt.c_lflag &= ~(ICANON | ECHO);  // makes sure also that ECHO does not imemdiately give me the char twice
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // set new attributes
    
    if (USE_FILE_FLAG) 
	context.splits_csv = fopen(FILE_OUTPUT_PATH, "w"); 

    // printing the controls
    // TODO: sorry yea, for now hardcoding the printing of <ENTER> -> fix that
    printf("Controls: Split <ENTER> | Start: %c | Quit: %c | Halt: %c\n\n", START, QUIT, HALT);

    // creating user input collection thread
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, collect_user_input, &context);

    // creating timer settings
    struct timeval before, intermediate;
    time_t diff_s;
    suseconds_t diff_us;

    gettimeofday(&before, NULL);

    while (context.user_command != HALT) 
    {

	gettimeofday(&intermediate, NULL);
	diff_s = intermediate.tv_sec - before.tv_sec;
	diff_us = ((diff_s * 1000000) + intermediate.tv_usec) - before.tv_usec;

	pthread_mutex_lock(&context.mutex);
	format_timestamp(context.format_time, diff_us);
	pthread_mutex_unlock(&context.mutex);

	usleep(TIMER_SLEEP_US); 
	fprintf(stderr, "\033[Atime\t\t%s\n", context.format_time);

	//printf("%c", c);
    }

    pthread_join(thread_id, NULL); // waiting for the input thread to finish
    
    if (USE_FILE_FLAG)
	fclose(context.splits_csv);

    printf("\n"); // purely for formatting
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restore terminal settings

    return 0;
}
