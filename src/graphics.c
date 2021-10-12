#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include "view.h"
#include "map.h"

const int ARRAY_LEN = 10;
const int QUAD_SIZE = 32;

#define WIN_W 640
#define WIN_H 480

#define FRAME (1000 / 60)

// #define ONELINE

View createViewer();
Line raycasting(float angle, View *v);
void localizeOnMap(View *v);
void CenterToCellMap(View *v);
void rotateLeftView(View *v);
void rotateRightView(View *v);
void forwardView(View *v);
void backwardView(View *v);
void rayCalculate(View *v);
void drawView(View *v, SDL_Renderer* renderer);
void drawMap(View* v, SDL_Renderer* renderer);
void drawScene(View* v, SDL_Renderer* renderer);
void sdl_ellipse(SDL_Renderer* r, int x0, int y0, int radiusX, int radiusY);


// static_assert( sizeof(int) == 4, "16 bits platform required" );


int main(int argc, char** argv)
{

    /* Initialisation simple */
    if (SDL_Init(SDL_INIT_VIDEO) != 0 )
    {
        fprintf(stdout,"Échec de l'initialisation de la SDL (%s)\n",SDL_GetError());
        return -1;
    }

    Uint32 lastTick = SDL_GetTicks();

    /* Création de la fenêtre */
    SDL_Window* pWindow = NULL;
    SDL_Renderer* renderer = NULL;

    int** map = open_map("data\\map.txt", ARRAY_LEN);

    print_map(map, ARRAY_LEN);
    
    if( SDL_CreateWindowAndRenderer(WIN_W, WIN_H, 0, &pWindow, &renderer) == 0 )
    {
        //
        // création d'une vue
        //
        View v = createViewer();
        v.map = map;
        v.renderer = renderer;

        // initialise le tableau des rayons
        v.rays = (Line*)malloc(WIN_W * sizeof(Line));
        v.ray_count = WIN_W;

        CenterToCellMap(&v);
        Movement m;
        m.Up = 0;
        m.Down = 0;
        m.Left = 0;
        m.Right = 0;

        SDL_bool done = SDL_FALSE;

        int update = 1;

        while (!done)
        {
            v.deltaTick = SDL_GetTicks() - lastTick;

            if(v.deltaTick > FRAME){

                lastTick = SDL_GetTicks();

                if(update){

                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                    SDL_RenderClear(renderer);

                    rayCalculate(&v);

                    drawScene(&v, renderer);

                    drawMap(&v, renderer);

                    drawView(&v, renderer);

                    SDL_RenderPresent(renderer);
                    update = 0;

                }

                SDL_Event event;

                while (SDL_PollEvent(&event)) {
                    switch(event.type) {

                        case SDL_QUIT:
                            done = SDL_TRUE;
                            break;

                        case SDL_KEYDOWN:

                            // test keycode
                            switch ( event.key.keysym.sym ) {
                                case SDLK_w:
                                    m.Up = 1;
                                    break;

                                case SDLK_s:
                                    m.Down = 1;
                                    break;

                                case SDLK_a:
                                    m.Left = 1;
                                    break;

                                case SDLK_d:
                                    m.Right = 1;
                                    break;
                                // etc
                            }
                            break;

                         case SDL_KEYUP:

                            // test keycode
                            switch ( event.key.keysym.sym ) {
                                case SDLK_w:
                                    m.Up = 0;
                                    break;

                                case SDLK_s:
                                    m.Down = 0;
                                    break;

                                case SDLK_a:
                                    m.Left = 0;
                                    break;

                                case SDLK_d:
                                    m.Right = 0;
                                    break;
                                // etc
                            }
                            break;

                    }

                }


                if(m.Up == 1){
                    forwardView(&v);
                    update = 1;
                }else
                if(m.Down == 1){
                    backwardView(&v);
                    update = 1;
                }
                if(m.Left == 1){
                    rotateLeftView(&v);
                    update = 1;
                }else
                if(m.Right == 1){
                    rotateRightView(&v);
                    update = 1;
                }


            }
        }

        SDL_Delay(10);

        free(v.rays);

    } else {
        fprintf(stderr,"Erreur de création de la fenêtre: %s\n",SDL_GetError());
    }

    for (int y=0; y < ARRAY_LEN; ++y ) {
        free(map[y]);
    }
    free(map);

    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (pWindow) {
        SDL_DestroyWindow(pWindow);
    }

    SDL_Quit();

    return 0;
}


View createViewer()
{
    View v;
    v.x = 40;
    v.y = 40;
    v.angle = M_PI * 2;
//    v.angle_fov = 60.0f;
    v.angle_fov = 60.0f;
    v.speed = 30.0f;
    v.speed_angle = 4.0f;

    v.map_x = 0;
    v.map_y = 0;
    localizeOnMap(&v);

    return v;

}

Line raycastingHorizontal(float angle, View *v)
{

    if(angle < 0){
        angle = (2 * M_PI) + angle;
    }
    angle = fmod(angle, (double)(2 * M_PI));

    Line ray;

    ray.x = v->x;
    ray.y = v->y;

    ray.x2 = v->x;
    ray.y2 = v->y;

    float ry, xy, rxo, ryo;
    int map_x, map_y;
    // distance of field
    int hit = 0;

    float unDemiPi = M_PI + M_PI / 2;
    float demiPi = M_PI / 2;

    //if(angle < M_PI){
    if( angle > unDemiPi || angle < demiPi ){

        ry = (QUAD_SIZE - (v->x - (v->map_x * QUAD_SIZE))) * tanf( angle );

        ryo = QUAD_SIZE * tanf( angle );
        rxo = QUAD_SIZE;

        ray.y2 = v->y + ry;
        ray.x2 = (((float)v->map_x + 1) * QUAD_SIZE);

#if defined(ONELINE)
        sdl_ellipse(v->renderer, ray.x2, ray.y2, 5, 5);
#endif // defined

        map_x = ray.x2 / QUAD_SIZE;
        map_y = ray.y2 / QUAD_SIZE;


    }

    //if(angle == M_PI || angle == 0.0){
    if( angle < unDemiPi && angle > demiPi ){

        ry = - (v->x - ((v->map_x) * QUAD_SIZE)) * tanf( angle );

        ryo = -QUAD_SIZE * tanf( angle );
        rxo = -QUAD_SIZE;

        ray.y2 = v->y + ry;
        ray.x2 = ((v->map_x) * QUAD_SIZE);

#if defined(ONELINE)
        sdl_ellipse(v->renderer, ray.x2, ray.y2, 5, 5);
#endif // defined

        map_x = ray.x2 / QUAD_SIZE;
        map_y = ray.y2 / QUAD_SIZE;

        map_x--;

    }

    if( angle == unDemiPi || angle == demiPi ){
        map_x = 0;
        map_y = 0;
        hit = 1;
    }

    while (!hit){

        if(map_x > 0 && map_x < ARRAY_LEN && map_y > 0 && map_y < ARRAY_LEN)
        {
            if(v->map[map_y][map_x] == 1){

                hit = 1;

            } else {

                ray.x2 += rxo;
                ray.y2 += ryo;

#if defined(ONELINE)
                sdl_ellipse(v->renderer, ray.x2, ray.y2, 5, 5);
#endif // defined

                map_x = ray.x2 / QUAD_SIZE;
                map_y = ray.y2 / QUAD_SIZE;

                if( angle < unDemiPi && angle > demiPi ){
                    map_x--;
                }

            }

        } else {

            hit = 1;

        }

    }

    return ray;
}


Line raycastingVertical(float angle, View *v)
{

    if(angle < 0){
        angle = (2 * M_PI) + angle;
    }
    angle = fmod(angle, (double)(2 * M_PI));

    Line ray;

    ray.x = v->x;
    ray.y = v->y;

    ray.x2 = v->x;
    ray.y2 = v->y;

    float rx, xy, rxo, ryo;
    int map_x, map_y;
    // distance of field
    int hit = 0;

    if(angle > M_PI){

        rx = (v->y - (v->map_y * QUAD_SIZE)) / - tanf( angle );

        rxo = QUAD_SIZE / -tanf( angle );
        ryo = -QUAD_SIZE;

        ray.x2 = v->x + rx;
        ray.y2 = ((v->map_y) * QUAD_SIZE);

#if defined(ONELINE)
        sdl_ellipse(v->renderer, ray.x2, ray.y2, 5, 5);
#endif // defined

        map_x = ray.x2 / QUAD_SIZE;
        map_y = (ray.y2) / QUAD_SIZE;
        if(angle > M_PI){
            map_y--;
        }

        //printf( "0: %d, %d, Pos : %f, %f, Map : %d, %d \n", v->map_x, v->map_y, ray.x2, ray.y2, map_x, map_y);

    }

    if(angle < M_PI){

        rx = (v->y - ((v->map_y + 1) * QUAD_SIZE)) / - tanf( angle );

        rxo = QUAD_SIZE / tanf( angle );
        ryo = QUAD_SIZE;

        ray.x2 = v->x + rx;
        ray.y2 = ((v->map_y + 1) * QUAD_SIZE);

#if defined(ONELINE)
        sdl_ellipse(v->renderer, ray.x2, ray.y2, 5, 5);
#endif // defined

        map_x = ray.x2 / QUAD_SIZE;
        map_y = ray.y2 / QUAD_SIZE;

        //printf( "0: %d, %d, Pos : %f, %f, Map : %d, %d \n", v->map_x, v->map_y, ray.x2, ray.y2, map_x, map_y);

    }

    if(angle == M_PI || angle == 0.0){
        map_x = 0;
        map_y = 0;
        hit = 1;
    }

    while (!hit){

        if(map_x > 0 && map_x < ARRAY_LEN && map_y > 0 && map_y < ARRAY_LEN)
        {
            if(v->map[map_y][map_x] == 1){

                hit = 1;

            } else {

                ray.x2 += rxo;
                ray.y2 += ryo;

#if defined(ONELINE)
        sdl_ellipse(v->renderer, ray.x2, ray.y2, 5, 5);
#endif // defined

                map_x = ray.x2 / QUAD_SIZE;
                map_y = ray.y2 / QUAD_SIZE;
                if(angle > M_PI){
                    map_y--;
                }

            }

        } else {

            hit = 1;

        }

    }

    return ray;
}


Line raycasting(float angle, View *v){

#if defined(ONELINE)
    SDL_SetRenderDrawColor(v->renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
#endif // defined
    Line rayh = raycastingHorizontal(angle, v);
#if defined(ONELINE)
    SDL_SetRenderDrawColor(v->renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
#endif // defined
    Line rayv = raycastingVertical(angle, v);

    if( sqrtf(abs(rayh.x2 - rayh.x)) + sqrtf(abs(rayh.y2 - rayh.y)) > sqrtf(abs(rayv.x2 - rayv.x)) + sqrtf(abs(rayv.y2 - rayv.y)) ){
        return rayv;
    } else {
        return rayh;
    }

}


void CenterToCellMap(View *v)
{
	v->x = (int)(v->map_x * QUAD_SIZE) + (QUAD_SIZE / 2);
	v->y = (int)(v->map_y * QUAD_SIZE) + (QUAD_SIZE / 2);
}

void localizeOnMap(View *v)
{
	v->map_x = (int)(v->x / QUAD_SIZE);
	v->map_y = (int)(v->y / QUAD_SIZE);
}

void rotateLeftView(View *v)
{
    v->angle -= v->speed_angle * (v->deltaTick / 1000.0f);
    if(v->angle < 0){
        v->angle = (2 * M_PI) + v->angle;
    }
    v->angle = fmod(v->angle, (double)(2 * M_PI));

    //printf("%f\n", v->angle);

}

void rotateRightView(View *v)
{

    v->angle += v->speed_angle * (v->deltaTick / 1000.0f);
    if(v->angle < 0){
        v->angle = (2 * M_PI) + v->angle;
    }
    v->angle = fmod(v->angle, (double)(2 * M_PI));

    //printf("%f\n", v->angle);
}

void forwardView(View *v)
{
    float len = v->speed * (v->deltaTick / 1000.0f);

    float y = sinf( v->angle ) * len;
    float x = cosf( v->angle ) * len;

    v->y += y;
    v->x += x;

    localizeOnMap(v);
}

void backwardView(View *v)
{
    float len = -v->speed * (v->deltaTick / 1000.0f);

    float y = sinf( v->angle ) * len;
    float x = cosf( v->angle ) * len;

    v->y += y;
    v->x += x;

    localizeOnMap(v);
}

void drawMap(View* v, SDL_Renderer* renderer)
{
    int** map = v->map;

    for (int y=0; y < ARRAY_LEN; ++y ) {
        for (int x=0; x < ARRAY_LEN; ++x ) {

            SDL_Rect srcrect;

            srcrect.x = QUAD_SIZE * x;
            srcrect.y = QUAD_SIZE * y;
            srcrect.w = QUAD_SIZE;
            srcrect.h = QUAD_SIZE;

            if(map[y][x] == 1) {

                SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);
                SDL_RenderFillRect(renderer, &srcrect);

            }

            SDL_SetRenderDrawColor(renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawRect(renderer, &srcrect);

        }
    }
}

//
// effectue un calcule de rayon pour chaque ligne
//
void rayCalculate(View *v){

    float min_fov = v->angle -((v->angle_fov /2) * M_PI / 180);
    float max_fov = v->angle +((v->angle_fov /2) * M_PI / 180);

    float step_fov = (max_fov - min_fov) / v->ray_count;

    float ray_angle = min_fov;

    for(int i = 0; i < v->ray_count; i++){
        v->rays[i] = raycasting(ray_angle, v);
        ray_angle += step_fov;
    }

}

//
// Dessine la mini map
//
void drawView(View* v, SDL_Renderer* renderer)
{
    float len = 10.0f;

    float y = sinf( v->angle ) * len;
    float x = cosf( v->angle ) * len;

    float min_fov = v->angle -((v->angle_fov /2) * M_PI / 180);
    float max_fov = v->angle +((v->angle_fov /2) * M_PI / 180);

    float step_fov = (max_fov - min_fov) / v->ray_count;

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, (int)v->x, (int)v->y, (int)(v->x + x), (int)(v->y + y));

    float ray_angle = min_fov;

#if defined(ONELINE)

    Line ray = raycasting(v->angle, v);
    SDL_RenderDrawLine(renderer, ray.x, ray.y, ray.x2, ray.y2);

#else

    for(int i = 0; i < v->ray_count; i++){

        Line ray = v->rays[i];
        SDL_RenderDrawLine(renderer, ray.x, ray.y, ray.x2, ray.y2);

    }
    /*
    for(int i = 0; i < v->ray_count; i++){

        Line ray = raycasting(ray_angle, v);
        SDL_RenderDrawLine(renderer, ray.x, ray.y, ray.x2, ray.y2);
        ray_angle += step_fov;
    }
    */
#endif

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
/*
    len = 30.0f;

    y = sinf( min_fov ) * len;
    x = cosf( min_fov ) * len;
    SDL_RenderDrawLine(renderer, (int)v->x, (int)v->y, (int)(v->x + x), (int)(v->y + y));

    y = sinf( max_fov ) * len;
    x = cosf( max_fov ) * len;
    SDL_RenderDrawLine(renderer, (int)v->x, (int)v->y, (int)(v->x + x), (int)(v->y + y));
*/

}

/**
 * dessine le monde en 3d
 *
 */
void drawScene(View* v, SDL_Renderer* renderer)
{

    float angle_pi = (v->angle_fov * M_PI / 180);
    float demi = angle_pi / 2;
    float angle_step = angle_pi / v->ray_count;
    float cos_a = -demi;
    float correction;


    for(int i = 0; i < v->ray_count; i++){

        if(cos_a < 0){
            correction = cosf(-cos_a);
        } else {
            correction = cosf(cos_a);
        }
        cos_a += angle_step;

        //printf("%f, %f, %f\n", correction, (cos_a), i);

        Line ray = v->rays[i];
        float p = sqrtf((ray.y2 - ray.y) * (ray.y2 - ray.y) + (ray.x2 - ray.x) * (ray.x2 - ray.x));

        p = p * correction;

        p = ((320.0f - p) / 320.0f);
        int h = (int)(p * (float)(WIN_H / 2.0));

        SDL_SetRenderDrawColor(renderer, 0, (p * 200), 0, SDL_ALPHA_OPAQUE);

        SDL_RenderDrawLine(renderer, i, (int)((WIN_H / 2) - h), i, (int)((WIN_H / 2) + h));

    }

}

//draw one quadrant arc, and mirror the other 4 quadrants
void sdl_ellipse(SDL_Renderer* r, int x0, int y0, int radiusX, int radiusY)
{
    float pi  = 3.14159265358979323846264338327950288419716939937510;
    float pih = pi / 2.0; //half of pi

    //drew  28 lines with   4x4  circle with precision of 150 0ms
    //drew 132 lines with  25x14 circle with precision of 150 0ms
    //drew 152 lines with 100x50 circle with precision of 150 3ms
    const int prec = 27; // precision value; value of 1 will draw a diamond, 27 makes pretty smooth circles.
    float theta = 0;     // angle that will be increased each loop

    //starting point
    int x  = (float)radiusX * cos(theta);//start point
    int y  = (float)radiusY * sin(theta);//start point
    int x1 = x;
    int y1 = y;

    //repeat until theta >= 90;
    float step = pih/(float)prec; // amount to add to theta each time (degrees)
    for(theta=step;  theta <= pih;  theta+=step)//step through only a 90 arc (1 quadrant)
    {
        //get new point location
        x1 = (float)radiusX * cosf(theta) + 0.5; //new point (+.5 is a quick rounding method)
        y1 = (float)radiusY * sinf(theta) + 0.5; //new point (+.5 is a quick rounding method)

        //draw line from previous point to new point, ONLY if point incremented
        if( (x != x1) || (y != y1) )//only draw if coordinate changed
        {
            SDL_RenderDrawLine(r, x0 + x, y0 - y,    x0 + x1, y0 - y1 );//quadrant TR
            SDL_RenderDrawLine(r, x0 - x, y0 - y,    x0 - x1, y0 - y1 );//quadrant TL
            SDL_RenderDrawLine(r, x0 - x, y0 + y,    x0 - x1, y0 + y1 );//quadrant BL
            SDL_RenderDrawLine(r, x0 + x, y0 + y,    x0 + x1, y0 + y1 );//quadrant BR
        }
        //save previous points
        x = x1;//save new previous point
        y = y1;//save new previous point
    }
    //arc did not finish because of rounding, so finish the arc
    if(x!=0)
    {
        x=0;
        SDL_RenderDrawLine(r, x0 + x, y0 - y,    x0 + x1, y0 - y1 );//quadrant TR
        SDL_RenderDrawLine(r, x0 - x, y0 - y,    x0 - x1, y0 - y1 );//quadrant TL
        SDL_RenderDrawLine(r, x0 - x, y0 + y,    x0 - x1, y0 + y1 );//quadrant BL
        SDL_RenderDrawLine(r, x0 + x, y0 + y,    x0 + x1, y0 + y1 );//quadrant BR
    }
}

