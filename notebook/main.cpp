#include "Notebook.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Notebook w;
    w.show();
    return a.exec();
}
