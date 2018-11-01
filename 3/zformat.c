#include <stdio.h>
#include <string.h>

#include "oufs_lib.h"

int main(int argc, char** argv) {
  if(argc == 1){
    char cwd[MAX_PATH_LENGTH];
    char disk_name[MAX_PATH_LENGTH];
    oufs_get_environment(cwd, disk_name);
    oufs_format_disk(disk_name);
  }else{
    fprintf(stderr, "Unknown argument (%s)\n", argv[1]);
  }
}

