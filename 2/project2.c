/* 
   project2  - CS3113 shell
  
   usage:

   ./project2 [batchfile]
          
   reads in a line of keyboard input at a time, parsing it into
   tokens that are separated by white spaces (set by #define
   SEPARATORS).

   can use redirected input or read from the provided batchfile
        
   if the first token is a recognized internal command, then that
   command is executed. otherwise the line is passed to the system
   shell.
   
   see https://oudalab.github.io/cs3113fa18/projects/project2.html
   for list of commands and specification.
*/
 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h> //for mkdir

#define MAX_BUFFER 1024            // max line buffer
#define MAX_ARGS 64                // max # args
#define SEPARATORS " \t\n"         // token sparators
#define ALL_PERM 00777             // maximally permissive permissions
//#define free(X)                    // define free to nothing to avoid segfaults
   
extern char **environ;             // environment array

//some helper functions:
int error(const char * msg){
  return fprintf(stderr, "%s\n", msg);
}
int streq(char * l, char * r){ //check if strings equal
  return !strcmp(l,r); //returns true if equal, false if not equal
}
struct dirent * getent(DIR * dirp){
  //exactly like readdir, but without the . and .. entries
  struct dirent * ent;
  do{
    ent = readdir(dirp);
  }while( ent && (streq(ent->d_name,".") || streq(ent->d_name,"..")) );
  return ent; //liek readdir, returns NULL if error or we reach the end
}

char * slash(const char * l, const char * r){ //this fn mallocs a string with the value l/r
  char * new = malloc(strlen(l)+strlen(r)+4); //4 seems like a good margin
  strcpy(new,l);
  strcat(new,"/");
  strcat(new,r);
  return new;
}

//some functions to manipulate files:

int erase(const char * target){
  if(remove(target)){
    fprintf(stderr, "Couldn't erase %s\n", target);
    return -1;
  }else{
    return 0;
  }
}

int eraser(const char * target){ //erase recursive
  int ret = 0;
  DIR * dir = opendir(target); //will be null if target isn't a dir 
  if (dir){
    //it is a dir
    //erase within dir
    struct dirent * ent;
    while( (ent = getent(dir)) ){
      char * s = slash(target, ent->d_name);
      ret |= eraser(s);
      free(s);
    }
  }
  ret |= erase(target); //it's either empty or a file now
  return ret;
}

int copy(const char * src, const char * dst){
  //copy src file to dst file
  int ret = 0; //error value to return 
  int s; //source file descriptor
  int d; //destination file descriptor
  char b; //buffer char
  //TODO: use more than one byte for buffer
  //using just one slows the copies down. (duh)
  //(copy speed is still acceptable as-is, though
  // and it cuts down on chance of buffer overflow errors)

  s = open(src, O_RDONLY);
  if(s==-1){
    error("couldn't open source file");
    ret |= 1;
  } else {
    d = open(dst, O_CREAT | O_WRONLY | O_TRUNC, ALL_PERM);
    // ALL_PERM  means everyone has rw priveleges
    if(d==-1){
      perror(dst);
      ret |= 2;
    } else {
      //we're all clear, ready to copy
      while(read(s,&b,1)==1){
	if (write(d,&b,1)!=1){
	  error("Couldn't write whole file.");
	  break;
	}
      }
      if (close(s)){
	error("Couldn't close source file when copying");
	ret |= 4;
      }
      if (close(d)){
	error("Couldn't close destination file when copying");
	ret |= 8;
      }
    }
  }
  return ret;
}

int mimic(const char * src, const char * dst, int recursive){
  //This is not a great function; it's too complex.
  //But it does the job. Probably.
  int ret = 0;  

  //if these are not dirs or don't exist, the pointers will be null.
  DIR * srcdir = opendir(src);
  DIR * dstdir = opendir(dst);

  //allocate all the strings we might need, and populate some pointers
  //note that basename and dirname modify in place,
  //so we need to make new copies of our strings for them.
  char * bsrc = malloc(strlen(src)+1); //the base of the src
  bsrc = basename(strcpy(bsrc,src));
  char * ddst = malloc(strlen(dst)+1); //the dir of the dst
  ddst = dirname(strcpy(ddst,dst));
  DIR * pdstdir = opendir(ddst);       //test for existence of pdst
  char * srcindst = slash(dst,bsrc);
  struct dirent * firstinsrc = NULL;

  //do all the logic. check the spec for the gnarly details
  if(srcdir){
    firstinsrc = getent(srcdir); //null on empty dir or error, we assume empty
  }
  if(srcdir && dstdir){
    if(recursive){
      ret = mkdir(srcindst, ALL_PERM);
      do{
	char * nextsrc = slash(src,firstinsrc->d_name);
	char * nextdst = slash(srcindst,firstinsrc->d_name);
	mimic(nextsrc, nextdst, recursive);
	free(nextsrc);
	free(nextdst);
      }while( (firstinsrc = getent(srcdir)) );
    } else if (!firstinsrc) {
      ret = mkdir(srcindst, ALL_PERM);
    } else {
      error("src folder non-empty and -r not supplied; nothing done.");
      ret = EXIT_FAILURE;
    }
  } else if (dstdir){
    ret = copy(src, srcindst);
  } else if (srcdir && pdstdir){
    if(recursive){
      if( (ret = mkdir(dst, ALL_PERM)) ){
	do{
	  char * nextsrc = slash(src,firstinsrc->d_name);
	  char * nextdst = slash(dst,firstinsrc->d_name);
	  mimic(nextsrc, nextdst, recursive);
	  free(nextsrc);
	  free(nextdst);
	}while( (firstinsrc = getent(srcdir)) );
      } else {
	error("couldn't mkdir in mimic");
	return ret;
      }
    } else if (!firstinsrc) {
      ret = mkdir(dst, ALL_PERM);
    } else {
      error("src folder non-empty and -r not supplied; nothing done.");
      ret = EXIT_FAILURE;
    }
  } else {
    ret = copy(src, dst);
  }

  //free all those things from before
  free(bsrc);
  free(ddst);
  free(srcindst);
  
  return ret;
}

int parsemimic(char ** args){ //parses args array and does a mimic call thereby
  //args must be terminated by a null string

  //parse args
  int recursive = 0;
  char * src = NULL;
  char * dst = NULL;
  int i = 1; //args 0 is name of command (morph or mimic)
  while (args[i]){
    if(!strcmp("-r",args[i])){
      if(recursive == 1){
	error("recursive flag specified multiple times"); //supererogatory error checking
	recursive = 2;
      } else {
	recursive = 1;
      }
    } else {
      if(!src){
	src = args[i];
      } else if (!dst){
	dst = args[i];
      } else {
	error("too many arguments!");
	return EXIT_FAILURE;
      }
    }
    i++;
  }
  if(!src || !dst){
    fprintf(stderr, "Usage: %s [-r] src dst\n", args[0]);
    return EXIT_FAILURE;
  }
  //check for one error, unnecessarily tbh
  if(access(src,R_OK)){ //access returns 0 if OK, so the following happens if not OK 
    error("can't read src");
    return EXIT_FAILURE;
  }
  return mimic(src, dst, recursive);
}

int fe(const char * command, char ** args){
  //replacement fn for system
  //forks and execs, and redirects (< > >>)
  //function adapted, with heavy modification from example code in
  //https://oudalab.github.io/cs3113fa18/projects/project2.html#your-todos
  //args should almost always be called with the value args in the code below
  //args[0] is the name of the program, as is convention. 
  int pid;
  int i = 0;
  switch( pid = fork() ){ 
  case -1:
    error("couldn't fork");
    break;
  case 0:
    //child
    //freopen according to redirection operators
    while( (args[++i]) ){
      //each of these nulls out the operator entry
      //so execvp doesn't hit them when we actually run it
      if(!strcmp(args[i], "<")){
	freopen(args[i+1], "r", stdin);
	args[i] = NULL;
      }else if(!strcmp(args[i], ">")){
	freopen(args[i+1], "w", stdout);	
	args[i] = NULL;
      }else if(!strcmp(args[i], ">>")){
	freopen(args[i+1], "a", stdout);
	args[i] = NULL;
      }
    }
    execvp(command, args); 
    error("couldn't exec");
    exit(EXIT_FAILURE);
    break;
  default:                // parent
    return waitpid(pid, NULL, WUNTRACED); //here we pass in NULL instead of the address of an int because this is the correct way to indicate we don't  need the int.
    break;
  }
  error("something went wrong in fe");
  return EXIT_FAILURE;
}

int main (int argc, char ** argv)
{
  setbuf(stdout, NULL);
  int isbatch = 0; //flag that indicates if we are in a batch script
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
  char cwd[MAX_BUFFER];                       //buffer to store (then print) cwd
  char * originalstr;                    // copy of original str to print in ditto
  int prevlineempty=0; //flag we use to keep from double-printing prompts like an idiot

  // keep reading input until "esc" command or eof of redirected input

  while (!feof(stdin)) {
    // get command line from input
    if(!prevlineempty || !isbatch){
      printf("%s%s", getcwd(cwd, sizeof(cwd)), prompt);   // write enhanced prompt
    }
    prevlineempty=1;

    if (fgets(buf, MAX_BUFFER, stdin) ) { // read a line
      originalstr = malloc(strlen(buf)+1); // allocate mem
      strcpy(originalstr, buf);
      // tokenize the input into args array
      arg = args;
      *arg++ = strtok(buf,SEPARATORS);   // tokenize input
      while ((*arg++ = strtok(NULL,SEPARATORS)));
      // last entry will be NULL 
 
      if (args[0]) {                     // if there's anything there
	prevlineempty=0;
	if(isbatch){ //print input if in batchfile, per spec
	  printf("%s",originalstr); //write command
	} 

	//use this macro to avoid some redundant typing:
#define chk(str) !strcmp(args[0],str)
	
	// check for internal/external command
	// commands are listed/checked here in the order the spec specs them

	if (!strcmp(args[0],"wipe")) { // "clear" command
	  args[0] = "clear";
	  fe(args[0],args);
	  continue;
	}
            
	if (!strcmp(args[0],"esc")){   // "quit" command
	  break;                     // break out of 'while' loop
	}
	
	if (chk("filez")){ //"filez" command
	  //do the filez command via a call to the system's ls -1 cmd
	  //must prepare an array for fe()
	  //ends up being ls [targets] -1
	  int i=0;
	  while(args[++i]); //set i to first null cmd
	  args[i] = "-1";
	  args[i+1] = NULL; //make sure there are no trailing arguments
	  args[0] = "ls";
	  fe(args[0],args);
	  continue;
	}

	if(chk("environ")){ //"environ" command
	  //code from environ.c
	  char ** env = environ;
	  while (*env) printf("%s\n",*env++);  // step through environment
	  continue;
	}

	if(chk("ditto")){ //"ditto" command
	  if(strlen(originalstr)>6){ //check to see if there's anything after "ditto "
	    //print comment line sans first 6 chars ("ditto ")
	    printf("%s",originalstr+6);
	  }
	  continue;
	}

	if(chk("help")){ //"help" command
	  //just cat the help file
	  char * tmp_args[] = {"cat", "/projects/2/README.txt", ""}; //can't pass in an array literal (sans confusing magic), must use this tmp variable.
	  //note also that I've written char * tmp_args[] instead of char ** tmp_args ONLY BECAUSE gcc complained to me about the talmudic misimplications of the latter form.
	  fe("cat", tmp_args);
	  continue;
	}

	//the real meat of the assignment here (the libc calls):
	if(chk("mimic")){ //"mimic" command
	  parsemimic(args); //parsemimic checks for correct args
	  continue;
	}

	if(chk("erase")){ //"erase" command
	  if(args[1]){
	    if (args[2] && !strcmp(args[1],"-r")){
	      eraser(args[2]);
	    } else {
	      erase(args[1]);
	    }
	  } else {
	    error("erase requires one argument: target.");
	  } 
	  continue;
	}

	if(chk("morph")){ //"morph"
	  if (!parsemimic(args)){ //parsemimic checks args
	    //only erase if the copy worked
	    if (args[2] && !strcmp(args[1],"-r")){
	      eraser(args[2]);
	    } else {
	      erase(args[1]);
	    }
	  }
	  continue;
	}

	//back to the non-meat:
	if(chk("chdir")){ //"chdir" command
	  if(args[1]){
	    if(chdir(args[1])){
	      fprintf(stderr, "Couldn't chdir to %s\n", args[1]);
	    } else {
	      //chdir worked, now set the pwd using setenv
	      setenv("PWD", args[1], 1); //will overwrite (hence the 1)
	    }
	  } else {
	    //use system command to print working directory
	    puts(getenv("PWD"));
	  }
	  continue;
	}

	//project2 new commands

	if(chk("rmdirz")){ //"rmdirz" command
	  if(args[1]){
	    if(rmdir(args[1]) != 0){
	      perror(args[1]);
	    }
	  } else {
	    error("rmdirz requires one argument: target.");
	  } 
	  continue;
	}

	if(chk("mkdirz")){ //"mkdirz" command
	  if(args[1]){
	    if(mkdir(args[1], ALL_PERM) != 0){
	      error("couldn't mkdirz");
	    }
	  } else {
	    error("mkdirz requires one argument: target.");
	  } 
	  continue;
	}
	
	// else pass command onto OS
	fe(args[0],args); //run cmd with rest of args array
      }
    }
  }
  return EXIT_SUCCESS; 
}
