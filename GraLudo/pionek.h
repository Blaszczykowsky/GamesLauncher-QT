#pragma once
#include <QString>

enum class KolorGracza { Czerwony = 0, Zielony = 1, Niebieski = 2, Zolty = 3 };

QString kolorNaTekst(KolorGracza k);

class Pionek
{
public:
    Pionek(KolorGracza kolor, int id);

    KolorGracza kolor() const { return m_kolor; }
    int id() const { return m_id; }

    int krok() const { return m_krok; }
    void ustawKrok(int nowyKrok) { m_krok = nowyKrok; }

    bool wBazie() const { return m_krok < 0; }
    bool naTorzeGlownym() const { return m_krok >= 0 && m_krok <= 51; }
    bool wDomu() const { return m_krok >= 52 && m_krok <= 55; }
    bool meta() const { return m_krok == 55; }

private:
    KolorGracza m_kolor;
    int m_id;
    int m_krok; 
};

