#pragma once
#include <QVector>
#include <QPointF>
#include "pionek.h"

class Gracz;

class Plansza
{
public:
    Plansza();

    const QVector<QPointF>& torGlowny() const { return m_torGlowny; }
    const QVector<QVector<QPointF>>& toryDomowe() const { return m_torDomowy; }

    QPointF pozycjaTorGlowny(int absIndex) const;
    QPointF pozycjaTorDomowy(KolorGracza kolor, int idx0_5) const;
    QPointF pozycjaBaza(KolorGracza kolor, int id0_3) const;

    QPointF pozycjaDlaPionka(const Gracz& gracz, const Pionek& pionek) const;

private:
    QPointF gridNaPunkt(int gx, int gy) const;

private:
    QVector<QPointF> m_torGlowny;              
    QVector<QVector<QPointF>> m_torDomowy;      
    QVector<QVector<QPointF>> m_baza;           

    double m_rozmiarPola = 40.0;                
};

