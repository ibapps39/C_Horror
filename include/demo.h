#include <stdio.h>
#include <stddef.h>
#include "raylib.h"
#include "raymath.h"
typedef Vector2 Vec2;

#define DEMO_MAX_IMAGES 3
#define DEMO_FPS 60
#define DEMO_WINDOW_TITLE "DEMO_RAYLIB_HORROR_POINT_CLICK"
#define DEMO_WINDOW_SETTING FLAG_WINDOW_RESIZABLE

// GLOBALS
int DEMO_RES_X = 400;
int DEMO_RES_Y = 600;
const char* DEMO_IMAGE_DIR = "./resources/";
Image DEMO_IMAGES[DEMO_MAX_IMAGES];
Texture DEMO_TEXTURES[DEMO_MAX_IMAGES];
Camera2D DEMO_CAM = (Camera2D){0};


bool demo_load_images(const char* dir, Image* dst)
{
    FilePathList images_dir = LoadDirectoryFiles(dir);
    
    if(images_dir.count < 1) {printf("ERROR: NO DETECTED PHOTOS\n"); return false;}
    if(images_dir.count < DEMO_MAX_IMAGES) {printf("ERROR: NOT ENOUGH PHOTOS\n");return false;}
    if(images_dir.count > DEMO_MAX_IMAGES) {printf("ERROR: PHOTOS EXCEED MAX\n");return false;}

    int img_processed = 0;
    for (size_t i = 0; i < images_dir.count; ++i)
    {
        if(!FileExists(images_dir.paths[i])) 
        { 
            printf("ERROR:NO IMAGE FOUND ON FILE %s", images_dir.paths[i]);
            return false;
        }
        Image img = LoadImage(images_dir.paths[i]);
        if(!IsImageValid(img)) { printf("ERROR: IMAGE[%i] IS NOT VALID\n", (int)i); return false;}
        dst[i] = img;
        ++img_processed;
    }
    printf("COMPLETE: Loading Images %i/%i\n", img_processed, images_dir.count);
    return true;
}
bool demo_resize_imgs(Image* img_array, int num_imgs, int scn_width, int scn_height)
{
    if(!img_array){printf("ERROR: NO IMAGES FOUND"); return false;}
    for (size_t i = 0; i < num_imgs; ++i)
    {
        ImageResize(&img_array[i], scn_width, scn_height);
    }
    printf("COMPLETE: Resizing %i Images\n", num_imgs);
    return true;
}
bool demo_load_textures(Image* src, Texture* dst, const int num_imgs)
{
    if(!src) { printf("ERROR: NO DETECTED IMAGES\n"); return false;}
    if(!dst) { printf("ERROR: NO DESTINATION VALID PROVIDED\n"); return false;}
    int tex_processed = 0;
    for (size_t i = 0; i < num_imgs; ++i)
    {
        Texture t = LoadTextureFromImage(src[i]);
        if(!IsTextureValid(t)) {printf("ERROR: TEXTURE[%i] IS NOT VALID\n", (int)i); return false;}
        dst[i] = t;
        ++tex_processed;
    }
    printf("COMPLETE: Loading Textures %i/%i\n", tex_processed, num_imgs);
    return true;
}

bool demo_unload_imgs(Image* imgs, const int num_imgs)
{
    for (size_t i = 0; i < num_imgs; i++)
    {
        UnloadImage(imgs[i]);
    }
    printf("COMPLETE: Unloaded Images %i/%i\n", num_imgs, num_imgs);
    return true;
}
bool demo_unload_textures(Texture* texs, const int num_imgs)
{
    for (size_t i = 0; i < num_imgs; i++)
    {
        UnloadTexture(texs[i]);
    }
    printf("COMPLETE: Unloaded Textures %i/%i\n", num_imgs, num_imgs);
    return true;
}
//centered at res/2 or res center
Camera2D demo_set_cam(const int scn_w, const int scn_h) {
    Camera2D cam = {0};
    cam.target = (Vec2){.x = scn_w/2, .y = scn_h/2};
    cam.offset = (Vec2){.x = scn_w/2, .y = scn_h/2};
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;
    return cam;
}
// GLOBALS 
void on_screen_resize(void)
{
                DEMO_RES_X = GetScreenWidth(); 
                DEMO_RES_Y = GetScreenHeight();
                DEMO_CAM = demo_set_cam(DEMO_RES_X, DEMO_RES_Y);
                demo_load_images(DEMO_IMAGE_DIR, DEMO_IMAGES);
                demo_resize_imgs(DEMO_IMAGES, DEMO_MAX_IMAGES, DEMO_RES_X, DEMO_RES_Y);
                demo_unload_textures(DEMO_TEXTURES, DEMO_MAX_IMAGES);
                demo_load_textures(DEMO_IMAGES, DEMO_TEXTURES, DEMO_MAX_IMAGES);
                return;
}


void demo_test(void)
{
        int demo_res_x = DEMO_RES_X;
        int demo_res_y = DEMO_RES_Y;
        const int default_fps = DEMO_FPS;
        SetConfigFlags(DEMO_WINDOW_SETTING);
        InitWindow(demo_res_x, demo_res_y, DEMO_WINDOW_TITLE);
        SetTargetFPS(DEMO_FPS);
        bool images_loaded = demo_load_images(DEMO_IMAGE_DIR, DEMO_IMAGES);
        bool resize_images = demo_resize_imgs(DEMO_IMAGES, DEMO_MAX_IMAGES, DEMO_RES_X, DEMO_RES_Y);
        bool textures_loaded = demo_load_textures(DEMO_IMAGES, DEMO_TEXTURES, DEMO_MAX_IMAGES);
        if(textures_loaded) images_loaded = !demo_unload_imgs(DEMO_IMAGES, DEMO_MAX_IMAGES);
        Camera2D demo_cam = demo_set_cam(demo_res_x, demo_res_y);

        while (!WindowShouldClose())
        {
            if(IsWindowResized)
            {
                on_screen_resize();
                demo_cam = DEMO_CAM;
            }
                // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC //
                float dt = GetFrameTime(); 
                // DRAWING // // // DRAWING // // // DRAWING // // // DRAWING // // // DRAWING // // // DRAWING // //
                BeginDrawing();
                ClearBackground(BLACK);
                BeginMode2D(demo_cam);
                DrawTexture(DEMO_TEXTURES[0], 0, 0, WHITE);
                EndMode2D();
                // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW //
                EndDrawing();
        }
        demo_unload_textures(DEMO_TEXTURES, DEMO_MAX_IMAGES);
        CloseWindow();
        return;
}