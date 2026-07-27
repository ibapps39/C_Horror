#pragma once
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include "raylib.h"
#include "raymath.h"

typedef Vector2 Vec2;

typedef struct HOTSPOT
{
    Rectangle r;
    Vec2 pos;
} Hotspot;

typedef int Demo_Node_id;
typedef struct DEMO_EDGE
{
    // could be argued to be const but im thinking ahead for some stuff
    Demo_Node_id from;
    Demo_Node_id to; 
} Demo_Edge;
typedef struct DEMO_SCENE
{
    Demo_Node_id id;
    const char* scn_name;
    char* scn_background;
    char* scn_sound;
    Hotspot* hotspots;
    Demo_Edge* edges;
    int demo_edge_count;
    int hotspot_count;
} Demo_Scene;


Demo_Scene DEMO_GAME_SCENES[] = 

{
// All hotspots assume an original resoluton of 1000x1000
{
    .id = 0,
    .scn_name = "untitled",
    .scn_background = "resources/images/horror_game_img_0.png",
    .scn_sound = "resources/images/freesound_community-cyprus-storm-61421.mp3",
    .hotspots = { 
        {100,700}, 
        {500, 500} , 
        {700, 100}, 
        {700, 700} 
    },
    .edges = {
        {0,1}
    },
    1,
    5
}
}
#define TOTAL_SCENES (sizeof(DEMO))