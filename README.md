# Tree of Space — My Approach & Complexity Notes

This README documents my reasoning and approach for the **Tree of Space** locking problem.

The main idea is to maintain information about locked descendants at each node so that we **do not need to traverse an entire subtree** whenever we want to check whether a node has locked descendants.

---

## 1. Maintaining `lockedDescendantCount`

For every node, maintain:

```cpp
lockedDescendantCount
```

This stores the **number of locked nodes in its subtree**.

When a node is locked, we do **not** traverse all of its descendants and update them.

Instead, we traverse **upwards from the node to the root** and update every ancestor:

```text
        Root
         ↑
       Parent
         ↑
       Parent
         ↑
       Node
```

For every ancestor:

```cpp
ancestor->lockedDescendantCount++;
```

Since we only traverse the ancestor chain, the cost is:

```text
O(height)
```

For the level-order M-ary tree in this problem:

```text
height = O(log_m N)
```

Therefore:

```text
Updating lockedDescendantCount = O(log_m N)
```

---

## 2. Checking Locked Descendants

When trying to lock a node, we need to know:

> Does this node already have any locked descendants?

Instead of traversing the entire subtree, we simply check:

```cpp
node->lockedDescendantCount > 0
```

This is:

```text
O(1)
```

because the count is already maintained.

So:

```text
Check for locked descendants = O(1)
```

---

## 3. Checking Locked Ancestors

For a node to be locked, **none of its ancestors can be locked**.

We therefore traverse upwards:

```text
Node
 ↑
Parent
 ↑
Parent
 ↑
...
 ↑
Root
```

At every ancestor:

```cpp
if (ancestor->isLocked)
    return false;
```

So the cost is proportional to the height of the tree:

```text
O(height)
```

For this problem:

```text
O(log_m N)
```

If the tree were arbitrary/unbalanced, we would write:

```text
O(height)
```

rather than automatically saying `O(log N)`.

---

## 4. Complexity of `lock()`

When locking a node, we need to perform three main things.

### Check whether the node itself is locked

```cpp
node->isLocked
```

Cost:

```text
O(1)
```

### Check whether it has locked descendants

```cpp
node->lockedDescendantCount
```

Cost:

```text
O(1)
```

### Check ancestors and update their information

We traverse upwards to the root.

Cost:

```text
O(log_m N)
```

Therefore:

```text
lock()
    = O(1)
    + O(1)
    + O(log_m N)

    = O(log_m N)
```

---

# 5. `unlock()`

When unlocking a node, we need to update all of its ancestors.

For every ancestor we:

```cpp
ancestor->lockedDescendantCount--;
```

and also remove the unlocked node from:

```cpp
ancestor->lockedByUser[userId]
```

The ancestor traversal takes:

```text
O(log_m N)
```

The `unordered_map` / `unordered_set` operations are **O(1) average**.

Therefore:

```text
unlock() = O(log_m N)
```

---

# 6. `lockedByUser` for `upgradeLock()`

For every node, maintain:

```cpp
unordered_map<int, unordered_set<Node*>>
    lockedByUser;
```

The idea is:

```text
userId → set of locked descendants belonging to that user
```

For example:

```text
Node X

lockedByUser:

    1 → {A, B, C}
    2 → {D, E}
```

This means:

```text
User 1 has locked A, B, C
User 2 has locked D, E
```

---

# 7. Updating `lockedByUser` During `lock()`

When a node is successfully locked, we already traverse upwards through its ancestors.

While doing that, we can also insert the locked node into the corresponding user's set.

For every ancestor:

```cpp
ancestor->lockedDescendantCount++;

ancestor->lockedByUser[userId].insert(node);
```

So we're doing both pieces of bookkeeping during the **same upward traversal**.

For example, if:

```text
        Root
         |
         A
         |
         B
         |
         C   ← locked by user 1
```

then:

```text
Root.lockedByUser[1] = {C}
A.lockedByUser[1]    = {C}
B.lockedByUser[1]    = {C}
```

The insertion into an `unordered_set` is:

```text
O(1) average
```

and there are `O(log_m N)` ancestors.

Therefore the total update is still:

```text
O(log_m N)
```

on average.

---

# 8. Removing From `lockedByUser` During `unlock()`

Suppose:

```text
A.lockedByUser[1] = {C, D}
```

and we unlock `C`.

We do:

```cpp
A.lockedByUser[1].erase(C);
```

Now:

```text
A.lockedByUser[1] = {D}
```

If `D` was also unlocked and the set becomes empty:

```text
A.lockedByUser[1] = {}
```

then we should remove the user entry itself:

```cpp
A.lockedByUser.erase(1);
```

This is important because we use:

```cpp
lockedByUser.size()
```

during `upgradeLock()` to determine how many different users have locked descendants.

---

# 9. `upgradeLock()`

Suppose we want to perform:

```cpp
upgradeLock(X, userId)
```

The conditions are:

1. `X` itself must not be locked.
2. `X` must have at least one locked descendant.
3. No ancestor of `X` can be locked.
4. **All locked descendants must belong to the same user.**
5. That user must be `userId`.

---

## Using `lockedByUser` to Check Ownership

Suppose:

```text
X.lockedByUser:

    1 → {A, B, C}
```

Then:

```cpp
X.lockedByUser.size() == 1
```

and the only user is:

```cpp
X.lockedByUser.begin()->first
```

If:

```cpp
X.lockedByUser.begin()->first == userId
```

then all locked descendants belong to the requested user.

---

### If there are multiple users

For example:

```text
X.lockedByUser:

    1 → {A, B}
    2 → {C}
```

Then:

```cpp
X.lockedByUser.size() == 2
```

Therefore the upgrade must fail.

---

### If there is one user

```text
X.lockedByUser:

    1 → {A, B, C}
```

Then:

```cpp
X.lockedByUser.size() == 1
```

and if:

```cpp
X.lockedByUser.begin()->first == userId
```

we know that **all locked descendants belong to that user**.

---

# 10. Getting the Locked Descendants

The `unordered_set<Node*>` contains the actual locked nodes:

```text
X.lockedByUser[userId]

        ↓

{ A, B, C, D }
```

We copy these nodes into a vector before unlocking:

```cpp
vector<Node*> nodesToUnlock;

for (Node* node : lockedNodes) {
    nodesToUnlock.push_back(node);
}
```

We do this because `unlock()` will modify the sets while we're iterating.

---

# 11. Unlocking During Upgrade

Suppose there are `K` locked descendants:

```text
K = number of locked descendants
```

We need to unlock all `K` nodes.

Each individual `unlock()` takes:

```text
O(log_m N)
```

because it traverses from that node up to the root.

Therefore:

```text
K unlocks
=
K × O(log_m N)

=
O(K log_m N)
```

Finally, we lock `X`:

```text
O(log_m N)
```

So overall:

```text
upgradeLock()
    = O(K log_m N)
```

because the `K` unlock operations dominate.

---

# 12. Final Complexity Summary

Let:

```text
H = height of tree
K = number of locked descendants during upgrade
```

For this problem's level-order M-ary tree:

```text
H = O(log_m N)
```

| Operation | Complexity |
|---|---:|
| Check node locked | `O(1)` |
| Check locked descendants | `O(1)` |
| Check ancestors | `O(log_m N)` |
| Update ancestor `lockedDescendantCount` | `O(log_m N)` |
| `lock()` | **`O(log_m N)`** |
| `unlock()` | **`O(log_m N)`** |
| `upgradeLock()` | **`O(K log_m N)`** |

For `unordered_map` / `unordered_set`:

```text
insert  → O(1) average
erase   → O(1) average
find    → O(1) average
size    → O(1)
```

---

# 13. The Core Idea

The most important idea is:

> **Update ancestors, NOT descendants.**

When `C` is locked:

```text
                 ROOT
                  |
             lockedDescCount
             lockedByUser
                  |
                  A
                  |
             lockedDescCount
             lockedByUser
                  |
                  B
                  |
                  C  ← locked by user 1
```

We update:

```text
C
↑
B   → count++, user1.insert(C)
↑
A   → count++, user1.insert(C)
↑
ROOT → count++, user1.insert(C)
```

We **do not** traverse through every descendant.

That's what keeps `lock()` at:

```text
O(log_m N)
```

Then `upgradeLock()` uses the stored:

```text
user → {locked descendant nodes}
```

information instead of scanning the entire subtree.

---

# Complexity at a Glance

```text
                 Tree of Space
                       |
          ┌────────────┴────────────┐
          │                         │
   lockedDescendantCount       lockedByUser
          │                         │
     check subtree             find owner(s)
        O(1)                    O(1) average
          │                         │
          └────────────┬────────────┘
                       │
                traverse ancestors
                       │
                 O(log_m N)
                       │
          ┌────────────┴────────────┐
          │                         │
        lock                     unlock
   O(log_m N)                  O(log_m N)
                                  │
                                  │
                             upgrade
                                  │
                         K × O(log_m N)
                                  │
                         O(K log_m N)
```

**Core mental model:**

```text
Don't update every descendant.
Update only the ancestors.

Don't search the entire subtree for locked nodes.
Maintain the information while locking.

Don't scan to find who owns locked descendants.
Maintain:
    userId → set<Node*>
```

