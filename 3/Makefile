#This make file was written by Wyatt S Carpenter
INCLUDES = oufs.h oufs_lib.h vdisk.h
LIBS = oufs_lib.o vdisk.o
EXES = zinspect zformat
FLAGS = -Wall -pedantic -g

all: $(EXES)

clean:
	rm $(EXES) *.o

zinspect: zinspect.o $(LIBS)
	cc $(FLAGS) -o zinspect zinspect.o $(LIBS)

zformat: zformat.o $(LIBS)
	cc $(FLAGS) -o zformat zformat.o $(LIBS)


#cool rule for object files from dr fagg
.c.o: $(INCLUDES)
	cc $(FLAGS) -c $< -o $@
