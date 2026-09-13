//
// openal-3D portat a devkitpro (devkitA64) per a Switch
// 

#include <stdio.h>
#include <stdlib.h>
#include <switch.h>

#include <SDL.h>
#include <AL/al.h>
#include <AL/alc.h>

int main (int argc, char **argv)
{
    PadState pad;
    ALCdevice *device;
    ALCcontext *context;
    ALuint buffer, source;
    ALsizei size, frequency; 
    ALenum format;
    ALint source_state;
    float angle, radius, speed, x, y, z;
    ALfloat listenerOri[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
 
    consoleInit(NULL);
    
    if ( (SDL_Init(SDL_INIT_AUDIO) < 0)  ) {
        SDL_Log("SDL_Init: %s\n", SDL_GetError());
        printf("SDL no inicializa audio.\n");
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
    
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    //. Establecer la orientación del oyente para que mire hacia (0, 0, -1)
    alListenerfv(AL_ORIENTATION, listenerOri); // orientation expressed as “at” and “up” vectors

    // 3. Cargar datos de audio en el búfer (como antes)
//     ALshort samples[] = {0, 10000, 20000, 30000, 20000, 10000, 0, -10000, -20000, -30000, -20000, -10000};
    ALshort samples[] = {0, 0x7FF0, 0x7FF0, 0x7FF0, 0x7FF0, 0x000F, 0x000f, 0x000F, 0x000F };
    size = sizeof( samples );
    frequency = 44100;
    format = AL_FORMAT_MONO16;
    alBufferData(buffer, format, samples, size, frequency); // Carga los datos en el buffer
    
    // Asignar buffer a la fuente y reproducir
    alSourcei(source, AL_BUFFER, buffer); // Asigna el búfer a la fuente
    alSourcei(source, AL_LOOPING, 1);     // ¡En bucle!
    alSourcePlay(source); // Reproduce la fuente

    
    angle = 0.0f;
    radius = 10.0f; // Radio del círculo
    speed = 0.1f;   // Velocidad de rotación
    x = y = z = 0.0f;

    // Main loop
    while (appletMainLoop())
    {
        alSource3f(source, AL_POSITION, x, y, z);
//         printf("\x1b[%d;%dH.", (int)x+1024/2, (int)y+780/2 );
//         printf(".");
        printf("%3.3f %3.3f ", x, y);
        
        padUpdate(&pad);


        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus)
            break;
        if (kDown & HidNpadButton_A) { // Reinicia
            angle = 0.0f;
            radius = 10.0f; 
            speed = 0.1f;   
            x = y = z = 0.0f;            
        }
                
        consoleUpdate(NULL);
//         printf("\x1b[%d;%dH ", (int)x+1024/2, (int)y+780/2 );
        printf("|");
        angle += speed;
        if (angle > 2 * M_PI) angle = 0; 
         // Calcular la posición de la fuente
        x = radius * cos(angle);
        y = 0; //radius * sin(angle);
        z = radius * sin(angle);
    }

    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    alcMakeContextCurrent(NULL);
    alcDestroyContext( context );
    alcCloseDevice( device );

    consoleExit(NULL);
    return 0;
}
