//A C file to accommodate and proliferate Wyatt's idiosyncrasies 
#include <string.h>

typedef unsigned char byte;

int streq(const char * l, const char * r){ //check if strings equal
  return !strcmp(l,r); //returns true if equal, false if not equal
}

#define NULLOUT(x) memset(&x, 0, sizeof(x))
