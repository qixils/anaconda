#include "coltree.h"
#include <algorithm>
#include "collision/dynamictree.cpp"

// StaticTreeNode

StaticTreeNode::StaticTreeNode(int x1, int y1, int x2, int y2, int level)
: x1(x1), y1(y1), x2(x2), y2(y2), level(level), children(0), items(NULL)
{
    if (level == MAX_TREE_LEVEL)
        return;

    int xx = (x1 + x2) / 2;
    int yy = (y1 + y2) / 2;
    int new_level = level+1;

    a = new StaticTreeNode(xx, y1, x2, yy, new_level);
    b = new StaticTreeNode(x1, y1, xx, yy, new_level);
    c = new StaticTreeNode(x1, yy, xx, y2, new_level);
    d = new StaticTreeNode(xx, yy, x2, y2, new_level);
}

StaticTreeNode::~StaticTreeNode()
{
    if (level == MAX_TREE_LEVEL) {
        if (items == NULL)
            return;
        delete items;
        return;
    }
    delete a;
    delete b;
    delete c;
    delete d;
}

void StaticTreeNode::add(unsigned int item, int x1, int y1, int x2, int y2)
{
    children++;
    if (level == MAX_TREE_LEVEL) {
        ProxyItems & v = get_items();
        v.push_back(item);
        return;
    }
    if (collides_tree(x1, y1, x2, y2, a))
        a->add(item, x1, y1, x2, y2);
    if (collides_tree(x1, y1, x2, y2, b))
        b->add(item, x1, y1, x2, y2);
    if (collides_tree(x1, y1, x2, y2, c))
        c->add(item, x1, y1, x2, y2);
    if (collides_tree(x1, y1, x2, y2, d))
        d->add(item, x1, y1, x2, y2);
}

void StaticTreeNode::remove(unsigned int item)
{
    if (children == 0)
        return;
    if (level == MAX_TREE_LEVEL) {
        ProxyItems & v = get_items();
        v.erase(std::remove(v.begin(), v.end(), item), v.end());
    } else {
        a->remove(item);
        b->remove(item);
        c->remove(item);
        d->remove(item);
    }
}

ProxyItems & StaticTreeNode::get_items()
{
    if (items == NULL)
        items = new ProxyItems;
    return *items;
}

void StaticTreeNode::clear()
{
    children = 0;
    if (level == MAX_TREE_LEVEL) {
        get_items().clear();
        return;
    }
    a->clear();
    b->clear();
    c->clear();
    d->clear();
}

// StaticTree

StaticTree::StaticTree(int x1, int y1, int x2, int y2)
: StaticTreeNode(x1, y1, x2, y2, 0), query_id(0)
{
}

unsigned int StaticTree::add(void * data, int x1, int y1, int x2, int y2)
{
    intersect(x1, y1, x2, y2, this->x1, this->y1, this->x2, this->y2,
              x1, y1, x2, y2);

    TreeItems::iterator it;
    int index = 0;
    for (it = store.begin(); it != store.end(); it++) {
        TreeItem & item = *it;
        if (item.data == NULL) {
            item.data = data;
            StaticTreeNode::add(index, x1, y1, x2, y2);
            return index;
        }
        index++;
    }

    index = store.size();
    store.resize(index + 1);
    TreeItem & item = store[index];
    item.data = data;
    item.mark = query_id;
    StaticTreeNode::add(index, x1, y1, x2, y2);
    return index;
}

void StaticTree::remove(unsigned int item)
{
    store[item].data = NULL;
    StaticTreeNode::remove(item);
}
