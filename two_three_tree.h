#ifndef MY_TWO_THREE_TREE
#define MY_TWO_THREE_TREE

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
            os << "], children: " << n.childs.size() << ")" << std::endl;
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
        if (root->KeysQuantity() == 3) {
            std::unique_ptr<Node> new_root = std::make_unique<Node>();
            new_root->AddChild(std::move(root));
            root = std::move(new_root);
            SplitChild(root.get(), 0);
        }
    }
    bool Find(T key) {
        if (root == nullptr) {
            return false;
        }
        return RecursiveFind(root.get(), key);
    }
    void Delete(T key) {
        Node* node_with_key = GetNodeWithKey(key);

        T lower_bound_key = PopKeyLowerBound(key);
        Node* parent_of_child_with_key = GetParentOfChildWithKey(key);

        return;
    }
private:
    std::size_t FindChildIdx(Node* node, T key) {
        std::size_t child_idx = 0;
        while (child_idx < node->KeysQuantity() && key > node->keys[child_idx]) {
            ++child_idx;
        }
        return child_idx;
    }
    Node* GetNodeWithKey(T key) {
        return new Node(0);
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
        for (T key_ : node->keys) {
            if (key_ == key) {
                return true;
            }
        }

        if (node->IsLeaf()) {
            return false;
        }

        std::size_t child_idx = FindChildIdx(node, key);
        return RecursiveFind(node->childs[child_idx].get(), key);
    }
    void RecursiveDelete(Node* node, T key) {
        
    }
    void MergeChild(Node* node, size_t child_idx) {
        return;
    }
};
#endif
