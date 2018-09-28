/* 
   project1  - a shell
  
   usage:

   ./project1 [batchfile]
          
   reads in a line of keyboard input at a time, parsing it into
   tokens that are separated by white spaces (set by #define
   SEPARATORS).

   can use redirected input or read from the provided batchfile
        
   if the first token is a recognized internal command, then that
   command is executed. otherwise the line is passed to the system
   shell.
   
   see https://oudalab.github.io/cs3113fa18/projects/project1.html
   for list of commands and specification.
*/
 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_BUFFER 1024                        // max line buffer
#define MAX_ARGS 64                            // max # args
#define SEPARATORS " \t\n"                     // token sparators
   
extern char **environ;                   // environment array

//some helper function to write files:

int error(char * msg){
  return fprintf(stderr, "%s", msg);
}

int erase(char * target){
  //unlink only erases files, rmdir only (empty) directories
  //it is easiest just to try both, rather than figure out which type the target is.
  if(unlink(target) && rmdir(target)){
    fprintf(stderr, "Couldn't erase %s\n", target);
    return -1;
  }else{
    return 0;
  }
}

int main (int argc, char ** argv)
{
  char isbatch = 0; //flag that indicates  if we are in a batch script
                    //would be a bool that's not truly C style
  //if argument provided, read from it as a batchfile:
  if(argc>1){ //executable is of course the 1st arg
    freopen(argv[1],"r",stdin); //stdin is now reading from the file! yay!
    isbatch = 1;
  }
  char buf[MAX_BUFFER];                      // line buffer
  char * args[MAX_ARGS];                     // pointers to arg strings
  char ** arg;                               // working pointer thru args
  char * prompt = "==>" ;                    // shell prompt
  char * originalstr;                        // copy of original str to print/pass to system
    
  // keep reading input until "quit" command or eof of redirected input
     
  while (!feof(stdin)) { 
    
    // get command line from input
  
    fputs(prompt, stdout);                // write prompt
    if (fgets(buf, MAX_BUFFER, stdin) ) { // read a line

      originalstr = malloc(strlen(buf)+1); // allocate mem
      strcpy(originalstr, buf);
      if(isbatch){puts(originalstr);} //print input if in batchfile, per spec
      // tokenize the input into args array

      arg = args;
      *arg++ = strtok(buf,SEPARATORS);   // tokenize input
      while ((*arg++ = strtok(NULL,SEPARATORS)));
      // last entry will be NULL 
 
      if (args[0]) {                     // if there's anything there
	//use this macro to avoid some redundant typing:
#define chk(str) !strcmp(args[0],str)
	// check for internal/external command

	if (!strcmp(args[0],"wipe")) { // "clear" command
	  system("clear");
	  continue;
	}
            
	if (!strcmp(args[0],"esc")){   // "quit" command
	  break;                     // break out of 'while' loop
	}
	
	if (chk("filez")){
	  const char * ls1 = "ls -1 ";
	  char * cmd;
	  if(args[1]){
	    cmd = malloc(strlen(ls1) + strlen(args[1]) + 1);
	    strcpy(cmd, ls1);
	    strcat(cmd,args[1]);
	    system(cmd);
	    free(cmd);
	  }else{
	    system(ls1);
	  }
	  continue;
	}

	if(chk("environ")){
	  //code from environ.c
	  char ** env = environ;
	  while (*env) printf("%s\n",*env++);  // step through environment
	  continue;
	}

	if(chk("ditto")){
	  puts(originalstr+6); //print comment line sans first 6 chars ("ditto ")
	  continue;
	}

	if(chk("help")){
	  system("cat /projects/1/README.txt");
	  continue;
	}

	//the real meat of the assignment here (the libc calls):
	if(chk("mimic")){
	  //TODO
	  continue;
	}

	if(chk("erase")){
	  if(args[1]){
	    erase(args[1]);
	  } else {
	    error("No argument.");
	  } 
	  continue;
	}

	if(chk("morph")){
	  //TODO
	  continue;
	}

	//back to the non-meat:
	if(chk("chdir")){
	  if(args[1]){
	    if(chdir(args[1])){
	      fprintf(stderr, "Couldn't chdir to %s\n", args[1]);
	    } else {
	      char * cmd = malloc(strlen("PWD=") + strlen(args[1]) + 1);
	      strcpy(cmd,"PWD=");
	      strcat(cmd,args[1]);
	      putenv(cmd); //this string is part of the env now, don't modify.
	    }
	  } else {
	    system("pwd");
	  }
	  continue;
	}
	
	  // else pass command onto OS
	  arg = args;
	  system(originalstr);
	  fputs ("\n", stdout);
	}
      }
    }
    return 0; 
  }
