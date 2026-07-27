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
// GLOBALS
int DEMO_RES_X = 400;
int DEMO_RES_Y = 600;

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

typedef struct HOTSPOT
{
    Rectangle r;
    Vec2 pos;
    Demo_Edge edge;

} Hotspot;

Hotspot demo_create_hotspot(Vec2 pos, float width, float height, Demo_Edge edge)
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
            .edge = edge};
    return h;
}

void demo_make_demo_hotspots(Hotspot *hotspots)
{
    Vec2 p = {.x = GetScreenWidth() / 2, .y = GetScreenHeight() / 2};
    int w, h;
    w = GetScreenWidth() / 10;
    h = w;
    Demo_Edge e = {.from = 0, .to = 0};
    hotspots[0] = demo_create_hotspot(p, w, h, e);
    // Top left
    p.x = GetScreenWidth() * .25;
    p.y = GetScreenHeight() * .25;
    hotspots[1] = demo_create_hotspot(p, w, h, e);
    // Bottom Left
    p.x = GetScreenWidth() * .25;
    p.y = GetScreenHeight() * .75;
    hotspots[2] = demo_create_hotspot(p, w, h, e);
    // Bottom Right
    p.x = GetScreenWidth() * .75;
    p.y = GetScreenHeight() * .75;
    hotspots[3] = demo_create_hotspot(p, w, h, e);
    // Top Right
    p.x = GetScreenWidth() * .75;
    p.y = GetScreenHeight() * .25;
    hotspots[4] = demo_create_hotspot(p, w, h, e);
}

void demo_draw_hotspots(Hotspot *hotspots)
{
    for (size_t i = 0; i < DEMO_MAX_HOTSPOTS; i++)
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
    Image img_i = LoadImage(file_name);
    ImageResize(&img_i, w, h);
    Texture2D t = LoadTextureFromImage(img_i);
    UnloadImage(img_i);
    return t;
}

typedef struct GAME_TEXTURES_DICTIONARY_ENTRY
{
    char* file_name;
    Texture2D texture;
} GAME_TEXTURES_DICTIONARY_ENTRY;
typedef GAME_TEXTURES_DICTIONARY_ENTRY* GAME_TEXTURES_DICTIONARY;
int demo_load_from_textures(const char *file_name, const GAME_TEXTURES_DICTIONARY td)
{
    
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
        scene.hotspots[i] = demo_create_hotspot((Vec2){0}, 0, 0, (Demo_Edge){0});
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

void update_scene(Current_Scene *target, Demo_Scene *source, bool unload_sound, Texture* textures)
{
    UnloadTexture(target->background);
    target->background = demo_load_texture(source->scn_background, GetScreenWidth(), GetScreenHeight());
    target->id = source->id;
    target->name = source->scn_name;
    printf("\n\nBACKGROUND:%s\n", source->scn_background);
    if(unload_sound) UnloadSound(target->sound);
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

void demo_test(void)
{

    int demo_res_x = 500;
    int demo_res_y = 500;
    DEMO_RES_X = demo_res_x;
    DEMO_RES_Y = demo_res_y;

    const int default_fps = DEMO_FPS;
    SetConfigFlags(DEMO_WINDOW_SETTING);
    InitWindow(demo_res_x, demo_res_y, DEMO_WINDOW_TITLE);
    InitAudioDevice();
    assert(IsAudioDeviceReady() == true);
    SetTargetFPS(DEMO_FPS);
    HideCursor();

    FilePathList demo_sounds = LoadDirectoryFilesSorted(DEMO_SOUND_DIR);
    FilePathList demo_images = LoadDirectoryFilesSorted(DEMO_IMAGE_DIR);
    printf("FIRST IMAGE LOADED: %s \n", demo_images.paths[0]);

    // "Globals"
    Sound CURRENT_SOUND = (Sound){0};
    Current_Scene CURRENT_SCENE = (Current_Scene){0};
    Camera2D DEMO_CAM = demo_get_cam(demo_res_x, demo_res_x);
    int CURRENT_NODE = 0;

    const int TOTAL_SCENE_COUNT = 4;
    Demo_Scene DEMO_GAME_SCENES[TOTAL_SCENE_COUNT];

    const int MAX_TEXTURES = 50;
    Texture2D* DEMO_GAME_TEXTURES = (Texture2D*)malloc(sizeof(Texture2D) * MAX_TEXTURES);
    for(size_t i = 0; i < MAX_TEXTURES; ++i)
    {
        DEMO_GAME_TEXTURES[i] = demo_load_texture(demo_images.paths[i], demo_res_x, demo_res_y);
    }

    Demo_Scene TEST_SCENE_0 = Demo_Scene_Init(0, "scene 1", demo_images.paths[1], demo_sounds.paths[0], 1, 5);
    Demo_Scene TEST_SCENE_1 = Demo_Scene_Init(1, "scene 2", demo_images.paths[2], demo_sounds.paths[0], 1, 5);
    Demo_Scene TEST_SCENE_2 = Demo_Scene_Init(2, "scene 3", demo_images.paths[3], demo_sounds.paths[0], 1, 5);
    Demo_Scene TEST_SCENE_3 = Demo_Scene_Init(3, "scene 4", demo_images.paths[4], demo_sounds.paths[0], 1, 5);

    DEMO_GAME_SCENES[0] = TEST_SCENE_0;
    DEMO_GAME_SCENES[1] = TEST_SCENE_1;
    DEMO_GAME_SCENES[2] = TEST_SCENE_2;
    DEMO_GAME_SCENES[3] = TEST_SCENE_3;

    for (int i = 0; i < 4; ++i)
    {
        demo_make_demo_hotspots(DEMO_GAME_SCENES[i].hotspots);
    }

    DEMO_GAME_SCENES[0].hotspots[0].edge.to = DEMO_GAME_SCENES[0].id + 1;
    DEMO_GAME_SCENES[1].hotspots[0].edge.to = DEMO_GAME_SCENES[1].id + 1;
    DEMO_GAME_SCENES[2].hotspots[0].edge.to = DEMO_GAME_SCENES[2].id + 1;
    DEMO_GAME_SCENES[3].hotspots[0].edge.to = DEMO_GAME_SCENES[0].id;

    update_scene(&CURRENT_SCENE, &DEMO_GAME_SCENES[0], true);

    bool in_hotspot = false;
    Texture2D current_cursor_texture = demo_load_texture(demo_images.paths[0], demo_res_x * .05, demo_res_y * .05);
    int CURRENT_HOTSPOT_IN_SCENE = -1;

    while (!WindowShouldClose())
    {
        if (!IsSoundPlaying(CURRENT_SCENE.sound))
        {
            PlaySound(CURRENT_SCENE.sound);
        }
        if (IsWindowResized())
        {
            demo_res_x = GetScreenWidth();
            demo_res_y = GetScreenHeight();
            UnloadTexture(current_cursor_texture);
            UnloadTexture(CURRENT_SCENE.background);
            current_cursor_texture = demo_load_texture(demo_images.paths[0], demo_res_x * .05, demo_res_y * .05);
            CURRENT_SCENE.background = demo_load_texture(DEMO_GAME_SCENES[CURRENT_SCENE.id].scn_background, demo_res_x, demo_res_y);
            demo_make_demo_hotspots(CURRENT_SCENE.hotspots);
            for (int i = 0; i < 4; ++i)
            {
                demo_make_demo_hotspots(DEMO_GAME_SCENES[i].hotspots);
            }
            DEMO_GAME_SCENES[0].hotspots[0].edge.to = DEMO_GAME_SCENES[0].id + 1;
            DEMO_GAME_SCENES[1].hotspots[0].edge.to = DEMO_GAME_SCENES[1].id + 1;
            DEMO_GAME_SCENES[2].hotspots[0].edge.to = DEMO_GAME_SCENES[2].id + 1;
            DEMO_GAME_SCENES[3].hotspots[0].edge.to = DEMO_GAME_SCENES[0].id;
        }
        float dt = GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(DEMO_CAM);
        DrawTexture(CURRENT_SCENE.background, 0, 0, WHITE);
        DrawText(CURRENT_SCENE.name, demo_res_x / 2, demo_res_y * .1, 20, YELLOW);
        DrawText(TextFormat("CURRENT_HOTSPOT_IN_SCENE: %i", CURRENT_HOTSPOT_IN_SCENE), demo_res_x / 2, demo_res_y * .12, 20, YELLOW);
#define in_z TextFormat("%i", (int)in_hotspot)
        DrawText(in_z, demo_res_x / 2, demo_res_y * .14, 20, YELLOW);
        CURRENT_HOTSPOT_IN_SCENE = demo_search_hotspots(CURRENT_SCENE.hotspots, CURRENT_SCENE.hotspot_count, &in_hotspot, GetMousePosition());
        if (IsKeyDown(KEY_ONE))
        {
            for (size_t i = 0; i < 5; ++i)
            {
                draw_hotspot(CURRENT_SCENE.hotspots[i]);
            }
        }
        // maybe just use the bool
        if (in_hotspot)
        {
            draw_hotspot(CURRENT_SCENE.hotspots[CURRENT_HOTSPOT_IN_SCENE]);
            DrawText(TextFormat("Hotspot: %i", CURRENT_HOTSPOT_IN_SCENE), demo_res_x / 2, demo_res_y * .16, 20, BLUE);
            // draw text for hotspot edges
            DrawText(TextFormat("From: %i, To: %i", CURRENT_SCENE.hotspots[CURRENT_HOTSPOT_IN_SCENE].edge.from, CURRENT_SCENE.hotspots[CURRENT_HOTSPOT_IN_SCENE].edge.to), demo_res_x / 2, demo_res_y * .19, 20, BLUE);
            Demo_Node_id nid = CURRENT_SCENE.hotspots[CURRENT_HOTSPOT_IN_SCENE].edge.to;
            DrawText(TextFormat("n(ode)id: %i", nid), demo_res_x / 2, demo_res_y * .25, 20, BLUE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                bool update_sound = TextIsEqual(CURRENT_SCENE.name, DEMO_GAME_SCENES[nid].scn_name);
                update_scene(&CURRENT_SCENE, &DEMO_GAME_SCENES[nid], update_sound);
            }
        }
        DrawTexture(current_cursor_texture, GetMouseX(), GetMouseY(), WHITE);
        EndMode2D();
        // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW //
        EndDrawing();
    }

    UnloadTexture(CURRENT_SCENE.background);
    UnloadTexture(current_cursor_texture);
    free(CURRENT_SCENE.hotspots);
    CloseWindow();
    return;
}