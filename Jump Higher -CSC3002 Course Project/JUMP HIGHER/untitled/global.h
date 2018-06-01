#ifndef GLOBAL_H
#define GLOBAL_H

#define MAP_WID 30
#define MAP_HEI 16
#define DIS 40

//MAP_WID*dis=NEWMAP_WIDTH
//MAP_HEI*dis=NEWMAP_HEIGHT

/* SIZE */

#define CANVAS_WIDTH 400
#define CANVAS_HEIGHT 500
#define NEWMAP_WIDTH 1200
#define NEWMAP_HEIGHT 680

#define CENTERX 640
#define CENTERY 360

#define DWIDTH 45
#define DHEIGHT 70
#define INIT_X 200
#define INIT_Y 100
#define NEWMAP_X 200
#define NEWMAP_Y 100

#define BWIDTH 80
#define BHEIGHT 16

#define HPWIDTH 30
#define HPHEIGHT 40

#define STARWIDTH 30
#define STARHEIGHT 40

#define DOORWIDTH 30
#define DOORHEIGHT 40

#define SPRINGWIDTH 30
#define SPRINGHEIGHT 20

#define STINGWIDTH 80
#define STINGHEIGHT 20

/* EDGES */

#define TIME_INTERVAL 15

#define HIGHEST 80

#define ITEMGENERATION 500

#define BOARDS_LIMIT 9
#define NBOARDS_LIMIT 10
#define HEALTHPACK_LIMIT 1
#define DOOR_LIMIT 1
#define STARPACK_LIMIT 5

/* SPEED CHANGE */

#define ACCELERATION 0.6
#define KEYBOARD_LEFT -4.8
#define KEYBOARD_RIGHT 4.8

#define STRETCH 0.75

/* FUNCTION */

#define Random(x) (rand() % x)
#define Velocity(h) 0.6*0.9*log(h+2)/log(1.22189954)

/* SCORES */

#define BONUS 250

#endif // GLOBAL_H
