#pragma once
#include "common.h"

typedef int Node_id;

typedef struct EDGE 
{
    Node_id from;
    Node_id to;
} Edge;

typedef struct NODE
{
    Node_id id;
    int num_edges;
    Edge* edges; // array of edges
} Node;

