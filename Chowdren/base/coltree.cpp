#include "coltree.h"
#include "mathcommon.h"
#include <algorithm>

inline bool collides_tree(int x1, int y1, int x2, int y2,
                          CollisionTree * tree)
{
    return collides(x1, y1, x2, y2, tree->x1, tree->y1, tree->x2, tree->y2);
}

// TreeItem

TreeItem::TreeItem(void * data)
: data(data), marked(false)
{
}

// CollisionTree

CollisionTree::CollisionTree(int x1, int y1, int x2, int y2, int level,
                             int max_level)
: x1(x1), y1(y1), x2(x2), y2(y2), level(level), max_level(max_level),
  children(0), items(NULL)
{
    if (level == max_level)
        return;

    int xx = (x1 + x2) / 2;
    int yy = (y1 + y2) / 2;
    int new_level = level+1;

    a = new CollisionTree(xx, y1, x2, yy, new_level, max_level);
    b = new CollisionTree(x1, y1, xx, yy, new_level, max_level);
    c = new CollisionTree(x1, yy, xx, y2, new_level, max_level);
    d = new CollisionTree(xx, yy, x2, y2, new_level, max_level);
}

CollisionTree::~CollisionTree()
{
    if (level == max_level) {
        if (items != NULL) {
            delete items;
        }
    } else {
        delete a;
        delete b;
        delete c;
        delete d;
    }
}

void CollisionTree::add(TreeItem * item, int x1, int y1, int x2, int y2)
{
    children++;
    if (level == 0) {
        intersect(x1, y1, x2, y2, this->x1, this->y1, this->x2, this->y2,
                  x1, y1, x2, y2);
    }
    if (level == max_level) {
        TreeItems & v = get_items();
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

void CollisionTree::get(int x1, int y1, int x2, int y2, TreeItems & v)
{
    if (children == 0)
        return;
    TreeItems::iterator it;
    if (level == max_level) {
        TreeItems::iterator it;
        TreeItems & vv = get_items();
        for (it = vv.begin(); it != vv.end(); it++) {
            TreeItem * item = *it;
            if (item->marked)
                continue;
            item->marked = true;
            v.push_back(item);
        }
        return;
    }
    if (collides_tree(x1, y1, x2, y2, a))
        a->get(x1, y1, x2, y2, v);
    if (collides_tree(x1, y1, x2, y2, b))
        b->get(x1, y1, x2, y2, v);
    if (collides_tree(x1, y1, x2, y2, c))
        c->get(x1, y1, x2, y2, v);
    if (collides_tree(x1, y1, x2, y2, d))
        d->get(x1, y1, x2, y2, v);

    if (level == 0) {
        for (it = v.begin(); it != v.end(); it++) {
            (*it)->marked = false;
        }
    }
}

void CollisionTree::remove(TreeItem * item)
{
    if (children == 0)
        return;
    if (level == max_level) {
        TreeItems & v = get_items();
        v.erase(std::remove(v.begin(), v.end(), item), v.end());
    } else {
        a->remove(item);
        b->remove(item);
        c->remove(item);
        d->remove(item);
    }
}

TreeItems & CollisionTree::get_items()
{
    if (items == NULL)
        items = new TreeItems;
    return *items;
}

void CollisionTree::clear()
{
    children = 0;
    if (level == max_level) {
        get_items().clear();
        return;
    }
    a->clear();
    b->clear();
    c->clear();
    d->clear();
}
