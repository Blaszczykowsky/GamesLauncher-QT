#pragma once
#include <QGraphicsScene>
#include <QHash>
#include "gra.h"
#include "plansza.h"
#include "tokenitem.h"

class BoardScene : public QGraphicsScene
{
    Q_OBJECT
public:
    BoardScene(Gra* gra, QObject* parent = nullptr);

signals:
    void pionekKlikniety(Pionek* p);

public slots:
    void odswiez();
    void resetujTlo();

private:
    void rysujPlansze();
    QColor kolorQt(KolorGracza k) const;
    void utworzTokenyJesliTrzeba();
    void ustawPozycjeTokenow();
    void ustawPodswietlenia();

    QString kluczPola(const Gracz& g, const Pionek& p) const;

private:
    Gra* m_gra = nullptr;
    Plansza m_plansza;

    QHash<Pionek*, TokenItem*> m_tokeny;
};
