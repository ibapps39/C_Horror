// #include "common.h"
// #include "camera.h"
// #include "world.h"
// #include "game.h"
// #include "image_handling.h"
#include "demo.h"
int main(void)
{
        demo_test();
//         // WINDOW SETUP // // WINDOW INITIALIZATION // // WINDOW SETUP // // WINDOW INITIALIZATION // // WINDOW SETUP // // WINDOW INITIALIZATION // // WINDOW SETUP // // WINDOW INITIALIZATION //
//         int res_x = 1000;
//         int res_y = 1000;
//         const int default_fps = 60;
//         const char *img_dir_path = "./resources/";
//         SetWindowState(FLAG_WINDOW_RESIZABLE);
//         InitWindow(res_x, res_y, "Raylib_PointClick_Horror");
//         SetTargetFPS(60);
// #if DEV_MODE
//         DisableCursor();
// #endif
//         Camera2D player_cam = (Camera2D){0};
//         player_cam = init_cam(res_x, res_y);
//         load_imgs(GAME_IMAGES, NUM_GAME_IMAGES, "./resources/");
//         load_tex(GAME_IMAGES, GAME_TEXTURES, NUM_GAME_IMAGES);
//         clear_images(GAME_IMAGES, NUM_GAME_IMAGES);
//         while (!WindowShouldClose())
//         {
//                 // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC // // GAME LOGIC //
//                 float dt = GetFrameTime(); // around with fps effects! Maybe players track their own???
//                 const float time_passed = GetTime();
// #ifdef DT_LIMIT
//                 if (dt > _DT_LIMIT_)
//                         dt = _DT_LIMIT_; // cap at ~20fps minimum
// #endif
//                 // DRAWING // // // DRAWING // // // DRAWING // // // DRAWING // // // DRAWING // // // DRAWING // //
//                 BeginDrawing();
//                 ClearBackground(BLACK);
//                 BeginMode2D(player_cam);
                
//                 EndMode2D();
// // UI LOGIC // // UI LOGIC // // UI LOGIC // // UI LOGIC // // UI LOGIC // // UI LOGIC // // UI LOGIC // // UI LOGIC // // UI LOGIC // // UI LOGIC //
// #if DEV_MODE
// #define vel_txt TextFormat("player->vel: (x:%.2f, y:%.2f, z:%.2f)", my_player->vel.x, my_player->vel.y, my_player->vel.z)
// #define pos_txt TextFormat("player->pos: (x:%.2f, y:%.2f, z:%.2f)", my_player->pos.x, my_player->pos.y, my_player->pos.z)
// #define stages_txt_0 TextFormat("my_player->grounded): %i", my_player->grounded)
//                 int basex = (int)(res_x / 100);
//                 int basey = (int)(res_y / 4);
//                 int text_s = 20;
//                 int ui_offset = text_s + 20;
//                 DrawFPS(basex, res_y - res_y / 10);
//                 DrawText(vel_txt, basex, basey, text_s, RED);
//                 DrawText(pos_txt, basex, basey + (ui_offset), text_s, RED);
//                 DrawText(stages_txt_0, basex, basey + (ui_offset) * 2, text_s, RED);
// #else
// #endif
//                 // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW // // END_DRAW // END_DRAW // END_DRAW // END_DRAW // END_DRAW //
//                 EndDrawing();
//         }
//         clear_textures(GAME_TEXTURES, NUM_GAME_IMAGES);
//         CloseWindow();
        return 0;
}