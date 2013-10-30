#ifndef CHOWDREN_COLTREE_H
#define CHOWDREN_COLTREE_H

#include <vector>

// #define USE_COL_TREE
#define MAX_TREE_LEVEL 5

class CollisionTree;

class TreeItem
{
public:
    TreeItem(void * data);

    void * data;
    bool marked;
};

typedef std::vector<TreeItem*> TreeItems;

class CollisionTree
{
public:
    int x1, y1, x2, y2;
    int level, max_level;

    CollisionTree * a;
    CollisionTree * b;
    CollisionTree * c;
    CollisionTree * d;

    TreeItems * items;

    int children;

    CollisionTree(int x1, int y1, int x2, int y2, 
        int level, int max_level);
    ~CollisionTree();
    TreeItems & get_items();
    void add(TreeItem * item, int x1, int y1, int x2, int y2);
    void get(int x1, int y1, int x2, int y2, TreeItems & items);
    void clear();
    void remove(TreeItem * item);
};

#endif // CHOWDREN_COLTREE_H
