/*
 *  v00 es mostrar que es pot gastar una matriu, inicialitzar-la aleatoriament i anar fent "ploure" caracters
 *  v01 ficar colors en seqüències d'ESCape i pensar si moure els caracters no més d'una columna.
 *  v02 ampliar la matriu (2*MAX_LLONG_GOTERA dalt i baix del que es veu en pantalla, aixina sempre menejes coses sense preocupar-te) per no necesitar atres variables auxiliars
 *  No fa falta guardar la posició de la gota, en poden haver-ne varies, ...
 *  Esta es la versió feta manualment, portà a Switch
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // sleep
#include <sys/select.h>
// #include <termios.h>  // ICSANOW
#include <time.h> // time()
#include <switch.h>

// console.h
#define NFILES 45
#define NCOLS 80
#define MAX_LLONG_GOTERA 15
int PROB_PLOURE = 95;
int LLONG_GOTERA = MAX_LLONG_GOTERA;

#define NFILES_BUFFER NFILES+(2*MAX_LLONG_GOTERA)
char matriu [NFILES_BUFFER][NCOLS];


// Caracters posibles
// const char alfabet[] = "ABCDEFGHIJKLMNÑOPQRSTUVWXYZabcdefghijklmnñopqrstuvwxyz0123456789ºª!·$%&/()=?¿|@#¬{[]}!áéióúàèìòùäëïöüâêîôû`+'ç,.-^*Ç;:_─·̣";
const char alfabet[] = "ABCDEFGHIJKLMNÑOPQRSTUVWXYZabcdefghijklmnñopqrstuvwxyz0123456789!$%&/()=?¿|@#{[]}!`+',.-^*;:_─·̣";
// Deixe fora el ' ' que serà el fi de la gota

const int ncaracters = sizeof(alfabet) - 1;
#define ESC 27

// Secuencias de escape
#define COLOR_RESET        "\x1b[0m"
#define COLOR_GREEN        "\033[32m"
#define COLOR_BRIGHT_GREEN "\x1b[1;32m"
#define COLOR_DIM_GREEN    "\033[2;32m"
#define COLOR_BLANC        "\033[37m"
#define CLEAR_SCREEN       "\x1b[2J"
#define CURSOR_HOME        "\033[H"
//POSICIONA
// \033[<fila>;<columna>H
// printf("\033[0;0H");


// Generar nuevo carácter aleatorio
char get_random_char() {
    return alfabet[rand() % ncaracters];
}

void demoSeqESC() {
//Proves bàsiques de secs. d'escape en C <-- octal o hexadecimal, no es pot ficar en decimal
 printf(CLEAR_SCREEN CURSOR_HOME);
 printf(COLOR_BRIGHT_GREEN "\x1b[1;0H" "1 " "\x1b[37m" "al" "\x1b[31m" "aaaaaaaaaaaaaaaaaaa" COLOR_RESET );
 printf(COLOR_DIM_GREEN "\033[2;2H" "2" COLOR_RESET "%s", "holaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
 printf(COLOR_GREEN "\x1b[33;1H" "3" COLOR_RESET);

 for (int j = 0; j < 37; j++) {
   printf("\x1b[%d;1H" "\x1b[%dm" "%d texttttttttttttttt" COLOR_RESET, 3+j, 30+(j%7), 3+j);
 }
 
 printf("\n\n%s \n", alfabet);
 // Fi de proves bàsiques de secs. d'escape en C <-- octal o hexadecimal, no es pot ficar en decimal
} // fi de demoSeqESC



void creaGotera( int j ) {
 int llongitut;

 if ( (rand() % 100) > PROB_PLOURE ) { // Que caiga una gota
       llongitut = 1 + rand() % LLONG_GOTERA;
       for (int i = 0; i < 0+llongitut+1; i++) {
         matriu[i][j] =  get_random_char();        
       }       
 }
 else
   matriu[0][j] =  ' ';
     
}// fi de creaGotera

void inicialitzar() {

    // Inicialitzar buffer, tot a blancs
    for (int i = 0; i < NFILES_BUFFER; i++) {
      for (int j = 0; j < NCOLS; j++) {
        matriu[i][j] =  ' ';   //  matriu[i][j] =  get_random_char();
      }
    } 

    // Crea goteres de Longitud aleatoria entre 1..LLONG_GOTERA
    for (int j = 0; j < NCOLS; j++) {
      creaGotera( j );
    }    
    
// sleep(2);
  
} // fi 'inicialitzar


int main() {
 // int llongitut = 0;
 // , acabar = 0;
 // int c;
 PadState pad;
 u64 kDown;

 consoleInit(NULL);
 padConfigureInput(1, HidNpadStyleSet_NpadStandard);
 padInitializeDefault(&pad);
 srand(time(NULL));

 demoSeqESC();
 printf("\x1b[44;1H" "+ para continuar");

 consoleUpdate(NULL);
 // svcSleepThread(50000 * 1000000);  // 100 ms
  while (appletMainLoop()) {
    padUpdate(&pad);
    consoleUpdate(NULL);

    kDown = padGetButtonsDown(&pad);
    if (kDown & HidNpadButton_Plus)
      break;
  }

  inicialitzar();

  //
 // Bucle principal
 //
  while (appletMainLoop()) {
    // Actualitzar pantalla: borra, baixa, pinta, lleig teclat i mira si cauen més gotes
    // Update the console, sending a new frame to the display
    consoleUpdate(NULL);

   printf(CLEAR_SCREEN CURSOR_HOME); // printf(CLEAR_SCREEN);

   // Baixar totes les lletres una posició
    for (int i = NFILES_BUFFER-1; i > 0; i--) {
      for (int j = 0; j < NCOLS; j++) {
        matriu[i][j] = matriu[i-1][j];
      }
    }

   // Les pinta, destacant la punta de la gota
    for (int i = MAX_LLONG_GOTERA; i < NFILES+MAX_LLONG_GOTERA; i++) {
      for (int j = 0; j < NCOLS; j++) {
        // Mirant si la següent es un blanc i esta no, identifiques la punta de la gota
        if ( (matriu[i][j] != ' ') && (matriu[i+1][j] == ' ') )
            printf(COLOR_BLANC "\033[%d;%dH""%c" COLOR_RESET, i-MAX_LLONG_GOTERA, j, matriu[i][j]);
            else
            printf(COLOR_GREEN "\033[%d;%dH""%c" COLOR_RESET, i-MAX_LLONG_GOTERA, j, matriu[i][j]);
      }
    }

    printf("\n");
    // usleep(100000);
    svcSleepThread(100 * 1000000);  // 100 ms


    padUpdate(&pad);
    kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus) break;

        if (kDown & HidNpadButton_A) {
          if (PROB_PLOURE > 0) PROB_PLOURE--;
        }
        if (kDown & HidNpadButton_B) {
          if (PROB_PLOURE < 100) PROB_PLOURE++;
        }
        if (kDown & HidNpadButton_X) {
          if (LLONG_GOTERA > 1)  LLONG_GOTERA--;
        }
        if (kDown & HidNpadButton_Y) {
          if (LLONG_GOTERA < MAX_LLONG_GOTERA) LLONG_GOTERA++;
        }
      
    // I ara vejes si has de fer caure noves gotes
       for (int j = 0; j < NCOLS; j++) {
         creaGotera( j );
       }    
 }// fi de While
    
 consoleExit(NULL);

//  exit( 0 );
 return( 0 );
}
