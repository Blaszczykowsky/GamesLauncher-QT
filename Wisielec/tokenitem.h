#pragma once
#include <QObject>
#include <QGraphicsEllipseItem>
#include "pionek.h"

class TokenItem : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    TokenItem(Pionek* pionek, const QColor& kolor, QGraphicsItem* parent = nullptr);

    Pionek* pionek() const { return m_pionek; }
    void ustawPodswietlenie(bool on);

signals:
    void klikniety(Pionek* pionek);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    Pionek* m_pionek = nullptr;
};
