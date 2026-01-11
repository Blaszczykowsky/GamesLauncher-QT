#include "kostka.h"
#include <QRandomGenerator>

int Kostka::rzut()
{
    return QRandomGenerator::global()->bounded(1, 7);
}
