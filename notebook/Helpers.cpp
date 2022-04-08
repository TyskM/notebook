#include "Helpers.h"
#include <qlayout.h>
#include <qwidget.h>

void Helpers::clearLayout(QLayout* layout, int from)
{
    QLayoutItem* item;
    while ( ( item = layout->takeAt(from) ) != NULL )
    {
        item->widget()->deleteLater();
    }
}
