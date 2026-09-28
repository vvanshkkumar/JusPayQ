#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std;

struct Node {
    string name;

    Node* parent;
    vector<Node*> children;

    bool isLocked;
    int lockedBy;

    int lockedDescendantCount;

    unordered_map<int, unordered_set<Node*>> lockedByUser;

    Node(const string& name)
        : name(name),
          parent(nullptr),
          isLocked(false),
          lockedBy(-1),
          lockedDescendantCount(0) {}
};

class Tree {
private:
    Node* root;

    unordered_map<string, Node*> nodeMap;

public:

    Tree(const vector<string>& names, int m) {
        if (names.empty()) {
            root = nullptr;
            return;
        }

        vector<Node*> nodes;

        for (const string& name : names) {
            Node* node = new Node(name);
            nodes.push_back(node);
            nodeMap[name] = node;
        }

        root = nodes[0];

        for (int i = 1; i < (int)nodes.size(); i++) {
            int parentIndex = (i - 1) / m;

            Node* parent = nodes[parentIndex];
            Node* child = nodes[i];

            child->parent = parent;
            parent->children.push_back(child);
        }
    }

    ~Tree() {
        for (auto& entry : nodeMap) {
            delete entry.second;
        }
    }

    
    Node* getNode(const string& name) {
        auto it = nodeMap.find(name);

        if (it == nodeMap.end()) {
            return nullptr;
        }

        return it->second;
    }



    bool lock(const string& name, int userId) {

        Node* node = getNode(name);

        if (node == nullptr) {
            return false;
        }

        // Condition 1:
        // Node itself must not already be locked.
        if (node->isLocked) {
            return false;
        }

        // Condition 2:
        // No locked descendant is allowed.
        if (node->lockedDescendantCount > 0) {
            return false;
        }

        // Condition 3:
        // No ancestor can be locked.
        Node* current = node->parent;

        while (current != nullptr) {

            if (current->isLocked) {
                return false;
            }

            current = current->parent;
        }

        // -----------------------------------------------------
        // Actual lock
        // -----------------------------------------------------

        node->isLocked = true;
        node->lockedBy = userId;

        current = node->parent;

        while (current != nullptr) {

            current->lockedDescendantCount++;

            current->lockedByUser[userId].insert(node);

            current = current->parent;
        }

        return true;
    }

    // ---------------------------------------------------------
    // UNLOCK
    // ---------------------------------------------------------

    bool unlock(const string& name, int userId) {

        Node* node = getNode(name);

        if (node == nullptr) {
            return false;
        }

        // Node must actually be locked.
        if (!node->isLocked) {
            return false;
        }

        // Only the owner can unlock.
        if (node->lockedBy != userId) {
            return false;
        }

        // -----------------------------------------------------
        // Actual unlock
        // -----------------------------------------------------

        node->isLocked = false;
        node->lockedBy = -1;

        // Update every ancestor.
        Node* current = node->parent;

        while (current != nullptr) {

            current->lockedDescendantCount--;

            // Remove this node from this user's set.
            auto userIt = current->lockedByUser.find(userId);

            if (userIt != current->lockedByUser.end()) {

                userIt->second.erase(node);

                // Very important:
                //
                // If the user has no more locked descendants
                // under this ancestor, remove the user entry.
                if (userIt->second.empty()) {
                    current->lockedByUser.erase(userIt);
                }
            }

            current = current->parent;
        }

        return true;
    }

    // ---------------------------------------------------------
    // UPGRADE LOCK
    // ---------------------------------------------------------

    bool upgradeLock(const string& name, int userId) {

        Node* node = getNode(name);

        if (node == nullptr) {
            return false;
        }

        // Condition 1:
        // Target itself must not be locked.
        if (node->isLocked) {
            return false;
        }

        // Condition 2:
        // There must be at least one locked descendant.
        if (node->lockedDescendantCount == 0) {
            return false;
        }

        // Condition 3:
        // No ancestor can be locked.
        Node* current = node->parent;

        while (current != nullptr) {

            if (current->isLocked) {
                return false;
            }

            current = current->parent;
        }

        // -----------------------------------------------------
        // Find who owns the locked descendants.
        // -----------------------------------------------------

        //
        // If there is more than one user:
        //
        //     user 1 -> {A, B}
        //     user 2 -> {C}
        //
        // Upgrade is impossible.
        //
        if (node->lockedByUser.size() != 1) {
            return false;
        }

        // There is exactly one user.
        auto it = node->lockedByUser.begin();

        int owner = it->first;

        // All locked descendants must belong to userId.
        if (owner != userId) {
            return false;
        }

        // We now have the actual locked descendant nodes.
        unordered_set<Node*>& lockedNodes = it->second;

        // -----------------------------------------------------
        // Copy nodes before modifying the sets.
        // -----------------------------------------------------

        vector<Node*> nodesToUnlock;

        for (Node* lockedNode : lockedNodes) {
            nodesToUnlock.push_back(lockedNode);
        }

        // -----------------------------------------------------
        // Unlock all locked descendants.
        // -----------------------------------------------------

        for (Node* lockedNode : nodesToUnlock) {

            // We already know these nodes belong to userId.
            //
            // Calling unlock() will:
            //   1. remove the lock
            //   2. decrement ancestor counts
            //   3. remove node from lockedByUser sets
            unlock(lockedNode->name, userId);
        }

        // -----------------------------------------------------
        // Finally lock the target node.
        // -----------------------------------------------------

        return lock(name, userId);
    }
};


// =============================================================
// MAIN
// =============================================================

int main() {

    int N, m, Q;

    cin >> N;
    cin >> m;
    cin >> Q;

    vector<string> names(N);

    for (int i = 0; i < N; i++) {
        cin >> names[i];
    }

    Tree tree(names, m);

    /*
        Query format:

        1 nodeName userId
            -> lock

        2 nodeName userId
            -> unlock

        3 nodeName userId
            -> upgradeLock
    */

    for (int i = 0; i < Q; i++) {

        int operation;
        string nodeName;
        int userId;

        cin >> operation >> nodeName >> userId;

        bool result = false;

        if (operation == 1) {
            result = tree.lock(nodeName, userId);
        }
        else if (operation == 2) {
            result = tree.unlock(nodeName, userId);
        }
        else if (operation == 3) {
            result = tree.upgradeLock(nodeName, userId);
        }

        cout << (result ? "true" : "false") << '\n';
    }

    return 0;
}