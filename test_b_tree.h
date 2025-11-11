#ifndef MY_TEST_BTREE
#define MY_TEST_BTREE

#include <iostream>
#include <cassert>
#include <vector>
#include <random>
#include <climits>
#include "b_tree.h" // Assumes template: BTree<KeyType, Order>

template<typename KeyType, int Order>
class TestBTree {
private:
    using Node = typename BTree<KeyType, Order>::Node;

    // Helper: recursively validate B-tree invariants
    bool ValidateNode(const Node* node, const KeyType& min_val, const KeyType& max_val) {
        if (!node) return true;

        const auto& keys = node->keys;
        const auto& childs = node->childs;

        // B-tree property: 1 <= keys.size() <= Order - 1 (except root may be empty if tree is empty)
        if (keys.empty()) return false;
        if (keys.size() > static_cast<size_t>(Order - 1)) {
            return false;
        }

        // Keys must be strictly increasing
        for (size_t i = 1; i < keys.size(); ++i) {
            if (keys[i - 1] >= keys[i]) {
                return false;
            }
        }

        // All keys must be in (min_val, max_val)
        for (const auto& key : keys) {
            if (key <= min_val || key >= max_val) {
                return false;
            }
        }

        // Leaf node: no children
        if (childs.empty()) {
            return true;
        }

        // Internal node: must have keys.size() + 1 children
        if (childs.size() != keys.size() + 1) {
            return false;
        }

        // For root with only one key, min/max are INT bounds; generalize for any KeyType if needed
        if (!ValidateNode(childs[0].get(), min_val, keys[0])) return false;
        for (size_t i = 0; i < keys.size(); ++i) {
            if (!ValidateNode(childs[i + 1].get(),
                              keys[i],
                              (i + 1 < keys.size()) ? keys[i + 1] : max_val)) {
                return false;
            }
        }

        return true;
    }

    bool IsValidTree(const BTree<KeyType, Order>& tree) {
        if (!tree.root) return true;

        // Use min/max for KeyType if possible; fallback for int
        KeyType min_bound = std::numeric_limits<KeyType>::lowest();
        KeyType max_bound = std::numeric_limits<KeyType>::max();

        // Special case: if KeyType is int, avoid overflow in comparisons
        if constexpr (std::is_same_v<KeyType, int>) {
            min_bound = INT32_MIN;
            max_bound = INT32_MAX;
        }

        return ValidateNode(tree.root.get(), min_bound, max_bound);
    }

public:
    void TestEmptyTree() {
        BTree<int, Order> tree;
        assert(!tree.Find(42));
        assert(IsValidTree(tree));
    }

    void TestInsertBasic() {
        BTree<int, Order> tree;
        tree.Insert(10);
        assert(tree.Find(10));
        assert(!tree.Find(5));
        assert(IsValidTree(tree));
    }

    void TestInsertMaxKeysInRoot() {
        BTree<int, Order> tree;
        // Insert up to Order - 1 keys into root
        for (int i = 1; i < Order; ++i) {
            tree.Insert(i * 10);
            assert(tree.Find(i * 10));
        }
        assert(tree.root->keys.size() == static_cast<size_t>(Order - 1));
        assert(tree.root->childs.empty()); // still leaf
        assert(IsValidTree(tree));
    }

    void TestInsertCausesSplit() {
        BTree<int, Order> tree;
        // Fill root and cause split on (Order)-th insert
        for (int i = 1; i <= Order; ++i) {
            tree.Insert(i * 10);
        }

        // After split: root has 1 key, 2 children
        assert(tree.root->keys.size() == 1);
        assert(tree.root->childs.size() == 2);
        for (int i = 1; i <= Order; ++i) {
            assert(tree.Find(i * 10));
        }
        assert(IsValidTree(tree));
    }

    void TestInsertComplex() {
        BTree<int, Order> tree;
        std::vector<int> values = {6, 5, 3, 8, 2, 9, 1, 7, 4};
        for (int v : values) {
            tree.Insert(v);
            assert(tree.Find(v));
        }
        for (int v : values) {
            assert(tree.Find(v));
        }
        assert(IsValidTree(tree));
    }

    void TestInsertDuplicates() {
        BTree<int, Order> tree;
        tree.Insert(5);
        tree.Insert(5); // Should be ignored (no duplicates)
        assert(tree.Find(5));
        // Ensure size didn't change (if you track size, assert it)
        assert(IsValidTree(tree));
    }

    void TestDeleteFromLeafWithExtraKeys() {
        BTree<int, Order> tree;
        // Create a leaf 3-node (for Order=3) or larger
        for (int i = 1; i <= Order; ++i) {
            tree.Insert(i * 10);
        }
        // Now delete one key from a leaf that has > min_keys
        tree.Delete(10);
        assert(!tree.Find(10));
        assert(tree.Find(20));
        assert(IsValidTree(tree));
    }

    void TestDeleteCausesMergeOrRedistribute() {
        BTree<int, Order> tree;
        // Build a multi-level tree
        std::vector<int> vals;
        for (int i = 1; i <= 2 * Order; ++i) {
            vals.push_back(i * 5);
        }
        for (int v : vals) tree.Insert(v);

        // Delete a key that may cause underflow
        tree.PrintTreeLevels();
        tree.Delete(vals.back());
        assert(!tree.Find(vals.back()));
        assert(IsValidTree(tree));
        tree.PrintTreeLevels();

        // Delete another to potentially trigger merge
        tree.Delete(vals[vals.size() - 2]);
        tree.PrintTreeLevels();
        assert(IsValidTree(tree));
    }

    void TestDeleteInternalNode() {
        BTree<int, Order> tree;
        // Force an internal node
        for (int i = 10; i <= 50; i += 10) {
            std::cout << "Number to insert: " << i << std::endl;
            tree.Insert(i);
            tree.PrintTreeLevels();
        }

        tree.Insert(5);
        tree.Insert(55);
        tree.Insert(60);

        tree.PrintTreeLevels();

        // Now delete the root key (internal)
        tree.Delete(30);
        // Should still have other keys
        tree.PrintTreeLevels();
        assert(tree.Find(10));
        assert(tree.Find(20));
        assert(tree.Find(55));
        assert(tree.Find(5));
        assert(tree.Find(50));
        assert(tree.Find(60));
        assert(tree.Find(40));
        assert(IsValidTree(tree));
    }

    void TestDeleteShrinksTree() {
        BTree<int, Order> tree;
        for (int i = 1; i <= Order + 2; ++i) {
            tree.Insert(i);
        }
        tree.PrintTreeLevels();
        // Now delete all but one
        for (int i = 2; i <= Order + 2; ++i) {
            tree.Delete(i);
            assert(IsValidTree(tree));
        }
        assert(tree.Find(1));
        assert(tree.root->childs.empty()); // Leaf
        tree.PrintTreeLevels();
        tree.Delete(1);
        assert(!tree.Find(1));
        assert(tree.root == nullptr);
        assert(IsValidTree(tree));
    }

    void TestDeleteNonExistent() {
        BTree<int, Order> tree;
        tree.Insert(10);
        tree.Insert(20);
        tree.Delete(999); // Should do nothing
        assert(tree.Find(10));
        assert(tree.Find(20));
        assert(!tree.Find(999));
        assert(IsValidTree(tree));
    }

    void TestDeleteManyRandom() {
        const int N = 1000;
        BTree<int, Order> tree;
        std::vector<int> values;
        for (int i = 1; i <= N; ++i) {
            values.push_back(i);
            tree.Insert(i);
            assert(tree.Find(i));
        }

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(values.begin(), values.end(), g);

        // Delete half randomly
        for (int i = 0; i < N / 2; ++i) {
            tree.PrintTreeLevels();
            tree.Delete(values[i]);
            assert(!tree.Find(values[i]));
            assert(IsValidTree(tree));
        }

        // Check remaining
        for (int i = N / 2; i < N; ++i) {
            assert(tree.Find(values[i]));
        }
        assert(IsValidTree(tree));
    }

    void RunAllTests() {
        std::cout << "Running B-tree tests (Order = " << Order << ")...\n";

        TestEmptyTree();
        TestInsertBasic();
        TestInsertMaxKeysInRoot();
        TestInsertCausesSplit();
        TestInsertComplex();
        TestInsertDuplicates();

        TestDeleteFromLeafWithExtraKeys();
        std::cout << "TestDeleteFromLeafWithExtraKeys...OK\n";
        TestDeleteCausesMergeOrRedistribute();
        std::cout << "TestDeleteCausesMergeOrRedistribute...OK\n";
        TestDeleteInternalNode();
        std::cout << "TestDeleteInternalNode...OK\n";
        TestDeleteShrinksTree();
        std::cout << "TestDeleteShrinksTree...OK\n";
        TestDeleteNonExistent();
        std::cout << "TestDeleteNonExistent...OK\n";
        TestDeleteManyRandom();
        std::cout << "TestDeleteManyRandom...OK\n";

        std::cout << "✅ All B-tree tests passed!\n";
    }
};

#endif