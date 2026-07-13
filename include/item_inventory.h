#pragma once
#include "common.h"
#define MAX_INVENTORY 5
#define TOTAL_GAME_ITEMS 1

char* ITEMS_NAMES[TOTAL_GAME_ITEMS];
char* ITEM_HINTS[TOTAL_GAME_ITEMS];

typedef struct ITEM
{
    bool consumable;
    bool removeable;
    int item_id;
    int quantity;
    char* item_hint;
    char* item_name;
} Item;

typedef Item Inventory[MAX_INVENTORY];
