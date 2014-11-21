#include "broadphase/grid.h"
#include "manager.h"
#include "frame.h"

inline int div_ceil(int x, int y)
{
    return (x + y - 1) / y;
}

UniformGrid::UniformGrid()
: query_id(0), grid(NULL)
{
    if (global_manager == NULL)
        // in case of DefaultLayer
        return;
    width = div_ceil(global_manager->frame->width, GRID_SIZE);
    height = div_ceil(global_manager->frame->height, GRID_SIZE);
    grid = new GridItemList[width*height];
}

UniformGrid::~UniformGrid()
{
    delete[] grid;
}

void UniformGrid::clear()
{
    for (int i = 0; i < width*height; i++) {
        grid[i].items.clear();
    }
}

inline void remove_proxy(GridItemList & list, int proxy)
{
    int static_count = list.static_items;
    vector<int>::iterator it;
    for (it = list.items.begin(); it != list.items.end(); ++it) {
        if (*it != proxy) {
            static_count--;
            continue;
        }
        if (static_count >= 1)
            list.static_items--;
        list.items.erase(it);
        break;
    }
}

inline void add_proxy(GridItemList & list, int proxy)
{
    list.items.push_back(proxy);
}

inline void add_static_proxy(GridItemList & list, int proxy)
{
    list.items.insert(list.items.begin() + list.static_items, proxy);
    list.static_items++;
}

int UniformGrid::add(void * data, int v[4])
{
    int index;
    if (free_list.empty()) {
        index = store.size();
        store.emplace_back();
    } else {
        index = free_list.back();
        free_list.pop_back();
    }

    GridItem & item = store[index];
    item.last_query_id = query_id;
    item.data = data;
    get_pos(v, item.box);

    for (int y = item.box[1]; y < item.box[3]; y++)
    for (int x = item.box[0]; x < item.box[2]; x++) {
        add_proxy(grid[GRID_INDEX(x, y)], index);
    }

    return index;
}

int UniformGrid::add_static(void * data, int v[4])
{
    int index;
    if (free_list.empty()) {
        index = store.size();
        store.resize(index + 1);
    } else {
        index = free_list.back();
        free_list.pop_back();
    }

    GridItem & item = store[index];
    item.last_query_id = query_id;
    item.data = data;
    get_pos(v, item.box);

    for (int y = item.box[1]; y < item.box[3]; y++)
    for (int x = item.box[0]; x < item.box[2]; x++) {
        add_static_proxy(grid[GRID_INDEX(x, y)], index);
    }

    return index;
}

void UniformGrid::remove(int proxy)
{
    GridItem & item = store[proxy];

    for (int y = item.box[1]; y < item.box[3]; y++)
    for (int x = item.box[0]; x < item.box[2]; x++) {
        remove_proxy(grid[GRID_INDEX(x, y)], proxy);
    }

    item.data = NULL;
    free_list.push_back(proxy);
}

inline bool overlaps(int x, int y, int box[4])
{
    return x >= box[0] && x < box[2] && y >= box[1] && y < box[3];
}

void UniformGrid::move(int proxy, int v[4])
{
    GridItem & item = store[proxy];
    int box[4];
    get_pos(v, box);

    if (box[0] == item.box[0] && box[1] == item.box[1] &&
        box[2] == item.box[2] && box[3] == item.box[3])
        return;

    // remove from lists the proxy is leaving
    for (int y = item.box[1]; y < item.box[3]; y++)
    for (int x = item.box[0]; x < item.box[2]; x++) {
        if (overlaps(x, y, box))
            continue;
        remove_proxy(grid[GRID_INDEX(x, y)], proxy);
    }

    // add to lists the proxy is entering
    for (int y = box[1]; y < box[3]; y++)
    for (int x = box[0]; x < box[2]; x++) {
        if (overlaps(x, y, item.box))
            continue;
        add_proxy(grid[GRID_INDEX(x, y)], proxy);
    }

    item.box[0] = box[0];
    item.box[1] = box[1];
    item.box[2] = box[2];
    item.box[3] = box[3];
}
