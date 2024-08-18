//
// Llista sprites d'este exemple tret de devkitPro per a la 3DS
// $ for i in gfxPNG/*; do convert  $i gfx/`basename $i .png`.png; done; for i in gfx/*; do echo "\"`basename $i`\","; done > llistaSprintes.h  I retoquets a mà.


#ifndef LLISTASPRITES

#define LLISTASPRITES

#define NSPRITES 56


char *llistaSprites[NSPRITES] = {
"gfx/enemyBlack1.png",
"gfx/enemyBlack2.png",
"gfx/enemyBlack3.png",
"gfx/enemyBlack4.png",
"gfx/enemyBlack5.png",
"gfx/enemyBlue1.png",
"gfx/enemyBlue2.png",
"gfx/enemyBlue3.png",
"gfx/enemyBlue4.png",
"gfx/enemyBlue5.png",
"gfx/enemyGreen1.png",
"gfx/enemyGreen2.png",
"gfx/enemyGreen3.png",
"gfx/enemyGreen4.png",
"gfx/enemyGreen5.png",
"gfx/enemyRed1.png",
"gfx/enemyRed2.png",
"gfx/enemyRed3.png",
"gfx/enemyRed4.png",
"gfx/enemyRed5.png",
"gfx/meteorBrown_big1.png",
"gfx/meteorBrown_big2.png",
"gfx/meteorBrown_big3.png",
"gfx/meteorBrown_big4.png",
"gfx/meteorBrown_med1.png",
"gfx/meteorBrown_med3.png",
"gfx/meteorBrown_small1.png",
"gfx/meteorBrown_small2.png",
"gfx/meteorBrown_tiny1.png",
"gfx/meteorBrown_tiny2.png",
"gfx/meteorGrey_big1.png",
"gfx/meteorGrey_big2.png",
"gfx/meteorGrey_big3.png",
"gfx/meteorGrey_big4.png",
"gfx/meteorGrey_med1.png",
"gfx/meteorGrey_med2.png",
"gfx/meteorGrey_small1.png",
"gfx/meteorGrey_small2.png",
"gfx/meteorGrey_tiny1.png",
"gfx/meteorGrey_tiny2.png",
"gfx/playerShip1_blue.png",
"gfx/playerShip1_green.png",
"gfx/playerShip1_orange.png",
"gfx/playerShip1_red.png",
"gfx/playerShip2_blue.png",
"gfx/playerShip2_green.png",
"gfx/playerShip2_orange.png",
"gfx/playerShip2_red.png",
"gfx/playerShip3_blue.png",
"gfx/playerShip3_green.png",
"gfx/playerShip3_orange.png",
"gfx/playerShip3_red.png",
"gfx/ufoBlue.png",
"gfx/ufoGreen.png",
"gfx/ufoRed.png",
"gfx/ufoYellow.png"};


#endif
