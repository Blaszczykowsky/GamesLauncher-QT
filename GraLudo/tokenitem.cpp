#include "tokenitem.h"
#include <QPen>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>

TokenItem::TokenItem(Pionek* pionek, const QColor& kolor, QGraphicsItem* parent)
    : QObject(), QGraphicsEllipseItem(parent), m_pionek(pionek)
{
    setRect(-14, -14, 28, 28);
    setBrush(QBrush(kolor));
    setPen(QPen(Qt::black, 1));
    setFlag(QGraphicsItem::ItemIsSelectable, false);
}

void TokenItem::ustawPodswietlenie(bool on)
{
    QPen p = pen();
    p.setWidth(on ? 4 : 1);
    setPen(p);
}

void TokenItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    emit klikniety(m_pionek);
    QGraphicsEllipseItem::mousePressEvent(event);
}
