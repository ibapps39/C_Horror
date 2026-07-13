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
