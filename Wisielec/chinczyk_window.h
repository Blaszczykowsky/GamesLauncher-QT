#pragma once
#include <QMainWindow>
#include <QGraphicsView>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QCloseEvent>
#include <QJsonObject>

#include "gra.h"
#include "boardscene.h"
#include "game_config.h"
#include "chinczyk_network.h"

class ChinczykWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit ChinczykWindow(QWidget* parent = nullptr);
    explicit ChinczykWindow(const GameLaunchConfig& config, QWidget* parent = nullptr);

signals:
    void gameClosed();

protected:
    void closeEvent(QCloseEvent* e) override;

private:
    void zbudujUI();
    void podlaczSygnaly();

    void initTryb();     // lokalny/solo/siec
    void initSiec();     // host/klient

    void startGrySieciowejHost();
    void rozpocznijNowaGreSieciowaHost();

    void ustawLobbyZJson(const QJsonObject& lobby);
    void odswiezUiSieci();

    bool czyMojaTura() const;
    KolorGracza kolorDlaSlot(int slot) const;
    Pionek* znajdzPionek(KolorGracza kolor, int id);

    void obsluzRzutKlik();
    void obsluzPionekKlik(Pionek* p);

    void obsluzMsgHost(int slot, const QJsonObject& msg);
    void obsluzMsgKlient(const QJsonObject& msg);

    void wyslijStanJesliHost();
    void wyslijGameOverJesliHost(const QString& zwyciezca);

    bool lobbyPelne() const;
    int liczbaPolaczonychWLobby() const;

private:
    GameLaunchConfig m_config;

    // --- gra ---
    Gra m_gra;
    BoardScene* m_scena = nullptr;

    // --- UI ---
    QGraphicsView* m_view = nullptr;
    QPushButton* m_btnNowa = nullptr;
    QPushButton* m_btnRzut = nullptr;
    QPushButton* m_btnStart = nullptr;

    QComboBox* m_comboGracze = nullptr;
    QLabel* m_lblRzut = nullptr;
    QLabel* m_lblTura = nullptr;

    QLabel* m_lblSiec = nullptr;
    QLabel* m_lblLobby = nullptr;

    // --- siec ---
    bool m_siecAktywna = false;
    bool m_jestemHostem = false;
    bool m_graRozpoczeta = false;

    int m_mojSlot = -1;          // klient dostaje z welcome
    int m_totalPlayers = 0;      // lobby/ustawienie hosta
    KolorGracza m_mojKolor = KolorGracza::Czerwony;

    int m_ostatniGid = -1;

    ChinczykSerwer m_serwer;
    ChinczykKlient m_klient;

    // ostatnie lobby (do wyswietlenia)
    QJsonObject m_lobby;
};
