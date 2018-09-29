Project 1

Name: Wyatt S Carpenter

Email address: w@ou.edu

Date: Wed Sep 26 22:07:41 UTC 2018

Description
To solve this programming problem, I essentially modified strtokeg.c to cover the custom commands specified in https://oudalab.github.io/cs3113fa18/projects/project1.html. To implement these commands, I mostly used calls to system() and libc wrapper functions.
My approach was, on the whole, uninteresting, and exactly what you would expect: need to do something -> call the appropriate libc function. However, my morph's copy phase is implemented in a surprising way: it just tries to mimic [src] [dst]/[src basename], and then if that fails it tries to mimic  [src] [dst].

To Run
Run the executable by itself to run in interactive mode. Pass a batchfile into stdin or as an argument to run the commands in the batchfile.
See https://oudalab.github.io/cs3113fa18/projects/project1.html for list of commands and specification.

Bugs and Assumptions
This project contains no bugs to the best of my knowledge at this time. However, it does contain several unoptimized sections of code, that I may one day make better. These are marked as "TODO" in the comments of the code.
The project contains these assumptions:
Morph and mimic are undefined if [src] is a destination.
Mimic is undefined unless both [src] and [dst] are files.
Files will not contain spaces and words in comments will not be separated by more than one space.
The README.txt is in its original location.
Error messages are not particularly important.
If you want to exactly match the reference implimentation, various Ubuntu assumptions must be true, such as ls sort order (ls sort order was different on the Debian machine I developed on, and frightened me for a while).
POSIX-compatible operating system.
Computer has at least a nickel's worth of memory.

References
For this project, I used no references beyond the man pages and the information and materials presented as part of the class. My program is decended from strtokeg.c, as provided in the project 1 specification, and uses code from environ.c (also in the spec). copy.c (provided as part of tlpi example code) was also consulted.
While testing, I used Ryan Stiles's batchfiles.
