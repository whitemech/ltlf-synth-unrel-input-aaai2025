# Generators 

Collection of python scripts that can be used to generate the sheep, hiker and trap examples used for tests for the thesis.

## Running the generators 

To generate all sheep examples, just run ``python3 sheep.py`` -- this will generate the ``.ltlf`` and ``.part`` files; however one has to then manually add them into the text directory (however, all examples generated are already contained there). 

For the ``hiker`` examples, running ``python3 hiker.py`` generates the hiker instances used for testing.
Again, they have to be moved by hand into the tests folder. 

For the ``graph``examples, the program expects a description of the graph in a file as an input.
These files (which can be found in the ``examples-graph`` subdirectory), have the following syntax
n m k 
g_1 ...  g_l
s_1 ... s_j
e_11 e_12
e_21 e_22
...
e_m1 e_m2 
te_1 e1 te_2 d2

Thus, the first line contains information about the number of nodes (n), the number of edges (m) and the number of traps (k). The following two lines specify the goal and safe regions, the following m lines the edges and lastl, the last k lines spcify the traps. 