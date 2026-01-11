#include "gracz.h"

Gracz::Gracz(KolorGracza kolor, int indeksStartu, int liczbaPionkow)
    : m_kolor(kolor), m_indeksStartu(indeksStartu)
{
    m_pionki.reserve(liczbaPionkow);
    for (int i = 0; i < liczbaPionkow; ++i)
        m_pionki.push_back(Pionek(kolor, i));
}

bool Gracz::wygral() const
{
    for (const auto& p : m_pionki)
        if (!p.wDomu())
            return false;
    return true;
}
