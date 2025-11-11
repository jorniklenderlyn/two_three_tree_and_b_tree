#include"test_b_tree.h"
#include"test_two_three_tree.h"
#include"two_three_tree.h"
#include"b_tree.h"


int main() {
    // TestTwoThreeTree test;
    // test.RunTests();
    TestBTree<int, 5> test;
    test.RunAllTests();
    // TwoThreeTree<int> tree;
    // tree.Insert(10);
    // tree.Insert(20);
    // tree.Insert(30);
    // std::cout << tree.root->KeysQuantity() << std::endl;
    return 0;
}