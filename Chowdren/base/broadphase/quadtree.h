#ifndef CHOWDREN_QUADTREE_H
#define CHOWDREN_QUADTREE_H

#include <vector>
#include "mathcommon.h"
#include "broadphase/growablestack.h"

#define MAX_TREE_LEVEL 5
// change this when MAX_TREE_LEVEL changes
#define QUADTREE_SIZE (4 + 4*4 + 4*4*4 + 4*4*4*4 + 4*4*4*4*4)

class QuadTreeItem
{
public:
    void * data;
    int mark;
};

typedef std::vector<QuadTreeItem> QuadTreeItems;
typedef std::vector<unsigned int> ProxyItems;

class QuadTree;

class QuadTreeNode
{
public:
    int x1, y1, x2, y2;
    int level;

    QuadTreeNode * a;
    QuadTreeNode * b;
    QuadTreeNode * c;
    QuadTreeNode * d;

    ProxyItems * items;

    int children;

    QuadTreeNode();
    ~QuadTreeNode();
    void add(unsigned int item, int v[4]);
    void clear();
    void remove(unsigned int item);
    ProxyItems & get_items();
};

class QuadTree : public QuadTreeNode
{
public:
    int current_node;
    QuadTreeNode nodes[QUADTREE_SIZE];
    QuadTreeItems store;
    int query_id;

    QuadTree(int x1, int y1, int x2, int y2);
    QuadTree();
    void init(int x1, int y1, int x2, int y2);
    unsigned int add(void * data, int v[4]);
    void move(unsigned int item, int v[4]);
    template <typename T>
    bool query(int v[4], T & callback);
    void remove(unsigned int item);
    QuadTreeNode * create_node(int x1, int y1, int x2, int y2, int level);
};

inline bool collides_tree(int v[4],
                          QuadTreeNode * tree)
{
    return collides(v[0], v[1], v[2], v[3],
                    tree->x1, tree->y1, tree->x2, tree->y2);
}

template <typename T>
bool QuadTree::query(int aabb[4], T & callback)
{
    if (children == 0)
        return true;
    query_id++;
    GrowableStack<QuadTreeNode*, 256> stack;
    stack.Push(this);
    while (stack.GetCount() > 0) {
        QuadTreeNode * node = stack.Pop();
        if (node->children == 0)
            continue;
        if (node->level == MAX_TREE_LEVEL) {
            ProxyItems::iterator it;
            ProxyItems & v = node->get_items();
            for (it = v.begin(); it != v.end(); it++) {
                QuadTreeItem & item = store[*it];
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
        if (collides_tree(aabb, node->a))
            stack.Push(node->a);
        if (collides_tree(aabb, node->b))
            stack.Push(node->b);
        if (collides_tree(aabb, node->c))
            stack.Push(node->c);
        if (collides_tree(aabb, node->d))
            stack.Push(node->d);
    }
    return true;
}

#endif // CHOWDREN_QUADTREE_H
