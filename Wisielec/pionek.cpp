#include "pionek.h"

QString kolorNaTekst(KolorGracza k)
{
    switch (k)
    {
    case KolorGracza::Czerwony: return "Czerwony";
    case KolorGracza::Zielony: return "Zielony";
    case KolorGracza::Niebieski: return "Niebieski";
    case KolorGracza::Zolty: return "Zolty";
    }
    return "Nieznany";
}

Pionek::Pionek(KolorGracza kolor, int id)
    : m_kolor(kolor), m_id(id), m_krok(-1)
{
}
