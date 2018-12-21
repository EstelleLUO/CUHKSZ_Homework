#ifndef WIDGET_H
#define WIDGET_H

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <math.h>

#include <QtWidgets/QWidget>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QKeyEvent>
#include <QTimer>

#include "classes.h"
#include "global.h"
#include <fstream>
using namespace std;

class Widget : public QWidget
{
    Q_OBJECT
/* This class is ovrloaded from the 'QWidget', the idea came from
 * a sneakySnake code on the Internet. But, of course, ours is so much
 * better and much more complicated than a snake.
 * By using this, all GUI of QT can be used. Really convenient. */
public:
    Widget(QWidget *parent = 0);
    ~Widget();
    virtual void paintEvent(QPaintEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    /* These two are overloaded from the 'QWidget', their prototype have been
     * included in the 'update()', so only their content need writing. The logic
     * has been built.
     * The paintEvent(QPaintEvent *) paints some of the objects every time it's called.
     * Which ones are to be painted is determained by where the doodle is.
     * The keyPressEvent(QKeyEvent *) checks if there's a keyboard input every time it's called.
     * Corresponding operation would be carried out, using a switch.
     * Not only can the velocities be altered with down, left, right, the game can also be
     * paused and resumed using P and R. */

public:
    void InitGame();
    /* Used to initialize the game. The parameters in the data field are set to their default value,
     * and the very first objects are generated here.
     * The most important part here is the connect() function, which is the core part of the signal-slot
     * system of QT. By linking a timer to an update() function, a loop is formed. */
    void backtoOriginal();
    /* This function is used to get back to the original map from the new one.
     * The loop in the new one is disconnect()-ed, and the original loop is connect()-ed again here.
     * Also, the widget is re-sized. */

    void InitNewMap();
    /* To initialize the new map.
     * Also includes disconnect(), connect() and resize.
     * The data in the original map is not deleted because going back is required.
     * But the data of this map would be deleted. */
    bool judge(double x, double y);
    /* Used in the last function, to determain whether a new board can be put on point (x,y). */

    void GameOver();
    /* Call out the buttons for the user to choose whether to continue playing or quit. */
    void GenerateItems();
    /* Used to generate all objects in the original map.
     * This function has been re-written many times.
     * Since it was first designed that one type of items should appear whenever the score is increased by
     * some certain number, sometimes many objects will be generated on the same board.
     * To solve this, another parameter flag was introduced. Its value refers to the type of next item to be
     * added. The new item won't appear immediately after the condition is satisfied. Instead, it's added with
     * the very next board. */

    bool IsGameOver();
    /* Check if the doodle's dead, based on its health points and position. */
    void nIsGameOver();
    /* To get out of the new map, only one thing is required: newMap == 0.
     * So this function simply change that integer to 0 when all stars are eaten or
     * the player decides to leave. */

    void takeItem();
    void takeStar();
    void onBoard();
    /* These three functions are kind of similar.
     * They go through the vectors of items and check if any of the items should do something with the doodle.
     * Items and stars are seperated since they are in different maps.
     * Boards are sepearted from the two above since they are not eaten.
     * Items like springs and stings, which will not be eaten, are actually also written in onBoards(). */

    void moveMap();
    /* Move down all things except for the doodle when the doodle reaches certain height.
     * Necessary for gameplay. */
    void deleteItems();
    /* Delete the pointers whose item falls out of screen. */

    void calculateScore();

    void clearData();
    /* Only called in the destructor and when a new game begins.
     * Used to free all the memory this program takes up. */

private slots:
    void startFunction();
    void exitFunction();
    /* These two functions are connected to the QPushButtons.
     * The startFunction() would clear all the data of last game and initialize a new one.
     * The exitFunction(), well, it can exit the program, quite obvious. */

    void dUpdate();
    void nUpdate();
    /* Used in different maps.
     * They are connected to the timer when the doodle's in the map they belong.
     * They can be considered as the main() function of ordinary projects.
     * All functions that should be called in every loop are written here. */

private:
    vector<QLabel *> pics;
    QPushButton * start;
    QPushButton * quit;
    /* The buttons. Occurs when the whole program starts and when one single run ends. */

    doodle * d;
    doodle * nd;
    QTimer * gameTimer;

    vector<board*> boards;

    vector<health*> healthPacks;
    vector<flexible *> flexibles;
    vector<spring *> springs;
    vector<sting *> stings;

    vector<entrance *> doors;
    vector<board*> nboards;
    vector<star*> stars;
    /* All the objects.
     * This format makes it easier to do different operations based on their types. */

    int flag;
    /* 0: default
     * 1: healthPacks
     * 2: springs
     * 3: stings
     * 4: entrances */

    int score;
    int totalBoards;
    /* Used to control the total number of the boards.
     * Criteria for adding new boards. */

    int yzero;
    int nyzero;
    /* Highest position of the doodle, at which the vertical velocity is zero.
     * Used to calculate the vertical velocity after stepping on something. */

    int newMap;
    /* 0: not in the new map
     * 1: in the new map */
    int pierced;
    /* 0: Safe
     * 3: Pierced by a sting! */
};

#endif // WIDGET_H
