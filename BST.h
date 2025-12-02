#ifndef BST_H
#define BST_H

// ================== Recursive BST ==================
// Generic Binary Search Tree (BST) template
// K - key type, must support comparison operators (<, ==)
// V - value type (data stored in each node)

template <typename K, typename V>
class BST
{
    // ----- Internal Node structure -----
    struct Node
    {
        K key;       // key used for ordering
        V val;       // associated value (payload)
        Node *left;  // pointer to left child (keys smaller than this node)
        Node *right; // pointer to right child (keys larger than this node)

        // Constructor initializes node with given key-value pair
        Node(const K &k, const V &v)
            : key(k), val(v), left(nullptr), right(nullptr) {}
    };

    Node *root = nullptr; // root pointer for the BST

public:
    int comparisons = 0; // counts number of comparisons made (for performance analysis)

    // ----- Destructor -----
    // Ensures all dynamically allocated nodes are freed
    ~BST() { clear(root); }

    // ----- Public wrapper: Insert -----
    // Inserts (key, value) into the BST
    // Returns true if inserted successfully, false if key already exists
    bool insert(const K &k, const V &v)
    {
        return insertRec(root, k, v);
    }

    // ----- Public wrapper: Find -----
    // Returns a pointer to the value associated with the key
    // or nullptr if key is not found
    V *find(const K &k)
    {
        return findRec(root, k);
    }

    // ----- Public wrapper: Erase -----
    // Removes a node by key if it exists
    // Returns true if a node was deleted, false otherwise
    bool erase(const K &k)
    {
        bool erased = false;
        root = eraseRec(root, k, erased);
        return erased;
    }

    // ----- Public wrapper: Range Apply -----
    // Applies a function `fn(key, value)` to all nodes with keys in [lo, hi]
    template <typename Fn>
    void rangeApply(const K &lo, const K &hi, Fn fn)
    {
        rangeRec(root, lo, hi, fn);
    }

    // ----- Resets the comparison counter -----
    void resetMetrics() { comparisons = 0; }

private:
    // ----- Helper: Clear -----
    // Recursively deletes all nodes in the tree (postorder traversal)
    void clear(Node *n)
    {
        if (!n)
            return;
        clear(n->left);
        clear(n->right);
        delete n;
    }

    // ----- Recursive Insert -----
    // Inserts a key-value pair into subtree rooted at n
    bool insertRec(Node *&n, const K &k, const V &v)
    {
        if (!n)
        {
            n = new Node(k, v); // base case: found empty spot
            return true;
        }
        ++comparisons;
        if (k == n->key)
            return false; // duplicate key not allowed
        ++comparisons;
        if (k < n->key)
            return insertRec(n->left, k, v); // recurse left
        else
            return insertRec(n->right, k, v); // recurse right
    }

    // ----- Recursive Find -----
    // Searches for key in subtree rooted at n
    // Returns pointer to value or nullptr if not found
    V *findRec(Node *n, const K &k)
    {
        if (!n)
            return nullptr; // base case: not found
        ++comparisons;
        if (k == n->key)
            return &n->val; // found it
        ++comparisons;
        if (k < n->key)
            return findRec(n->left, k); // search left
        else
            return findRec(n->right, k); // search right
    }

    // ----- Recursive Erase -----
    // Removes a node with given key from subtree rooted at n
    // Returns new subtree root after deletion
    Node *eraseRec(Node *n, const K &k, bool &erased)
    {
        if (!n)
            return nullptr;

        ++comparisons;
        if (k < n->key)
            n->left = eraseRec(n->left, k, erased); // go left
        else if (n->key < k)
            n->right = eraseRec(n->right, k, erased); // go right
        else
        {
            // Found node to delete
            erased = true;

            // Case 1: no left child
            if (!n->left)
            {
                Node *r = n->right;
                delete n;
                return r;
            }
            // Case 2: no right child
            else if (!n->right)
            {
                Node *l = n->left;
                delete n;
                return l;
            }
            // Case 3: two children → replace with inorder successor
            Node *succ = minNode(n->right); // smallest in right subtree
            n->key = succ->key;
            n->val = succ->val;
            n->right = eraseRec(n->right, succ->key, erased);
        }
        return n;
    }

    // ----- Find Minimum Node -----
    // Returns pointer to node with smallest key in subtree
    Node *minNode(Node *n)
    {
        return n->left ? minNode(n->left) : n;
    }

    // ----- Recursive Range Traversal -----
    // Applies fn(key, value) to all nodes with lo <= key <= hi
    template <typename Fn>
    void rangeRec(Node *n, const K &lo, const K &hi, Fn fn)
    {
        if (!n)
            return;

        ++comparisons;
        if (lo < n->key)
            rangeRec(n->left, lo, hi, fn); // explore left if possible

        ++comparisons;
        if (!(n->key < lo) && !(hi < n->key))
            fn(n->key, n->val); // apply function in range

        ++comparisons;
        if (n->key < hi)
            rangeRec(n->right, lo, hi, fn); // explore right if possible
    }
};

#endif
