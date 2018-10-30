#This reusable make file was written by Wyatt S Carpenter
INCLUDES = oufs.h oufs_lib.h vdisk.h
LIB = oufs_lib_support.o vdisk.o

all: zformat

clean:
	rm zformat *.o

zformat: zformat.o $(LIB)
	cc -o zformat zformat.o $(LIB)
#cool rule for object files from dr fagg
.c.o: $(INCLUDES)
	cc -c $< -o $@
