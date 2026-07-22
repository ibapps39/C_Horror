#include <stdio.h>
#include <stddef.h>
#include "raylib.h"
#include "raymath.h"
typedef Vector2 Vec2;

#define DEMO_MAX_IMAGES 3
#define DEMO_FPS 60
#define DEMO_MAX_HOTSPOTS 5
#define DEMO_WINDOW_TITLE "DEMO_RAYLIB_HORROR_POINT_CLICK"
#define DEMO_WINDOW_SETTING FLAG_WINDOW_RESIZABLE
#define DEMO_MAX_SOUNDS 1
// GLOBALS
int DEMO_RES_X = 400;
int DEMO_RES_Y = 600;


const char *DEMO_IMAGE_DIR = "./resources/images/";
const char *DEMO_MUSIC_DIR = "./resources/music/";
const char *DEMO_SOUND_DIR = "./resources/sounds";

Image DEMO_IMAGES[DEMO_MAX_IMAGES];
Texture DEMO_TEXTURES[DEMO_MAX_IMAGES];
Sound DEMO_SOUNDS[DEMO_MAX_SOUNDS];
Camera2D DEMO_CAM = (Camera2D){0};
bool DEMO_IN_HOTZONE = false;

bool demo_load_images(const char *dir, Image *dst)
{
    FilePathList images_dir = LoadDirectoryFiles(dir);

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

typedef struct HOTSPOT
{
    Rectangle r;
    Vec2 pos;
} Hotspot;

Hotspot demo_create_hotspot(Vec2 pos, float width, float height)
{
    Rectangle rec = {
        .x = pos.x,
        .y = pos.y,
        .width = width,
        .height = height};
    Hotspot h =
        {
            .pos = pos,
            .r = rec};
    return h;
}

bool demo_in_hotzone(const Rectangle hotspot_rec, const Vec2 cursor_pos)
{
    return CheckCollisionPointRec(cursor_pos, hotspot_rec);
}
bool demo_search_hotspots(const Hotspot *hotspots, const int num_hotspots, const Vec2 cursor_pos)
{
    for (size_t i = 0; i < num_hotspots; ++i)
    {
        if (demo_in_hotzone(hotspots[i].r, cursor_pos))
        {
            return true;
        }
    }
    return false;
}
Hotspot DEMO_HOTSPOTS[DEMO_MAX_HOTSPOTS];
Hotspot DEMO_HOTSPOT_1 = (Hotspot){0};
Hotspot DEMO_HOTSPOT_2 = (Hotspot){0};
Hotspot DEMO_HOTSPOT_3 = (Hotspot){0};
Hotspot DEMO_HOTSPOT_4 = (Hotspot){0};
Hotspot DEMO_HOTSPOT_5 = (Hotspot){0};
void demo_make_demo_hotspots(void)
{
    Vec2 p = {.x = GetScreenWidth() / 2, .y = GetScreenHeight() / 2};
    int w, h;
    w = 100;
    h = w;
    DEMO_HOTSPOT_1 = demo_create_hotspot(p, w, h);
    p.x = GetScreenWidth() / 9;
    p.y = GetScreenHeight() / 9;
    DEMO_HOTSPOT_2 = demo_create_hotspot(p, w, h);
    p.y = GetScreenHeight() - GetScreenHeight() / 9;
    DEMO_HOTSPOT_3 = demo_create_hotspot(p, w, h);
    p.x = GetScreenWidth() - GetScreenWidth() / 9;
    DEMO_HOTSPOT_4 = demo_create_hotspot(p, w, h);
    p.y = GetScreenHeight() / 9;
    DEMO_HOTSPOT_5 = demo_create_hotspot(p, w, h);

    DEMO_HOTSPOTS[0] = DEMO_HOTSPOT_1;
    DEMO_HOTSPOTS[1] = DEMO_HOTSPOT_2;
    DEMO_HOTSPOTS[2] = DEMO_HOTSPOT_3;
    DEMO_HOTSPOTS[3] = DEMO_HOTSPOT_4;
    DEMO_HOTSPOTS[4] = DEMO_HOTSPOT_5;
}

void demo_draw_hotspot()
{
    for (size_t i = 0; i < DEMO_MAX_HOTSPOTS; i++)
    {
        Hotspot h = DEMO_HOTSPOTS[i];
        DrawRectangleLines(h.r.x, h.r.y, h.r.width, h.r.height, RED);
    }
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
    Image img_i = LoadImage(file_name);
    ImageResize(&img_i, w, h);
    LoadTextureFromImage(img_i);
    UnloadImage(img_i);
    return LoadTextureFromImage(img_i);
}

// could change to
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
} Demo_Scene;

Demo_Scene Demo_Scene_Init(
    const Demo_Node_id id,
    const char* name,
    const char* background,
    const char* sound,
    const Hotspot* hotspots,
    const Demo_Edge* edges,
    const int edge_count
)
{
    Demo_Scene scene;

    scene.scn_name = name;
    scene.scn_background = background;
    scene.scn_sound = sound;
    scene.hotspots = hotspots;
    scene.id = id;
    scene.edges = edges;
    scene.demo_edge_count = edge_count;

    return scene;
}

Demo_Node_id demo_traverse_edge(Demo_Node_id* current_scene_id, const Demo_Edge edge)
{
    *current_scene_id = edge.to;
    return edge.to;
}

typedef struct CURRENT_SCENE
{
    Demo_Node_id id;
    char* name;
    Texture2D background;
    Sound sound;
    Hotspot* hotspots;
    Demo_Edge* edges;
    int edge_count;
} Current_Scene;

void demo_test(void)
{
    
    int demo_res_x = 500;
    int demo_res_y = 500;
    DEMO_RES_X = demo_res_x;
    DEMO_RES_Y = demo_res_y;

    const int default_fps = DEMO_FPS;
    SetConfigFlags(DEMO_WINDOW_SETTING);
    InitWindow(demo_res_x, demo_res_y, DEMO_WINDOW_TITLE);
    SetTargetFPS(DEMO_FPS);
    HideCursor();

    FilePathList demo_sounds = LoadDirectoryFiles(DEMO_SOUND_DIR);
    FilePathList demo_images = LoadDirectoryFiles(DEMO_IMAGE_DIR);

    // "Globals"
    Sound CURRENT_SOUND = (Sound){0};
    Current_Scene CURRENT_SCENE;
    Camera2D DEMO_CAM = demo_get_cam(demo_res_x, demo_res_x);
    int CURRENT_NODE = 0;

    Demo_Scene TEST_SCENE = Demo_Scene_Init(0, "untitled", demo_images.paths[1], demo_sounds.paths[0], NULL, NULL, 0);

    CURRENT_SCENE.id = TEST_SCENE.id;
    CURRENT_SCENE.name = TEST_SCENE.scn_name;
    CURRENT_SCENE.background = LoadTexture(TEST_SCENE.scn_background);
    CURRENT_SCENE.sound = LoadSound(TEST_SCENE.scn_sound);
    CURRENT_SCENE.hotspots = TEST_SCENE.hotspots;
    CURRENT_SCENE.edges = TEST_SCENE.edges;
    CURRENT_SCENE.edge_count = TEST_SCENE.demo_edge_count;

    Texture2D current_cursor_texture = demo_load_texture(demo_images.paths[0], demo_res_x*.05, demo_res_y*.05);
    while (!WindowShouldClose())
    {
        if(!IsSoundPlaying(CURRENT_SCENE.sound))
        if(IsWindowResized()) { 
            demo_res_x = GetScreenWidth(); demo_res_y = GetScreenHeight();
            current_cursor_texture = demo_load_texture(demo_images.paths[0], demo_res_x*.05, demo_res_y*.05);
        }
        float dt = GetFrameTime();
        
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(DEMO_CAM);
        DrawTexture(CURRENT_SCENE.background, 0, 0, WHITE);
        DrawText(CURRENT_SCENE.name, demo_res_x/2, demo_res_y*.1, 20, YELLOW);
        //demo_draw_cursor();
        //demo_draw_hotspots();
        DrawTexture(current_cursor_texture, GetMouseX(), GetMouseY(), WHITE);
        EndMode2D();
        // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW //
        EndDrawing();
    }
    demo_unload_textures(DEMO_TEXTURES, DEMO_MAX_IMAGES);
    CloseWindow();
    return;
}