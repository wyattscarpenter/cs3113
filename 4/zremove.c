/**
Make a directory in the OU File System.

CS3113

*/

#include <stdio.h>
#include <string.h>

#include "oufs_lib.h"

int main(int argc, char** argv) {
  // Fetch the key environment vars
  char cwd[MAX_PATH_LENGTH];
  char disk_name[MAX_PATH_LENGTH];
  oufs_get_environment(cwd, disk_name);
  if(argc==2){ // Check arguments
  
  vdisk_disk_open(disk_name);
  
  oufs_remove(cwd, argv[1]);
  
  vdisk_disk_close();
  
  }else{
    // Wrong number of parameters
    fprintf(stderr, "Usage: zremove <filename>\n");
  }
}
