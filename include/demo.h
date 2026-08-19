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
    printf("\n\n\n DEMO LOAD TEXTURE  - START \n\n\n");
    printf("Loading image %s\n", file_name);
    Image img_i = LoadImage(file_name);
    ImageResize(&img_i, w, h);
    Texture2D t = LoadTextureFromImage(img_i);
    UnloadImage(img_i);
    printf("\n\n\n DEMO LOAD TEXTURE  - END \n\n\n");
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
    const int hotspot_count)
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

void update_scene(Current_Scene *target, Demo_Scene *source, bool unload_sound, Texture2D *TEXTURES_CACHE)
{

    target->background = TEXTURES_CACHE[source->id];
    target->id = source->id;
    target->name = source->scn_name;
    printf("\n\nBACKGROUND:%s\n", source->scn_background);
    if (unload_sound)
        UnloadSound(target->sound);
    target->sound = LoadSound(source->scn_sound);
    target->hotspots = source->hotspots;
    target->edge_count = source->demo_edge_count;
    target->hotspot_count = source->hotspot_count;
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

void refresh_hotspots(Current_Scene *CS)
{
    if(CS->hotspot_count < 1) {
        return; 
    }
    float rw = fmaxf(100.0f, GetScreenWidth()*0.05f);
    float rh = fmaxf(100.0f, GetScreenHeight()*0.05f);
    demo_hotspot_spots_adjust(CS->hotspots, CS->hotspot_count, rw, rh); 
    
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

void setup(int TOTAL_SCENE_COUNT, Demo_Scene *DEMO_GAME_SCENES, int recw, int rech)
{
    for (int i = 0; i < TOTAL_SCENE_COUNT; ++i)
    {
        for (int j = 0; j < DEMO_GAME_SCENES[i].hotspot_count; ++j)
        {
            switch(j)
            {
                case 0:
                    DEMO_GAME_SCENES[i].hotspots[j].pos = CENTER;
                    DEMO_GAME_SCENES[i].hotspots[j].type = TRANSITION;
                    if(i < TOTAL_SCENE_COUNT - 1) 
                    { 
                        DEMO_GAME_SCENES[i].hotspots[j].edge.from = i;
                        DEMO_GAME_SCENES[i].hotspots[j].edge.to = i + 1;
                    } else {
                        DEMO_GAME_SCENES[i].hotspots[j].edge.from = i;
                        DEMO_GAME_SCENES[i].hotspots[j].edge.to = 0;
                    }
                    break;
                case 1:
                    DEMO_GAME_SCENES[i].hotspots[j].pos = TOP_LEFT;
                    DEMO_GAME_SCENES[i].hotspots[j].type = INSPECT;
                    DEMO_GAME_SCENES[i].hotspots[j].description = "What's there?";
                    break;
                case 2:
                    DEMO_GAME_SCENES[i].hotspots[j].pos = BOTTOM_LEFT;
                    DEMO_GAME_SCENES[i].hotspots[j].type = INSPECT;
                    break;
                case 3:
                    DEMO_GAME_SCENES[i].hotspots[j].pos = BOTTOM_RIGHT;
                    DEMO_GAME_SCENES[i].hotspots[j].type = INSPECT;
                    break;
                case 4:
                    DEMO_GAME_SCENES[i].hotspots[j].pos = TOP_RIGHT;
                    DEMO_GAME_SCENES[i].hotspots[j].type = INSPECT;
                    break;
            }

            DEMO_GAME_SCENES[i].hotspots[j].r.width = recw;
            DEMO_GAME_SCENES[i].hotspots[j].r.height = rech;
            DEMO_GAME_SCENES[i].hotspots[j].r.x = DEMO_GAME_SCENES[i].hotspots[j].pos.x;
            DEMO_GAME_SCENES[i].hotspots[j].r.y = DEMO_GAME_SCENES[i].hotspots[j].pos.y;

        }
    }
}


void demo_test(void)
{
    // Settings and config.
    int demo_res_x = 600;
    int demo_res_y = 600;
    const int default_fps = DEMO_FPS;
    SetConfigFlags(DEMO_WINDOW_SETTING);
    InitWindow(demo_res_x, demo_res_y, DEMO_WINDOW_TITLE);
    InitAudioDevice();
    assert(IsAudioDeviceReady() == true);
    SetTargetFPS(DEMO_FPS);
    HideCursor();


    const char *DEMO_BCKGS_DIR = TextFormat("%s/%s", DEMO_IMAGE_DIR, "scenes");
    const char *DEMO_CURSORS_DIR = TextFormat("%s/%s", DEMO_IMAGE_DIR, "cursors");

    FilePathList demo_sounds = LoadDirectoryFilesSorted(DEMO_SOUND_DIR);
    FilePathList demo_bckgs = LoadDirectoryFilesSorted(DEMO_BCKGS_DIR);
    FilePathList demo_cursors = LoadDirectoryFilesSorted(DEMO_CURSORS_DIR);

    // "Globals"
    Sound CURRENT_SOUND = (Sound){0};
    Current_Scene CURRENT_SCENE = (Current_Scene){0};

    Camera2D DEMO_CAM = demo_get_cam(demo_res_x, demo_res_x);
    const int TOTAL_SCENE_COUNT = demo_bckgs.count;
    Demo_Scene DEMO_GAME_SCENES[TOTAL_SCENE_COUNT];

    const int MAX_TEXTURES = demo_bckgs.count + demo_cursors.count;
    Texture2D *DEMO_GAME_TEXTURES = (Texture2D *)malloc(sizeof(Texture2D) * MAX_TEXTURES);
    Texture2D *DEMO_CURSOR_TEXTURES = (Texture2D *)malloc(sizeof(Texture2D) * demo_cursors.count);

    for (size_t i = 0; i < MAX_TEXTURES; ++i)
    {
        DEMO_GAME_TEXTURES[i] = demo_load_texture(demo_bckgs.paths[i], demo_res_x, demo_res_y);
        if (i < demo_cursors.count)
            DEMO_CURSOR_TEXTURES[i] = demo_load_texture(demo_cursors.paths[i], demo_res_x, demo_res_y);
    }

    Demo_Scene TEST_SCENE_0 = Demo_Scene_Init(0, "scene 0", demo_bckgs.paths[0], demo_sounds.paths[0], 1, 5);
    Demo_Scene TEST_SCENE_1 = Demo_Scene_Init(1, "scene 1", demo_bckgs.paths[1], demo_sounds.paths[0], 1, 5);
    Demo_Scene TEST_SCENE_2 = Demo_Scene_Init(2, "scene 2", demo_bckgs.paths[2], demo_sounds.paths[0], 1, 5);
    Demo_Scene TEST_SCENE_3 = Demo_Scene_Init(3, "scene 3", demo_bckgs.paths[3], demo_sounds.paths[0], 1, 5);
    Demo_Scene TEST_SCENE_4 = Demo_Scene_Init(4, "scene 4", demo_bckgs.paths[4], demo_sounds.paths[0], 1, 5);

    DEMO_GAME_SCENES[0] = TEST_SCENE_0;
    DEMO_GAME_SCENES[1] = TEST_SCENE_1;
    DEMO_GAME_SCENES[2] = TEST_SCENE_2;
    DEMO_GAME_SCENES[3] = TEST_SCENE_3;
    DEMO_GAME_SCENES[4] = TEST_SCENE_4;

    
    setup(5, &DEMO_GAME_SCENES, GetScreenWidth()*.05f, GetScreenHeight()*.05f);
    
    update_scene(&CURRENT_SCENE, &DEMO_GAME_SCENES[0], false, DEMO_GAME_TEXTURES);

    bool in_hotspot = false;
    Texture2D current_cursor_texture = demo_load_texture(TextFormat("%s/cursor_0.png", DEMO_CURSORS_DIR), demo_res_x * .05, demo_res_y * .05);
    int CURRENT_HOTSPOT_IN_SCENE = -1;

    while (!WindowShouldClose())
    {
        if (!IsSoundPlaying(CURRENT_SCENE.sound))
        {
            PlaySound(CURRENT_SCENE.sound);
        }
        if (IsWindowResized())
        {
            UnloadTexture(current_cursor_texture);
            current_cursor_texture = demo_load_texture(demo_cursors.paths[0], demo_res_x * .05, demo_res_y * .05);
            refresh_hotspots(&CURRENT_SCENE);
        }

        refresh_hotspots(&CURRENT_SCENE);

        float dt = GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(DEMO_CAM);

        draw_scene(&CURRENT_SCENE, DEMO_GAME_TEXTURES, demo_res_x, demo_res_y);
        int font_s = 20;
        DrawText(CURRENT_SCENE.name, TOP_LEFT.x, TOP_LEFT.y, font_s, YELLOW);

        CURRENT_HOTSPOT_IN_SCENE = demo_search_hotspots(CURRENT_SCENE.hotspots, CURRENT_SCENE.hotspot_count, &in_hotspot, GetMousePosition());
        if (IsKeyDown(KEY_ONE))
        {
            refresh_hotspots(&CURRENT_SCENE);
            if (CURRENT_SCENE.hotspot_count == 0)
            {
                DrawText("NO HOTSPOTS", GetScreenWidth() / 2, GetScreenHeight() / 2, 20, RED);
            }
            demo_draw_hotspots(CURRENT_SCENE.hotspots, CURRENT_SCENE.hotspot_count);
            Hotspot h = CURRENT_SCENE.hotspots[CURRENT_HOTSPOT_IN_SCENE];
            if(h.type == INSPECT) DrawText(h.description, GetScreenWidth() / 2, GetScreenHeight() / 2, 20, RED);
            if(h.type == TRANSITION) DrawText(TextFormat("Transition to: %i", h.edge.to), GetScreenWidth() / 2, GetScreenHeight() / 2, 20, RED);
        }

        if (in_hotspot)
        {
            draw_hotspot(CURRENT_SCENE.hotspots[CURRENT_HOTSPOT_IN_SCENE]);
            DrawText(TextFormat("Hotspot: %i", CURRENT_HOTSPOT_IN_SCENE), demo_res_x / 2, demo_res_y * .16, 20, BLUE);
            switch(CURRENT_SCENE.hotspots[CURRENT_HOTSPOT_IN_SCENE].type)
                {
                    case TRANSITION:
                        DrawText(TextFormat("Transition to: %i", CURRENT_SCENE.hotspots[CURRENT_HOTSPOT_IN_SCENE].edge.to), demo_res_x / 2, demo_res_y * .2, 20, RED);
                        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) 
                        {
                            update_scene(&CURRENT_SCENE, &DEMO_GAME_SCENES[CURRENT_SCENE.hotspots[CURRENT_HOTSPOT_IN_SCENE].edge.to], false, DEMO_GAME_TEXTURES);
                        }
                        break;
                    case INSPECT:
                        DrawText(CURRENT_SCENE.hotspots[CURRENT_HOTSPOT_IN_SCENE].description, demo_res_x / 2, demo_res_y * .2, 20, RED);
                        break;
                    default:
                        break;
                }    
        }
    DrawTexture(current_cursor_texture, GetMouseX(), GetMouseY(), WHITE);
    EndMode2D();
    // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW //
    EndDrawing();
}

UnloadTexture(CURRENT_SCENE.background);
UnloadTexture(current_cursor_texture);
const size_t tx_c = sizeof(DEMO_GAME_TEXTURES) / sizeof(DEMO_GAME_TEXTURES[0]);
for (size_t i = 0; i < tx_c; i++)
{
    UnloadTexture(DEMO_GAME_TEXTURES[i]);
}
free(CURRENT_SCENE.hotspots);
CloseWindow();
return;
}