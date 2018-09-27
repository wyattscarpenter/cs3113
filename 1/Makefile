#This reusable make file was written by Wyatt S Carpenter
all: project

clean:
	rm project? project?.o

project: project?.c
	gcc project?.c -Wall -o $(shell basename project?.c .c)
