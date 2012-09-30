csci4061
========
Seth and Michael's CSCI 4061 assignments

-Purpose of Assignment 1: Program-Graph Executor-

The purpose of this program is to correctly implement and execute a processes graph of terminal commands/programs read in by text file.

To that end, the program correctly implements fork(), execvp(), and wait.



-How to compile this program-

Include with the program is a make file.

It's recommended that you use the make file to build the program, the following command is used to compile the program from the terminal prompt.



make





-How to use Assignment 1 from the terminal ... the proper syntax to evoke the program.

while at the terminal prompt follow this synatx < ProgramName SomeFile.txt >

Example :: graphexec textfile.txt

This will lunch graphexec with textfile.txt as the user entered input.



-Overview of what this program does-

This program takes as input a text file entered by the user at the terminal prompt.  It then reads this file line by line  parsing out information(colon delimited)

following this convention < program name with arguments:list of children ID's:input file:output file >

With this information the program creates a node(data structure) for every line.

The Program then analysis each node, and determines node children and node parents, using this info the Program determiners eligibility to execute.

Thereby, creating a node tree based on a parent/children structure.

Finally the program executes ready node(s), and updates children node(s) changing node status as needed.

In order to create the proper parent/child execution order.

