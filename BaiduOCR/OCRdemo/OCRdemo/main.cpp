#include "OCRdemo.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OCRdemo w;
    w.show();
    return a.exec();
}
