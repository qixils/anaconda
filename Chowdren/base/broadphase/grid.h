#ifndef CHOWDREN_GRID_H
#define CHOWDREN_GRID_H

#include "../types.h"
#include <algorithm>

struct GridItem
{
    void * data;
    int box[4];
    int last_query_id;

    GridItem()
    {
    }
};

typedef vector<int> GridItemList;

#define GRID_INDEX(x, y) ((x) + (y) * width)
#define GRID_SIZE 256

class UniformGrid
{
public:
    int width, height;
    vector<GridItem> store;
    GridItemList free_list;
    GridItemList * grid;
    int query_id;

    UniformGrid();
    ~UniformGrid();
    int add(void * data, int v[4]);
    void move(int proxy, int v[4]);
    void remove(int proxy);
    void clear();

    template <typename T>
    bool query(int v[4], T & callback);

    template <typename T>
    bool query(int proxy, T & callback);

    void get_pos(int in[4], int out[4]);
};

#undef max
#undef min

inline int clamp(int v, int minval, int maxval)
{
    return std::min(maxval, std::max(v, minval));
}

inline void UniformGrid::get_pos(int in[4], int out[4])
{
    out[0] = clamp(in[0] / GRID_SIZE, 0, width-1);
    out[1] = clamp(in[1] / GRID_SIZE, 0, height-1);
    out[2] = clamp(in[2] / GRID_SIZE + 1, 1, width);
    out[3] = clamp(in[3] / GRID_SIZE + 1, 1, height);
}

template <typename T>
inline bool UniformGrid::query(int proxy, T & callback)
{
    GridItem & item = store[proxy];
    GridItemList::iterator it;

    query_id++;

    for (int y = item.box[1]; y < item.box[3]; y++)
    for (int x = item.box[0]; x < item.box[2]; x++) {
        GridItemList & list = grid[GRID_INDEX(x, y)];

        for (it = list.begin(); it != list.end(); ++it) {
            int index = *it;
            if (index == proxy)
                continue;
            GridItem & vv = store[index];
            if (vv.last_query_id == query_id)
                continue;
            vv.last_query_id = query_id;
            if (!callback.on_callback(vv.data))
                return false;
        }
    }
    return true;
}

template <typename T>
inline bool UniformGrid::query(int v[4], T & callback)
{
    GridItemList::iterator it;
    int vv[4];
    get_pos(v, vv);

    query_id++;

    for (int y = vv[1]; y < vv[3]; y++)
    for (int x = vv[0]; x < vv[2]; x++) {
        GridItemList & list = grid[GRID_INDEX(x, y)];

        for (it = list.begin(); it != list.end(); ++it) {
            GridItem & vv = store[*it];
            if (vv.last_query_id == query_id)
                continue;
            vv.last_query_id = query_id;
            if (!callback.on_callback(vv.data))
                return false;
        }
    }
    return true;
}

#endif // CHOWDREN_GRID_H
