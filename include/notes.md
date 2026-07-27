# CAVEMAN + YAGNI Answer: Just `#include` It

The most primitive, no-nonsense solution: **make it a header file**. Zero parsing code needed.

## Fix the struct first (avoid pointer weirdness)

```c
// demo_types.h
#define MAX_HOTSPOTS 16
#define MAX_EDGES 16

typedef struct {
    int x, y;
} Hotspot;

typedef struct {
    int from, to;
} Demo_Edge;

typedef struct {
    int id;
    const char *name;
    const char *image_path;
    const char *audio_path;
    Hotspot hotspots[MAX_HOTSPOTS];
    Demo_Edge edges[MAX_EDGES];
    int edge_count;
    int hotspot_count;
} Demo_Scene;
```

## The "file" - just a .h file

```c
// scenes_data.h
#ifndef SCENES_DATA_H
#define SCENES_DATA_H

#include "demo_types.h"

Demo_Scene DEMO_GAME_SCENES[] = {
    {
        .id = 0,
        .name = "untitled",
        .image_path = "resources/images/horror_game_img_0.png",
        .audio_path = "resources/images/freesound_community-cyprus-storm-61421.mp3",
        .hotspots = {
            {100, 100},
            {100, 700},
            {500, 500},
            {700, 100},
            {700, 700}
        },
        .edges = {
            {0, 1}
        },
        .edge_count = 1,
        .hotspot_count = 5
    },
    {
        .id = 1,
        .name = "scene_two",
        // ...
    }
};

#define TOTAL_SCENE_COUNT (sizeof(DEMO_GAME_SCENES) / sizeof(DEMO_GAME_SCENES[0]))

#endif
```

## Use it

```c
// main.c
#include "raylib.h"
#include "scenes_data.h"

int main(void) {
    InitWindow(800, 600, "Demo");
    
    // Just use it - already loaded, no parsing
    for (int i = 0; i < TOTAL_SCENE_COUNT; i++) {
        printf("Scene %d: %s\n", DEMO_GAME_SCENES[i].id, DEMO_GAME_SCENES[i].name);
    }
    
    CloseWindow();
    return 0;
}
```

---

## If You NEED Runtime Loading (modding, etc.)

Then use a dead-simple format, not C syntax:

```
# scenes.txt
0
untitled
resources/images/horror_game_img_0.png
resources/images/freesound_community-cyprus-storm-61421.mp3
5
100 100
100 700
500 500
700 100
700 700
1
0 1
```

```c
// Simple line-by-line reader
void LoadScenes(const char *path, Demo_Scene *scenes, int *count) {
    FILE *f = fopen(path, "r");
    if (!f) return;
    
    char line[256];
    int scene_idx = 0;
    
    while (fgets(line, sizeof(line), f) && scene_idx < *count) {
        Demo_Scene *s = &scenes[scene_idx];
        
        // Skip comments/empty
        if (line[0] == '#' || line[0] == '\n') continue;
        
        sscanf(line, "%d", &s->id);
        fgets(line, sizeof(line), f); // name - strip newline
        line[strcspn(line, "\n")] = 0;
        strncpy((char*)s->name, line, 63);
        fgets(line, sizeof(line), f); line[strcspn(line, "\n")] = 0;
        strncpy((char*)s->image_path, line, 127);
        fgets(line, sizeof(line), f); line[strcspn(line, "\n")] = 0;
        strncpy((char*)s->audio_path, line, 127);
        
        fscanf(f, "%d", &s->hotspot_count);
        for (int i = 0; i < s->hotspot_count; i++) {
            fscanf(f, "%d %d", &s->hotspots[i].x, &s->hotspots[i].y);
        }
        
        fscanf(f, "%d", &s->edge_count);
        for (int i = 0; i < s->edge_count; i++) {
            fscanf(f, "%d %d", &s->edges[i].from, &s->edges[i].to);
        }
        
        scene_idx++;
    }
    *count = scene_idx;
    fclose(f);
}
```

---

## TL;DR

| Approach | When to use |
|----------|-------------|
| `#include "scenes.h"` | Data is static, ships with game (CAVEMAN choice) |
| Simple text parser | Need runtime loading/modding |

**YAGNI says**: Use the `#include` approach until you actually need runtime loading. Don't write a parser for a feature you don't have yet.