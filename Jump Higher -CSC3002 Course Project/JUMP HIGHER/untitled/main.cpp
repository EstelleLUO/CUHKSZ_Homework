#include <QCoreApplication>
#include "widget.h"
#include "classes.h"
#include "global.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
