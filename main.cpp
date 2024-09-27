#include "horario.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Horario w;
    w.show();
    return a.exec();
}
