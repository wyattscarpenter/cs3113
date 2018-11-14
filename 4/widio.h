//A C file to accommodate and proliferate Wyatt's idiosyncrasies 
#include <string.h>

typedef unsigned char byte;

#define streq(l,r) !strcmp(l,r) //returns true if equal, false if not equal
#define NULLOUT(x) memset(&x, 0, sizeof(x))
#define STRNULLCOPY(dst,src,size) strncpy(dst,src,size);dst[size-1]='\0'
//#define STRNULLCOPY(dst,src) strncpy(dst,src,sizeof(dst));dst[sizeof(dst)-1]=0;
