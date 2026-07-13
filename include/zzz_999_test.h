#pragma once
#include <assert.h>
#include <stdalign.h> //do i need this?
#include <stdbool.h>
#include "world.h"
void test_suite_01(const Game_World *gm)
{
    assert(gm);
    assert(gm->edge_count < MAX_EDGES);
    assert(gm->edge_count >= 0);
    assert(gm->nodes);
    assert(gm->node_count < MAX_NODES);
    assert(gm->node_count >= 0);
}
int valid_nodes(const Game_World *gm)
{
    for (size_t i = 0; i < gm->node_count; i++)
    {
        Node n = gm->nodes[i];
        if (i == 0)
        {
            if (gm->nodes[0].pos.x > 0 && gm->nodes[0].pos.y > 0)
            {
                continue;
            }
            else
            {
                return 0;
            }
        }
        if (n.id > 0 && n.pos.x > 0 && n.pos.y > 0)
        {
            continue;
        }
        else
        {
            printf("invalid node %i, node: id = %i, pos = %f, %f\n", i, n.id, n.pos.x, n.pos.y);
            return i;
        }
    }
    return gm->node_count;
}