#include "rson.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString fname;
    if(argc > 2) {
        // first optional argument is path to RSON file
        fname = argv[1];
    }
    else {
        fname = "./RadioFiles/FT-450D.json";
    }
    RSON w(nullptr, fname);
    w.show();
    return QApplication::exec();
}
