// Conversió a SDL 2.0 per a la Switch de l'exemple gpusprites (de 3DS en Citro2D)
// Referencias:
// * sdl2-demo (devkitpro, Switch)
// * sdl2-simple (devkitpro, Switch)
// * SDL 1.2 to 2.0 Migration Guide <// https://wiki.libsdl.org/SDL2/MigrationGuide>
// * Getting started with the SDL2 library for Game Development <// https://blog.conan.io/2023/07/20/introduction-to-game>-dev-with-sdl2.html
// 
// ryujinx.sh sdl2_0-sprites.nro 
// No funciona nxlink???
// nxlink `hostname -i` -s sdl2_0-sprites.nro & ryujinx.sh sdl2_0-sprites.nro 
// $ nxlink -a `hostname -i` -s nxlink_stdio.nro
// Connection to 158.42.180.130 failed
// $ nxlink -a 127.0.0.1  -s nxlink_stdio.nro
// Connection to 127.0.0.1 failed
// $ nxlink -a 127.0.1.1  -s nxlink_stdio.nro
// Connection to 127.0.1.1 failed


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> //time
#include <sys/resource.h>
#include <sys/unistd.h> // chdir
#include <arpa/inet.h> // inet_ntoa
#include <switch.h>

#include "llistaSprites.h" // els NSPRITES i llistaSprites



// Per a depurar fent ús de printfs 
// De $DEVKITPRO/switch/network/nxlink_stdio/source/main.c
//i de la major part de exemples de gràfics

//-----------------------------------------------------------------------------
// nxlink support
//-----------------------------------------------------------------------------

#ifndef ENABLE_NXLINK
#define TRACE(fmt,...) ((void)0)
#else
#include <unistd.h>
#define TRACE(fmt,...) printf("%s: " fmt "\n", __PRETTY_FUNCTION__, ## __VA_ARGS__)

static int s_nxlinkSock = -1;


static void initNxLink()
{
    if (R_FAILED(socketInitializeDefault()))
        return;
    
    s_nxlinkSock = nxlinkStdio();
    if (s_nxlinkSock >= 0)
        TRACE("printf output now goes to nxlink server");
    else
        socketExit();
}

static void deinitNxLink()
{
    if (s_nxlinkSock >= 0)
    {
        close(s_nxlinkSock);
        socketExit();
        s_nxlinkSock = -1;
    }
}

extern "C" void userAppInit()
{
    initNxLink();
}

extern "C" void userAppExit()
{
    deinitNxLink();
}

#endif




#include <SDL.h>
#include <SDL_image.h> // IMG_Load
#include <SDL_ttf.h>   // TTF_Font, ...


#define MAX_SPRITES   1024
//768
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

int tamanyLletra = 36;
SDL_Rect mensatges_rect, mensatges_rect2;
// SDL_Surface *mensatges_surf = NULL;
// SDL_Texture *mensatges_tex = NULL;
TTF_Font* font;


// Copie les estructures x als sprites mantenint el nom per minimitzar canvits durant la portabilitat
typedef struct {
    struct {
        float   x;
        float   y;
        float   w;
        float   h;
    } 	pos;
    struct {
        float   x;
        float   y;
    } 	center;
    float 	depth;
    float 	angle;
}  C2D_DrawParams;

typedef struct {
    //   SDL_Surface      *image; //REPENSAR?
    SDL_Texture     *image;  // Recorda SDL2: SDL_Surface (CPU) vs SDL_Texture (GPU)
    //Pero puc anar generant textures a partir de Surfaces i rotar-les, etc. Que en Textures no es pot fer, no?
    C2D_DrawParams    params;
} C2D_Sprite;

// Simple sprite struct
typedef struct {
    C2D_Sprite spr;
    float dx, dy; // velocity
} Sprite;



static Sprite sprites[MAX_SPRITES];
static size_t numSprites = MAX_SPRITES/2;
// SDL 2.0
SDL_Window *window;
// SDL_Texture *pantalla;
SDL_Renderer *renderer = NULL;

SDL_Color colors[] = {
    { 128, 128, 128, 0 }, // gray
    { 255, 255, 255, 0 }, // white
    { 255, 0, 0, 0 },     // red
    {   0, 255, 0, 0 },     // green
    {   0, 0, 255, 0 },     // blue
    { 255, 255, 0, 0 },   // brown
    {   0, 255, 255, 0 },   // cyan
    { 255, 0, 255, 0 },   // purple
};

//     int col = 0, snd = 0;


static void deinitSprites() {
    
    for (size_t i = 0; i < MAX_SPRITES; i++)
    {
        // 		SDL_FreeSurface( sprites[i].spr.image );
        SDL_DestroyTexture( sprites[i].spr.image );
    }
}



// SDL_Surface * render_text(SDL_Surface *renderer, const char* text, TTF_Font *font, 
//                           SDL_Color color, SDL_Rect *rect) 
// SDL_Surface * render_text(SDL_Window *laPantalla, const char* text, TTF_Font *font, 
//                           SDL_Color color, SDL_Rect *rect) 
// SDL_Surface * render_text(const char* text, TTF_Font *font,
void render_text(const char* text, TTF_Font *font, 
            SDL_Color color, SDL_Rect *rect, SDL_Renderer *renderer )
{
    SDL_Surface *mensatges_surf;
    SDL_Texture *mensatges_tex = NULL;

    mensatges_surf = TTF_RenderText_Solid(font, text, color);
    rect->w = mensatges_surf->w;
    rect->h = mensatges_surf->h;
    
//     return surface;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
    SDL_RenderFillRect(renderer, rect);
    mensatges_tex = SDL_CreateTextureFromSurface(renderer, mensatges_surf);
    SDL_RenderCopy(renderer, mensatges_tex, NULL, rect);
    SDL_DestroyTexture( mensatges_tex );    
}



//---------------------------------------------------------------------------------
static void initSprites() {
    //---------------------------------------------------------------------------------
    SDL_Surface      *unaImage;
    
    size_t numImages = NSPRITES;
    srand(time(NULL));
    
    for (size_t i = 0; i < MAX_SPRITES; i++)
    {
        Sprite* sprite = &sprites[i];
        
        // Random image, position, rotation and speed
        unaImage = IMG_Load( llistaSprites[rand() % numImages] );
        if (unaImage) {
            sprite->spr.params.pos.w = unaImage->w;  // Sobreescriu la 3a i 4a component, que serveixen per reescalar el tamany de la imatge
            sprite->spr.params.pos.h = unaImage->h;  // si no coincideix en el tamany d'esta.
            sprite->spr.image = SDL_CreateTextureFromSurface(renderer, unaImage);
            //         SDL_FreeSurface( unaImage ); // Esta al final de este bucle for
        }       
        
        
        sprite->spr.params.center.x = unaImage->w/2; //0.5f;
        sprite->spr.params.center.y = unaImage->h/2; //0.5f;
        
        sprite->spr.params.pos.x = rand() % SCREEN_WIDTH;
        sprite->spr.params.pos.y = rand() % SCREEN_HEIGHT;
        
        sprite->spr.params.angle = rand()/(float)RAND_MAX;
        sprite->dx = rand()*4.0f/RAND_MAX - 2.0f;
        sprite->dy = rand()*4.0f/RAND_MAX - 2.0f;
        
        SDL_FreeSurface( unaImage );               
    }
}

//---------------------------------------------------------------------------------
static void moveSprites() {
    //---------------------------------------------------------------------------------
    
//     SDL_Rect pos = { 0, 0, 0, 0 };
    Sprite *sprite;
    
    for (size_t i = 0; i < numSprites; i++)	{            
        sprite = &sprites[i];
        // Calcula quant avança 
//         pos.x = sprite->spr.params.pos.x + sprite->dx;
//         pos.y = sprite->spr.params.pos.y + sprite->dy;
//         pos.w = sprite->spr.params.pos.w;
//         pos.h = sprite->spr.params.pos.h;        
        //i actualitza-ho
        sprite->spr.params.pos.x = sprite->spr.params.pos.x + sprite->dx;
        sprite->spr.params.pos.y = sprite->spr.params.pos.y + sprite->dy;
        
        if (sprite->spr.params.angle < 350 )
            sprite->spr.params.angle += 10;
        else
            sprite->spr.params.angle = 0;
        
        // Check for collision with the screen boundaries
        if ((sprite->spr.params.pos.x < sprite->spr.params.pos.w / 2.0f && sprite->dx < 0.0f) ||
            (sprite->spr.params.pos.x > (SCREEN_WIDTH-(sprite->spr.params.pos.w / 2.0f)) && sprite->dx > 0.0f))
            sprite->dx = -sprite->dx;
        
        if ((sprite->spr.params.pos.y < sprite->spr.params.pos.h / 2.0f && sprite->dy < 0.0f) ||
            (sprite->spr.params.pos.y > (SCREEN_HEIGHT-(sprite->spr.params.pos.h / 2.0f)) && sprite->dy > 0.0f))
            sprite->dy = -sprite->dy;
        
    }
    
}


// int rand_range(int min, int max){
//    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
// }
// 

//---------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    //---------------------------------------------------------------------------------
    int exit_requested = 0;
    Uint32 start_time, stop_time, nCuadres, nTics;    
    clock_t start, end;
    double cpu_time_used;
    int wait = 25;
    
    srand(time(NULL));
    romfsInit();
    chdir("romfs:/");
    
    
    // iniciar nxlinkStdio
    // Initialise sockets
    socketInitializeDefault();
    printf("Hello World!\n");
    
    // Display arguments sent from nxlink
    printf("%d arguments\n", argc);
    
    for (int i=0; i<argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    
    // the host ip where nxlink was launched
    printf("nxlink host is %s\n", inet_ntoa(__nxlink_host));
    
    // redirect stdout & stderr over network to nxlink
    nxlinkStdio();
    
    // this text should display on nxlink host
    printf("printf output now goes to nxlink server\n");
    //fi de iniciar nxlinkStdio
    
    
    
    //     SDL_Init(SDL_INIT_VIDEO ); //|SDL_INIT_TIMER);
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
    SDL_ShowCursor( 0 ); // No vull que es veja
    
    // Create screen
    window = SDL_CreateWindow("SDL 2.0 sprites en Switch", 
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    //     renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE); 
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED );
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGBA    
    SDL_RenderClear( renderer );
    printf("renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);\n");
    
    IMG_Init(IMG_INIT_PNG); // Conter a les dependències: miara el Makefile que he habut de modificar
    TTF_Init();
    // load font from romfs
    //     font = TTF_OpenFont("ttf/LeroyLetteringLightBeta01.ttf", 36);
    font = TTF_OpenFont("ttf/liberationitalic.ttf", tamanyLletra);    
    
    mensatges_rect.x = 0;
    mensatges_rect.y = SCREEN_HEIGHT - tamanyLletra;
    mensatges_rect.w = 0;
    mensatges_rect.h = 0;
    mensatges_rect2.x = 0;
    mensatges_rect2.y = SCREEN_HEIGHT - (tamanyLletra *2);
    mensatges_rect2.w = 0; 
    mensatges_rect2.h = 0;
    
//     mensatges_surf = render_text("SDL 2.0 sprites!", font, colors[1], &mensatges_rect);
    render_text("SDL 2.0 sprites!", font, colors[1], &mensatges_rect, renderer);    
    // Fique a negre el rectangle baix del mensatge
//     SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); SDL_RenderFillRect(renderer, &mensatges_rect);
//     mensatges_tex = SDL_CreateTextureFromSurface(renderer, mensatges_surf );
//     SDL_RenderCopy(renderer, mensatges_tex, NULL, &mensatges_rect); // Ho farem més tard?
    
    
    // Load graphics
    // Initialize sprites
    initSprites();
    
    stop_time = nCuadres = nTics = 0;
    
    start = clock();
    start_time = SDL_GetTicks();
    
    // Gestió d'events per libnx: 
    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    
    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeDefault(&pad);
    //     hidInitializeTouchScreen();
    //     
    //     HidTouchScreenState state={0};
    //     if (hidGetTouchScreenStates(&state, 1)) {
    //            for(s32 i=0; i<state.count; i++)
    //             {
    //                 // Print the touch screen coordinates
    //                 printf("[%d] x=%03d, y=%03d, diameter_x=%03d, diameter_y=%03d, rotation_angle=%03d\n", i, state.touches[i].x, state.touches[i].y, state.touches[i].diameter_x, state.touches[i].diameter_y, state.touches[i].rotation_angle);
    //             }
    //     }
    
    
    while (!exit_requested && appletMainLoop() ) {
        padUpdate(&pad);
        
        // padGetButtonsDown returns the set of buttons that have been
        // newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);
        
        // padGetButtons returns the set of buttons that are currently pressed
        u64 kHeld = padGetButtons(&pad);
        
        if (kDown & HidNpadButton_Plus)
            break; // break in order to return to hbmenu
            // Pulsacions "Any": /opt/devkitpro/libnx/include/switch/services/hid.h
        if ((kHeld & HidNpadButton_AnyUp) && numSprites < MAX_SPRITES) 
                numSprites++;
         if ((kHeld & HidNpadButton_AnyDown) && numSprites > 1)
                numSprites--;        
            
            
            /* Gestió d'events per SDL
             * // //        const Uint8* keystates = SDL_GetKeyState(NULL);
             * //         const Uint8* keystates = SDL_GetKeyboardState( NULL ); // int *numkeys si vols saber la llongitud del vector keystates
             *        while (SDL_PollEvent(&event)) {                                   
             * 
             * //NO funciona la detecció de tecla pulsada???
             * //(2)
             * //             keys = SDL_GetKeyState(NULL);
             * //             if ( keys[SDLK_UP] == SDL_PRESSED ) 
             * //                     if (numSprites < MAX_SPRITES) numSprites++;
             * //             else
             * //             if ( keys[SDLK_DOWN] == SDL_PRESSED ) 
             * //                 if (numSprites > 1) numSprites--;
             * 
             * //            if ( event.key.state == SDL_PRESSED ) {
             * //                     switch( event.key.keysym.sym ) {
             * //                         case SDLK_UP:
             * //                             if (numSprites < MAX_SPRITES) numSprites++;
             * //                             break;
             * //                         case SDLK_DOWN:
             * //                             if (numSprites > 1) numSprites--;
             * //                             break;
             * //                         default:
             * //                             break;
             * //                     }
             * //            }
             *                
             * //(3)
             *            const Uint8* keystates = SDL_GetKeyState(NULL);
             * // SDL 2.0        const Uint8* keystates = SDL_GetKeyboardState( NULL ); // int *numkeys si vols saber la llongitud del vector keystates
             *            if(keystates[SDLK_UP]){
             *              if (numSprites < MAX_SPRITES) numSprites++;
    }
    if(keystates[SDLK_DOWN]){
        if (numSprites > 1) numSprites--;
    }
    
    //(1)
    printf("\x1b[1;7 state %d == %d && event.key.keysym.sym %d == SDLK_UP %d\n",event.key.state, SDL_PRESSED, event.key.keysym.sym, SDLK_UP );
    if((event.key.state == SDL_PRESSED) && (event.key.keysym.sym  == SDLK_UP)){
        printf("Es repeteix SDLK_UP: numSprites \n");
        if (numSprites < MAX_SPRITES) numSprites++;
        printf("Es repeteix SDLK_UP: numSprites %u.\n", numSprites);
    }
    if((event.key.state == SDL_PRESSED) && (event.key.keysym.sym  == SDLK_DOWN)){
        printf("Es repeteix SDLK_DOWN\n");
        if (numSprites > 1) numSprites--;
    }
    
    switch( event.type ) {
        // https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlkeyboardevent.html
        // The type and state actually report the same information, they just use different values to do it! A keyboard event occurs when a key is released (type=SDK_KEYUP or state=SDL_RELEASED) and when a key is pressed (type=SDL_KEYDOWN or state=SDL_PRESSED). The information on what key was pressed or released is in the keysym structure.
        case SDL_KEYDOWN:
            switch( event.key.keysym.sym ) {
                //                         case SDLK_ESCAPE:
                //                             exit_requested = 1; 
                //                             break;
                case SDLK_UP:
                    if (numSprites < MAX_SPRITES) numSprites++;
                    break;
                case SDLK_DOWN:
                    if (numSprites > 1) numSprites--;
                    break;
                default:
                    break;
    } // switch( event.key.keysym.sym )
    break;
                case SDL_QUIT:
                    exit_requested = 1; 
                    break;
                default:  
                    break;                                 
    } // switch( event.type ) 
    } //         while (SDL_PollEvent(&event)) 
    
    */      
            //        printf( " lógica \n");
            
            
            
            // Borra la pantalla, pintant de negre
            // Vols vore l'estela (el "trail" com feia sdl_demo)? comenta estes dos línies.
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGBA
            SDL_RenderClear( renderer );
            
            moveSprites(); // No més que actualitza posicions <-- f(velocitat, colisions)
            
            // Render the scene
            for (size_t i = 0; i < numSprites; i ++) {
                
                SDL_Rect pos = { 0, 0, 0, 0 };
                Sprite* sprite = &sprites[i];                 
                pos.x = sprite->spr.params.pos.x;
                pos.y = sprite->spr.params.pos.y;
                pos.w = sprite->spr.params.pos.w;
                pos.h = sprite->spr.params.pos.h;     
                if (sprite->spr.image) {       
                    // La versió de gpusprites no modificava mai l'angle; seria aixina
                    // SDL_RenderCopy(renderer, sprite->spr.image, NULL, &pos);                                
                    // però si vols fer-ho, pots ;-)
                    SDL_RenderCopyEx(renderer, sprite->spr.image, NULL, &pos, sprite->spr.params.angle, NULL, SDL_FLIP_NONE);
                }
            } // for (size_t i = 0; i < numSprites; i ++) {
            
            
            
            char textMensatge[1024];
            sprintf(textMensatge, "SDL 2.0 sprites: %lu/%d (usa Up/Down para +/-sprites)", numSprites, MAX_SPRITES );
//             SDL_Surface *mensatges_surf = NULL;
//             mensatges_surf = render_text(textMensatge, font, colors[1], &mensatges_rect);              
//             if ( mensatges_tex )
//                 SDL_DestroyTexture( mensatges_tex );
//             mensatges_tex = SDL_CreateTextureFromSurface(renderer, mensatges_surf);
//             SDL_RenderCopy(renderer, mensatges_tex, NULL, &mensatges_rect);
            render_text(textMensatge, font, colors[1], &mensatges_rect, renderer);              
            
            nCuadres++;
            
            stop_time = SDL_GetTicks();        
            nTics = stop_time - start_time;
            if( nTics >= 1000)  {
                printf("FPS: %4u\n", nCuadres);
                nCuadres = 0;
                start_time = SDL_GetTicks();                      
            }
            
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            //        printf("Tiempo de CPU utilizado: %f segundos vs %u \n", cpu_time_used, stop_time - start_time);
            //        printf(textMensatge, "Tiempo de CPU utilizado: %f segundos vs %u ms \n", cpu_time_used, stop_time - start_time);
            sprintf(textMensatge, "Tiempo de CPU utilizado: %f segundos vs %u ms \n", cpu_time_used, stop_time - start_time);
            TRACE(textMensatge);
//             mensatges_surf = render_text(textMensatge, font, colors[1], &mensatges_rect2, &renderer);
            render_text(textMensatge, font, colors[1], &mensatges_rect2, renderer);              
//             if ( mensatges_tex )
//                 SDL_DestroyTexture( mensatges_tex );
//             mensatges_tex = SDL_CreateTextureFromSurface(renderer, mensatges_surf);
//             SDL_RenderCopy(renderer, mensatges_tex, NULL, &mensatges_rect2);
            
            start = end;
            start_time = stop_time;
            
            
            SDL_RenderPresent(renderer);          
            SDL_Delay(wait);       
            
    }//     while (!exit_requested && appletMainLoop() )
        
    // Delete graphics
    deinitSprites();
    
    // Deinit libs
    // no need to keep the font loaded
    TTF_CloseFont(font);
    
//     if ( mensatges_surf )
//         SDL_FreeSurface( mensatges_surf );
//     
//     if ( mensatges_tex )
//         SDL_DestroyTexture( mensatges_tex );
    
//     if (pantalla)
//         //         SDL_FreeSurface( pantalla ); // https://www.libsdl.org/release/SDL-1.2.15/docs/html/index.html
//         SDL_DestroyTexture( pantalla );
//     
    
    IMG_Quit();
    TTF_Quit(); // Undefinet reference
    
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow( window );
    
    
    // tancar a tancar la conexió en nxlink          
    socketExit();
    
    SDL_Quit();
    
    return 0;
}
