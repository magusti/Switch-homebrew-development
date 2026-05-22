//     enum personatges { womanNDS, manNDS, womanRGBANDS, manRGBANDS, reimu3DS };
// Prueba de concepto: carregar un spritesheet i mostrar un trocet --> sprite!
//
// Exemple bàsic d'animació mitjançant sprites a partir d'un "spritesheet".
// Referencias:
// * Exemple NDS /Graphics/Sprites/animate_simple_NDS
// * que podemos ver explicado en NDS/Tutorials Animation  / Dev-Scene.com . Copia en <https://d2l62mv7qj2i3f.cloudfront.net/NDS/Tutorials_Animation>
// * "Simple basic animation using the citro2d library" - GitHub <https://github.com/NyankoTear/reimu_idle/blob/main/README.md>
// * Conversió a SDL 2.0 per a la Switch de l'exemple gpusprites (de 3DS en Citro2D)


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> //time
#include <sys/resource.h>
#include <sys/unistd.h> // chdir
#include <malloc.h>
#include <switch.h>

#include <SDL.h>
#include <SDL_image.h> // IMG_Load
#include <SDL_ttf.h> //TTF_Font, ...


#define MAX_SPRITES   6              // Núm. d'objectes en l'escena
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768
#define MAX_IMAGES    128

SDL_Window *window;
SDL_Texture *pantalla;
SDL_Renderer *renderer = NULL;

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

int tamanyLletra = 36;
SDL_Rect mensatges_rect, mensatges_rect2;
SDL_Surface *mensatges_surf = NULL;
SDL_Texture *mensatges_tex = NULL;
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
    struct {      // Per a  l'animació: quin cuadre cal mostrar en pantalla de tot el SpriteSheet?
        float   x;
        float   y;
        float   w;
        float   h;
    } 	cuadreActual;
}  C2D_DrawParams;

typedef struct {
    SDL_Texture     *image[MAX_IMAGES];  // SDL2: SDL_Surface (CPU) vs SDL_Texture (GPU)
    //Pero puc anar generant textures a partir de Surfaces i rotar-les, etc. Que en Textures no es pot fer, no?
    C2D_DrawParams    params;
} C2D_Sprite;



typedef struct {
    int tipo;                   // Origin of graphics for sprite: 0 SpriteSheet / 1 frames sueltos
    C2D_Sprite spr;
    float dx, dy;               // velocity    
    float ampleImage, altImage; // Properties of graphics for sprite
    int nDireccions;
    int longAnimacio;
    int frame;
    int direccioAnterior;
} Sprite;


// typedef enum{ ARRIBA = 0, DERECHA, ABAJO, IZQUIERDA} tDireccio;
typedef enum{ ATRAS = 0, DERECHA, FRENTE, IZQUIERDA} tDireccio;
                
static Sprite sprites[MAX_SPRITES];       
static size_t numSprites = MAX_SPRITES;


static void deinitSprites() {
    
  for (int i = 0; i < MAX_SPRITES; i++)
  {
    for (int j = 0; j < sprites[i].nDireccions * sprites[i].longAnimacio ; j++)
        SDL_DestroyTexture( sprites[i].spr.image[j] );
  }
}



SDL_Surface * render_text(const char* text, TTF_Font *font, 
                          SDL_Color color, SDL_Rect *rect) 
{
    SDL_Surface *surface;
    
    surface = TTF_RenderText_Solid(font, text, color);
    rect->w = surface->w;
    rect->h = surface->h;
    
    return surface;
}


//---------------------------------------------------------------------------------
static void moveSprites(Sprite *sprite, tDireccio direccio) {
//---------------------------------------------------------------------------------
        sprite->frame = (sprite->frame + 1) % sprite->longAnimacio;
        if ((direccio != sprite->direccioAnterior) && (sprite->nDireccions > 1) ) {
                sprite->frame = 0;
                sprite->direccioAnterior = direccio;
        }
                
        // Calcula quant avança 
        if (direccio == DERECHA) {
            sprite->spr.params.pos.x = sprite->spr.params.pos.x + sprite->dx;
        }
        if (direccio == IZQUIERDA) {
            sprite->spr.params.pos.x = sprite->spr.params.pos.x - sprite->dx;
        }
        if (direccio == ATRAS) { //ARRIBA) {
            sprite->spr.params.pos.y = sprite->spr.params.pos.y - sprite->dy;
        }
        if (direccio == FRENTE) { //ABAJO) {
            sprite->spr.params.pos.y = sprite->spr.params.pos.y + sprite->dy;
        }

        if (sprite->nDireccions > 1)  {
          sprite->spr.params.cuadreActual.x = sprite->frame*sprite->spr.params.cuadreActual.w;
          sprite->spr.params.cuadreActual.y = (int)direccio*sprite->spr.params.cuadreActual.h;
        }
        
        
        // Check for collision with the screen boundaries
        if ( (sprite->spr.params.pos.x <= -(sprite->spr.params.pos.w / 2.0f)) ) {
            sprite->spr.params.pos.x = sprite->spr.params.pos.x + sprite->dx;
        }

        if ( (sprite->spr.params.pos.x > (SCREEN_WIDTH-(sprite->spr.params.pos.w / 2.0f))) ) {
             sprite->spr.params.pos.x = sprite->spr.params.pos.x - sprite->dx;
        }
        
        if ( (sprite->spr.params.pos.y <= -(sprite->spr.params.pos.h / 2.0f)) )  {
            sprite->spr.params.pos.y = sprite->spr.params.pos.y + sprite->dy;
        }
           
        if ( (sprite->spr.params.pos.y > (SCREEN_HEIGHT-(sprite->spr.params.pos.h / 2.0f))) ) {
            sprite->spr.params.pos.y = sprite->spr.params.pos.y - sprite->dy;
        }
                    
}


Sprite*  initSspriteAnimat( char *rutaSpriteSheet[], int nCuadres,
                            int ampleCuadres, int altCuadres, 
                            int nDireccions, // Orden: Detrás/derecha/Frente/Izquierda    
                            int longAnimacio,
                            int escalaX, int escalaY ) { 
     SDL_Surface  *unaImage;
     Sprite*      sprite;
     int i;
     
     
     sprite = (Sprite *)malloc( sizeof(Sprite) * nCuadres);
     
     if ( !sprite )
         return NULL;
     else {
         //   srand(time(NULL));
         
         sprite->spr.params.pos.x = rand() % SCREEN_WIDTH;
         sprite->spr.params.pos.y = rand() % SCREEN_HEIGHT;
         
         for (i=0; i < nCuadres; i++) {
             unaImage = IMG_Load( rutaSpriteSheet[i] );
             if (unaImage) {
                 sprite->ampleImage = unaImage->w;
                 sprite->altImage = unaImage->h;
                 sprite->spr.params.pos.w = ampleCuadres * escalaX; // Sobreescriu la 3a i 4a component, que serveixen per reescalar el tamany de la imatge
                 sprite->spr.params.pos.h = altCuadres*escalaY; // si no coincideix en el tamany d'esta.
                 sprite->spr.image[i] = SDL_CreateTextureFromSurface(renderer, unaImage);
                 SDL_FreeSurface( unaImage ); // Esta al final de este bucle for
             }       
             
         } // for (i=0; i < nCuadres; i++) {
         
         
         sprite->spr.params.center.x = ampleCuadres/2;
         sprite->spr.params.center.y = altCuadres/2; 
         
         
         sprite->spr.params.angle = rand()/(float)RAND_MAX;
         sprite->dx = fabs( rand()*4.0f/RAND_MAX); // - 2.0f;
         sprite->dy = fabs(rand()*4.0f/RAND_MAX); // - 2.0f;
                  
         sprite->frame = 0;
         sprite->spr.params.cuadreActual.x = 0;
         sprite->spr.params.cuadreActual.y = 0;
         sprite->spr.params.cuadreActual.w = ampleCuadres;
         sprite->spr.params.cuadreActual.h = altCuadres;
         
         sprite->nDireccions = nDireccions;
         sprite->longAnimacio = longAnimacio;
         sprite->direccioAnterior = FRENTE; //ABAJO;
         
     } // if ( !sprite ) else
 
  return sprite;
 
 } // Sprite*  initSspriteAnimat
    



//---------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    //---------------------------------------------------------------------------------
    int exit_requested = 0;
    Uint32 start_time, stop_time, nCuadres, nTics;    
    clock_t start, end;
    double cpu_time_used;
    int wait = 25;
    char textMensatge0[1024];
    char textMensatge[1024];
    Sprite* losSprites[MAX_SPRITES];

    srand(time(NULL));
    romfsInit();
    chdir("romfs:/");
       
    
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
    
    mensatges_surf = render_text("SDL 2.0 sprites animats!", font, colors[1], &mensatges_rect);
    // Fique a negre el rectangle baix del mensatge
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); SDL_RenderFillRect(renderer, &mensatges_rect);
    mensatges_tex = SDL_CreateTextureFromSurface(renderer, mensatges_surf );
    SDL_RenderCopy(renderer, mensatges_tex, NULL, &mensatges_rect); // Ho farem més tard?
    
    
    // Load graphics & ...
    char *rutaWoman[] = {"gfx/woman.png"};
    losSprites[0] = initSspriteAnimat( rutaWoman, 1, 32, 32, 4, 3, 3, 3); 
    if (!losSprites[0] ) {
        mensatges_surf = render_text("Fallo en carregar gfx/woman.png", font, colors[1], &mensatges_rect);
         // Fique a negre el rectangle baix del mensatge
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); SDL_RenderFillRect(renderer, &mensatges_rect);
        mensatges_tex = SDL_CreateTextureFromSurface(renderer, mensatges_surf );
        SDL_RenderCopy(renderer, mensatges_tex, NULL, &mensatges_rect); // Ho farem més tard?
        SDL_Delay( 5000 );       

        exit(-1);
    }
    char *rutaMan[] = {"gfx/man.png"};
    losSprites[1] = initSspriteAnimat( rutaMan, 1, 32, 32, 4, 3, 3, 3); // Orden: Detrás/derecha/Frente/Izquierda
    if (!losSprites[1] ) {
        mensatges_surf = render_text("Fallo en carregar gfx/man.png", font, colors[1], &mensatges_rect);
        // Fique a negre el rectangle baix del mensatge
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); SDL_RenderFillRect(renderer, &mensatges_rect);
        mensatges_tex = SDL_CreateTextureFromSurface(renderer, mensatges_surf );
        SDL_RenderCopy(renderer, mensatges_tex, NULL, &mensatges_rect); // Ho farem més tard?
        SDL_Delay( 5000 );       

        exit(-1);
    }
      
   char *rutaWomanRGBA[] = {"gfx/womanRGBA.png"};
   losSprites[2] = initSspriteAnimat( rutaWomanRGBA, 1, 32, 32, 4, 3, 2, 2); 
   char *rutaManRGBA[] = {"gfx/manRGBA.png"};
   losSprites[3] = initSspriteAnimat( rutaManRGBA, 1, 32, 32, 4, 3, 2, 2);

    
   char *rutaSDLTutorial[] = {"gfx/spritesheet3.bmp"};
   losSprites[4] = initSspriteAnimat( rutaSDLTutorial, 1, 32, 64, 3, 9, 4, 4);

   char *rutaReimu[] = { // 14 cuadres
     "gfx/frame_00_delay-0.1s.png",
     "gfx/frame_01_delay-0.1s.png",
     "gfx/frame_02_delay-0.1s.png",
     "gfx/frame_03_delay-0.1s.png",
     "gfx/frame_04_delay-0.1s.png",
     "gfx/frame_05_delay-0.1s.png",
     "gfx/frame_06_delay-0.1s.png",
     "gfx/frame_07_delay-0.1s.png",
     "gfx/frame_08_delay-0.1s.png",
     "gfx/frame_09_delay-0.1s.png",
     "gfx/frame_10_delay-0.1s.png",
     "gfx/frame_11_delay-0.1s.png",
     "gfx/frame_12_delay-0.1s.png",
     "gfx/frame_13_delay-0.1s.png" };
   losSprites[5] = initSspriteAnimat( rutaReimu, 14, 256, 256, 1, 14, 1, 1);
   
    SDL_Delay( 3000 );       
   
    
   
// 
//        
    //  ... & Initialize sprites    
    SDL_Rect pos = { 0, 0, 0, 0 }, rectActual = { 0, 0, 0, 0 };
    for( int i=0; i < numSprites; i++) {        
        pos.x = losSprites[i]->spr.params.pos.x;
        pos.y = losSprites[i]->spr.params.pos.y;
        pos.w = losSprites[i]->ampleImage;
        pos.h = losSprites[i]->altImage;     
        rectActual.x = losSprites[i]->spr.params.cuadreActual.x;
        rectActual.y = losSprites[i]->spr.params.cuadreActual.y;
        rectActual.w = losSprites[i]->spr.params.cuadreActual.w;
        rectActual.h = losSprites[i]->spr.params.cuadreActual.h;     
        if (losSprites[i]->spr.image[0]) {               
            // La versió de gpusprites no modificava mai l'angle; seria aixina
            SDL_RenderCopy(renderer, losSprites[i]->spr.image[0], NULL, &pos);
            pos.x += 20;
            pos.w = losSprites[i]->spr.params.pos.w;
            pos.h = losSprites[i]->spr.params.pos.h;     
            SDL_RenderCopy(renderer, losSprites[i]->spr.image[0], &rectActual, &pos);                                
            // però si vols fer-ho, pots ;-)
            // SDL_RenderCopyEx(renderer, losSprites[0]->spr.image, NULL, &pos, sprite->spr.params.angle, NULL, SDL_FLIP_NONE);
    }
    
    
    } //     for( int i=0; i < numSprites; ...

    sprintf(textMensatge0, "SDL 2.0 sprite animats: %2.0fx%2.0f woman %4.1f,%4.1f + man %4.1f,%4.1f",  
            losSprites[0]->spr.params.pos.w, losSprites[0]->spr.params.pos.h,
            losSprites[0]->spr.params.pos.x, losSprites[0]->spr.params.pos.y,
            losSprites[1]->spr.params.pos.x, losSprites[1]->spr.params.pos.y   );
    mensatges_surf = render_text(textMensatge0, font, colors[1], &mensatges_rect);            
    // Fique a negre el rectangle baix del mensatge
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); SDL_RenderFillRect(renderer, &mensatges_rect);
    mensatges_tex = SDL_CreateTextureFromSurface(renderer, mensatges_surf );
    SDL_RenderCopy(renderer, mensatges_tex, NULL, &mensatges_rect); // Ho farem més tard?
    
    
    SDL_RenderPresent(renderer);          
    SDL_Delay( 3000 );       
//     exit(1);
    
    
    
    
    stop_time = nCuadres = nTics = 0;
    
    start = clock();
    start_time = SDL_GetTicks();
    
    // Gestió d'events per libnx: 
    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    
    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeDefault(&pad);
  
    
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
        if ((kDown & HidNpadButton_AnyUp) || (kHeld & HidNpadButton_AnyUp) ) {
            for( int i=0; i < numSprites;  i++)
                moveSprites( losSprites[i], ATRAS);
        }
        if ((kDown & HidNpadButton_AnyDown) || (kHeld & HidNpadButton_AnyDown) ) {
            for( int i=0; i < numSprites;  i++)
                moveSprites( losSprites[i], FRENTE);
        }
        if ((kDown & HidNpadButton_AnyLeft) || (kHeld & HidNpadButton_AnyLeft) ) {
            for( int i=0; i < numSprites;  i++)
                moveSprites( losSprites[i], IZQUIERDA);
        }       
        if ((kDown & HidNpadButton_AnyRight) || (kHeld & HidNpadButton_AnyRight) ) {
            for( int i=0; i < numSprites;  i++)
                moveSprites( losSprites[i], DERECHA);
        }
            
            // Borra la pantalla, pintant de negre
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGBA negre
//             SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // RGBA blanc
            SDL_RenderClear( renderer );
            
            
            Sprite* sprite;
            // Render the scene
            for (size_t i = 0; i < numSprites; i++) {
                
                SDL_Rect pos = { 0, 0, 0, 0 };
                sprite = losSprites[i]; //&sprites[i];                 
                
                pos.x = sprite->spr.params.pos.x;
                pos.y = sprite->spr.params.pos.y;
                pos.w = sprite->spr.params.pos.w;
                pos.h = sprite->spr.params.pos.h;     
                rectActual.x = losSprites[i]->spr.params.cuadreActual.x;
                rectActual.y = losSprites[i]->spr.params.cuadreActual.y;
                rectActual.w = losSprites[i]->spr.params.cuadreActual.w;
                rectActual.h = losSprites[i]->spr.params.cuadreActual.h;     
                if (sprite->nDireccions > 1) {
                    if (sprite->spr.image[0]) {       
                      // La versió de gpusprites no modificava mai l'angle; seria aixina
                      SDL_RenderCopy(renderer, sprite->spr.image[0], &rectActual, &pos);                                
                      // però si vols fer-ho, pots ;-)
                      // SDL_RenderCopyEx(renderer, sprite->spr.image, NULL, &pos, sprite->spr.params.angle, NULL, SDL_FLIP_NONE);
                    }                    
                } else {
                    if (sprite->spr.image[sprite->frame]) {       
                      // La versió de gpusprites no modificava mai l'angle; seria aixina
                      SDL_RenderCopy(renderer, sprite->spr.image[sprite->frame], &rectActual, &pos);                                
                      // però si vols fer-ho, pots ;-)
                      // SDL_RenderCopyEx(renderer, sprite->spr.image[sprite->frame], NULL, &pos, sprite->spr.params.angle, NULL, SDL_FLIP_NONE);
                    }
                    
                }
            } // for (size_t i = 0; i < numSprites; i ++) {
            
            
            
    sprintf(textMensatge, 
            "Sprites animats: w %4.1f,%4.1f, %d, %2.0f, %2.0f; m %4.1f,%4.1f, %d, %2.0f, %2.0f",  
            losSprites[0]->spr.params.pos.x, losSprites[0]->spr.params.pos.y,
            losSprites[0]->direccioAnterior,
            losSprites[0]->dx, losSprites[0]->dy,
            losSprites[1]->spr.params.pos.x, losSprites[1]->spr.params.pos.y,
            losSprites[1]->direccioAnterior,
            losSprites[1]->dx, losSprites[1]->dy );
            SDL_Surface *mensatges_surf = NULL;
            mensatges_surf = render_text(textMensatge, font, colors[1], &mensatges_rect);              
            if ( mensatges_tex )
                SDL_DestroyTexture( mensatges_tex );
            mensatges_tex = SDL_CreateTextureFromSurface(renderer, mensatges_surf);
            SDL_RenderCopy(renderer, mensatges_tex, NULL, &mensatges_rect);
            
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
            sprintf(textMensatge, "Tiempo de CPU utilizado: %f segundos vs %u ms \n", cpu_time_used, stop_time - start_time);
            mensatges_surf = render_text(textMensatge, font, colors[1], &mensatges_rect2);              
            if ( mensatges_tex )
                SDL_DestroyTexture( mensatges_tex );
            mensatges_tex = SDL_CreateTextureFromSurface(renderer, mensatges_surf);
            SDL_RenderCopy(renderer, mensatges_tex, NULL, &mensatges_rect2);
            
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
    
    if ( mensatges_surf )
        SDL_FreeSurface( mensatges_surf );
    
    if ( mensatges_tex )
        SDL_DestroyTexture( mensatges_tex );
    
    if (pantalla)
        //         SDL_FreeSurface( pantalla ); // https://www.libsdl.org/release/SDL-1.2.15/docs/html/index.html
        SDL_DestroyTexture( pantalla );
    
    
    IMG_Quit();
    TTF_Quit(); // Undefinet reference
    
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow( window );
    
    
    // tancar a tancar la conexió en nxlink          
    socketExit();
    
    SDL_Quit();
    
    return 0;
}
