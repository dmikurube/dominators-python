#!/usr/bin/env python

# This Python code computes a dominator tree for a given graph. It implements
# the Algorithm GD, Version 2 in "Finding Dominators via Disjoint Set Union"
# by Wojciech Fraczak, Loukas Georgiadis, Andrew Miller and Robert E. Tarjan.
# See http://arxiv.org/abs/1310.2118 for the article.
#
# Note that it is a slow implementation because it uses naive Python lists
# to represent sets "same", "out" and "in". It can be faster by replacing them
# by singly-circular linked lists.
#
# It uses the following libraries for disjoint set union and least common
# ancestors.
# Union-find:
#   http://www.ics.uci.edu/~eppstein/PADS/UnionFind.py
# LCA (least common ancestors):
#   http://www.ics.uci.edu/~eppstein/PADS/LCA.py
#
# It assumes Python 2.7.


import json
import sys

from collections import OrderedDict  # pylint: disable=E0611,W0611

from dominators import is_root_reachable, verify_spanning_tree


def dimacs(root, edges, parents):
  print 'p %d %d %d %d' % (len(parents)+1, len(edges), root+1, len(parents)+1)
  for edge in edges:
    print 'a %d %d' % (edge[0]+1, edge[1]+1)


def dimacs_parents(root, parents):
  print 'parents %d %d' % (len(parents)+1, root+1)
  for node, parent in parents.iteritems():
    print '%d %d' % (node+1, parent+1)


def dimacs_preorder(root, parents, preorder):
  print 'preorder %d %d' % (len(parents)+1, root+1)
  for preorder, ordinal in preorder.iteritems():
    print '%d %d' % (preorder+1, ordinal+1)


def dimacs_postorder(root, parents, postorder):
  print 'postorder %d %d' % (len(parents)+1, root+1)
  for postorder, ordinal in postorder.iteritems():
    print '%d %d' % (postorder+1, ordinal+1)


def main(argv):
  with open('edges.json', 'r') as edges_f:
    edges = json.load(edges_f, object_pairs_hook=OrderedDict)['edges']
  with open('postorder.json', 'r') as postorder_f:
    raw_postorder = json.load(postorder_f, object_pairs_hook=OrderedDict)
    postorder = {}
    for post, ordinal in raw_postorder.iteritems():
      postorder[int(post)] = ordinal
  with open('preorder.json', 'r') as preorder_f:
    raw_preorder = json.load(preorder_f, object_pairs_hook=OrderedDict)
    preorder = {}
    for pre, ordinal in raw_preorder.iteritems():
      preorder[int(pre)] = ordinal
  with open('parents.json', 'r') as parents_f:
    raw_parents = json.load(parents_f, object_pairs_hook=OrderedDict)
    roots = []
    parents = {}
    for src, dst in raw_parents.iteritems():
      src = int(src)
      if src == dst:
        roots.append(src)
        continue
      parents[src] = dst
    if len(roots) > 1:
      raise "Multiple roots."

  root = roots[0]

  edges = verify_spanning_tree(root, edges, parents, postorder, preorder)
  if len(argv) > 1 and argv[1] == 'parents':
    dimacs_parents(root, parents)
  elif len(argv) > 1 and argv[1] == 'preorder':
    dimacs_preorder(root, parents, preorder)
  elif len(argv) > 1 and argv[1] == 'postorder':
    dimacs_postorder(root, parents, postorder)
  else:
    dimacs(root, edges, parents)

  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
