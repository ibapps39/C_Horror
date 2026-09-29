#pragma once
#include "common.h"

Camera2D init_cam(const int scn_w, const int scn_h) {
    Camera2D cam = {0};
    cam.target = (Vec2){.x = scn_w/2, .y = scn_h/2};
    cam.offset = (Vec2){.x = scn_w/2, .y = scn_h/2};
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;
    return cam;
}

void move_cam(Camera2D* cam, const float speed)
{
    //WSAD
    if (IsKeyDown(KEY_W)) cam->offset.y -= speed;
    if (IsKeyDown(KEY_S)) cam->offset.y += speed;
    if (IsKeyDown(KEY_A)) cam->offset.x -= speed;
    if (IsKeyDown(KEY_D)) cam->offset.x += speed;
}
void rotate_cam(Camera2D* cam, const float speed)
{
    if (IsKeyDown(KEY_Q)) cam->rotation -= speed;
    if (IsKeyDown(KEY_E)) cam->rotation += speed;
}

Vec2 get_move_norm() 
{
    Vec2 move = {0};
    if (IsKeyDown(KEY_W)) move.y -= 1;
    if (IsKeyDown(KEY_S)) move.y += 1;
    if (IsKeyDown(KEY_A)) move.x -= 1;
    if (IsKeyDown(KEY_D)) move.x += 1;
    return Vector2Normalize(move);
}
Vec2 scale_norm(Vec2 move, const float scalar) {
    return Vector2Scale(move, scalar);
}
Vec2 apply_move(Vec2 move, Vec2 pos) {
    return Vector2Add(pos, move);
}
void update_cam(Camera2D* cam, float move_speed, float rotate_speed) 
{
    Vec2 move_norm = get_move_norm();
    Vec2 move = scale_norm(move_norm, move_speed);
    cam->offset = apply_move(move, cam->offset);
    rotate_cam(cam, rotate_speed);
    cam->target = apply_move(move, cam->target);
    if(IsKeyDown(KEY_W) || IsKeyDown(KEY_S) || IsKeyDown(KEY_A) || IsKeyDown(KEY_D)) printf("x:%.2f, y:%.2f\n", cam->offset.x, cam->offset.y);
}
