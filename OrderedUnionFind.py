#!/usr/bin/env python


import sys

from UnionFind import UnionFind


class OrderedUnionFind():
  """UnionFind that always unites into the first argument."""
  def __init__(self):
    self.unionfind = UnionFind()
    self.name = {}

  def __getitem__(self, object):
    found = self.unionfind[object]
    if found not in self.name:
      self.name[found] = found
    return self.name[self.unionfind[object]]

  def __iter__(self):
    return iter(self.unionfind)

  def union(self, object1, object2):
    root1 = self.unionfind[object1]
    root2 = self.unionfind[object2]
    self.unionfind.union(root1, root2)
    if root1 not in self.name:
      self.name[root1] = root1
    self.name[root2] = self.name[root1]


def main(argv):
  unionfind = OrderedUnionFind()

  unionfind.union(1, 2)
  assert unionfind[0] == 0
  assert unionfind[1] == 1
  assert unionfind[2] == 1

  unionfind.union(2, 3)
  assert unionfind[0] == 0
  # The root of 2 is still 1.
  assert unionfind[1] == 1
  assert unionfind[2] == 1
  assert unionfind[3] == 1

  unionfind.union(4, 3)
  # All 1-3 is now under 4.
  assert unionfind[0] == 0
  assert unionfind[1] == 4
  assert unionfind[2] == 4
  assert unionfind[3] == 4
  assert unionfind[4] == 4

  unionfind.union(5, 4)
  # All 1-4 is now under 5.
  assert unionfind[0] == 0
  assert unionfind[1] == 5
  assert unionfind[2] == 5
  assert unionfind[3] == 5
  assert unionfind[4] == 5
  assert unionfind[5] == 5

  unionfind.union(5, 6)
  # All 1-5 and 6 is still under 5.
  assert unionfind[0] == 0
  assert unionfind[1] == 5
  assert unionfind[2] == 5
  assert unionfind[3] == 5
  assert unionfind[4] == 5
  assert unionfind[5] == 5
  assert unionfind[6] == 5

  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
