/*

Name: Archit Jaiswal
Shell
This program is a shell program like bash shell or linux command line terminal

*/


#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
// so we need to define what delimits our tokens.
// In this case  white space
// will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11     // Mav shell only supports five arguments
#define MAX_HISTORY_CAPACITY 15  // It will only store last 15 commands in history
#define MAX_LISTPID_CAPACITY 15  // It will only allow 15 Child PID to store


/**
handle_signal function
parameters: int
return: void
It receives the input interger when "CTRL + C" or "CTRL + Z" is pressed my the user. It determines which signal is caught
and handles the interupt without stopping the shell.
This function is used from https://github.com/CSE3320/Code-Samples/blob/master/multiple_signals.c
*/
static void handle_signal (int sig )
{


  switch( sig )
  {
    case SIGINT:

    break;

    case SIGTSTP:

    break;

    default:

    break;

  }

}


/**
* main function
* input parameters: void
* return: int
* It maintains the continous execution of shell and prints command prompt as it executes the entered command
*/

int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  char history[MAX_HISTORY_CAPACITY][MAX_COMMAND_SIZE]; // Holds the recent 15 commands entered bby user
  char PIDlist[MAX_LISTPID_CAPACITY][MAX_COMMAND_SIZE]; // Holds the recent 15 processes created by the user
  int histIndex = 0; // counter of history array
  int PIDlistIndex = 0; // counter of PIDlist array

  // this innitialized all the cells of history and PIDlist array to '\0'
  int i;
  for( i = 0; i < MAX_HISTORY_CAPACITY; i++ )
  {
    bzero(history[i], sizeof(history[i]));
    bzero(PIDlist[i], sizeof(PIDlist[i]));
  }



  struct sigaction act; // sigaction examines and changes a signal action

  /*
  Zero out the sigaction struct
  */
  memset (&act, '\0', sizeof(act));

  /*
  Set the handler to use the function handle_signal()
  */
  act.sa_handler = &handle_signal;

  /*
  Install the handler for SIGINT and SIGTSTP and check the
  return value.
  */
  if (sigaction(SIGINT , &act, NULL) < 0)
  {
    perror ("sigaction: ");
    return 1;
  }

  if (sigaction(SIGTSTP , &act, NULL) < 0)
  {
    perror ("sigaction: ");
    return 1;
  }


  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );


    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;
    int childPID;
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;
    char *working_str;

    if(cmd_str[0] == '!') // check for "!n" command
    {
      // removing the "!" from command and leaving a string with just number and null character
      working_str  = strdup( cmd_str+1 );

      // input of atoi() is a string with null character at the entered
      // checking if the number entered by user is <=15 and the user have already typed "n" # of commands before
      if( atoi(working_str) <= 15 && history[atoi(working_str)-1][0] != '\0' )
      {
        strcpy(working_str, history[atoi(working_str)-1]);
      }
      else
      {
        printf( "Command not in history.\n" );
        continue;
      }
    }
    else // if the user did not entered "!n" command then process input in the regular manner
    {
      working_str  = strdup( cmd_str );
    }

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;


    // reset the pointers of history and PIDlist arrays to avoid segmentation fault
    if( histIndex > 14)
    {
      histIndex = 0;
    }
    if (PIDlistIndex > 14 )
    {
      PIDlistIndex = 0;
    }


    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
    (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );

      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }

      token_count++;
    }



    if(token[0] != '\0') // if the input is empty space then re-prompt for the input
    {
      strcpy( history[histIndex++], cmd_str ); // copy the command to "history" array

      int status;

      // first check if the command is exit
      if( (strcmp( token[0], "exit" ) == 0) || (strcmp( token[0], "quit") == 0) )
      {
        exit(0);
      }

      // checking for "cd" command
      else if ( strcmp( token[0], "cd" ) == 0 )
      {
        if( chdir( token[1] ) < 0 ) // if directory doen not exists then "chdir" will return -1
        {
          printf("%s : No such file or directory\n", token[1] );
        }
      }

      // history command
      else if ( strcmp(token[0], "history") == 0)
      {
        int i;

        for ( i = 0; i < MAX_HISTORY_CAPACITY; i++ )
        {
          // only prints the commands that were entered and prevents printing unused space in array
          if(history[i][0] != 0)
          {
            printf("%d  %s",i+1, history[i] );
          }
        }
      }


      // listpid command
      else if (strcmp( token[0], "listpids") == 0)
      {
        int i;

        for ( i = 0; i < MAX_LISTPID_CAPACITY; i++ )
        {
          // only prints the commands that were entered and prevents printing unused space in array
          if(PIDlist[i][0] != 0)
          {
            printf("%d  %s\n",i, PIDlist[i] );
          }
        }
      }

      // bg command, it continues the commands suspended by "CTRL +Z"
      else if ( strcmp( token[0], "bg" ) == 0 )
      {
        kill( childPID, SIGCONT);
      }

      // this creates the new child process and executes it
      else
      {
        // fork initiales another copy of ongoing process and both processes then runs simultaneously
        // from the next line where they were created
        childPID = fork();
        int retExecvp;

        if ( childPID == 0 ) // new command needs to be exected in the child process
        {
          // execvp(): executes the command if it is valid otherwise returns -1
          // It automatically finds the path of command for execution
          // input parameters: first input = command only, second input = enter al other attributes including the command
          retExecvp =  execvp( token[0], token);

          if (retExecvp < 0) // if the user enterns invalid command
          {
            printf("%s : Command not found\n", token[0] );
            exit(0);
          }

        }
        // parent proces will enter here to record the childPID and store it in PIDlist array
        else
        {
          sprintf(PIDlist[PIDlistIndex++], "%d", childPID); // converting the int to string
          waitpid ( childPID, &status, 0 ); // holding the parent process till the child process is done executing
        }

      }

    }

    free( working_root );
  }

  free(cmd_str);

  return 0;
}
