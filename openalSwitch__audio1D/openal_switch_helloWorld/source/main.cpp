//
// openal-helloWorld portat a devkitpro (devkitA64) per a Switch
// 

#include <stdio.h>
#include <stdlib.h>
#include <switch.h>

#include <SDL.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "openal__helloWorld_buffer.h"

int main (int argc, char **argv)
{
    PadState pad;
    ALCdevice *device;
    ALuint buffer, source;
    ALsizei size, frequency; 
    ALenum format;
    ALint source_state;
    ALCcontext *context;
 
    consoleInit(NULL);
    // Inicializa SDL con SDL_INIT_VIDEO? SDL_INIT_AUDIO!!!
    if ( (SDL_Init(SDL_INIT_AUDIO) < 0)  ) {
        SDL_Log("SDL_Init: %s\n", SDL_GetError());
        printf("SDL no inicializada.\n");
    }
    
    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    padInitializeDefault(&pad);

    // Inicializar OpenAL
    device = alcOpenDevice(NULL); // Abre el dispositivo de audio predeterminado
    if (!device) {
        printf("Error al abrir el dispositivo de audio\n");
        consoleExit( NULL );
        return 1;
    }

    context = alcCreateContext(device, NULL); // Crea un contexto de OpenAL
    if (!context) {
        printf("Error al crear el contexto de OpenAL\n");
        alcCloseDevice( device );
        consoleExit( NULL );
        return 1;
    }
    
    alcMakeContextCurrent( context ); // Hace que el contexto sea el actual
    
    // Versió d'OpenAL (estàndard) + implementació d'OpenAL Soft ()
    printf("OpenAL Renderer is '%s'\n", alGetString(AL_RENDERER) );
    printf("OpenAL Version is '%s'\n", alGetString(AL_VERSION) );		    
    printf("OpenAL Vendor is '%s'\n", alGetString(AL_VENDOR) );
    printf("OpenAL hardware presente: %s\n", alcGetString(NULL, ALC_DEVICE_SPECIFIER) );
    
    // Generar buffer y fuente
    alGenBuffers(1, &buffer); // Genera un búfer
    alGenSources(1, &source); // Genera una fuente
    //
    // Los datos están en un buffer: "helloWorldSample" en openal__helloWorld_buffer.h
    //  static uint8_t helloWorldSample[] = { ... ]
    size = sizeof( helloWorldSample );
    frequency = 11025; 
    format = AL_FORMAT_MONO8; 
    alBufferData(buffer, format, helloWorldSample, size, frequency); // Carga los datos en el buffer
    
    // Asignar buffer a la fuente y reproducir
    alSourcei(source, AL_BUFFER, buffer); // Asigna el búfer a la fuente
    alSourcei(source, AL_LOOPING, 1);     // ¡En bucle!

    alSourcePlay(source); // Reproduce la fuente

    // Main loop
    while (appletMainLoop())
    {
        alGetSourcei(source, AL_SOURCE_STATE, &source_state);
        if (source_state == AL_PLAYING) 
            printf(".");        
        else
            break;

        padUpdate(&pad);

        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus)
            break;
                
        consoleUpdate(NULL);
    }

    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);

    alcMakeContextCurrent(NULL);
    alcDestroyContext( context );
    alcCloseDevice( device );

    consoleExit(NULL);
    return 0;
}
