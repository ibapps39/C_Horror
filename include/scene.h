#pragma once
#include <stdbool.h>
#include "common.h"
#include "node_edge.h"
typedef int Hotspot_id;
typedef enum TYPES_HOTSPOT
{
    NAV,
    USE,
    LOOK,
    TAKE,
    TALK
} Hotspot_t;

typedef struct HOTSPOT
{
    Color highlight_color;
    Rectangle rect_dim;
} Hotspot;

typedef struct SCENE
{
    Node_id scene_node_id;
    int background_texture_id; // typically #0 of scene_images
    Hotspot_id* hotspots // array
} Scene;

//int LOCAL_HOTSPOTS[MAX_SCENE_OBJECTS]

void draw_hotspot(Hotspot* h, bool on_hotspot)
{
   DrawRectangle(h->rect_dim.x, h->rect_dim.y, h->rect_dim.width, h->rect_dim.height, h->highlight_color);
}
