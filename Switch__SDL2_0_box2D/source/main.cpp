/*
 * Adaptació del còdic de 3DS:pysincs/box2d + Mini sdl2/sdl2_demo (SDL Demo)
 * 
 *  Mini SDL Demo
 * featuring SDL2 + SDL2_mixer + SDL2_image + SDL2_ttf
 * on Nintendo Switch using libnx
 *
 * Copyright 2018 carsten1ns
 *           2020 WinterMute
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
#include <stdio.h>
#include <box2d/box2d.h>
#include <memory>
#include <math.h>

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <switch.h>


// some switch buttons
#define JOY_A     0
#define JOY_B     1
#define JOY_X     2
#define JOY_Y     3
#define JOY_PLUS  10
#define JOY_MINUS 11
#define JOY_LEFT  12
#define JOY_UP    13
#define JOY_RIGHT 14
#define JOY_DOWN  15

// #define SCREEN_WIDTH  320
#define SCREEN_WIDTH  1280
// #define SCREEN_HEIGHT 240
#define SCREEN_HEIGHT 720

SDL_Texture * render_text(SDL_Renderer *renderer, const char* text, TTF_Font *font, SDL_Color color, SDL_Rect *rect) 
{
    SDL_Surface *surface;
    SDL_Texture *texture;

    surface = TTF_RenderText_Solid(font, text, color);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect->w = surface->w;
    rect->h = surface->h;

    SDL_FreeSurface(surface);

    return texture;
}



int main(int argc, char **argv)
{
    int exit_requested = 0, wait = 25;

    SDL_Texture *dynamicBuffer_tex = NULL, *groundBody_tex = NULL;
    SDL_Rect groundBody_r, dynamicBox_r;
#define TAMANY_LLETRA 36
    SDL_Rect dynamicBuffer_rect = { 0, 0, SCREEN_WIDTH-TAMANY_LLETRA, TAMANY_LLETRA };
    
    char buffer[160];

    SDL_Color colors[] = {
        { 128, 128, 128, 0 }, // gray
        { 255, 255, 255, 0 }, // white
        { 255, 0, 0, 0 },     // red
        { 0, 255, 0, 0 },     // green
        { 0, 0, 255, 0 },     // blue
        { 255, 255, 0, 0 },   // brown
        { 0, 255, 255, 0 },   // cyan
        { 255, 0, 255, 0 },   // purple
    };
    float gravetat = 100.0f;
    b2Vec2 position;
    float angle = 0.0f;

    
    romfsInit();
    chdir("romfs:/");

    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);

    SDL_Window* window = SDL_CreateWindow("Box2D + SDL 2.0", 
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    // Ha aparegut en el github --> Aceleració HW == executar per la GPU
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // SDL_RENDERER_SOFTWARE);
    
    TTF_Init();
    // load font from romfs
    TTF_Font* font = TTF_OpenFont("data/LeroyLetteringLightBeta01.ttf", TAMANY_LLETRA );

    // render text as texture
    dynamicBuffer_tex = render_text(renderer, "Switch: Box2D + SDL 2.0", font, colors[1], &dynamicBuffer_rect);  

    // Define the gravity vector.
    b2Vec2 gravity(0.0f, gravetat); //100.0f);

    // Construct a world object, which will hold and simulate the rigid bodies.
    // std::unique_ptr allows for this to destroy automatically when the program exits
    std::unique_ptr<b2World> world = std::make_unique<b2World>(gravity);

    // Define the ground body.
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, SCREEN_HEIGHT);

    // Call the body factory which allocates memory for the ground body
    // from a pool and creates the ground box shape (also from a pool).
    // The body is also added to the world.
    b2Body *groundBody = world->CreateBody(&groundBodyDef);

    // Define the ground box shape.
    b2PolygonShape groundBox;

    // The extents are the half-widths of the box.
    groundBox.SetAsBox(SCREEN_WIDTH, 32); //320, 32);

    // Add the ground fixture to the ground body.
    groundBody->CreateFixture(&groundBox, 0.0f);

    // Define the dynamic body. We set its position and call the body factory.
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set((float)(SCREEN_WIDTH/2), 0.0f); //152.0f, 0.0f);
    b2Body* body = world->CreateBody(&bodyDef);

    // Define another box shape for our dynamic body.
    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(16.0f, 16.0f);

    // Define the dynamic body fixture.
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;

    // Set the box density to be non-zero, so it will be dynamic.
    fixtureDef.density = 1.0f;
    // Override the default friction.
    fixtureDef.friction = 0.8f;

    // Add the shape to the body.
    body->CreateFixture(&fixtureDef);

    // Prepare for simulation. Typically we use a time step of 1/60 of a
    // second (60Hz) and 10 iterations. This provides a high quality simulation
    // in most game scenarios.
    float timeStep = 1.0f / 60.0f;

    int32_t velocityIterations = 6;
    int32_t positionIterations = 2;

    position = body->GetPosition();
    angle = body->GetAngle();

    // When the world destructor is called, all bodies and joints are freed. This can
    // create orphaned pointers, so be careful about your world management.
      
    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeDefault(&pad);

    HidTouchScreenState state={0};
    hidInitializeTouchScreen(); 
   
   
    // Main loop

    while (!exit_requested && appletMainLoop() ) {
        // Instruct the world to perform a single step of simulation.
        // It is generally best to keep the time step and iterations fixed.
        world->Step(timeStep, velocityIterations, positionIterations);

        // Scan the gamepad. This should be done once for each frame
        padUpdate(&pad);

        // padGetButtonsDown returns the set of buttons that have been
        // newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus)
            break; // break in order to return to hbmenu

        // Com actualitzar la gravetat en el "mon"
        if (kDown & HidNpadButton_A) {
            gravetat += 10.0;       
            gravity.Set(0.0f, gravetat); 
            world->SetGravity( gravity );
        }
        if (kDown & HidNpadButton_B) {
            gravetat -= 10.0;            
            gravity.Set(0.0f, gravetat); 
            world->SetGravity( gravity );
        }
        
        // Una pulsació reinicialitza la posició de l'objecte
        hidGetTouchScreenStates(&state, 1);
        if (state.count > 0) { //!haCanviat && ) {
          world->DestroyBody(body);
          bodyDef.position.Set(state.touches[0].x, state.touches[0].y );
          body = world->CreateBody(&bodyDef);
          body->CreateFixture(&fixtureDef);          
        }

        // Now print the position and angle of the body.
        position = body->GetPosition();
        angle = body->GetAngle();

        snprintf(buffer, sizeof(buffer), "Gravetat: %3.1f Body Pos: %4.2f, %4.2f / Angle: %4.2f\nTouch to position Body!", gravetat, position.x, position.y, angle);
        dynamicBuffer_tex = render_text(renderer, buffer, font, colors[1], &dynamicBuffer_rect);

        /* Render the scene, clear to black */      
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
        SDL_RenderClear(renderer);
     
        // put text on screen
        if (dynamicBuffer_tex)
            SDL_RenderCopy(renderer, dynamicBuffer_tex, NULL, &dynamicBuffer_rect);

        
        b2Vec2 groundPos = groundBody->GetPosition();
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        groundBody_r = {(int)groundPos.x, (int)groundPos.y - 32, SCREEN_WIDTH, 64}; //x, y, 64, 64};
        SDL_RenderFillRect(renderer, &groundBody_r);
//         SDL_RenderCopy(renderer, groundBody_tex, NULL, &groundBody_r);

        /* draw the dynamic box */
        // No cal preguntar per la posició d'este???
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        dynamicBox_r  = {(int)position.x, (int)position.y, 16, 16}; // x, y, 64, 64};
        SDL_RenderFillRect(renderer, &dynamicBox_r);

        
        SDL_RenderPresent(renderer);

        SDL_Delay(wait);
    } //     while (!exit_requested && appletMainLoop() ) {


    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    romfsExit();

    return 0;
}
