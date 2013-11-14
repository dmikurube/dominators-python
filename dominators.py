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


def verify_spanning_tree(root, edges, parents, postorder, preorder):
  reachable = [ False ] * (len(parents) + 1)
  working_stack = [ -1 ] * (len(parents) + 1)

  for node in range(len(parents) + 1):  # Iterate all nodes.
    if node == root:
      print "Root: %d" % root
      continue
    is_root_reachable(node, root, edges, parents, reachable, working_stack)
  print "All nodes are reachable to the root in the spanning tree."

  new_edges = []
  edge_table = [ None ] * (len(parents) + 1)
  for node in range(len(parents) + 1):  # Iterate all nodes.
    edge_table[node] = set()
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
      raise BaseException(
          "A tree edge (%d, %d) is not included in the original graph." % (
              parent, node))
  print "All tree edges are included in the original graph."

  visited_count = 0
  visited = [ False ] * (len(parents) + 1)
  for node_postorder in range(len(parents) + 1):  # Iterate all nodes.
    node_ordinal = postorder[node_postorder]
    if node_ordinal == root:
      if visited_count != len(parents):
        raise "[Post] Not ordered in a bottom-up order: root is not at last."
      break  # check count
    visited[node_ordinal] = True
    visited_count += 1
    if visited[parents[node_ordinal]]:
      raise "[Post] Not ordered in a bottom-up order."
  print "[Post] Ordered in a bottom-up order."

  visited_count = 0
  visited = [ False ] * (len(parents) + 1)
  for node_preorder in reversed(range(len(parents) + 1)):  # Iterate all nodes.
    node_ordinal = preorder[node_preorder]
    if node_ordinal == root:
      if visited_count != len(parents):
        raise "[Pre] Not ordered in a bottom-up order: root is not at last."
      break  # check count
    visited[node_ordinal] = True
    visited_count += 1
    if visited[parents[node_ordinal]]:
      raise "[Pre] Not ordered in a bottom-up order."
  print "[Pre] Ordered in a bottom-up order."

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


def GD2(root, edges, parents, postorder, preorder, total, arcs):
  d = [ None ] * (len(parents) + 1)
  d[root] = root

  out_node = [ None ] * (len(parents) + 1)
  in_node = [ None ] * (len(parents) + 1)
  unionfind = OrderedUnionFind()
  same = [ None ] * (len(parents) + 1)
  added = [ 0 ] * (len(parents) + 1)

  for node_postorder in range(len(parents) + 1):  # Iterate all nodes.
    u = postorder[node_postorder]
    #for node_preorder in reversed(range(len(parents) + 1)):  # Iterate all nodes.
    #  u = preorder[node_preorder]
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
    arcs[u] = []

    while len(out_node[u]) > 0:
      y = out_node[u].pop()
      v = unionfind[y]
      if v != u:
        if total[v] < 0:
          raise BaseException("hoge")
        total[v] -= 1
        added[v] -= 1
      if total[v] < 0:
        raise BaseException("moga")
      if total[v] == 0:
        x = unionfind[parents[v]]
        if u == x:
          for w in same[v]:
            d[w] = u
        else:
          same[x].extend(same[v])
        unionfind.union(parents[v], v)
        out_node[x].extend(out_node[v])

    while len(in_node[u]) > 0:
      z = in_node[u].pop()
      v = unionfind[z]
      while v != u:
        same[u].extend(same[v])
        x = unionfind[parents[v]]
        unionfind.union(parents[v], v)
        in_node[x].extend(in_node[v])
        out_node[x].extend(out_node[v])
        if total[v] < 0:
          raise BaseException("hoga")
        total[x] += total[v]
        total[v] = -10000
        added[x] += added[v]
        v = x

    total[u] -= added[u]
    added[u] = 0

  all_total = 0
  all_arcs = 0
  all_in = 0
  for t in total:
    if t > 0:
      all_total += t
  for arc in arcs:
    all_arcs += len(arc)
  for ins in in_node:
    if ins:
      all_in += len(ins)
  if all_total != all_arcs:
    print '%d: %d - %d - %d' % (node_postorder, all_total, all_arcs, all_in)

  return d


def rcompress(v, parents, label, c, ccount):
  ccount += 1
  p = parents[v]
  if p > c:
    ccount = rcompress(p, parents, label, c, ccount)
    ccount += 1
    if label[p] < label[v]:
      label[v] = label[p];
    parents[v] = parents[p];
  return ccount


def snca(root, edges, parents, preorder, rpreorder):
  nvertices = len(parents) + 1
  nedges = len(edges)
  # initialize arrays
  in_arcs = [ [] for i in range(nedges)]

  # insert the arcs; in the process, last_in and last_out will end up being correct
  for edge in reversed(edges):
    in_arcs[edge[1]].append(edge[0])

  bsize = len(parents) + 2
  dom = [0] * bsize
  label = [i for i in range(bsize)]
  semi = [i for i in range(bsize)]

  idom = [0] * bsize
  icount = 0
  scount = 0
  ccount = 0

  for i in reversed(range(len(parents) + 1)):  # Iterate all nodes.
    if preorder[i] == root:
      continue

    dom[i] = parents[i];

    for p in in_arcs[preorder[i]]:
      v = rpreorder[p]
      ccount += 1
      if v <= i:  # v is an ancestor of i
        u = v
      else:
        rcompress(v, parents, label, i, ccount)
        u = label[v]
      ccount += 1
      if semi[u] < semi[i]:
        semi[i] = semi[u]

    label[i] = semi[i]

  dom[0] = 0
  idom[root] = root
  for i in range(1, nvertices):
    j = dom[i];
    while j > semi[i]:
      j = dom[j]
      ccount += 1
    ccount += 1
    dom[i] = j;
    idom[preorder[i]] = preorder[dom[i]];

  return idom


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
    rpreorder = {}
    for pre, ordinal in raw_preorder.iteritems():
      preorder[int(pre)] = ordinal
      rpreorder[ordinal] = int(pre)
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
  lca = LCA(parents)
  print "Built LCA."

  total, arcs = prepare_GD2(root, edges, parents, lca)
  print "Prepared total and arcs."

  dominators = GD2(root, edges, parents, postorder, preorder, total, arcs)
  # dominators = snca(root, edges, parents, preorder, rpreorder)
  for node in range(len(parents) + 1):  # Iterate all nodes.
    print '%d: %s' % (node, dominators[node])

  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
