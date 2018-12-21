#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    resize(CANVAS_WIDTH, CANVAS_HEIGHT);
    move(QPoint(CENTERX-CANVAS_WIDTH/2, CENTERY-CANVAS_HEIGHT/2));
    this->setWindowFlags(Qt::FramelessWindowHint);
    // This line makes the window frameless, but also un-draggable.
//    this->setAttribute(Qt::WA_TranslucentBackground,true);
    // This line can make the window opaque.

    QLabel *doodlepic = new QLabel(this);
    doodlepic->setPixmap(QPixmap("doodle.png"));
    doodlepic->setGeometry(QRect(CANVAS_WIDTH/2-50, CANVAS_HEIGHT/4-50, 180, 88));
    doodlepic->show();
    pics.push_back(doodlepic);

    QLabel *title = new QLabel(this);
    title->setText("JUMP HIGHER");
    title->setGeometry(QRect(CANVAS_WIDTH/2-120, CANVAS_HEIGHT/4-100, 400, 400));
    title->setStyleSheet("QWidget{font:20pt Ravie}");
    title->show();
    pics.push_back(title);

    start = new QPushButton(QString("START!"), this);
    start->setGeometry(QRect(CANVAS_WIDTH/4, CANVAS_HEIGHT/2+50, 80, 50));
    start->setStyleSheet("QWidget{background-color:rgb(155,215,60);border-top-right-radius:15px;border-top-left-radius:15px; "
                         "border-bottom-left-radius:15px; border-bottom-right-radius:15px;font:12pt Kristen ITC}");
    connect(start, SIGNAL(clicked()), this, SLOT(startFunction()));
    quit = new QPushButton(QString("QUIT!"), this);
    quit->setGeometry(QRect(CANVAS_WIDTH/2, CANVAS_HEIGHT/2+50, 80, 50));
    quit->setStyleSheet("QWidget{background-color:rgb(155,215,60);border-top-right-radius:15px;border-top-left-radius:15px;"
                        "border-bottom-left-radius:15px;border-bottom-right-radius:15px;font:12pt Kristen ITC}");
    connect(quit, SIGNAL(clicked()), this, SLOT(exitFunction()));
    // The starting interface. A little bit ugly but enough to illustrate the idea.
}

Widget::~Widget() {
    clearData();
}

void Widget::startFunction() {
    clearData();

    resize(CANVAS_WIDTH, CANVAS_HEIGHT);
    move(QPoint(CENTERX-CANVAS_WIDTH/2, CENTERY-CANVAS_HEIGHT/2));

    disconnect(start, SIGNAL(clicked()), this, SLOT(startFunction()));
    disconnect(quit, SIGNAL(clicked()), this, SLOT(exitFunction()));
    delete start;
    delete quit;

    if (!pics.empty()) {
        for (int i = 0; i < pics.size(); i++) {
            delete pics[i];
            pics.erase(pics.begin()+i);
            i--;
        }
    }

    InitGame();
}

void Widget::exitFunction() {
    exit(0);
}

void Widget::InitGame() {
    srand(time(0));
    score=0;
    flag = 0;
    pierced = 0;
    yzero = INIT_Y;
    newMap = 0;
    totalBoards = 0;
    d = new doodle();

    for (int i = 0; i < BOARDS_LIMIT; i++) {
        double y = CANVAS_HEIGHT*0.9 - i * CANVAS_HEIGHT/10;
        double x = CANVAS_WIDTH/10 + Random(CANVAS_WIDTH*0.8);
        if (i == 2) {
            x = CANVAS_WIDTH/2;
        }
        board *temp = new board(x, y);
        boards.push_back(temp);
        totalBoards += 1;
    }

    gameTimer=new QTimer(this);
    connect(gameTimer,SIGNAL(timeout()),this,SLOT(dUpdate()));
    gameTimer->start(TIME_INTERVAL);
}

void Widget::backtoOriginal() {
    resize(CANVAS_WIDTH, CANVAS_HEIGHT);
    move(QPoint(CENTERX-CANVAS_WIDTH/2, CENTERY-CANVAS_HEIGHT/2));
    // Resize the widget.

    (*d).hp = (*nd).hp;
    delete nd;
    for (int i = 0; i < nboards.size(); i++) {
        delete nboards[0];
        nboards.erase(nboards.begin());
    }

    score += BONUS;
    // Awards for getting into the new map.

    disconnect(gameTimer,SIGNAL(timeout()),this,SLOT(nUpdate()));
    // Stop looping in the new map.
    delete gameTimer;

    gameTimer=new QTimer(this);
    connect(gameTimer,SIGNAL(timeout()),this,SLOT(dUpdate()));
    // Continue looping in the original one.

    gameTimer->start(TIME_INTERVAL);
}

void generate_board_map(int board_map[MAP_HEI][MAP_WID]){
    if(Random(30) == 29){
        int temp_map[MAP_HEI][MAP_WID] = {
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},//1
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,1,      1,1,1,1,0,       0,0,0,1,1,     1,1,1,0,0,     0,0,0,0,0,},
            {0,3,0,0,0,     0,0,0,0,1,      0,0,0,1,0,       0,0,0,1,0,     0,0,1,0,0,     0,0,0,0,1,},
            {0,0,1,0,0,     0,0,0,0,1,      0,0,0,1,0,       0,0,0,1,0,     0,0,1,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,1,      0,0,0,1,0,       0,0,0,1,0,     0,0,1,0,0,     0,0,0,0,0,},//6
            {0,0,0,1,0,     0,0,0,0,1,      1,1,1,1,0,       0,0,0,1,1,     1,1,1,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,1,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,1,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,1,1,1,       1,1,1,1,0,     0,0,0,0,0,     0,0,1,0,0,},//11
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,1,0,0,0,},
            {0,0,0,0,0,     1,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,1,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,1,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,1,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,1,      1,0,1,0,1,       0,1,0,1,0,     1,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},//16
            };
        // One special "egg": HE will appear, with probability 1/30
        for(int i=0; i<MAP_HEI;i++){
           for(int j=0; j<MAP_WID; j++){
              board_map[i][j] = temp_map[i][j];
              }
          }
        // Just copy this map
        return ;
    }
    if(Random(30) == 28){
        int temp_map[MAP_HEI][MAP_WID] = {
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},//1
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,3,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,1,1,1,1,      0,0,0,0,0,       0,1,1,1,1,     0,0,0,0,0,     0,0,0,0,0,},//6
            {0,0,0,0,0,     1,0,0,0,0,      1,0,0,0,0,       1,0,0,0,0,     1,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     1,0,0,0,0,      0,0,0,0,0,       1,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     1,0,0,0,0,      0,0,0,1,0,       1,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     1,0,0,0,0,      0,0,0,0,0,       1,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     1,0,0,0,0,      0,0,0,0,0,       1,0,0,0,0,     1,0,0,0,0,     0,0,0,0,0,},//11
            {0,0,0,0,0,     1,0,0,0,0,      1,0,0,0,0,       1,0,0,0,0,     1,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     1,0,0,0,0,      1,0,0,0,0,       0,1,1,1,1,     1,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,1,1,1,1,      1,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},//16
            };
        // Another special "egg": GG! with probability 1/30
        for(int i=0; i<MAP_HEI;i++){
           for(int j=0; j<MAP_WID; j++){
              board_map[i][j] = temp_map[i][j];
              }
          }
        ///just copy this map
        return ;
    }
    // 1->board 2->star 3->initial position
    int temp_map[MAP_HEI][MAP_WID] = {
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},//1
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,3,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,1,},
            {0,0,1,0,0,     0,0,0,0,1,      0,0,0,0,0,       1,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},//6
            {0,0,0,1,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,1,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       1,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,1,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,1,0,0,},//11
            {0,0,0,0,0,     0,0,0,0,0,      0,0,1,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,1,0,0,0,},
            {0,0,0,0,0,     1,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,1,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,1,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,1,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,0,      1,0,1,0,1,       0,1,0,1,0,     1,0,0,0,0,     0,0,0,0,0,},
            {0,0,0,0,0,     0,0,0,0,0,      0,0,0,0,0,       0,0,0,0,0,     0,0,0,0,0,     0,0,0,0,0,},//16
        };
    for(int i=0; i<MAP_HEI; i++){
      for(int j=0; j<MAP_WID; j++){
          board_map[i][j] = temp_map[i][j];
      }
    }
    for(int i=0; i<MAP_HEI; i++){
      for(int j=0; j<MAP_WID; j++){
          if(board_map[i][j] == 0 && board_map[i+1][j]!=1 && board_map[i-1][j]!=1
                                  && board_map[i+2][j]!=1 && board_map[i-2][j]!=1
                                  && board_map[i][j+1]!=1 && board_map[i][j-1]!=1
                                  && board_map[i-1][j-1]==0 && board_map[i-1][j+1]==0
                                  && board_map[i][j+2]!=1 && board_map[i][j-2]!=1
                                  && j > 4 && i > 3){
              // Make sure the boards would not lie too close to each other
              int judge = Random(30);
              if (judge > 28) {
                  board_map[i][j] = 1;
              }
              // The probability is 1/20
          }
      }
    }
    return ;
}

void generate_star_map(int star_map[][MAP_WID],int board_map[][MAP_WID]){
    for(int i=0; i<MAP_HEI; i++){
      for(int j=0; j<MAP_WID; j++){
          star_map[i][j] = board_map[i][j];
      }
    }
    for(int i=0; i<MAP_HEI; i++){
      for(int j=0; j<MAP_WID; j++){
          if(star_map[i+1][j] == 1 && star_map[i][j]  == 0 && i>4 && i <MAP_HEI-2
                                    && star_map[i][j+1]== 0 && star_map[i][j-1]==0
                                    && star_map[i][j+2]== 0 && star_map[i][j-2]==0
                                    && star_map[i-1][j-1]==0 && star_map[i-1][j+1]==0
                                    && star_map[i-1][j-2]==0 && star_map[i-1][j+2]==0
                                    && star_map[i-1][j]== 0 && i>2){
              int judge = Random(20);
              if (judge > 9){
                  star_map[i][j] = 2;
                  // The star will randomly appear on one board.
                  // The probability of it was set to be 1/2, this probability can be changed.
                  // Just change the number of "judge > 9"
              }
          }
      }
    }
}

void Widget::InitNewMap() {
    resize(NEWMAP_WIDTH, NEWMAP_HEIGHT);
    move(QPoint(CENTERX-NEWMAP_WIDTH/2, CENTERY-NEWMAP_HEIGHT/2));

    srand(time(0));
    nyzero = NEWMAP_Y;
    int healthPoint = d->hp;
    nd = new doodle(healthPoint/2+1);
    /* Acoording to our design, when the doodle enter the door, a fraction of
     * its health was donated to someone and the doodle wants to get its life back.
     * So he tried to eat his health points back. The doodle comes to one of "someone"'s
     * place and saw many "star"s. When the doodle gets one star, it will gain 1 hp. */

    board *temp = new board(INIT_X, INIT_Y+2*DIS);
    nboards.push_back(temp);
    int board_map[MAP_HEI][MAP_WID];
    generate_board_map(board_map);

    for(int m=0;m<MAP_HEI;m++){
        for(int n=0;n<MAP_WID;n++){
            if(board_map[m][n] == 1){
                board *temp = new board(n * DIS,m*DIS);
                nboards.push_back(temp);
            };
        }
    }

    int star_map[MAP_HEI][MAP_WID];
    generate_star_map(star_map,board_map);
    for(int m=0;m<MAP_HEI;m++){
        for(int n=0;n<MAP_WID;n++){
            if(star_map[m][n] == 2){
                star *newstar = new star(n * DIS,m*DIS);
                stars.push_back(newstar);
            };
        }
    }
    // Stars always appear above the board.

    disconnect(gameTimer,SIGNAL(timeout()),this,SLOT(dUpdate()));
    delete gameTimer;

    gameTimer = new QTimer(this);
    connect(gameTimer,SIGNAL(timeout()),this,SLOT(nUpdate()));

    gameTimer->start(TIME_INTERVAL);
}

void Widget::GenerateItems() {
    if (score%1000 == 1) {
        flag = 1; // HealthPacks
    } else if (score%675 == 200) {
        flag = 2; // Springs
    } else if (score%555 == 300) {
        flag = 3; // Stings
    } else if (score%525 == 400) {
        flag = 4; // Entrances
    }
    // The first number refers to the distance between two same items,
    // the second one refers to the score at which the first one of its kind appears.

    if (boards.size() + flexibles.size() < BOARDS_LIMIT) {
        // When one board falls out of the screen, a new one should occur at the top.
        double newx = CANVAS_WIDTH/10 + Random(CANVAS_WIDTH*0.8);
        double newy = 0;

        if (totalBoards%5 == 1) {
            flexible *newf = new flexible(newx, newy);
            flexibles.push_back(newf);
        } else {
            board *newBoard = new board(newx, newy);
            boards.push_back(newBoard);
        }

        switch(this->flag) {
        case 1:
        {
            health *newHealth = new health(newx, newy - HPHEIGHT/2 - BHEIGHT/2);
            healthPacks.push_back(newHealth);
            flag = 0;
            break;
        }
        case 2:
        {
            spring *newSpring = new spring(newx, newy - SPRINGHEIGHT/2 - BHEIGHT/2);
            springs.push_back(newSpring);
            flag = 0;
            break;
        }
        case 3:
        {
            sting *newSting = new sting(newx, newy - STINGHEIGHT/2 - BHEIGHT/2);
            stings.push_back(newSting);
            flag = 0;
            break;
        }
        case 4:
        {
            entrance * newEntrance = new entrance(newx, newy - DOORHEIGHT/2 - BHEIGHT/2);
            doors.push_back(newEntrance);
            flag = 0;
            break;
        }
        default:
            break;
        }
        // Mark the type of the item to be generated.

        totalBoards += 1;
    }   
}

void Widget::paintEvent(QPaintEvent *) {
    if (newMap == 0) {
        QPainter painter(this);
        if (pierced > 0) {
            painter.fillRect(QRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT), Qt::red);
            pierced--;
            // Represents the time period during which the screen is red
            // due to the pain of the doodle.
        } else if (pierced == 0) {
            painter.fillRect(QRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT), Qt::white);
        }

        for (int i = 0; i < healthPacks.size(); i++) {
            int cx = ((*healthPacks[i]).center)[0];
            int cy = ((*healthPacks[i]).center)[1];
            painter.drawImage(QRect(cx-HPWIDTH/2, cy-HPHEIGHT/2, HPWIDTH, HPHEIGHT),
                              QImage("healthPack.png"));
        }

        for (int i = 0; i < doors.size(); i++) {
            int cx = ((*doors[i]).center)[0];
            int cy = ((*doors[i]).center)[1];
            painter.drawImage(QRect(cx-DOORWIDTH/2, cy-DOORHEIGHT/2, DOORWIDTH, DOORHEIGHT),
                              QImage("door.png"));
        }

        for (int i = 0; i < boards.size(); i++) {
            int cx = ((*boards[i]).center)[0];
            int cy = ((*boards[i]).center)[1];
            painter.drawImage(QRect(cx-BWIDTH/2, cy-BHEIGHT/2, BWIDTH, BHEIGHT),
                              QImage("platform.png"));
        }

        for (int i = 0; i < flexibles.size(); i++) {
            int cx = ((*flexibles[i]).center)[0];
            int cy = ((*flexibles[i]).center)[1];
            painter.drawImage(QRect(cx - flexibles[i]->length, cy-BHEIGHT/2,
                                    2*flexibles[i]->length, BHEIGHT),
                              QImage("flexible.png"));
        }

        for (int i = 0; i < springs.size(); i++) {
            int cx = ((*springs[i]).center)[0];
            int cy = ((*springs[i]).center)[1];
            painter.drawImage(QRect(cx-SPRINGWIDTH/2, cy-SPRINGHEIGHT/2, SPRINGWIDTH, SPRINGHEIGHT),
                              QImage("spring.png"));
        }

        for (int i = 0; i < stings.size(); i++) {
            int cx = ((*stings[i]).center)[0];
            int cy = ((*stings[i]).center)[1];
            painter.drawImage(QRect(cx-STINGWIDTH/2, cy-2*STINGHEIGHT/3, STINGWIDTH, 7*STINGHEIGHT/6),
                              QImage("sting.png"));
        }

        int cx = (*d).dcenter[0];
        int cy = (*d).dcenter[1];
        painter.drawImage(QRect(cx-DWIDTH/2, cy-DHEIGHT/2, DWIDTH, DHEIGHT),
                          QImage("doodle.png"));

        painter.setPen(Qt::black);
        painter.setFont(QFont("Consolas",14));
        painter.drawText(20, 20, " H P : "+QString::number((*d).hp));
        painter.drawText(20, 40, "SCORE: "+QString::number(score));

    } else if (newMap == 1) {
        // In the new map, different objects are painted.
        QPainter painter(this);
        painter.fillRect(QRect(0, 0, NEWMAP_WIDTH, NEWMAP_HEIGHT), Qt::white);

        for (int i = 0; i < nboards.size(); i++) {
            int cx = ((*nboards[i]).center)[0];
            int cy = ((*nboards[i]).center)[1];
            painter.drawImage(QRect(cx-BWIDTH/2, cy-BHEIGHT/2, BWIDTH, BHEIGHT),
                              QImage("platform.png"));
        }

        for (int i = 0; i < stars.size(); i++) {
            int cx = ((*stars[i]).center)[0];
            int cy = ((*stars[i]).center)[1];
            painter.drawImage(QRect(cx-STARWIDTH/2, cy-STARHEIGHT/2, STARWIDTH, STARHEIGHT),
                              QImage("star.png"));
        }

        int cx = (*nd).dcenter[0];
        int cy = (*nd).dcenter[1];
        painter.drawImage(QRect(cx-DWIDTH/2, cy-DHEIGHT/2, DWIDTH, DHEIGHT),
                          QImage("doodle.png"));

        painter.setPen(Qt::black);
        painter.setFont(QFont("Consolas",14));
        painter.drawText(NEWMAP_WIDTH/2, 40, "Your life was stolen by someone! Get them back!");
        painter.drawText(20, 20, " H P : "+QString::number((*nd).hp));
        painter.drawText(20, 40, "SCORE: "+QString::number(score));
        painter.drawText(20, 60, "Number of Remaining Stars: "+QString::number(stars.size()));
    }
}

void Widget::keyPressEvent(QKeyEvent *event) {
    if (newMap == 0) {
        switch (event->key()) {
        case Qt::Key_Left:
            (*d).dvx = KEYBOARD_LEFT;
            break;
        case Qt::Key_Right:
            (*d).dvx = KEYBOARD_RIGHT;
            break;
        case Qt::Key_Down:
            (*d).dvx = 0;
            break;
        case Qt::Key_P:
            gameTimer->stop();
            break;
        case Qt::Key_R:
            gameTimer->start(TIME_INTERVAL);
            break;
            // For PAUSE and CONTINUE.
            // P: PAUSE
            // R: RESUME
        default:
            break;
        }
    } else if (newMap == 1) {
        switch (event->key()) {
        case Qt::Key_Left:
            (*nd).dvx = KEYBOARD_LEFT;
            break;
        case Qt::Key_Right:
            (*nd).dvx = KEYBOARD_RIGHT;
            break;
        case Qt::Key_Down:
            (*nd).dvx = 0;
            break;
        case Qt::Key_P:
            gameTimer->stop();
            break;
        case Qt::Key_R:
            gameTimer->start(TIME_INTERVAL);
            break;
        default:
            break;
        }
    }
}

bool Widget::IsGameOver() {
    if ((*d).hp == 0) {
        return 1;
    }
    if ((*d).dcenter[1] >= CANVAS_HEIGHT + DHEIGHT/2) {
        return 1;
    }
    return 0;
}

void Widget::nIsGameOver() {
    if ((*nd).dcenter[1] >= NEWMAP_HEIGHT + DHEIGHT/2) {
        newMap = 0;
    }
}

void Widget::takeItem(){
    for (int i = 0; i < healthPacks.size(); i++) {
        if ((*d).dcenter[0] >= ((*healthPacks[i]).center)[0]-HPWIDTH/2
                && (*d).dcenter[0] <= ((*healthPacks[i]).center)[0]+HPWIDTH/2
                && (*d).dcenter[1]+DHEIGHT/2 >= ((*healthPacks[i]).center)[1]-0.8*HPHEIGHT
                && (*d).dcenter[1]+DHEIGHT/2 <= ((*healthPacks[i]).center)[1]+HPHEIGHT/2) {
            // The judging range is a little bit wider to make sure they never misses.
            health *temp = new health(((*healthPacks[i]).center)[0],((*healthPacks[i]).center)[1]);
            delete healthPacks[i];
            healthPacks.erase(healthPacks.begin()+i);
            (*d).take(temp);
            i--;
        }
    }
    for (int i = 0; i < doors.size(); i++) {
        if ((*d).dcenter[0] >= ((*doors[i]).center)[0]-DOORWIDTH/2
                && (*d).dcenter[0] <= ((*doors[i]).center)[0]+DOORWIDTH/2
                && (*d).dcenter[1]+DHEIGHT/2 >= ((*doors[i]).center)[1]-0.8*DOORHEIGHT
                && (*d).dcenter[1]+DHEIGHT/2 <= ((*doors[i]).center)[1]+DOORHEIGHT/2) {
            newMap = 1;
            delete doors[i];
            doors.erase(doors.begin()+i);
            i--;
        }
    }
}

void Widget::takeStar() {
    if (stars.empty()) {
        newMap = 0;
    } else {
        for (int i = 0; i < stars.size(); i++) {
            if ((*nd).dcenter[0] >= ((*stars[i]).center)[0]-DOORWIDTH/2
                    && (*nd).dcenter[0] <= ((*stars[i]).center)[0]+DOORWIDTH/2
                    && (*nd).dcenter[1]+DHEIGHT/2 >= ((*stars[i]).center)[1]-0.8*DOORHEIGHT
                    && (*nd).dcenter[1]+DHEIGHT/2 <= ((*stars[i]).center)[1]+DOORHEIGHT/2) {
                delete stars[i];
                (*nd).hp++;
                stars.erase(stars.begin()+i);
                i--;
            }
        }
    }
}

void Widget::onBoard() {
    if (newMap == 0) {
        for (int i = 0; i < boards.size(); i++) {
            if ((*d).dcenter[0] >= ((*boards[i]).center)[0]-BWIDTH/2-DWIDTH/4
                    && (*d).dcenter[0] <= ((*boards[i]).center)[0]+BWIDTH/2+DWIDTH/4
                    && (*d).dcenter[1]+DHEIGHT/2 >= ((*boards[i]).center)[1]-0.8*BHEIGHT
                    && (*d).dcenter[1]+DHEIGHT/2 <= ((*boards[i]).center)[1]+BHEIGHT/2
                    && (*d).dvy > 0) {
                double h = -1 * (yzero-((*boards[i]).center)[1]);
                if (h < 0) {
                    h = -1 *h;
                    // The absolute value.
                    // The reason for this step was writtrn in "classes.h".
                }
                double newdvy = Velocity(h);
                (*d).jump(((*boards[i]).center)[1], newdvy);
            }
        }
        for (int i = 0; i < flexibles.size(); i++) {
            flexibles[i]->move();
            if ((*d).dcenter[0] >= ((*flexibles[i]).center)[0]-DWIDTH/8-flexibles[i]->length
                    && (*d).dcenter[0] <= ((*flexibles[i]).center)[0]+DWIDTH/8+flexibles[i]->length
                    && (*d).dcenter[1]+DHEIGHT/2 >= ((*flexibles[i]).center)[1]-0.8*BHEIGHT
                    && (*d).dcenter[1]+DHEIGHT/2 <= ((*flexibles[i]).center)[1]+BHEIGHT/2
                    && (*d).dvy > 0) {
                double h = -1 * (yzero-((*flexibles[i]).center)[1]);
                if (h < 0) {
                    h = -1 *h;
                }
                double newdvy = Velocity(h);
                (*d).jump(((*flexibles[i]).center)[1], newdvy);
            }
        }
        for (int i = 0; i < springs.size(); i++) {
            springs[i]->move();
            if ((*d).dcenter[0] >= ((*springs[i]).center)[0]-SPRINGWIDTH/2-DWIDTH/4
                    && (*d).dcenter[0] <= ((*springs[i]).center)[0]+SPRINGWIDTH/2+DWIDTH/4
                    && (*d).dcenter[1]+DHEIGHT/2 >= ((*springs[i]).center)[1]-0.8*SPRINGHEIGHT
                    && (*d).dcenter[1]+DHEIGHT/2 <= ((*springs[i]).center)[1]+SPRINGHEIGHT/2
                    && (*d).dvy > 0) {
                double h = -1 * (yzero-((*springs[i]).center)[1]);
                if (h < 0) {
                    h = -1 *h;
                }
                double newdvy = 2.5 * Velocity(h);
                (*d).jump(((*springs[i]).center)[1], newdvy);
            }
        }
        for (int i = 0; i < stings.size(); i++) {
            stings[i]->move();
            if ((*d).dcenter[0] >= ((*stings[i]).center)[0]-STINGWIDTH/2-DWIDTH/4
                    && (*d).dcenter[0] <= ((*stings[i]).center)[0]+STINGWIDTH/2+DWIDTH/4
                    && (*d).dcenter[1]+DHEIGHT/2 >= ((*stings[i]).center)[1]-STINGHEIGHT*0.4
                    && (*d).dcenter[1]+DHEIGHT/2 <= ((*stings[i]).center)[1]+STINGHEIGHT/2
                    && (*d).dvy > 0 && pierced == 0) {
                d->hp --;
                pierced = 3;
                // To show the red color for longer time.
            }
        }
    } else if (newMap == 1) {
        for (int i = 0; i < nboards.size(); i++) {
            if ((*nd).dcenter[0] >= ((*nboards[i]).center)[0]-BWIDTH/2-DWIDTH/4
                    && (*nd).dcenter[0] <= ((*nboards[i]).center)[0]+BWIDTH/2+DWIDTH/4
                    && (*nd).dcenter[1]+DHEIGHT/2 >= ((*nboards[i]).center)[1]-0.8*BHEIGHT
                    && (*nd).dcenter[1]+DHEIGHT/2 <= ((*nboards[i]).center)[1]+BHEIGHT/2
                    && (*nd).dvy > 0) {
                double h = -1 * (nyzero-((*nboards[i]).center)[1]);
                if (h < 0) {
                    h = -1 *h;
                }
                double newdvy = 0.6*0.9*log(h+2)/log(1.22189954);
                (*nd).jump(((*nboards[i]).center)[1], newdvy);
            }
        }
    }
}

void Widget::moveMap() {
    if ((*d).dcenter[1] <= HIGHEST && (*d).dvy < 0) {
        for (int i = 0; i < healthPacks.size(); i++) {
            ((*healthPacks[i]).center)[1] -= (*d).dvy;
        }
        for (int i = 0; i < doors.size(); i++) {
            ((*doors[i]).center)[1] -= (*d).dvy;
        }
        for (int i = 0; i < boards.size(); i++) {
            ((*boards[i]).center)[1] -= (*d).dvy;
        }
        for (int i = 0; i < flexibles.size(); i++) {
            ((*flexibles[i]).center)[1] -= (*d).dvy;
        }
        for (int i = 0; i < springs.size(); i++) {
            ((*springs[i]).center)[1] -= (*d).dvy;
        }
        for (int i = 0; i < stings.size(); i++) {
            ((*stings[i]).center)[1] -= (*d).dvy;
        }

        (*d).dcenter[1] -= (*d).dvy;
    }
}

void Widget::deleteItems() {
    for (int i = 0; i < boards.size(); i++) {
        if ( ((*boards[i]).center)[1] > CANVAS_HEIGHT ) {
            delete boards[i];
            boards.erase(boards.begin()+i);
            i--;
        }
    }
    for (int i = 0; i < healthPacks.size(); i++) {
        if ( ((*healthPacks[i]).center)[1] > CANVAS_HEIGHT ) {
            delete healthPacks[i];
            healthPacks.erase(healthPacks.begin()+i);
            i--;
        }
    }
    for (int i = 0; i < doors.size(); i++) {
        if ( ((*doors[i]).center)[1] > CANVAS_HEIGHT ) {
            delete doors[i];
            doors.erase(doors.begin()+i);
            i--;
        }
    }
    for (int i = 0; i < flexibles.size(); i++) {
        if ( ((*flexibles[i]).center)[1] > CANVAS_HEIGHT ) {
            delete flexibles[i];
            flexibles.erase(flexibles.begin()+i);
            i--;
        }
    }
    for (int i = 0; i < springs.size(); i++) {
        if ( ((*springs[i]).center)[1] > CANVAS_HEIGHT ) {
            delete springs[i];
            springs.erase(springs.begin()+i);
            i--;
        }
    }
    for (int i = 0; i < stings.size(); i++) {
        if ( ((*stings[i]).center)[1] > CANVAS_HEIGHT ) {
            delete stings[i];
            stings.erase(stings.begin()+i);
            i--;
        }
    }
}

void Widget::calculateScore() {
    if (d->dcenter[1] < yzero && d->dvy < 0) {
        score += (yzero-d->dcenter[1]) / 10;
    }
}

void Widget::dUpdate() {
    if (d->dvy == 0) {
        yzero = d->dcenter[1];
    }

    this->takeItem();

    if (newMap == 1) {
        this->InitNewMap();
    }
    else{
        this->onBoard();
        d->move();

        this->deleteItems();
        this->GenerateItems();
        this->moveMap();
        this->calculateScore();

        if (IsGameOver()) {
            GameOver();
            return;
        }
    }
    update();
}

void Widget::nUpdate() {
    if (newMap == 0) {
        this->backtoOriginal();
    }
    // Without this 'if', the following codes would be carried out without
    // corresponding parameters since they are deleted in backtoOriginal(),
    // which would result in an error, some times a crash.

    else{
        if (nd->dvy == 0) {
            nyzero = nd->dcenter[1];
        }

        this->takeStar();
        this->onBoard();
        nd->nmove();

        nIsGameOver();
    }
    update();
}

void Widget::clearData() {
    delete d;
    if (newMap == 1) {
        delete nd;
    }
    if (!boards.empty()) {
        for (board *temp: boards) {
            delete temp;
        }
        boards.clear();
    }
    if (!nboards.empty()) {
        for (board *temp: nboards) {
            delete temp;
        }
        nboards.clear();
    }
    if (!healthPacks.empty()) {
        for (health *temp: healthPacks) {
            delete temp;
        }
        healthPacks.clear();
    }
    if (!stars.empty()) {
        for (star *temp: stars) {
            delete temp;
        }
        stars.clear();
    }
    if (!doors.empty()) {
        for (entrance *temp: doors) {
            delete temp;
        }
        doors.clear();
    }
    if (!flexibles.empty()) {
        for (flexible *temp: flexibles) {
            delete temp;
        }
        flexibles.clear();
    }
    if (!springs.empty()) {
        for (spring *temp: springs) {
            delete temp;
        }
        springs.clear();
    }
    if (!stings.empty()) {
        for (sting *temp: stings) {
            delete temp;
        }
        stings.clear();
    }
}

void Widget::GameOver() {
    gameTimer->stop();
    update();
    // Make sure the red color is shown when the doodle is pierced to death.

    start = new QPushButton(QString("AGAIN?"), this);
    start->setGeometry(QRect(CANVAS_WIDTH/4, CANVAS_HEIGHT/4, 80, 50));
    start->setStyleSheet("QWidget{background-color:rgb(155,215,60);border-top-right-radius:15px;border-top-left-radius:15px; "
                         "border-bottom-left-radius:15px; border-bottom-right-radius:15px;font:12pt Kristen ITC}");
    connect(start, SIGNAL(clicked()), this, SLOT(startFunction()));
    quit = new QPushButton(QString("NOPE!"), this);
    quit->setGeometry(QRect(CANVAS_WIDTH/2, CANVAS_HEIGHT/4, 80, 50));
    quit->setStyleSheet("QWidget{background-color:rgb(155,215,60);border-top-right-radius:15px;border-top-left-radius:15px; "
                         "border-bottom-left-radius:15px; border-bottom-right-radius:15px;font:12pt Kristen ITC}");
    connect(quit, SIGNAL(clicked()), this, SLOT(exitFunction()));

    start->show();
    quit->show();
    // Ask whether to try again.
}

