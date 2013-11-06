This Python code computes a dominator tree for a given graph. It implements
the Algorithm GD, Version 2 in "Finding Dominators via Disjoint Set Union"
by Wojciech Fraczak, Loukas Georgiadis, Andrew Miller and Robert E. Tarjan.
See http://arxiv.org/abs/1310.2118 for the article.

Note that it is a slow implementation because it uses naive Python lists
to represent sets "same", "out" and "in". It can be faster by replacing them
by singly-circular linked lists.

It uses the following libraries for disjoint set union and least common
ancestors.
* Union-find:
    * http://www.ics.uci.edu/~eppstein/PADS/UnionFind.py
* LCA (least common ancestors):
    * http://www.ics.uci.edu/~eppstein/PADS/LCA.py

It assumes Python 2.7.
