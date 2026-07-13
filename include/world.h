#pragma once
#include <stdlib.h> // do I really need this just to use size_t? 
#include <stdio.h>
#include "common.h"
#include "scene.h"
#include "node_edge.h"
#define MAX_SCENE_IMAGES 5

#define MAX_NODES 64
#define MAX_EDGES 2 * MAX_NODES // 128
#define NUM_GAME_IMAGES 1 // double check me
#define MAX_FLAGS 256   // story flags, "hasTalkedToGhost", etc.
#define MAX_HOTSPOTS 8  // per scene


typedef struct GAME_GRAPH
{
    Node nodes[MAX_NODES];
    Edge edges[MAX_EDGES];
    int node_count;
    int edge_count;
} Game_World;

int add_node(Game_World* gm, Node_id id, Vec2 pos)
{
    gm->nodes[id].id = id;
    gm->node_count++;
    return id;
}
void cull_node(Game_World* gm, Node_id id)
{
    gm->nodes[id] = (Node){0};
    gm->node_count--;
}
void add_edge(Game_World* gm, Node_id from, Node_id to)
{
    gm->edges[gm->edge_count].from = from;
    gm->edges[gm->edge_count].to = to;
    gm->edge_count++;
}
bool cull_edge(Game_World* gm, Node_id from, Node_id to)
{
    if(gm->edge_count < 1) return false;
    for (size_t i = 0; i < gm->edge_count; i++)
    {
        Edge e = gm->edges[i];
        if(e.from == from && e.to == to) 
        {
            gm->edges[i] = (Edge){0};
            gm->edge_count--;
            return true;
        }
    }
    printf("edge not found\n");
    return false;
}

void init_game(Game_World* gw, Node* nodes, Edge* edges, int node_count, int edge_count)
{
    gw->node_count = node_count;
    gw->edge_count = edge_count;
    
    for (int i = 0; i < node_count; i++) {
        gw->nodes[i] = nodes[i];
    }
    for (int i = 0; i < edge_count; i++) {
        gw->edges[i] = edges[i];
    }
}

