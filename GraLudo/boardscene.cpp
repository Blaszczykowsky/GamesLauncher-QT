#include "boardscene.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QPen>
#include <QBrush>

BoardScene::BoardScene(Gra* gra, QObject* parent)
    : QGraphicsScene(parent), m_gra(gra)
{
    setSceneRect(-420, -420, 840, 840);
    resetujTlo();
}

static QColor kolorWyszarzony(const QColor& c)
{
    QColor g(220, 220, 220);
    return QColor((c.red() + g.red())/2,
                  (c.green() + g.green())/2,
                  (c.blue() + g.blue())/2);
}


QColor BoardScene::kolorQt(KolorGracza k) const
{
    switch (k)
    {
    case KolorGracza::Czerwony: return QColor(220, 60, 60);
    case KolorGracza::Zielony: return QColor(60, 180, 80);
    case KolorGracza::Niebieski: return QColor(70, 120, 240);
    case KolorGracza::Zolty: return QColor(230, 200, 60);
    }
    return Qt::gray;
}

void BoardScene::rysujPlansze()
{
    clear();

    QPen pen(Qt::black, 1);
    for (int i = 0; i < 52; ++i)
    {
        QPointF p = m_plansza.torGlowny()[i];
        auto* kolo = addEllipse(p.x() - 10, p.y() - 10, 20, 20, pen, QBrush(Qt::white));
        kolo->setZValue(-10);
    }

    for (int k = 0; k < 4; ++k)
    {
        KolorGracza kg = static_cast<KolorGracza>(k);
        QBrush pedzelDom(kolorWyszarzony(kolorQt(kg)));

        for (int i = 0; i < 4; ++i)
        {
            QPointF p = m_plansza.toryDomowe()[k][i];
            auto* kolo = addEllipse(p.x() - 9, p.y() - 9, 18, 18, pen, pedzelDom);
            kolo->setZValue(-10);
        }
    }
    for (int k = 0; k < 4; ++k)
    {
        KolorGracza kg = static_cast<KolorGracza>(k);
        QBrush pedzelBaza(kolorWyszarzony(kolorQt(kg)));

        for (int id = 0; id < 4; ++id)
        {
            QPointF p = m_plansza.pozycjaBaza(kg, id);
            auto* kolo = addEllipse(p.x() - 9, p.y() - 9, 18, 18, pen, pedzelBaza);
            kolo->setZValue(-10);
        }
    }
    addEllipse(-16, -16, 32, 32, pen, QBrush(QColor(240,240,240)))->setZValue(-10);

}

QString BoardScene::kluczPola(const Gracz& g, const Pionek& p) const
{
    if (p.wBazie())
        return "baza_" + QString::number((int)p.kolor()) + "_" + QString::number(p.id());

    if (p.naTorzeGlownym())
    {
        int abs = (g.indeksStartu() + p.krok()) % 52;
        return "tor_" + QString::number(abs);
    }
    return "dom_" + QString::number((int)p.kolor()) + "_" + QString::number(p.krok() - 52);
}

void BoardScene::utworzTokenyJesliTrzeba()
{
    if (!m_gra) return;

    for (auto& g : m_gra->gracze())
    {
        for (auto& p : const_cast<QVector<Pionek>&>(g.pionki()))
        {
            if (!m_tokeny.contains(&p))
            {
                auto* item = new TokenItem(&p, kolorQt(p.kolor()));
                addItem(item);
                m_tokeny.insert(&p, item);

                connect(item, &TokenItem::klikniety, this, [this](Pionek* pp){
                    emit pionekKlikniety(pp);
                });

            }
        }
    }
}

void BoardScene::ustawPozycjeTokenow()
{
    QHash<QString, QVector<TokenItem*>> grupy;

    for (const auto& g : m_gra->gracze())
    {
        for (const auto& p : g.pionki())
        {
            TokenItem* item = m_tokeny.value(const_cast<Pionek*>(&p), nullptr);
            if (!item) continue;

            QString key = kluczPola(g, p);
            grupy[key].push_back(item);
        }
    }

    for (auto it = grupy.begin(); it != grupy.end(); ++it)
    {
        const QString& key = it.key();
        auto& lista = it.value();
        QPointF bazowa(0,0);
        bool znaleziono = false;
        
        for (const auto& g : m_gra->gracze())
        {
            for (const auto& p : g.pionki())
            {
                if (kluczPola(g, p) == key)
                {
                    bazowa = m_plansza.pozycjaDlaPionka(g, p);
                    znaleziono = true;
                    break;
                }
            }
            if (znaleziono) break;
        }

        for (int i = 0; i < lista.size(); ++i)
        {
            QPointF off(0,0);
            if (lista.size() == 2)
                off = (i == 0) ? QPointF(-10, 0) : QPointF(10, 0);
            else if (lista.size() >= 3)
                off = QPointF(-10 + i*10, (i%2==0)? -8 : 8);

            lista[i]->setPos(bazowa + off);
        }
    }
}

void BoardScene::ustawPodswietlenia()
{
    for (auto* item : m_tokeny)
        item->ustawPodswietlenie(false);

    if (!m_gra || !m_gra->czyRzucono()) return;

    auto mozliwe = m_gra->mozliwePionki();
    for (auto* p : mozliwe)
    {
        if (m_tokeny.contains(p))
            m_tokeny[p]->ustawPodswietlenie(true);
    }
}

void BoardScene::odswiez()
{
    if (!m_gra) return;

    utworzTokenyJesliTrzeba();
    ustawPozycjeTokenow();
    ustawPodswietlenia();
}

void BoardScene::resetujTlo()
{
    clear();          
    m_tokeny.clear(); 
    rysujPlansze();   
}

