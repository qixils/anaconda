#ifndef CHOWDREN_COLTREE_H
#define CHOWDREN_COLTREE_H

#include <vector>
#include "collision/growablestack.h"
#include "mathcommon.h"
#include "collision/dynamictree.h"

#define MAX_TREE_LEVEL 5

class CollisionTree;

class TreeItem
{
public:
    void * data;
    int mark;
};

typedef std::vector<TreeItem> TreeItems;
typedef std::vector<unsigned int> ProxyItems;

class StaticTreeNode
{
public:
    int x1, y1, x2, y2;
    int level;

    StaticTreeNode * a;
    StaticTreeNode * b;
    StaticTreeNode * c;
    StaticTreeNode * d;

    ProxyItems * items;

    int children;

    StaticTreeNode(int x1, int y1, int x2, int y2, int level);
    ~StaticTreeNode();
    void add(unsigned int item, int x1, int y1, int x2, int y2);
    void clear();
    void remove(unsigned int item);
    ProxyItems & get_items();
};

class StaticTree : public StaticTreeNode
{
public:
    TreeItems store;
    int query_id;

    StaticTree(int x1, int y1, int x2, int y2);
    unsigned int add(void * data, int x1, int y1, int x2, int y2);
    template <typename T>
    bool query(int x1, int y1, int x2, int y2, T & callback);
    void remove(unsigned int item);
};

inline bool collides_tree(int x1, int y1, int x2, int y2,
                          StaticTreeNode * tree)
{
    return collides(x1, y1, x2, y2, tree->x1, tree->y1, tree->x2, tree->y2);
}

template <typename T>
bool StaticTree::query(int x1, int y1, int x2, int y2, T & callback)
{
    if (children == 0)
        return true;
    query_id++;
    GrowableStack<StaticTreeNode*, 256> stack;
    stack.Push(this);
    while (stack.GetCount() > 0) {
        StaticTreeNode * node = stack.Pop();
        if (node->children == 0)
            continue;
        if (node->level == MAX_TREE_LEVEL) {
            ProxyItems::iterator it;
            ProxyItems & v = node->get_items();
            for (it = v.begin(); it != v.end(); it++) {
                TreeItem & item = store[*it];
                if (item.mark == query_id)
                    continue;
                item.mark = query_id;
                bool ret = callback.on_callback(item.data);
                if (!ret) {
                    return false;
                }
            }
            continue;
        }
        if (collides_tree(x1, y1, x2, y2, node->a))
            stack.Push(node->a);
        if (collides_tree(x1, y1, x2, y2, node->b))
            stack.Push(node->b);
        if (collides_tree(x1, y1, x2, y2, node->c))
            stack.Push(node->c);
        if (collides_tree(x1, y1, x2, y2, node->d))
            stack.Push(node->d);
    }
    return true;
}

#endif // CHOWDREN_COLTREE_H
