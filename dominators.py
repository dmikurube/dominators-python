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

from LCA import LCA
from OrderedUnionFind import OrderedUnionFind

from collections import OrderedDict  # pylint: disable=E0611,W0611


def is_root_reachable(node, root, edges, parents, reachable, stack):
  if reachable[node]:
    return True

  height = 0
  stack[height] = node

  while parents[stack[height]] != root:
    parent = parents[stack[height]]
    height += 1
    stack[height] = parent

  while height >= 0:
    reachable[stack[height]] = True
    height -= 1

  return True


def verify_spanning_tree(root, edges, parents, postorder):
  reachable = [ False ] * (len(parents) + 1)
  working_stack = [ -1 ] * (len(parents) + 1)

  for node in range(len(parents) + 1):  # Iterate all nodes.
    if node == root:
      print "Root: %d" % root
      continue
    is_root_reachable(node, root, edges, parents, reachable, working_stack)
  print "All nodes are reachable to the root."

  new_edges = []
  edge_table = [ set() ] * (len(parents) + 1)
  for edge in edges:
    if edge[0] == edge[1]:
      # Skip self-looping edges.
      continue
    if edge[1] in edge_table[edge[0]]:
      # Skip duplicated edges.
      continue
    edge_table[edge[0]].add(edge[1])
    new_edges.append((edge[0], edge[1]))
  print "Removed all self-looping and duplicated edges."
  edges = new_edges
  for node, parent in parents.iteritems():
    if node not in edge_table[parent]:
      raise "A tree edge (%d, %d) is not included in the original graph."
  print "All tree edges are included in the original graph."

  visited_count = 0
  visited = [ False ] * (len(parents) + 1)
  for node_postorder in range(len(parents) + 1):  # Iterate all nodes.
    node_ordinal = postorder[node_postorder]
    if node_ordinal == root:
      if visited_count != len(parents):
        raise "Not ordered in post-order: root is not at last."
      break  # check count
    visited[node_ordinal] = True
    visited_count += 1
    if visited[parents[node_ordinal]]:
      raise "Not ordered in post-order."
  print "Ordered in post-order."

  return edges


def prepare_GD2(root, edges, parents, lca):
  total = [ 0 ] * (len(parents) + 1)
  arcs = [ None ] * (len(parents) + 1)
  for i in range(len(parents) + 1):
    arcs[i] = []
  for edge in edges:
    total[edge[1]] += 1
    arcs[lca(edge[0], edge[1])].append((edge[0], edge[1]))
  return total, arcs


def GD2(root, edges, parents, postorder, total, arcs):
  d = [ None ] * (len(parents) + 1)
  d[root] = root

  out_node = [ None ] * (len(parents) + 1)
  in_node = [ None ] * (len(parents) + 1)
  unionfind = OrderedUnionFind()
  same = [ None ] * (len(parents) + 1)
  added = [ 0 ] * (len(parents) + 1)

  for node_postorder in range(len(parents) + 1):  # Iterate all nodes.
    u = postorder[node_postorder]
    out_node[u] = []
    in_node[u] = []
    # unionfind.makeset(u)
    added[u] = 0
    same[u] = [u]

    for x, y in arcs[u]:
      find_x = unionfind[x]
      find_y = unionfind[y]
      out_node[find_x].append(y)
      in_node[find_y].append(x)
      added[find_y] += 1

    while out_node[u]:
      y = out_node[u].pop()
      v = unionfind[y]
      if v != u:
        total[v] -= 1
        added[v] -= 1
      if total[v] == 0:
        x = unionfind[parents[v]]
        if u == x:
          for w in same[v]:
            d[w] = u
        else:
          same[x].extend(same[v])
        unionfind.union(parents[v], v)
        out_node[x].extend(out_node[v])

    while in_node[u]:
      z = in_node[u].pop()
      v = unionfind[z]
      while v != u:
        same[u].extend(same[v])
        x = unionfind[parents[v]]
        unionfind.union(parents[v], v)
        in_node[x].extend(in_node[v])
        out_node[x].extend(out_node[v])
        total[x] += total[v]
        added[x] += added[v]
        v = x

    total[u] -= added[u]
    added[u] = 0

  return d


def main(argv):
  with open('edges.json', 'r') as edges_f:
    edges = json.load(edges_f, object_pairs_hook=OrderedDict)['edges']
  with open('postorder.json', 'r') as postorder_f:
    raw_postorder = json.load(postorder_f, object_pairs_hook=OrderedDict)
    postorder = {}
    for post, ordinal in raw_postorder.iteritems():
      postorder[int(post)] = ordinal
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

  edges = verify_spanning_tree(root, edges, parents, postorder)
  lca = LCA(parents)
  print "Built LCA."

  total, arcs = prepare_GD2(root, edges, parents, lca)
  print "Prepared total and arcs."

  dominators = GD2(root, edges, parents, postorder, total, arcs)
  for node in range(len(parents) + 1):  # Iterate all nodes.
    print '%d: %s' % (node, dominators[node])

  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
