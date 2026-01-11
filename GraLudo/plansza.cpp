#include "plansza.h"
#include "gracz.h"
#include <algorithm>

static int kolorIndex(KolorGracza k) { return static_cast<int>(k); }

QPointF Plansza::gridNaPunkt(int gx, int gy) const
{
    // siatka 15x15: (0..14), srodek (7,7) -> (0,0)
    const double x = (gx - 7) * m_rozmiarPola;
    const double y = (gy - 7) * m_rozmiarPola;
    return QPointF(x, y);
}

Plansza::Plansza()
{
    // ========= TOR GLOWNY (52) =========
    // Kolejnosc indeksow 0..51 (start = (6,0)), potem zgodnie z ruchem wskazowek zegara.
    // Starty sa co 13 pol: 0 (gora), 13 (lewo), 26 (dol), 39 (prawo).

    const QVector<QPoint> sciezka = {
        {6,0},{7,0},{8,0},{8,1},{8,2},{8,3},{8,4},{8,5},
        {9,6},{10,6},{11,6},{12,6},{13,6},{14,6},{14,7},{14,8},
        {13,8},{12,8},{11,8},{10,8},{9,8},
        {8,9},{8,10},{8,11},{8,12},{8,13},{8,14},{7,14},{6,14},
        {6,13},{6,12},{6,11},{6,10},{6,9},
        {5,8},{4,8},{3,8},{2,8},{1,8},{0,8},{0,7},{0,6},
        {1,6},{2,6},{3,6},{4,6},{5,6},
        {6,5},{6,4},{6,3},{6,2},{6,1}
    };

    m_torGlowny.clear();
    m_torGlowny.reserve(52);
    for (const auto& p : sciezka)
        m_torGlowny.push_back(gridNaPunkt(p.x(), p.y()));

    // ========= TORY DOMOWE (4 x 6) =========
    // 6 pol "do mety" (meta u Ciebie to krok==57, czyli 6-te pole domu)
    // Czerwony (gora)   -> kolumna 7: (7,1..6)
    // Zolty (lewo)      -> wiersz 7:  (1..6,7)
    // Niebieski (dol)   -> kolumna 7: (7,13..8) w gore
    // Zielony (prawo)   -> wiersz 7:  (13..8,7) w lewo

    m_torDomowy.resize(4);
    for (int k = 0; k < 4; ++k)
        m_torDomowy[k].resize(4);

    // Czerwony
    for (int i = 0; i < 4; ++i) m_torDomowy[kolorIndex(KolorGracza::Czerwony)][i] = gridNaPunkt(7, 1 + i);
    // Zolty
    for (int i = 0; i < 4; ++i) m_torDomowy[kolorIndex(KolorGracza::Zolty)][i]   = gridNaPunkt(1 + i, 7);
    // Niebieski
    for (int i = 0; i < 4; ++i) m_torDomowy[kolorIndex(KolorGracza::Niebieski)][i]= gridNaPunkt(7, 13 - i);
    // Zielony
    for (int i = 0; i < 4; ++i) m_torDomowy[kolorIndex(KolorGracza::Zielony)][i]  = gridNaPunkt(13 - i, 7);

    // ========= BAZY (4 x 4) =========
    // Rogi: czerwony (LT), zielony (PT), zolty (LB), niebieski (PB)

    m_baza.resize(4);
    for (int k = 0; k < 4; ++k)
        m_baza[k].resize(4);

    // Czerwony: (1,1)(2,1)(1,2)(2,2)
    m_baza[kolorIndex(KolorGracza::Czerwony)][0] = gridNaPunkt(1,1);
    m_baza[kolorIndex(KolorGracza::Czerwony)][1] = gridNaPunkt(2,1);
    m_baza[kolorIndex(KolorGracza::Czerwony)][2] = gridNaPunkt(1,2);
    m_baza[kolorIndex(KolorGracza::Czerwony)][3] = gridNaPunkt(2,2);

    // Zielony: (12,1)(13,1)(12,2)(13,2)
    m_baza[kolorIndex(KolorGracza::Zielony)][0] = gridNaPunkt(12,1);
    m_baza[kolorIndex(KolorGracza::Zielony)][1] = gridNaPunkt(13,1);
    m_baza[kolorIndex(KolorGracza::Zielony)][2] = gridNaPunkt(12,2);
    m_baza[kolorIndex(KolorGracza::Zielony)][3] = gridNaPunkt(13,2);

    // Zolty: (1,12)(2,12)(1,13)(2,13)
    m_baza[kolorIndex(KolorGracza::Zolty)][0] = gridNaPunkt(1,12);
    m_baza[kolorIndex(KolorGracza::Zolty)][1] = gridNaPunkt(2,12);
    m_baza[kolorIndex(KolorGracza::Zolty)][2] = gridNaPunkt(1,13);
    m_baza[kolorIndex(KolorGracza::Zolty)][3] = gridNaPunkt(2,13);

    // Niebieski: (12,12)(13,12)(12,13)(13,13)
    m_baza[kolorIndex(KolorGracza::Niebieski)][0] = gridNaPunkt(12,12);
    m_baza[kolorIndex(KolorGracza::Niebieski)][1] = gridNaPunkt(13,12);
    m_baza[kolorIndex(KolorGracza::Niebieski)][2] = gridNaPunkt(12,13);
    m_baza[kolorIndex(KolorGracza::Niebieski)][3] = gridNaPunkt(13,13);
}

QPointF Plansza::pozycjaTorGlowny(int absIndex) const
{
    int idx = (absIndex % 52 + 52) % 52;
    return m_torGlowny[idx];
}

QPointF Plansza::pozycjaTorDomowy(KolorGracza kolor, int idx0_5) const
{
    int k = kolorIndex(kolor);
    return m_torDomowy[k][idx0_5];
}

QPointF Plansza::pozycjaBaza(KolorGracza kolor, int id0_3) const
{
    int k = kolorIndex(kolor);
    return m_baza[k][id0_3];
}

QPointF Plansza::pozycjaDlaPionka(const Gracz& gracz, const Pionek& pionek) const
{
    if (pionek.wBazie())
        return pozycjaBaza(pionek.kolor(), pionek.id());

    // tor glowny 0..51
    if (pionek.krok() <= 51)
    {
        int abs = (gracz.indeksStartu() + pionek.krok()) % 52;
        return pozycjaTorGlowny(abs);
    }

    // dom 52..57
    int idx = pionek.krok() - 52; // 0..5
    return pozycjaTorDomowy(pionek.kolor(), idx);
}
