#ifndef CLASSES_H
#define CLASSES_H

#include "global.h"
#include <string>

using namespace std;

class objects {
/* This is an abstract super class.
 * it contains only the common data field
 * of many different items. */
public:
    objects();
    objects(double x, double y);
    ~objects();

    void move();

/* Parameters */

    double vx;
    double vy;          // Horizontal and vertical velocities.
    double acc;         // Acceleration.
    double * center;    // Coordinates.
    string name;        // Used to specify the item type in the doodle::takeItem(objects *).
};

class entrance: public objects {
/* The entrance to the new map. */
public:
    entrance(double x, double y);
};

class star: public objects {
/* Items to be eaten in the new map. */
public :
    star(double x, double y);
};

class health: public objects {
/* Health packs, which will grant you one HP when taken. */
public:
    health(double x, double y);
};

class board: public objects {
/* Boards, to be stepped upon. */
public:
    board(double x, double y);
};

class flexible: public objects {
/* Flexible boards, whose length varies with time going by. */
public:
    flexible(double x, double y);
    virtual void move();

    int flag;      // Used to record the direction of its expansion.
    double length; // This records half of the board's length.
};

class spring: public objects {
/* Springs, which will grant you higher speed when stepped upon. */
public:
    spring(double x, double y);
};

class sting: public objects {
/* Stings, which will harm you and take one HP from you. */
public:
    sting(double x, double y);
};

class doodle {
/* The class for the character.
 * This is not in the 'objects' class since
 * the character should be able to take
 * some of the items when coming across them. */
public:

    doodle();
    doodle(int healthPoint);
    /* Used in the new map, to record the hp from the original one. */
    ~doodle();

    void move();                           // Move the doodle based on the velocities and acceleration.
    void nmove();                          // Since the doodle is designed to be able to appear on one
                                           // edge of the screen aftering going out of the other, the
                                           // screen width is included in doodle::move().
                                           // In the new map, the width changes, so another is required.
    void take(objects *item);              // Used to take items. Very efficient since the parameter
                                           // pointer would be automatically deleted.
    void jump(double y, double newdvy);    // When jumping, the coordinate of the board is used to get
                                           // the doodle's coordinate and the velocity is calculated based
                                           // on the last point at which the vertical velocity is zero.

    /* This calculation of velocity is very important.
     * Since the calculation of the positions are carried out every 15 milliseconds, when repeating jumping
     * on one single board, the positions at which the doodle is judged to collide with the board are
     * never the same. Thus, if the vertical velocity is calculated based on the distance between the highest
     * position and the colliding position, the vertical velocity would be differetn every time.
     *
     * When the first prototype was finished, this problem occured and the doodle just fall down right
     * through the board. This is solved by taking the distance between the highest position and the board's
     * y-coordinate and using it to calculate the theoretical velocity.
     *
     * However, this cannot be calculatd using the physical formula (2*g*H)**(1/2) because it called error
     * when we tried = =
     * So the logarithm function was considered. The base and the phase shift of the logarithm were obtained
     * by the boundary conditions: (0, 0) and (CANVAS_HEIGHT, (2*ACCELERATION*CANVAS_HEIGHT)**(1/2)).
     * After getting the rough function, the variable in this function was changed from the distance to the
     * absolute value of the distance, since in some rare conditions, the collision appears higher than the zero
     * point. */

/* Parameters */

    int hp;             // Health points, when 0, die.
    double dvx;         // Honrizontal velocity.
    double dvy;         // Vertical velocity.
    double dacc;        // Acceleration.
    double * dcenter;   //Coordinates stored in an array.
};

#endif // CLASSES_H
