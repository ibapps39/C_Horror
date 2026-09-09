#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <mm_malloc.h>
#include "raylib.h"
#include "raymath.h"

typedef Vector2 Vec2;

#define DEMO_MAX_IMAGES 6
#define DEMO_FPS 60
#define DEMO_MAX_HOTSPOTS 5
#define DEMO_WINDOW_TITLE "DEMO_RAYLIB_HORROR_POINT_CLICK"
#define DEMO_WINDOW_SETTING FLAG_WINDOW_RESIZABLE
#define DEMO_MAX_SOUNDS 1

#define TOP_LEFT (Vec2){.x = 0.0f, .y = 0.0f}
#define TOP_RIGHT (Vec2){.x = (float)GetScreenWidth(), .y = 0.0f}
#define BOTTOM_LEFT \
    (Vec2) { .x = 0.0f, .y = (float)GetScreenHeight() }
#define BOTTOM_RIGHT \
    (Vec2) { .x = (float)GetScreenWidth(), .y = (float)GetScreenHeight() }
#define CENTER (Vec2){.x = (float)GetScreenWidth()/2, .y = (float)GetScreenHeight()/2}

const char *DEMO_IMAGE_DIR = "./resources/images";
const char *DEMO_MUSIC_DIR = "./resources/music";
const char *DEMO_SOUND_DIR = "./resources/sounds";
const char *DEMO_OBJECTS_DIR = "./resources/images/objects";

Camera2D DEMO_CAM = (Camera2D){0};
bool DEMO_IN_hotspot = false;

bool demo_load_images(const FilePathList dir, Image *dst)
{
    FilePathList images_dir = dir;

    if (images_dir.count < 1)
    {
        printf("ERROR: NO DETECTED PHOTOS\n");
        return false;
    }
    if (images_dir.count < DEMO_MAX_IMAGES)
    {
        printf("ERROR: NOT ENOUGH PHOTOS\n");
        return false;
    }
    if (images_dir.count > DEMO_MAX_IMAGES)
    {
        printf("ERROR: PHOTOS EXCEED MAX\n");
        return false;
    }

    int img_processed = 0;
    for (size_t i = 0; i < images_dir.count; ++i)
    {
        if (!FileExists(images_dir.paths[i]))
        {
            printf("ERROR:NO IMAGE FOUND ON FILE %s", images_dir.paths[i]);
            return false;
        }
        Image img = LoadImage(images_dir.paths[i]);
        if (!IsImageValid(img))
        {
            printf("ERROR: IMAGE[%i] IS NOT VALID\n", (int)i);
            return false;
        }
        dst[i] = img;
        ++img_processed;
    }
    printf("COMPLETE: Loading Images %i/%i\n", img_processed, images_dir.count);
    return true;
}
bool demo_resize_imgs(Image *img_array, int num_imgs, int scn_width, int scn_height)
{
    if (!img_array)
    {
        printf("ERROR: NO IMAGES FOUND");
        return false;
    }
    for (size_t i = 0; i < num_imgs; ++i)
    {
        ImageResize(&img_array[i], scn_width, scn_height);
    }
    printf("COMPLETE: Resizing %i Images\n", num_imgs);
    return true;
}
bool demo_load_textures(Image *src, Texture *dst, const int num_imgs)
{
    if (!src)
    {
        printf("ERROR: NO DETECTED IMAGES\n");
        return false;
    }
    if (!dst)
    {
        printf("ERROR: NO DESTINATION VALID PROVIDED\n");
        return false;
    }
    int tex_processed = 0;
    for (size_t i = 0; i < num_imgs; ++i)
    {
        Texture t = LoadTextureFromImage(src[i]);
        if (!IsTextureValid(t))
        {
            printf("ERROR: TEXTURE[%i] IS NOT VALID\n", (int)i);
            return false;
        }
        dst[i] = t;
        ++tex_processed;
    }
    printf("COMPLETE: Loading Textures %i/%i\n", tex_processed, num_imgs);
    return true;
}

bool demo_unload_imgs(Image *imgs, const int num_imgs)
{
    for (size_t i = 0; i < num_imgs; i++)
    {
        UnloadImage(imgs[i]);
    }
    printf("COMPLETE: Unloaded Images %i/%i\n", num_imgs, num_imgs);
    return true;
}
bool demo_unload_textures(Texture *texs, const int num_imgs)
{
    for (size_t i = 0; i < num_imgs; i++)
    {
        UnloadTexture(texs[i]);
    }
    printf("COMPLETE: Unloaded Textures %i/%i\n", num_imgs, num_imgs);
    return true;
}
// centered at res/2 or res center
Camera2D demo_get_cam(const int scn_w, const int scn_h)
{
    Camera2D cam = {0};
    cam.target = (Vec2){.x = scn_w / 2, .y = scn_h / 2};
    cam.offset = (Vec2){.x = scn_w / 2, .y = scn_h / 2};
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;
    return cam;
}

// could change to
typedef int Demo_Node_id;
typedef struct DEMO_EDGE
{
    // could be argued to be const but im thinking ahead for some stuff
    Demo_Node_id from;
    Demo_Node_id to;
} Demo_Edge;

typedef enum HOTSPOT_TYPE
{
    TRANSITION,
    INSPECT,
    GRAB,
    ITEM
} Hotspot_t;

typedef struct HOTSPOT
{
    Rectangle r;
    Vec2 pos;
    Hotspot_t type;
    union
    {
        Demo_Edge edge;
        int item_id;
        char *description;
    };
} Hotspot;

Hotspot demo_create_hotspot(Vec2 pos, float width, float height, Hotspot_t type, Demo_Edge edge, int item_id, char *description)
{
    Rectangle rec = {
        .x = pos.x,
        .y = pos.y,
        .width = width,
        .height = height};
    Hotspot h =
        {
            .pos = pos,
            .r = rec,
            .type = type,
        };

    switch (type)
    {
    case GRAB:
        break;
    case ITEM:
        h.item_id = item_id;
        break;
    case INSPECT:
        h.description = description;
        break;
    case TRANSITION:
        h.edge = edge;
        break;
    default:
        break;
    }
    return h;
}

void demo_draw_hotspots(Hotspot *hotspots, int hotspot_count)
{
    for (size_t i = 0; i < (size_t)hotspot_count; i++)
    {
        DrawRectangleLines(hotspots[i].r.x, hotspots[i].r.y, hotspots[i].r.width, hotspots[i].r.height, RED);
    }
}

void draw_hotspot(Hotspot h)
{
    DrawRectangleLines(h.pos.x, h.pos.y, h.r.width, h.r.height, RED);
}

int demo_search_hotspots(const Hotspot *hotspots, const int num_hotspots, bool *in_hotspot, const Vec2 cursor_pos)
{
    *in_hotspot = false; // Assume false until proven otherwise

    for (size_t i = 0; i < num_hotspots; ++i)
    {
        if (CheckCollisionPointRec(cursor_pos, hotspots[i].r))
        {
            *in_hotspot = true;
            draw_hotspot(hotspots[i]);
            return i;
        }
    }
    return -1;
}

bool demo_load_sounds(const char *dir, Sound *dst)
{
    FilePathList sound_dir = LoadDirectoryFiles(dir);

    if (sound_dir.count < 1)
    {
        printf("ERROR: NO DETECTED PHOTOS\n");
        return false;
    }
    if (sound_dir.count < DEMO_MAX_SOUNDS)
    {
        printf("ERROR: NOT ENOUGH PHOTOS\n");
        return false;
    }
    if (sound_dir.count > DEMO_MAX_SOUNDS)
    {
        printf("ERROR: PHOTOS EXCEED MAX\n");
        return false;
    }

    int snd_processed = 0;
    for (size_t i = 0; i < sound_dir.count; ++i)
    {
        if (!FileExists(sound_dir.paths[i]))
        {
            printf("ERROR:NO IMAGE FOUND ON FILE %s", sound_dir.paths[i]);
            return false;
        }
        Sound snd = LoadSound(sound_dir.paths[i]);
        if (!IsSoundValid(snd))
        {
            printf("ERROR: SOUND[%i] IS NOT VALID\n", (int)i);
            return false;
        }
        dst[i] = snd;
        ++snd_processed;
    }
    printf("COMPLETE: Loading Sounds %i/%i\n", snd_processed, sound_dir.count);
    return true;
}

Texture2D demo_load_texture(const char *file_name, const int w, const int h)
{
    printf("\nDEMO LOAD TEXTURE  - START \n");
    printf("Loading image %s\n", file_name);
    Image img_i = LoadImage(file_name);
    ImageResize(&img_i, w, h);
    Texture2D t = LoadTextureFromImage(img_i);
    UnloadImage(img_i);
    printf("\nDEMO LOAD TEXTURE  - END \n");
    return t;
}

typedef struct DEMO_SCENE
{
    Demo_Node_id id;
    const char *scn_name;
    char *scn_background;
    char *scn_sound;
    Hotspot *hotspots;
    int demo_edge_count;
    int hotspot_count;
} Demo_Scene;

Demo_Scene Demo_Scene_Init(
    const Demo_Node_id id,
    const char *name,
    const char *background,
    const char *sound,
    const int edge_count,
    const int hotspot_count,
    unsigned char* total_scene_count
)
{
    Demo_Scene scene;

    scene.scn_name = name;
    scene.scn_background = background;
    scene.scn_sound = sound;
    scene.id = id;
    scene.demo_edge_count = edge_count;
    scene.hotspot_count = hotspot_count;

    scene.hotspots = malloc(sizeof(Hotspot) * hotspot_count);
    for (int i = 0; i < hotspot_count; i++)
    {
        scene.hotspots[i] = demo_create_hotspot((Vec2){0}, 0, 0, TRANSITION, (Demo_Edge){0}, 0, NULL);
    }
    total_scene_count+=1;
    return scene;
}

Demo_Node_id demo_traverse_edge(Demo_Node_id *current_scene_id, const Demo_Edge edge)
{
    *current_scene_id = edge.to;
    return edge.to;
}

typedef struct CURRENT_SCENE
{
    Demo_Node_id id;
    char *name;
    Texture2D background;
    Sound sound;
    Hotspot *hotspots;
    Demo_Edge *edges;
    int edge_count;
    int hotspot_count;
} Current_Scene;

void update_scene(Current_Scene *current_scene, Demo_Scene *incoming_scene, bool change_sound, Texture2D *BCKGRD_TEX_CACHE)
{
    current_scene->background = BCKGRD_TEX_CACHE[incoming_scene->id];
    current_scene->id = incoming_scene->id;
    current_scene->name = incoming_scene->scn_name;
    current_scene->sound;
    current_scene->hotspots = incoming_scene->hotspots;
    current_scene->edge_count = incoming_scene->demo_edge_count;
    current_scene->hotspot_count = incoming_scene->hotspot_count;
    if (change_sound) { UnloadSound(current_scene->sound); current_scene->sound = LoadSound(incoming_scene->scn_sound); }
    printf("\n\nBACKGROUND: file:%s id:%i\n", incoming_scene->scn_background, incoming_scene->id);
}

static int get_trailing_num(const char *str)
{
    int i = TextLength(str) - 1;
    while (i >= 0 && str[i] != '.')
        i--;
    i--;
    int num_end = i;
    while (i >= 0 && str[i] >= '0' && str[i] <= '9')
        i--;
    if (i + 1 > num_end)
        return -1; // No number found

    int val = 0;
    for (int j = i + 1; j <= num_end; j++)
    {
        val = val * 10 + (str[j] - '0');
    }
    return val;
}

static int compare_by_suffix(const void *a, const void *b)
{
    return get_trailing_num(*(const char **)a) - get_trailing_num(*(const char **)b);
}

FilePathList LoadDirectoryFilesSorted(const char *dir)
{
    FilePathList list = LoadDirectoryFiles(dir);
    qsort(list.paths, list.count, sizeof(char *), compare_by_suffix);
    return list;
}

void demo_hotspot_spots_adjust(Hotspot *hotspots, int count, float recw, float rech)
{
    if(!hotspots) return;
    for (size_t i = 0; i < count; i++)
    {
        switch(i)
            {
                case 0:
                    hotspots[i].pos = CENTER;
                    break;
                case 1:
                    hotspots[i].pos = TOP_LEFT;
                    break;
                case 2:
                    hotspots[i].pos = BOTTOM_LEFT;
                    hotspots[i].pos.y = BOTTOM_LEFT.y - rech;
                    break;
                case 3:
                    hotspots[i].pos.x = BOTTOM_RIGHT.x - recw;
                    hotspots[i].pos.y = BOTTOM_RIGHT.y - rech;
                    break;

                case 4:
                    hotspots[i].pos.x = TOP_RIGHT.x - recw;
                    break;
            }

            hotspots[i].r.x = hotspots[i].pos.x;
            hotspots[i].r.y = hotspots[i].pos.y;
            hotspots[i].r.width = recw;
            hotspots[i].r.height = rech;
    }
}



void draw_scene(Current_Scene *CS, Texture2D *TEXTURE_POOL, int internal_res_x, int internal_res_y)
{
    Rectangle src = {0, 0, internal_res_x, internal_res_y};
    Rectangle dst = {0, 0, GetScreenWidth(), GetScreenHeight()};
    DrawTexturePro(TEXTURE_POOL[CS->id], src, dst, (Vec2){0}, 0, WHITE);
}

typedef struct TEX_MAP_KEY
{
    const char *file_name;
    unsigned int index;
    Texture2D *TX_POOL;
} Texture_Key;

void setup_scene_hotspots(int total_scene_count, Demo_Scene *GS, int recw, int rech)
{
    for (int i = 0; i < total_scene_count; ++i)
    {
        for (int j = 0; j < GS[i].hotspot_count; ++j)
        {
            GS[i].hotspots[j].r.width = recw;
            GS[i].hotspots[j].r.height = rech;
            switch(j)
            {
                case 0:
                    GS[i].hotspots[j].pos = CENTER;
                    GS[i].hotspots[j].type = TRANSITION;
                    if(i < total_scene_count - 1) 
                    { 
                        GS[i].hotspots[j].edge.from = i;
                        GS[i].hotspots[j].edge.to = i + 1;
                    } else {
                        GS[i].hotspots[j].edge.from = i;
                        GS[i].hotspots[j].edge.to = 0;
                    }
                    break;
                case 1:
                    GS[i].hotspots[j].pos = TOP_LEFT;
                    GS[i].hotspots[j].type = INSPECT;
                    GS[i].hotspots[j].description = "What's there?";
                    break;
                case 2:
                    GS[i].hotspots[j].pos = BOTTOM_LEFT;
                    GS[i].hotspots[j].pos.y -= rech;
                    GS[i].hotspots[j].type = INSPECT;
                    break;
                case 3:
                    GS[i].hotspots[j].pos = BOTTOM_RIGHT;
                    GS[i].hotspots[j].pos.x -= recw;
                    GS[i].hotspots[j].pos.y -= rech;
                    GS[i].hotspots[j].type = INSPECT;
                    break;
                case 4:
                    GS[i].hotspots[j].pos = TOP_RIGHT;
                    GS[i].hotspots[j].pos.x -= recw;
                    GS[i].hotspots[j].type = INSPECT;
                    break;
            }
            GS[i].hotspots[j].r.x = GS[i].hotspots[j].pos.x;
            GS[i].hotspots[j].r.y = GS[i].hotspots[j].pos.y;

        }
    }
}



typedef struct ITEM_T {
    char* name;
    char* sprite_name;
    char* hint;
    Hotspot hotspot;
    Texture_Key t_key;
} Item;


void open_inventory(bool on, Item* inventory, unsigned char MAX_INVENTORY)
{
    if(!on) return;
    
    Rectangle RecInvBack = {
        .x = 0.0f,
        .y = 0.0f,
        .width = GetScreenWidth(),
        .height = GetScreenHeight()
    };
    DrawRectanglePro(RecInvBack, (Vec2){0}, 0.0f, GRAY);
    
    for (size_t i = 0; i < MAX_INVENTORY; ++i)
    {
        Rectangle r = inventory[i].hotspot.r;
        DrawRectangle(r.x, r.y, r.width, r.height, BLUE);
        Texture2D t = inventory[i].t_key.TX_POOL[inventory[i].t_key.index];
        DrawTexturePro(t, (Rectangle){0, 0, (float)t.width, (float)t.height}, r, (Vec2){0, 0}, 0.0f, WHITE);
    }
}

void setup_texkeys(const FilePathList list, const Texture2D* texture_pool, Texture_Key* texs, const int w, const int h)
{
    for (size_t i = 0; i < (size_t)list.count; ++i)
    {
        printf("SETUP TEXKEYS %s\n", list.paths[i]);
        texs[i].file_name = list.paths[i];
        texs[i].index = i;
        texs[i].TX_POOL = texture_pool;
        texs[i].TX_POOL[i] = demo_load_texture(list.paths[i], w, h);
    }
}
#define FIRST_INVENTORY_SLOT_X (GetScreenWidth()  * .1)
#define FIRST_INVENTORY_SLOT_Y (GetScreenHeight() * .5)
#define INVENTORY_SLOT_WIDTH (GetScreenWidth()  * .2)
#define INVENTORY_SLOT_HEIGHT (GetScreenHeight() * .2)
// assumes texture key generated
void setup_inventory(Texture_Key* texs, Item* inventory, unsigned char inv_count, Hotspot* inv_hotspots)
{
    for(size_t i = 0; i < inv_count; ++i)
    {
        inventory[i].name = texs[i].file_name;
        inventory[i].sprite_name = texs[i].file_name;
        inventory[i].hint = texs[i].file_name;
        inventory[i].t_key = texs[i];
        
        float y = GetScreenHeight() * .5;
        float w = (GetScreenWidth()-GetScreenWidth()*.2) / inv_count;
        float h = GetScreenHeight() * .2;
        float x = i==0 ? GetScreenWidth()  * .1  : (GetScreenWidth()  * .1)+(w*i);
        
        inventory[i].hotspot = (Hotspot){
            .pos = (Vec2){
                .x =  x, 
                .y = y
            },
            .r = (Rectangle){
                .x = x,
                .y = y,
                .width = w,
                .height = h
            },
            .type = ITEM
        };
        inv_hotspots[i] = inventory[i].hotspot;
    }
}

void get_inventory_hotspots(Item* player_inventory, unsigned char inventory_count, Hotspot* out)
{
    for (size_t i = 0; i < inventory_count; ++i)
    {
        out[i] = player_inventory[i].hotspot;
    }
}

void hotspot_behavior(Current_Scene *CS, int active_hotspot_id, Demo_Scene *DGS, Texture2D *BCKGD_TEXS, int RES_X, int RES_Y)
{
    draw_hotspot(CS->hotspots[active_hotspot_id]);
    DrawText(TextFormat("Hotspot: %i", active_hotspot_id), RES_X / 2, RES_Y * .16, 20, BLUE);
    switch (CS->hotspots[active_hotspot_id].type)
    {
    case TRANSITION:
        DrawText(TextFormat("Transition to: %i", CS->hotspots[active_hotspot_id].edge.to), RES_X / 2, RES_Y * .2, 20, RED);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            update_scene(CS, &DGS[CS->hotspots[active_hotspot_id].edge.to], false, BCKGD_TEXS);
        }
        break;
    case INSPECT:
        DrawText(CS->hotspots[active_hotspot_id].description, RES_X / 2, RES_Y * .2, 20, RED);
        break;
    case ITEM:
        DrawText(CS->hotspots[active_hotspot_id].description, RES_X / 2, RES_Y * .2, 20, RED);
        break;
    default:
        break;
    }
}

void populate_texture_pool(Texture2D* pool, FilePathList path_list, int internal_x, int internal_y, unsigned int count)
{
    for (size_t i = 0; i < count; ++i)
    {
        pool[i] = demo_load_texture(path_list.paths[i], internal_x, internal_y);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void demo_test(void)
{
    // Settings and config.
    int DEMO_INTERNAL_RES_X = 600;
    int DEMO_INTERNAL_RES_Y = 600;
    const int default_fps = DEMO_FPS;
    SetConfigFlags(DEMO_WINDOW_SETTING);
    InitWindow(DEMO_INTERNAL_RES_X, DEMO_INTERNAL_RES_Y, DEMO_WINDOW_TITLE);
    InitAudioDevice();
    assert(IsAudioDeviceReady() == true);
    SetTargetFPS(DEMO_FPS);
    HideCursor();


    const char *DEMO_BCKGS_DIR = TextFormat("%s/%s", DEMO_IMAGE_DIR, "scenes");
    const char *DEMO_CURSORS_DIR = TextFormat("%s/%s", DEMO_IMAGE_DIR, "cursors");

    FilePathList demo_sounds    = LoadDirectoryFilesSorted(DEMO_SOUND_DIR);
    FilePathList demo_bckgs     = LoadDirectoryFilesSorted(DEMO_BCKGS_DIR);
    FilePathList demo_cursors   = LoadDirectoryFilesSorted(DEMO_CURSORS_DIR);
    FilePathList demo_objs      = LoadDirectoryFilesSorted(DEMO_OBJECTS_DIR);

    // "Globals"
    Sound CURRENT_SOUND = (Sound){0};
    Current_Scene CURRENT_SCENE = (Current_Scene){0};
    Camera2D DEMO_CAM = demo_get_cam(DEMO_INTERNAL_RES_X, DEMO_INTERNAL_RES_X);
    const unsigned char TOTAL_SCENE_COUNT = demo_bckgs.count;
    Demo_Scene DEMO_GAME_SCENES[TOTAL_SCENE_COUNT];
    const unsigned char  MAX_BACKGROUND_TEXTURES = demo_bckgs.count;
    const unsigned char MAX_INVENTORY = demo_objs.count;

    //Texture2D *DEMO_GAME_TEXTURES   = (Texture2D *)malloc(sizeof(Texture2D) * MAX_TEXTURES);
    Texture2D *DEMO_GAME_BACKGROUND_TEXTURES   = (Texture2D *)malloc(sizeof(Texture2D) * demo_bckgs.count);
    Texture2D *DEMO_CURSOR_TEXTURES = (Texture2D *)malloc(sizeof(Texture2D) * demo_cursors.count);
    Texture2D *DEMO_OBJECT_SPRITES_TEXTURES  = (Texture2D *)malloc(sizeof(Texture2D) * demo_objs.count);

    Texture_Key* DEMO_SPRITE_KEYS = (Texture_Key*)malloc(sizeof(Texture_Key) * demo_objs.count);
    
    populate_texture_pool(DEMO_GAME_BACKGROUND_TEXTURES, demo_bckgs, DEMO_INTERNAL_RES_X, DEMO_INTERNAL_RES_Y, demo_bckgs.count);
    populate_texture_pool(DEMO_CURSOR_TEXTURES, demo_cursors, DEMO_INTERNAL_RES_X, DEMO_INTERNAL_RES_Y, demo_cursors.count);
    populate_texture_pool(DEMO_OBJECT_SPRITES_TEXTURES, demo_objs, DEMO_INTERNAL_RES_X, DEMO_INTERNAL_RES_Y, demo_objs.count);

    unsigned char test_scene_count = 0;
    Demo_Scene TEST_SCENE_0 = Demo_Scene_Init(0, "scene 0", demo_bckgs.paths[0], demo_sounds.paths[0], 1, 5, &test_scene_count);
    Demo_Scene TEST_SCENE_1 = Demo_Scene_Init(1, "scene 1", demo_bckgs.paths[1], demo_sounds.paths[0], 1, 5, &test_scene_count);
    Demo_Scene TEST_SCENE_2 = Demo_Scene_Init(2, "scene 2", demo_bckgs.paths[2], demo_sounds.paths[0], 1, 5, &test_scene_count);
    Demo_Scene TEST_SCENE_3 = Demo_Scene_Init(3, "scene 3", demo_bckgs.paths[3], demo_sounds.paths[0], 1, 5, &test_scene_count);
    Demo_Scene TEST_SCENE_4 = Demo_Scene_Init(4, "scene 4", demo_bckgs.paths[4], demo_sounds.paths[0], 1, 5, &test_scene_count);

    DEMO_GAME_SCENES[0] = TEST_SCENE_0;
    DEMO_GAME_SCENES[1] = TEST_SCENE_1;
    DEMO_GAME_SCENES[2] = TEST_SCENE_2;
    DEMO_GAME_SCENES[3] = TEST_SCENE_3;
    DEMO_GAME_SCENES[4] = TEST_SCENE_4;

    setup_texkeys(demo_objs, DEMO_OBJECT_SPRITES_TEXTURES, DEMO_SPRITE_KEYS, DEMO_INTERNAL_RES_X, DEMO_INTERNAL_RES_Y);
    setup_scene_hotspots(5, DEMO_GAME_SCENES, DEMO_INTERNAL_RES_X*.05f, DEMO_INTERNAL_RES_Y*.05f);
    update_scene(&CURRENT_SCENE, &DEMO_GAME_SCENES[0], true, DEMO_GAME_BACKGROUND_TEXTURES);

    bool in_hotspot = false;
    Texture2D current_cursor_texture = demo_load_texture(TextFormat("%s/cursor_0.png", DEMO_CURSORS_DIR), DEMO_INTERNAL_RES_X * .05, DEMO_INTERNAL_RES_Y * .05);
    int CURRENT_HOTSPOT_IN_SCENE = -1;
    
    Item* PLAYER_INVENTORY = (Item*)malloc(sizeof(Item) * MAX_INVENTORY);
    Hotspot INVENTORY_HOTSPOTS[MAX_INVENTORY];
    setup_inventory(DEMO_SPRITE_KEYS, PLAYER_INVENTORY, MAX_INVENTORY, INVENTORY_HOTSPOTS);
    
    while (!WindowShouldClose())
    {
        if (!IsSoundPlaying(CURRENT_SCENE.sound))
        {
            PlaySound(CURRENT_SCENE.sound);
        }
        if (IsWindowResized())
        {
            UnloadTexture(current_cursor_texture);
            current_cursor_texture = demo_load_texture(demo_cursors.paths[0], DEMO_INTERNAL_RES_X * .05, DEMO_INTERNAL_RES_Y * .05);
            
        }
        float dt = GetFrameTime();


        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(DEMO_CAM);

        draw_scene(&CURRENT_SCENE, DEMO_GAME_BACKGROUND_TEXTURES, DEMO_INTERNAL_RES_X, DEMO_INTERNAL_RES_Y);
        int font_s = 20;
        DrawText(CURRENT_SCENE.name, TOP_LEFT.x, TOP_LEFT.y, font_s, YELLOW);
        
        static bool toggle_inv = false;
        
        CURRENT_HOTSPOT_IN_SCENE = (toggle_inv) ? demo_search_hotspots(INVENTORY_HOTSPOTS, MAX_INVENTORY, &in_hotspot, GetMousePosition()) : demo_search_hotspots(CURRENT_SCENE.hotspots, CURRENT_SCENE.hotspot_count, &in_hotspot, GetMousePosition());
        
        if (IsKeyDown(KEY_ONE))
        {
            
            
            if (CURRENT_SCENE.hotspot_count == 0)
            {
                DrawText("NO HOTSPOTS", GetScreenWidth() / 2, GetScreenHeight() / 2, 20, RED);
            }
            demo_draw_hotspots(CURRENT_SCENE.hotspots, CURRENT_SCENE.hotspot_count);
            Hotspot h = CURRENT_SCENE.hotspots[CURRENT_HOTSPOT_IN_SCENE];
            if(h.type == INSPECT) DrawText(h.description, GetScreenWidth() / 2, GetScreenHeight() / 2, 20, RED);
            if(h.type == TRANSITION) DrawText(TextFormat("Transition to: %i", h.edge.to), GetScreenWidth() / 2, GetScreenHeight() / 2, 20, RED);
        }

        
        if(IsKeyPressed(KEY_T))
        {
            toggle_inv = !toggle_inv;
        }
        
        if (in_hotspot)
        {
            hotspot_behavior(&CURRENT_SCENE, CURRENT_HOTSPOT_IN_SCENE, DEMO_GAME_SCENES, DEMO_GAME_BACKGROUND_TEXTURES, DEMO_INTERNAL_RES_X, DEMO_INTERNAL_RES_Y);
        }
    EndMode2D();
    // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW //
    if(toggle_inv) {
        open_inventory(toggle_inv, PLAYER_INVENTORY, MAX_INVENTORY);
        if (in_hotspot)
        {
            draw_hotspot(PLAYER_INVENTORY[CURRENT_HOTSPOT_IN_SCENE].hotspot);
        }
    }
    DrawTexture(current_cursor_texture, GetMouseX(), GetMouseY(), WHITE);
    EndDrawing();
}

UnloadTexture(CURRENT_SCENE.background);
UnloadTexture(current_cursor_texture);


for(size_t i = 0; i < test_scene_count; ++i)
{
    free(DEMO_GAME_SCENES[i].hotspots);
}
free(CURRENT_SCENE.hotspots);
free(DEMO_GAME_BACKGROUND_TEXTURES);
free(DEMO_SPRITE_KEYS);
free(DEMO_CURSOR_TEXTURES);
CloseWindow();
return;
}