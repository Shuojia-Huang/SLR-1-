#include "mywindow.h"
#include <QApplication>

int main(int argc, char *argv[]){
    QApplication app(argc, argv);
    Mywindow w;
    w.show();
    return app.exec();
}