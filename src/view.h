# include <SDL2/SDL.h>

#ifndef VIEW_H
# define VIEW_H

typedef struct s_Movement {
    int Left;
    int Right;
    float Up;
    float Down;
} Movement;

typedef struct s_Line {
    float x;
    float y;
    float x2;
    float y2;
} Line;

typedef struct s_View {
	// position on window
    float x;
    float y;

    // map structure
    int** map;
    // position on map
    int map_x;
    int map_y;
    // angle on position
    float angle;
    // field of view
    float angle_fov;
    float speed;
    float speed_angle;
    int deltaTick;

    Line* rays;
    int ray_count;

    SDL_Renderer* renderer;
} View;

#endif
