#pragma once
#include "common.h"
#include "player.h"
#include "mouse.h"
#include "world.h"
#include "camera.h"
#include "item_inventory.h"
#include "node_edge.h"



typedef struct GAME_SAVE
{} Game_Save;

typedef struct GAME
{
    int stress;
    int health;
    int inventory[MAX_INVENTORY];
    Game_World* gw;
    Node_id current_node;
} Game;