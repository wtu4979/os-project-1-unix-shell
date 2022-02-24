#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


#define MAX_CMD_LINE_ARGS  128
#define BUFFER_SIZE 25
#define READ_END	0
#define WRITE_END	1


int min(int a, int b) { return a < b ? a : b; }
// break a string into its tokens, putting a \0 between each token
//   save the beginning of each string in a string of char *'s (ptrs to chars)
int parse(char* s, char* argv[]) {
  const char break_chars[] = " \t;";
  char* p;
  int c = 0;

  /* TODO */    // write parser that breaks input into argv[] structure
  // e.g., cal  -h  2022\0      // would be
  //       |    |   |
  //       |  \0| \0|   \0      // '\0' where all the spaces are
  //       p0   p1  p2          // array of ptrs to begin. of strings ("cal", "-h","2022")
  //                            // char* argv[]


    while(*s != '\0'){
        while(*s == ' ' || *s == '\t'){
            *s++ = '\0';
        }
        p = s;
        *argv++ = p;

        while(*s != '\0' && *s != ' ' && *s != '\t' && *s != '\n'){
            s++;
        }

        c++;

    }
    *argv++ = NULL;

        printf("%s\n", argv[0]);
        
    // }


  return c;   // int argc
}
// execute a single shell command, with its command line arguments
//     the shell command should start with the name of the command
int execute(char* input) {
  int i = 0;
  bool ampersand = false;
  bool in_command = false;
  bool out_command = false;
  bool pipe_command = false;
  int in_index = 0;
  int out_index = 0;
  int ampersand_index = 0;
  int pipe_index = 0;
  int fd[2];
  int file;

  char* shell_argv[MAX_CMD_LINE_ARGS];
  char* command_one[MAX_CMD_LINE_ARGS];
  char* command_two[MAX_CMD_LINE_ARGS];

  memset(shell_argv, 0, MAX_CMD_LINE_ARGS * sizeof(char));
  
  int shell_argc = parse(input, shell_argv);
  printf("after parse, what is input: %s\n", input);      // check parser
  printf("argc is: %d\n", shell_argc);
  while (shell_argc > 0) {
    printf("argc: %d: %s\n", i, shell_argv[i]);
    // Sets ampersand to true
    if(*shell_argv[i] == '&'){
        ampersand = true;
        ampersand_index = i;
        
    }
    if (*shell_argv[i] == '>'){
      out_index = i + 1;
      out_command = true;

    }
    if(*shell_argv[i] == '<'){
      in_index = i + 1;
      printf("in_index found \n");
      printf("in_index is: %d\n", in_index);  
      in_command = true; 

    } 
    if(*shell_argv[i] == '|'){
      pipe_index = i + 1;
      printf("pipe command found \n");
      pipe_command = true;
    }

    if(!pipe_command){
      command_one[i] = shell_argv[i];
      printf("command one %s \n", command_one[i]);
      
    }
    else{
      if(*shell_argv[i] != '|'){
        command_two[i] = shell_argv[i];
      }
    }
    --shell_argc;
    ++i;
  }

  command_one[i] = NULL;
  command_two[i] = NULL;

  if(pipe_index > 0){
    pipe(fd);
    printf("pipe created \n");
  }
  
  int status = 0;
  pid_t pid = fork();
  
  if (pid < 0) { fprintf(stderr, "Fork() failed\n"); }  // send to stderr
  else if (pid == 0) { // child
    int ret = 0;
    // if ((ret = execlp("cal", "cal", NULL)) < 0) {  // can do it arg by arg, ending in NULL
    // if ((ret = execlp(*shell_argv, *shell_argv, "<", shell_argv)) < 0){
    //   printf("input found\n");
    // }


    if(out_command){
      printf("out_index is: %d\n", out_index);   

      file = open(shell_argv[out_index], O_WRONLY | O_CREAT, 0644);
      dup2(file, STDOUT_FILENO);
      shell_argv[out_index - 1] = NULL;
      shell_argv[out_index] = NULL;

    }

    if(in_command){
      printf("in_index is: %d\n", in_index);   

      file = open(shell_argv[in_index], O_RDONLY);
      dup2(file, STDIN_FILENO);
      shell_argv[in_index - 1] = NULL;
      shell_argv[in_index] = NULL;
    }

    if(ampersand) {
      printf("ampersand found \n");
      // parent and child are both running concurrently i guess
      // After running, sets ampersand back to false for next command
      shell_argv[ampersand_index] = NULL;
      ampersand = false;
    }

    if(pipe_command){
      int fd1[2];

    }

    if ((ret = execvp(*shell_argv, shell_argv)) < 0) {
      fprintf(stderr, "execlp(%s) failed with error code: %d\n", *shell_argv, ret);
    }
    printf("\n");



  }
  else { // parent -----  don't wait if you are creating a daemon (background) process
        if(ampersand){
          wait(NULL);
        }
        while (wait(&status) != pid) {
        }
  }
  
  return 0;
}
int main(int argc, const char * argv[]) {
  char input[BUFSIZ];
  char last_input[BUFSIZ];  
  bool finished = false;
  memset(last_input, 0, BUFSIZ * sizeof(char));  
  while (!finished) {
    memset(input, 0, BUFSIZ * sizeof(char));
    printf("osh > ");
    fflush(stdout);
    if (strlen(input) > 0) {
      strncpy(last_input, input, min(strlen(input), BUFSIZ));
      memset(last_input, 0, BUFSIZ * sizeof(char));
    }
    if ((fgets(input, BUFSIZ, stdin)) == NULL) {   // or gets(input, BUFSIZ);
      fprintf(stderr, "no command entered\n");
      exit(1);
    }
    input[strlen(input) - 1] = '\0';          // wipe out newline at end of string
    // printf("input was: '%s'\n", input);
    // printf("last_input was: '%s'\n", last_input);
    if (strncmp(input, "exit", 4) == 0) {   // only compare first 4 letters
      finished = true;
    } else if (strncmp(input, "!!", 2) == 0) { // check for history command
      // TODO
        if (strlen(last_input) > 0) {
            execute(last_input);
        }
        else{
            printf("No commands in history\n");
        }

    } else { 
        // save input to last_input
        strncpy(last_input, input, min(strlen(input), BUFSIZ));
        printf("last_input is: %s\n", last_input);   
        execute(input); }
  }
  
  printf("\t\t...exiting\n");
  return 0;
}