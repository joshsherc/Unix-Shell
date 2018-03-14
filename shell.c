#include <sys/wait.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>


#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 10

//History Struct to store all important history data
struct history {
	char historyArray[HISTORY_DEPTH][COMMAND_LENGTH];
	int sizeOfHistory;
	int totalCommandsExecuted;
};

struct history historyStruct;
void pwdCommand();
void printHistory();
void addHistoryCommand(char *command);
void executeHistoryNumber(int command);
void signalHandleFunction();
int read_command(char *buff, char *tokens[], _Bool *in_background);
void executeCommand(_Bool in_background, char *tokens[]);
int processToken(char* buff, char* tokens[], _Bool* in_background);
int tokenize_command(char *buff, char *tokens[]);
void cleanExitedProcesses();

int main(int argc, char* argv[])
{

	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS] = { 0 };

	//Signal Handler Setup and history struct
	historyStruct.sizeOfHistory = 0;
	historyStruct.totalCommandsExecuted = 0;

	struct sigaction signalHandler;
	signalHandler.sa_handler = signalHandleFunction;
	sigemptyset(&signalHandler.sa_mask);
	sigaction(SIGINT, &signalHandler, NULL);

	_Bool in_background = false;

	/* Start the read command -> execute command loop*/
	while (true)
	{
		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().
		//Call pwd every loop, to display the current directory
		pwdCommand();
		write(STDOUT_FILENO, "> ", strlen("> "));
		if(read_command(input_buffer, tokens, &in_background))
		{
            executeCommand(in_background, tokens);
        }


		//reset the array
		int i = 0;
		for (i = 0; i < NUM_TOKENS; i++)
		{
			if (tokens[i] == '\0')
			{
				break;
			}
			tokens[i] = '\0';
		}
	}
	return 0;
}

//HISTORY COMMANDS
//HISTORY COMMANDS
//HISTORY COMMANDS
//HISTORY COMMANDS
//HISTORY COMMANDS
//HISTORY COMMANDS
//HISTORY COMMANDS
//HISTORY COMMANDS
//HISTORY COMMANDS
//HISTORY COMMANDS

void addHistoryCommand(char *command)
{
    //history array is currently full
	if (historyStruct.sizeOfHistory == HISTORY_DEPTH)
	{
		for (int i = 1; i < HISTORY_DEPTH; i++)
		{
			strcpy(historyStruct.historyArray[i-1], historyStruct.historyArray[i]);
		}
		strcpy(historyStruct.historyArray[HISTORY_DEPTH-1], command);
	}
	else
	{
		//historyArray array isn't full
		strcpy(historyStruct.historyArray[historyStruct.sizeOfHistory], command);
		historyStruct.sizeOfHistory++;
	}
	historyStruct.totalCommandsExecuted++;
}

void printHistory()
{
	char buffer[100];
	int i = 0;
	int currentCommand = historyStruct.totalCommandsExecuted - historyStruct.sizeOfHistory +1;
	for (i = 0; i < historyStruct.sizeOfHistory; i++)
	{
		sprintf(buffer, "%d", currentCommand + i );
		strcat(buffer, "\t");
		write(STDOUT_FILENO, buffer, strlen(buffer));
		write(STDOUT_FILENO, historyStruct.historyArray[i], strlen(historyStruct.historyArray[i]));
		write(STDOUT_FILENO, "\n", strlen("\n"));
	}
}


void executeHistoryNumber(int command)
{
	//History is under 10
	if (historyStruct.sizeOfHistory < HISTORY_DEPTH)
	{
			char commandBuffer[COMMAND_LENGTH];
			strcpy(commandBuffer, historyStruct.historyArray[command -1]);
			char *tokens[NUM_TOKENS];
			_Bool in_background = false;
			write(STDOUT_FILENO, commandBuffer, strlen(commandBuffer));
			write(STDOUT_FILENO, "\n", strlen("\n"));
			processToken(commandBuffer, tokens, &in_background);
			executeCommand(in_background, tokens);
	}
	//History is greater than 10
	else
	{
		if (historyStruct.totalCommandsExecuted - command >= HISTORY_DEPTH)
		{
			write(STDOUT_FILENO, "Invalid command number\n", strlen("Invalid command number\n"));
			return;
		}
		else
		{
			char commandBuffer[COMMAND_LENGTH];
			strcpy(commandBuffer, historyStruct.historyArray[HISTORY_DEPTH - (historyStruct.totalCommandsExecuted - command + 1)]);
			char *tokens[NUM_TOKENS];
			_Bool in_background = false;
			write(STDOUT_FILENO, commandBuffer, strlen(commandBuffer));
			write(STDOUT_FILENO, "\n", strlen("\n"));
            processToken(commandBuffer, tokens, &in_background);
			executeCommand(in_background, tokens);
		}
	}
}


//NON HISTORY FUNCTIONS
//NON HISTORY FUNCTIONS
//NON HISTORY FUNCTIONS
//NON HISTORY FUNCTIONS
//NON HISTORY FUNCTIONS
//NON HISTORY FUNCTIONS
//NON HISTORY FUNCTIONS
//NON HISTORY FUNCTIONS
//NON HISTORY FUNCTIONS
//NON HISTORY FUNCTIONS

/* Signal handler function */
void signalHandleFunction()
{
	write(STDOUT_FILENO, "\n", strlen("\n"));
	printHistory();
	//historyStruct.sizeOfHistory--;
	//historyStruct.totalCommandsExecuted--;

}

//Read command function, as taken from the CMPT-300 Website. REturns 0 if unsuccessful, 1 if successful.
int read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

	if ( (length < 0) && (errno !=EINTR) ){
		perror( "Unable to read command. Terminating.\n ");
		exit(-1);
	}
//
	// Null terminate input and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n')
	 {
		buff[strlen(buff) - 1] = '\0';
	}
    if(buff[0] == '\0')
	{
        return 0;
	}

    processToken(buff, tokens, in_background);
    return 1;
}

//Executes the processed command
void executeCommand(_Bool in_background, char *tokens[])
{
	/*Check if is an internal command*/
	//check if pwd command
	if(strcmp(tokens[0], "pwd") == 0)
	{
		pwdCommand();
	}
	//check if !! command
	else if (strcmp(tokens[0], "!!") == 0)
	{
        if(historyStruct.totalCommandsExecuted == 0)
        {
            write(STDOUT_FILENO, "There are no previous commands to execute!\n", strlen("There are no previous commands to execute!\n"));
            return;
        }
		executeHistoryNumber(historyStruct.totalCommandsExecuted);
	}
	//check if history command
	else if (strcmp(tokens[0], "history") == 0)
	{
		printHistory();
	}
	//Check if cd command
	else if (strcmp(tokens[0], "cd") == 0)
	{
		//Check to see if chdir returns a -1
		if (chdir(tokens[1]) == -1)
		{
			write(STDOUT_FILENO, "ERROR: chdir returned a negative value", strlen("ERROR: chdir returned a negative value\n"));
		}
	}
	//Check if empty command
	else if (strcmp(tokens[0], "\0") == 0)
	{
        return;
	}
	//check if exit command
	else if(strcmp(tokens[0], "exit") == 0)
	{
		exit(0);
	}
	//check if ! command
	else if (tokens[0][0] == '!')
	{
		//Check to see if the nth command actually exists
		double command = strtod(&tokens[0][1], NULL);
		if(command == 0 || command != (int)command)
		{
			write(STDOUT_FILENO, "Command does not exist!\n" , strlen("Command does not exist!\n"));
			return;
		}
		else if ((int)command < 1 || command > historyStruct.totalCommandsExecuted)
		{
			write(STDOUT_FILENO, "Command number is greater than the total commands previously executed!\n", strlen("Command number is greater than the total commands previously executed!\n"));
			return;
		}
		else
		{
			executeHistoryNumber(command);
		}
	}

	/*Is not an internal command, create a child process to execute it*/
	else
	{
		pid_t pid = fork();
		//Child
		if (pid == 0) {
			if (execvp(tokens[0], tokens) == -1)
			{
				write(STDOUT_FILENO, strerror(errno), strlen(strerror(errno)));
				write(STDOUT_FILENO, "\n", strlen("\n"));
				exit(0);
			}
		}
		if (pid < 0)
		{
			write(STDOUT_FILENO, "Child could not fork\n" , strlen("Child could not fork\n"));
			return;
		}

		//parent
		if (!in_background)
		{
			if (waitpid(pid, NULL, 0) == -1)
			{
				write(STDOUT_FILENO, "Child could not exit\n" , strlen("Child could not exit\n"));
			}
		}
		else
		{	//wait for the child process to finish
			nanosleep((struct timespec[]){{0,50000000}}, NULL);
		}
		//Clean any zombie processes
		cleanExitedProcesses();
	}
}

//Executes the pwd command, need a seperate function because this is run in the main as well as other places.
void pwdCommand()
{
	char directoryBuffer[300];
	if (!getcwd(directoryBuffer, 300))
	{
		write(STDOUT_FILENO, "Cannot find the working directory\n" , strlen("Cannot find the working directory\n"));
	}
	else
	{
		write(STDOUT_FILENO, directoryBuffer, strlen(directoryBuffer));
	}
}


//Processes the token passed into the function via buff, and stores the processed tokens into tokens[]
//Returns a 1 if successful, returns a 0 if unsuccessful
int processToken(char* buff, char* tokens[], _Bool* in_background)
{
	if (buff[0] != '!' && buff[0] != '\0')
	{
		addHistoryCommand(buff);
	}
	int amountOfTokens = tokenize_command(buff, tokens);
	//Make sure tokenize_command doesnt return 0
	if (amountOfTokens == 0)
	{
		return 0;
	}
	//Set the in_background back to true, assuming token_count was successful
	if (amountOfTokens > 0 && strcmp(tokens[amountOfTokens - 1], "&") == 0)
	{
		*in_background = true;
		tokens[amountOfTokens - 1] = 0;
	}
	return 1;
}

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int tokenize_command(char *buff, char *tokens[])
{
	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);
	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}


// Cleanup any previously exited background child processes
// (The zombies)
void cleanExitedProcesses()
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
	// do nothing.
}


