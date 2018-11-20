Project 4

Name: Wyatt S Carpenter

Email address: w@ou.edu

Date: 11/8/2018

Description
To solve this programming problem, I essentially created several small executables that serve only to call library code for their associated functions. These executables and the expected behavior are specified in https://oudalab.github.io/cs3113fa18/projects/project3.html. I found the suggested helper functions to be pretty good ideas, so I implemented them as well, along with some other helper functions that occured to me along the way.
With regard to library code, I made essentially all of my edits to oufs_lib.c/.h
My implementation of this project is not very exciting. It is mostly perfunctory and as-expected, as can be seen by how I just used the suggested helper functions. Attempts were made at all levels to do the right thing. Perhaps the only thing in my code I am vaguely fond of is dirpdir, a function that progresses from one dir to the next based on name provided.
My implementation of this project was actually hindered by a conceptual misstep I made early on: I decided to focus my project around directory blocks, instead of inodes. Conceptually, I found this simpler, as it allowed me to pretend I was dealing with a slightly convulted linked list. However, it then became apparent that this was the wrong strategy, as I commonly needed to edit the inodes. So all my code is, like, left-handed, funhouse-mirrored.
During this project, I began development of "widio.h", a file that implements crucial idiosyncracies I use in C, such as streq, a function that checks if strs are equal (like strcmp, but the right way around)
While programming, I began to use a development style called "continuous antiquation". In it, I would program something several times, each time making better code & more general hleper functions, as I better understood the problem domain. However, and this is the important part, I never removed the old working code unless I absolutely had to. This is in accordance with the principle of Wu Wei and the ancient Confucian saying, "Man who modify working code, idiot".

To Run
Run the appropriate executables located in this folder. If there are no such executables, run make. You may wish to set your environment variables to foreign and exciting values, as specified in the specification, but sensible defaults are provided otherwise.

Bugs and Assumptions
This project contains no bugs to the best of my knowledge at this time.
The project assumes you have permission to write to whatever file your ZDISK environment variable is set to (or vdisk1 in the current directory)

References
For this project, I used as references the man pages and the information and materials presented as part of the class.
I also used https://arstechnica.com/civis/viewtopic.php?f=20&t=607813 to figure out that I had to write a wrapper function to use qsort correct. Because the function is one line of straightforward C code that there's only really one way to write, my string_compare function ended up looking quite like the function on that website. However, I did not copy and paste, only used the website for guidance.
To test my code, I used a variety of tests, including the ones provided by the instructors and the ones discussed on canvas. (I modified most of these tests in the course of using them). Therefore I would like to extend thanks to Dr Andrew Fagg, Dr Christian Grant, Lucas Bowker, Kyle Roller, Ryan Hernandez for the tests they provided.
