#This make file was written by Wyatt S Carpenter
INCLUDES = oufs.h oufs_lib.h vdisk.h
LIBS = oufs_lib.o vdisk.o
EXES = zinspect zformat zfilez zmkdir zrmdir ztouch
FLAGS = -Wall -pedantic -g

all: $(EXES)

clean:
	rm $(EXES) *.o

zinspect: zinspect.o $(LIBS)
	cc $(FLAGS) -o zinspect zinspect.o $(LIBS)

zformat: zformat.o $(LIBS)
	cc $(FLAGS) -o zformat zformat.o $(LIBS)

zfilez: zfilez.o $(LIBS)
	cc $(FLAGS) -o zfilez zfilez.o $(LIBS)

zmkdir: zmkdir.o $(LIBS)
	cc $(FLAGS) -o zmkdir zmkdir.o $(LIBS)

zrmdir: zrmdir.o $(LIBS)
	cc $(FLAGS) -o zrmdir zrmdir.o $(LIBS)

ztouch: ztouch.o $(LIBS)
	cc $(FLAGS) -o ztouch ztouch.o $(LIBS)

#cool rule for object files from dr fagg
.c.o: $(INCLUDES)
	cc $(FLAGS) -c $< -o $@
