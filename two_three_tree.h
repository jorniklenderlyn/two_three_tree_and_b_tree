#ifndef MY_TWO_THREE_TREE
#define MY_TWO_THREE_TREE
#ifdef ENABLE_LOGGING
    #include <iostream>
    #define LOG_DEBUG(msg) do { std::cerr << "[DEBUG] " << msg << std::endl; } while(0)
    #define LOG_DEBUG_EXPR(expr) do { std::cerr << "[DEBUG] " << #expr << " = " << (expr) << std::endl; } while(0)
#else
    #define LOG_DEBUG(msg) do {} while(0)
    #define LOG_DEBUG_EXPR(expr) do {} while(0)
#endif

#include<iostream>
#include<vector>
#include<algorithm>
#include<memory>


template <typename T>
class TwoThreeTree {
public:
    struct Node {
        std::vector<T> keys;
        std::vector<std::unique_ptr<Node> > childs;
        Node() = default;
        Node(T key) {
            keys.push_back(key);
        }
        Node(T key, std::unique_ptr<Node> left_child, std::unique_ptr<Node> right_child) {
            keys.push_back(key);
            childs.push_back(left_child);
            childs.push_back(right_child);
        }
        void InsertKey(T key) {
            int i = static_cast<int>(keys.size()) - 1;
            keys.push_back(key); // make space
            while (i >= 0 && keys[i] > key) {
                keys[i + 1] = keys[i];
                --i;
            }
            keys[i + 1] = key;
        }
        void DeleteKey(const T& key) {
            auto it = std::find(keys.begin(), keys.end(), key);
            if (it != keys.end()) {
                keys.erase(it);
            }
        }
        void AddChild(std::unique_ptr<Node> child) {
            childs.push_back(std::move(child));
        }
        void AddChild(typename std::vector<std::unique_ptr<Node> >::iterator pos, std::unique_ptr<Node> child) {
            childs.insert(pos, std::move(child));
        }
        void DeleteChild(std::size_t idx) {
            childs.erase(childs.begin() + idx);
        }
        bool HasKey(T key) {
            for (T key_ : keys) {
                if (key_ == key) {
                    return true;
                }
            }
            return false;
        }
        bool Is2Node() {
            return keys.size() == 1;
        }
        bool Is3Node() {
            return keys.size() == 2;
        }
        bool IsLeaf() {
            return childs.size() == 0;
        }
        std::size_t KeysQuantity() {
            return keys.size();
        }

        friend std::ostream& operator<<(std::ostream& os, const Node& n) {
            os << "Node(keys: [";
            for (size_t i = 0; i < n.keys.size(); ++i) {
                if (i > 0) os << ", ";
                os << n.keys[i];
            }
            os << "], children: " << n.childs.size() << ")";
            return os;
        }
        void Print() {
            std::cout << "Node(keys: [";
            for (size_t i = 0; i < keys.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << keys[i];
            }
            std::cout << "], children: " << childs.size() << ")" << std::endl;
        }
    };
    std::unique_ptr<Node> root;
public:
    void FixRootOverflow() {
        if (root->KeysQuantity() == 3) {
            std::unique_ptr<Node> new_root = std::make_unique<Node>();
            new_root->AddChild(std::move(root));
            root = std::move(new_root);
            SplitChild(root.get(), 0);
        }
    }
    void Insert(T key) {
        // no duplicates
        if (Find(key)) {
            return;
        }
        if (root == nullptr) {
            root = std::make_unique<Node>(key);
            return;
        }
        RecursiveInsert(root.get(), key);
        FixRootOverflow();
    }
    bool Find(T key) {
        if (root == nullptr) {
            return false;
        }
        return RecursiveFind(root.get(), key);
    }
    void Delete(T key) {
        LOG_DEBUG("Attempt to delete key: " << key);
        if (!Find(key)) {
            return;
        }
        RecursiveDelete(root.get(), key);

        if (root.get()->KeysQuantity() == 0) {
            if (root.get()->childs.size() == 0) {
                root = nullptr;
            } else {
                root = std::move(root.get()->childs[0]);
            }
        }
        FixRootOverflow();
    }
    // void PrintTree() const {
    //     if (!root) {
    //         std::cout << "(empty tree)" << std::endl;
    //         return;
    //     }
    //     PrintTreeRecursive(root.get(), 0);
    // }
        void PrintTreeLevels() const {
        if (!root) {
            std::cout << "(empty tree)" << std::endl;
            return;
        }

        std::vector<const Node*> current_level = {root.get()};
        int level = 0;

        while (!current_level.empty()) {
            std::vector<const Node*> next_level;
            std::cout << "Level " << level << ": ";

            // Print all nodes on this level
            for (const auto* node : current_level) {
                std::cout << "[";
                for (size_t i = 0; i < node->keys.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << node->keys[i];
                }
                std::cout << "]  ";

                // Collect children for next level
                for (const auto& child : node->childs) {
                    if (child) next_level.push_back(child.get());
                }
            }
            std::cout << std::endl;

            current_level = std::move(next_level);
            ++level;
        }
    }
private:
    std::size_t FindChildIdx(Node* node, T key) {
        std::size_t child_idx = 0;
        while (child_idx < node->KeysQuantity() && key > node->keys[child_idx]) {
            ++child_idx;
        }
        return child_idx;
    }
    void RecursiveInsert(Node* node, T key) {
        if (node->IsLeaf()) {
            node->InsertKey(key);
        } else {
            std::size_t child_idx = FindChildIdx(node, key);
            RecursiveInsert(node->childs[child_idx].get(), key);
            SplitChild(node, child_idx);
        }
    }
    void SplitChild(Node* node, size_t child_idx) {
        if (node->childs.size() > child_idx) {
            LOG_DEBUG("SRT_SPLITING: " << *node << " with child(" << child_idx << "): " << *(node->childs[child_idx].get()));
        } else {
            LOG_DEBUG("SRT_SPLITING: " << *node << " without childs");
            return;
        }
        // PrintTreeLevels();
        
        auto& child_ptr = node->childs[child_idx];
        Node* child_raw = child_ptr.get(); 

        if (child_raw->KeysQuantity() != 3) {
            return;
        }

        T mid_key = child_raw->keys[1];
        node->InsertKey(mid_key);

        auto left  = std::make_unique<Node>(child_raw->keys[0]);
        auto right = std::make_unique<Node>(child_raw->keys[2]);

        if (!child_raw->IsLeaf()) {
            left->AddChild(std::move(child_ptr->childs[0]));
            left->AddChild(std::move(child_ptr->childs[1]));
            right->AddChild(std::move(child_ptr->childs[2]));
            right->AddChild(std::move(child_ptr->childs[3]));
        }

        node->childs[child_idx] = std::move(left); 
        node->childs.insert(
            node->childs.begin() + child_idx + 1,
            std::move(right)
        );
    }
    bool RecursiveFind(Node* node, T key) {
        if (node->HasKey(key)) {
            return true;
        }

        if (node->IsLeaf()) {
            return false;
        }

        std::size_t child_idx = FindChildIdx(node, key);
        return RecursiveFind(node->childs[child_idx].get(), key);
    }
    void RecursiveDelete(Node* node, T key) {
        if (!node->IsLeaf()) {
            LOG_DEBUG("BEFORE:");
            LOG_DEBUG(node);
            LOG_DEBUG(*node);
            LOG_DEBUG("Childs:");
            for (size_t i = 0; i < node->childs.size(); ++i) {
                LOG_DEBUG(node->childs[i].get());
                LOG_DEBUG(*(node->childs[i].get()));
            }
            LOG_DEBUG("END_BEFORE");
        }
        size_t child_idx = FindChildIdx(node, key);
        if (node->HasKey(key)) {
            if (node->IsLeaf()) {
                node->DeleteKey(key);
                return;
            } else {
                T changing_key;
                Node* changing_key_subtree;
                if (node->keys[0] == key) {
                    child_idx = 0;
                    changing_key = FindMaximalKey(node->childs[0].get());
                    changing_key_subtree = node->childs[0].get();
                } else {
                    child_idx = 2;
                    changing_key = FindMinimalKey(node->childs[2].get());
                    changing_key_subtree = node->childs[2].get();
                }
                node->DeleteKey(key);
                node->InsertKey(changing_key);
                // child_idx = FindChildIdx(node, changing_key);
                RecursiveDelete(changing_key_subtree, changing_key);
            }
        } else {
            RecursiveDelete(node->childs[child_idx].get(), key);
        }
        if (!node->IsLeaf()) {
            MergeChild(node, child_idx);
            LOG_DEBUG("AFTER:\n" << node << ' ' << *node);
            LOG_DEBUG("childs: \n");
            for (size_t i = 0; i < node->childs.size(); ++i) {
                LOG_DEBUG("" << node->childs[i].get() << ' ' << *(node->childs[i].get()));
            }
            LOG_DEBUG("END_AFTER");
            SplitChild(node, child_idx);
        }
        
    }
    void MergeChild(Node* node, size_t child_idx) {
        LOG_DEBUG("SRT_MERGING:");
        LOG_DEBUG(node);
        LOG_DEBUG(*node);
        LOG_DEBUG("child_idx: " << child_idx);
        for (size_t i = 0; i < node->childs.size(); ++i) {
            LOG_DEBUG("" << node->childs[i].get() << ' ' << *(node->childs[i].get()));
        }
        Node* child = node->childs[child_idx].get();
        if (child->KeysQuantity() == 0) {
            if (!child->IsLeaf()) {
                size_t brother_idx;
                Node* brother = nullptr;
                if (child_idx == 0) {
                    brother_idx = child_idx + 1;
                    brother = node->childs[brother_idx].get();
                    for (size_t i = 0; i < child->childs.size(); ++i) {
                        brother->AddChild(brother->childs.begin() + i, std::move(child->childs[i]));
                    }
                } else {
                    brother_idx = child_idx - 1;
                    brother = node->childs[brother_idx].get();
                    for (size_t i = 0; i < child->childs.size(); ++i) {
                        brother->AddChild(brother->childs.end(), std::move(child->childs[i]));
                    }
                }
                LOG_DEBUG("brother(idx_" << brother_idx << "): " << brother << ' ' << *brother);
                LOG_DEBUG("node: " << node << ' ' << *node);
                if (child_idx < 2) {
                    LOG_DEBUG("Taken key fron parent: " << node->keys[0]);
                    brother->InsertKey(node->keys[0]);
                    node->DeleteKey(node->keys[0]);
                } else {
                    LOG_DEBUG("Taken key fron parent: " << node->keys[1]);
                    brother->InsertKey(node->keys[1]);
                    node->DeleteKey(node->keys[1]);
                }
                node->DeleteChild(child_idx);
                if (child_idx < brother_idx) --brother_idx;
                SplitChild(node, brother_idx);
            } else {
                node->DeleteChild(child_idx);
            }
        }
        if (node->KeysQuantity() == node->childs.size()) {
            Node* first_child = node->childs[0].get();
            T first_key = node->keys[0];
            if (node->keys.size() > 1 && first_child->keys[first_child->KeysQuantity() - 1] < first_key) {
                Node* second_child = node->childs[1].get();
                T second_key = node->keys[1];
                second_child->InsertKey(second_key);
                node->DeleteKey(second_key);
                SplitChild(node, 1);
            } else {
                first_child->InsertKey(first_key);
                node->DeleteKey(first_key);
                SplitChild(node, 0);
            } 
        }
        LOG_DEBUG("END_MERGING");
    }
    T FindMaximalKey(Node* node) {
        if (node->IsLeaf()) {
            return node->keys[node->keys.size() - 1];
        } else {
            return FindMaximalKey(node->childs[node->childs.size() - 1].get());
        }
    }
    T FindMinimalKey(Node* node) {
        if (node->IsLeaf()) {
            return node->keys[0];
        } else {
            return FindMinimalKey(node->childs[0].get());
        }
    }

};
#endif
