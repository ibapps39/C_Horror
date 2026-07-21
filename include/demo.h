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
    const char* scn_name;
    Texture2D scene_background;
    Sound scene_sound;
    Hotspot* hotspots;
    Demo_Node_id id;
    Demo_Edge* edges;
    int demo_edge_count;
} Demo_Scene;

Demo_Scene Demo_Scene_Init(
    const char* name,
    const Texture2D background,
    const Sound sound,
    Hotspot* hotspots,
    Demo_Node_id id,
    Demo_Edge* edges,
    int edge_count)
{
    Demo_Scene new_scene;

    new_scene.scn_name = name;
    new_scene.scene_background = background;
    new_scene.scene_sound = sound;
    new_scene.hotspots = hotspots;
    new_scene.id = id;

    new_scene.edges = edges;
    new_scene.demo_edge_count = edge_count;

    return new_scene;
}

Demo_Node_id demo_traverse_edge(Demo_Node_id* current_scene_id, const Demo_Edge edge)
{
    *current_scene_id = edge.to;
    return edge.to;
}

void demo_load_scene(Demo_Scene* scene)
{
    // draw the background
    DrawTexture(scene->scene_background, 0, 0, WHITE);
    // etc ... IN PROGRESS
}

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

    Demo_Scene TEST_SCENE = Demo_Scene_Init("00", demo_load_texture(demo_images.paths[1], demo_res_x, demo_res_y), LoadSound(demo_sounds.paths[0]), NULL, 0, NULL, 0);

    Sound current_sound = (Sound){0};
    Demo_Scene current_scene = (Demo_Scene){0};
    Camera2D demo_cam = demo_get_cam(demo_res_x, demo_res_x);
    int current_node = 0;
    current_scene = TEST_SCENE;
    current_sound = current_scene.scene_sound;
    while (!WindowShouldClose())
    {
        
        if (!IsSoundPlaying(current_sound))
            PlaySound(current_sound);
        //on_resize(IsWindowResized());
        
        float dt = GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(demo_cam);
        demo_load_scene(&current_scene);
        //demo_draw_cursor();
        EndMode2D();
        // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW //
        EndDrawing();
    }
    demo_unload_textures(DEMO_TEXTURES, DEMO_MAX_IMAGES);
    CloseWindow();
    return;
}