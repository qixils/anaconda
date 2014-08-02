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
        grid[i].clear();
    }
}

int UniformGrid::add(void * data, int v[4])
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

    for (int x = item.box[0]; x < item.box[2]; x++)
    for (int y = item.box[1]; y < item.box[3]; y++) {
        grid[GRID_INDEX(x, y)].push_back(index);
    }

    return index;
}

inline void remove_proxy(GridItemList & list, int proxy)
{
    GridItemList::reverse_iterator it;
    for (it = list.rbegin(); it != list.rend(); it++) {
        if (*it != proxy)
            continue;
        list.erase(--(it.base()));
        break;
    }
}

void UniformGrid::remove(int proxy)
{
    GridItem & item = store[proxy];

    for (int x = item.box[0]; x < item.box[2]; x++)
    for (int y = item.box[1]; y < item.box[3]; y++) {
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
    for (int x = item.box[0]; x < item.box[2]; x++)
    for (int y = item.box[1]; y < item.box[3]; y++) {
        if (overlaps(x, y, box))
            continue;
        remove_proxy(grid[GRID_INDEX(x, y)], proxy);
    }

    // add to lists the proxy is entering
    for (int x = box[0]; x < box[2]; x++)
    for (int y = box[1]; y < box[3]; y++) {
        if (overlaps(x, y, item.box))
            continue;
        grid[GRID_INDEX(x, y)].push_back(proxy);
    }

    item.box[0] = box[0];
    item.box[1] = box[1];
    item.box[2] = box[2];
    item.box[3] = box[3];
}
