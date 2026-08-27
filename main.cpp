#include "rson.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RSON w(nullptr, "./RadioFiles/FT-450D.json");
    w.show();
    return QApplication::exec();
}
