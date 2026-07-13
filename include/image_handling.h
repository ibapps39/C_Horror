#pragma once
#include "common.h"
#include "stddef.h"
#include "stdio.h"


#define NUM_GAME_IMAGES 1 // double check me
Image GAME_IMAGES[NUM_GAME_IMAGES];    
Texture2D GAME_TEXTURES[NUM_GAME_IMAGES];  // what you actually draw

/** GAME VISUAL ASSETS PROTOCOL AND FLOW 
 * IMAGES SHOULD BE PNG
 * NUM_GAME_IMAGES MUST MATCH
 * ALL THOSE IMAGES ARE CONVERTED INTO TEXTURES2D
 * ALL CONVERTED, VALID, TEXTURES ARE LOADED INTO GAME_TEXTURES
 * GAME_IMAGES IS FREED -- UNLOADED TEXTURES ONLY AT END OF GAME SESSION
**/

//include path in img_file_monicker

void load_imgs(Image* dst_img_array, const int NUMBER_OF_MAX_IMAGES,const char* dir)
{
    FilePathList images = LoadDirectoryFiles(dir);
    if(images.count < 1) {printf("WARNING: NO DETECTED PHOTOS\n");}
    if(images.count < NUMBER_OF_MAX_IMAGES) {printf("WARNING: NOT ENOUGH PHOTOS\n");}
    if(images.count > NUMBER_OF_MAX_IMAGES) {printf("WARNING: PHOTOS EXCEED MAX\n");}
    size_t i = 0;
    for (i; i < images.count; ++i)
    {
        if(!FileExists(images.paths[i])) 
        { 
            printf("NO IMAGE FOUND AT %s", images.paths[i]);
            continue;
        }
        dst_img_array[i] = LoadImage(images.paths[i]);
    }
    printf("COMPLETE: Loading Images %i/%i\n", i, images.count);
}
void resize_imgs(Image* img_array, int num_imgs, int scn_width, int scn_height)
{
    if(!img_array){printf("ERROR: NO IMAGES FOUND"); return;}
    for (size_t i = 0; i < num_imgs; ++i)
    {
        ImageResize(&img_array[i], scn_width, scn_height);
    }
    printf("COMPLETE: Resizing Images\n");
}
void load_tex(Image* src_img_array, Texture2D* dst_tex_array, int num_imgs)
{
    if(!src_img_array){printf("ERROR: NO VALID IMAGE ARRAY"); return;}
    if(!dst_tex_array){printf("ERROR: NO VALID TEXTURE ARRAY"); return;}
    if(num_imgs < 1){printf("ERROR: NO VALID NUMBER OF IMAGES"); return;}
    size_t i = 0;
    for (i; i < num_imgs; ++i)
    {
       Texture t = LoadTextureFromImage(src_img_array[i]);
       if(!IsTextureValid(t)) {printf("WARNING: TEXTURE FROM IMAGES[ %i ] NOT VALID TEXTURE\n"); break;}
       dst_tex_array[i] = t;
    }
    printf("COMPLETE: Loading Textures %i/%i\n", i, num_imgs);
}

void clear_images_textures(Image* images, Texture2D* texs, int num_imgs)
{
    if(!images){printf("no images found");}
    if(!texs){printf("no textures found");}
    for (size_t i = 0; i < num_imgs; i++)
    {
        UnloadImage(images[i]);
        UnloadTexture(texs[i]);
    }
}

void clear_images(Image* images, int num_imgs)
{
    if(!images){printf("no images found");}
    for (size_t i = 0; i < num_imgs; i++)
    {
        UnloadImage(images[i]);
    }
}
void clear_textures(Texture2D* texs, int num_imgs)
{
    if(!texs){printf("no textures found");}
    for (size_t i = 0; i < num_imgs; i++)
    {
        UnloadTexture(texs[i]);
    }
}
