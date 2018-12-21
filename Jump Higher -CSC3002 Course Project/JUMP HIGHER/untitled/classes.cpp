

#include "global.h"
#include "classes.h"

objects::objects() {
    center = new double[2];
}

objects::objects(double x, double y) {
    vx = 0;
    vy = 0;
    acc = 0;
    center = new double[2];
    center[0] = x;
    center[1] = y;
}

objects::~objects() {
    delete center;
}

void objects::move() {
    vx += acc;
    vy += acc;
    center[0] += vx;
    center[1] += vy;
}

entrance::entrance(double x, double y) {
    vx = 0;
    vy = 0;
    acc = 0;
    center = new double[2];
    center[0] = x;
    center[1] = y;
    name = "entrance";
}


star::star(double x, double y) {
    vx = 0;
    vy = 0;
    acc = 0;
    center = new double[2];
    center[0] = x;
    center[1] = y;
    name = "star";
}

health::health(double x, double y) {
    vx = 0;
    vy = 0;
    acc = 0;
    center = new double[2];
    center[0] = x;
    center[1] = y;
    name = "healthPack";
}

board::board(double x, double y) {
    vx = 0;
    vy = 0;
    acc = 0;
    center = new double[2];
    center[0] = x;
    center[1] = y;
    name = "board";
}

flexible::flexible(double x, double y) {
    vx = 0;
    vy = 0;
    acc = 0;
    center = new double[2];
    center[0] = x;
    center[1] = y;
    name = "flexible";

    flag = -1;
    length = BWIDTH;
}

void flexible::move() {
    vx += acc;
    vy += acc;
    center[0] += vx;
    center[1] += vy;

    if (length == BWIDTH) {
        flag = -1;
    } else if (length == BWIDTH/4) {
        flag = 1;
    }

    length += flag * STRETCH;
}

spring::spring(double x, double y) {
    vx = 0;
    vy = 0;
    acc = 0;
    center = new double[2];
    center[0] = x;
    center[1] = y;
    name = "spring";
}

sting::sting(double x, double y) {
    vx = 0;
    vy = 0;
    acc = 0;
    center = new double[2];
    center[0] = x;
    center[1] = y;
    name = "sting";
}

doodle::doodle() {
    hp = 1;
    dvx = 0;
    dvy = 0;
    dacc = ACCELERATION;
    dcenter = new double[2];
    dcenter[0] = INIT_X;
    dcenter[1] = INIT_Y;
}

doodle::doodle(int healthPoint) {
    hp = healthPoint;
    dvx = 0;
    dvy = 0;
    dacc = ACCELERATION;
    dcenter = new double[2];
    dcenter[0] = NEWMAP_X;
    dcenter[1] = NEWMAP_Y;
}

doodle::~doodle() {
    delete dcenter;
}

void doodle::move() {
    if (dvx > 0) {
        dvx -= dacc/4;
    } else if (dvx < 0) {
        dvx += dacc/4;
    }
    dvy += dacc;
    dcenter[0] += dvx;
    dcenter[1] += dvy;
    if (dcenter[0] < 0) {
        dcenter[0] += CANVAS_WIDTH;
    }
    if (dcenter[0] > CANVAS_WIDTH) {
        dcenter[0] -= CANVAS_WIDTH;
    }
}

void doodle::nmove() {
    if (dvx > 0) {
        dvx -= dacc/4;
    } else if (dvx < 0) {
        dvx += dacc/4;
    }
    dvy += dacc;
    dcenter[0] += dvx;
    dcenter[1] += dvy;
    if (dcenter[0] < 0) {
        dcenter[0] += NEWMAP_WIDTH;
    }
    if (dcenter[0] > NEWMAP_WIDTH) {
        dcenter[0] -= NEWMAP_WIDTH;
    }
}

void doodle::take(objects *item) {
    if (item->name == "healthPack") {
        hp ++;
    }
}

void doodle::jump(double y, double newdvy) {
    dcenter[1] = y - DHEIGHT/2 - BHEIGHT/2;
    dvy = -1 * newdvy;
}
