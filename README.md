This Python code tries to compute dominators for a given graph. It (tries to)
implement the Algorithm GD, Version 2 in "Finding Dominators via Disjoint Set
Union" by Wojciech Fraczak, Loukas Georgiadis, Andrew Miller and Robert E.
Tarjan. See http://arxiv.org/abs/1310.2118 for the article.

Note that the implementation is not working for the attached data. Needs more
work.

* dominators.py:
    * The implementation. Run it with downloading UnionFInd.py and LCA.py described as below. It reads *.json files in the same directory.
* OrderedUnionFind.py:
    * A wrapper of UnionFind. It always unifies into the first argument.
* UnionFind.py and LCA.py:
    * Required external libraries which are not in the repository. Download it as described below.
* *.json:
    * Data files. edges.json is edges in the original graph, parents.json is p() in the DFS spanning tree (root=0) and postorder.json represents a bottom-up order.
* RESULT.txt:
    * A dominator result for the data files. You can see some dominators are "None" (e.g. for node 13) which are failing to compute at this time.

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
