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
  if(argc==3){
  // Check arguments
  char * path = argv[1];
  char * dst = argv[2];
    // Open the virtual disk
    vdisk_disk_open(disk_name);

    // do the thing
    oufs_link(cwd, path, dst);

    // Clean up
    vdisk_disk_close();
    
  }else{
    // Wrong number of parameters
    fprintf(stderr, "Usage: zlink <existing> <new_name>\n");
  }
}
