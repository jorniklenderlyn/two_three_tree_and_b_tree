#ifndef MY_B_TREE
#define MY_B_TREE
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


template <typename T, int Order>
class BTree {
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
        if (!root) {
            return;
        }
        if (root->KeysQuantity() >= Order) {
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
        LOG_DEBUG("Key=" << key << " found");
        RecursiveDelete(root.get(), key);
        LOG_DEBUG("End of RecursiveDelete");

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

        if (child_raw->KeysQuantity() < Order) {
            return;
        }

        T mid_key = child_raw->keys[child_raw->KeysQuantity() / 2];
        node->InsertKey(mid_key);

        auto left = std::make_unique<Node>();
        for (size_t i = 0; i < child_raw->KeysQuantity() / 2; ++i) {
            left->InsertKey(child_raw->keys[i]);
        }
        auto right = std::make_unique<Node>();
        for (size_t i = child_raw->KeysQuantity() / 2 + 1; i < child_raw->KeysQuantity(); ++i) {
            right->InsertKey(child_raw->keys[i]);
        }
        if (!child_raw->IsLeaf()) {
            for (size_t i = 0; i <= child_raw->KeysQuantity() / 2; ++i) {
                left->AddChild(std::move(child_ptr->childs[i]));
            }
            for (size_t i = child_raw->KeysQuantity() / 2 + 1; i <= child_raw->KeysQuantity(); ++i) {
                right->AddChild(std::move(child_ptr->childs[i]));
            }
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
        // debug
        if (!node->IsLeaf()) {
            LOG_DEBUG("BEFORE:");
            LOG_DEBUG("Parent: " << node << ' ' << *node);
            LOG_DEBUG("Childs:");
            for (size_t i = 0; i < node->childs.size(); ++i) {
                LOG_DEBUG("    " << node->childs[i].get() << ' ' << *(node->childs[i].get()));
            }
            LOG_DEBUG("END_BEFORE");
        }

        size_t child_idx = FindChildIdx(node, key);
        if (node->HasKey(key)) {
            if (node->IsLeaf()) {
                LOG_DEBUG("Key=" << key << " deleted");
                node->DeleteKey(key);
                return;
            } else {
                T changing_key = 0;
                Node* changing_key_subtree;
                size_t key_idx = 0;
                for (; key_idx < node->KeysQuantity() && node->keys[key_idx] != key; ++key_idx); 

                
                if (node->keys[0] == key) {
                    child_idx = 0;
                    changing_key = FindMaximalKey(node->childs[child_idx].get());
                } else {
                    child_idx = key_idx + 1;
                    changing_key = FindMinimalKey(node->childs[child_idx].get());
                }
                
                changing_key_subtree = node->childs[child_idx].get();
                LOG_DEBUG("Take changing_key=" << changing_key << " from subtree(" << *changing_key_subtree << ")" << " child=" << child_idx);
                
                node->DeleteKey(key);
                node->InsertKey(changing_key);
                // child_idx = FindChildIdx(node, changing_key);
                LOG_DEBUG("Take changin_key=" << changing_key << " from child(" << child_idx << ")");
                RecursiveDelete(changing_key_subtree, changing_key);
            }
        } else {
            RecursiveDelete(node->childs[child_idx].get(), key);
        }
        if (!node->IsLeaf()) {

            MergeChild(node, child_idx);
            LOG_DEBUG("AFTER:\nParent: " << node << ' ' << *node);
            LOG_DEBUG("Childs: ");
            for (size_t i = 0; i < node->childs.size(); ++i) {
                LOG_DEBUG("    " << node->childs[i].get() << ' ' << *(node->childs[i].get()));
            }
            LOG_DEBUG("END_AFTER");
            SplitChild(node, child_idx);
        }
        
    }
    void MergeChild(Node* node, size_t child_idx) {
        Node* child = node->childs[child_idx].get();
        LOG_DEBUG("SRT_MERGING:");
        LOG_DEBUG("Parent: " << node << ' ' << *node);
        LOG_DEBUG("Child_idx: " << child_idx << " " << *child);
        for (size_t i = 0; i < node->childs.size(); ++i) {
            LOG_DEBUG("" << node->childs[i].get() << ' ' << *(node->childs[i].get()));
        }
        size_t brother_idx;
        if (child_idx == 0) {
            brother_idx = 0;
        } else {
            brother_idx = child_idx - 1;
        }
        if (child->KeysQuantity() < (Order + 1) / 2 - 1) {
            if (!child->IsLeaf()) {
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
                if (child_idx < Order - 1) {
                    LOG_DEBUG("Taken key fron parent: " << node->keys[0]);
                    brother->InsertKey(node->keys[0]);
                    node->DeleteKey(node->keys[0]);
                } else {
                    LOG_DEBUG("Taken key fron parent: " << node->keys[Order - 2]);
                    brother->InsertKey(node->keys[Order - 2]);
                    node->DeleteKey(node->keys[Order - 2]);
                }
                LOG_DEBUG("Delete child(" << child_idx << ")");
                node->DeleteChild(child_idx);
                if (child_idx < brother_idx) --brother_idx;
                SplitChild(node, brother_idx);
            } else {
                LOG_DEBUG("Delete child(" << child_idx << ")");
                node->DeleteChild(child_idx);
            }
        }
        if (node->KeysQuantity() == node->childs.size()) {
            LOG_DEBUG("Equal quantity of keys and childs");
            LOG_DEBUG("Brother: " << brother_idx);
            Node* child_for_inserting = node->childs[brother_idx].get();
            T overflow_key = node->keys[brother_idx];
            child_for_inserting->InsertKey(overflow_key);
            node->DeleteKey(overflow_key);
            SplitChild(node, brother_idx); 
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
