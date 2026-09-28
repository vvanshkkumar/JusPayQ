descendent count is updated in logN
(updating ancestor of a node, when locking)

DescendentLock in O(1).

Dont update ancestor count by traversing
all descendents.
when finding ancestorLock traverse upwards
till the root to find all counts of locked
ancestors. which will take O(logn).

In conclusion,
, when locking its O(logN) for descendent and
O(1) for ancestor('cause we are not updating every
descendent) and then when checking, its
O(1) for descendent('cause we have the count
already) and O(logN) for ancestor('cause we have
to traverse upwards till root).

if Balanced then O(logN) and if arbitrary then O(height)


Unlock Node : when unlocking it needs to update its ancestors
(i.e updating its ancestor nodes descendentCount-- and
every node's lockedByUser map, in this map erase the unlocked Node....
if map get empty delete the user entry.
takes O(logN) of time to traverse upwards.

Upgrade Lock Node : Maintain a map with userID : Set<Node*>
which contains all the descendents who are locked by a particular user.
when In lock() function while traversing upwards to update lockedDescendents,
so, in mean time ...insert start node into the current Node map as
(startNodeUser.insert(startNode))
for every traverse upwards.
when upgrading and checking for users, check the map size() and then if map.size()==1 and
map.first==userID
then traverse the set as a copy to vector to unlock all the descendents and unlock() function will be called
for this, which will automatically delete the Node from the set of that user, taking logm time.
K is the number of descendents of the node you want to upgrade and logm is the deletion and insertion time for a set.