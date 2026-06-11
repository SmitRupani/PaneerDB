#include "ds/RedBlackTree.h"
#include <cassert>

int main() {
  RedBlackTree tree;

  tree.insert(10);
  tree.insert(5);
  tree.insert(17);
  tree.insert(19);
  tree.insert(7);
  tree.insert(6);
  tree.insert(4);
  tree.insert(3);

  tree.print();
}
