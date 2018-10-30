#This reusable make file was written by Wyatt S Carpenter
INCLUDES = oufs.h oufs_lib.h vdisk.h
LIB = oufs_lib_support.o vdisk.o
EXES = zinspect

all: $(EXES)

clean:
	rm $(EXES) *.o

zinspect: zinspect.o $(LIB)
	cc -o zinspect zinspect.o $(LIB)

#cool rule for object files from dr fagg
.c.o: $(INCLUDES)
	cc -c $< -o $@
