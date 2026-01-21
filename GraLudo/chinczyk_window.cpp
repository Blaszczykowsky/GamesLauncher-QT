#include "chinczyk_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QStatusBar>
#include <QPainter>
#include <QJsonArray>

ChinczykWindow::ChinczykWindow(const GameLaunchConfig& config, QWidget* parent)
    : QMainWindow(parent), m_config(config)
{
    zbudujUI();
    podlaczSygnaly();
    initTryb();
}

ChinczykWindow::ChinczykWindow(QWidget* parent)
    : ChinczykWindow(GameLaunchConfig{}, parent)
{
}

void ChinczykWindow::zbudujUI()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);

    m_view = new QGraphicsView(this);
    m_view->setRenderHint(QPainter::Antialiasing, true);

    m_scena = new BoardScene(&m_gra, this);
    m_view->setScene(m_scena);

    m_btnNowa = new QPushButton("Nowa gra", this);
    m_btnRzut = new QPushButton("Rzut kostka", this);
    m_btnStart = new QPushButton("Start gry", this);

    m_comboGracze = new QComboBox(this);
    m_comboGracze->addItem("2 graczy", 2);
    m_comboGracze->addItem("3 graczy", 3);
    m_comboGracze->addItem("4 graczy", 4);
    m_comboGracze->setCurrentIndex(2);

    m_lblRzut = new QLabel("Rzut: -", this);
    m_lblTura = new QLabel("Tura: -", this);

    m_lblSiec = new QLabel("", this);
    m_lblLobby = new QLabel("", this);

    auto* gora = new QHBoxLayout();
    gora->addWidget(m_comboGracze);
    gora->addWidget(m_btnNowa);
    gora->addSpacing(15);
    gora->addWidget(m_btnRzut);
    gora->addSpacing(15);

    gora->addWidget(m_lblTura);
    gora->addWidget(m_lblRzut);

    gora->addSpacing(15);
    gora->addWidget(m_lblSiec);
    gora->addWidget(m_lblLobby);
    gora->addWidget(m_btnStart);

    gora->addStretch(1);

    auto* layout = new QVBoxLayout(central);
    layout->addLayout(gora);
    layout->addWidget(m_view);

    resize(900, 900);
    setWindowTitle("Chinczyk (Ludo) - Qt");
    statusBar()->showMessage("Gotowe.");
}

void ChinczykWindow::podlaczSygnaly()
{
    connect(m_scena, &BoardScene::pionekKlikniety, this, [this](Pionek* p){
        obsluzPionekKlik(p);
    });

    connect(m_btnStart, &QPushButton::clicked, this, [this](){
        if (!m_siecAktywna || !m_jestemHostem) return;
        if (m_graRozpoczeta) return;

        if (!m_serwer.czyPelny())
        {
            QMessageBox::information(this, "Lobby", "Brakuje graczy do startu.");
            return;
        }

        startGrySieciowejHost();
    });

    connect(m_comboGracze, &QComboBox::currentIndexChanged, this, [this](int){
        if (!m_siecAktywna || !m_jestemHostem) return;
        if (m_graRozpoczeta) return;
        int n = m_comboGracze->currentData().toInt();
        m_serwer.ustawDocelowaLiczbeGraczy(n);
    });

    connect(m_btnNowa, &QPushButton::clicked, this, [this](){

        if (m_siecAktywna && !m_jestemHostem)
            return;

        if (m_siecAktywna && m_jestemHostem)
        {
            auto odp = QMessageBox::question(
                this,
                "Nowa gra",
                "Zakoncz obecna gre i rozpocznij nowa?",
                QMessageBox::Yes | QMessageBox::No
                );

            if (odp == QMessageBox::No)
                return;

            rozpocznijNowaGreSieciowaHost();
            return;
        }

        auto czyGraTrwa = [this]() -> bool
        {
            if (m_gra.gracze().isEmpty()) return false;
            if (m_gra.czyRzucono() || m_gra.ostatniRzut() != 0) return true;

            for (const auto& g : m_gra.gracze())
                for (const auto& p : g.pionki())
                    if (!p.wBazie())
                        return true;

            return false;
        };

        if (czyGraTrwa())
        {
            auto odp = QMessageBox::question(
                this,
                "Nowa gra",
                "Zakoncz obecna gre i rozpocznij nowa?",
                QMessageBox::Yes | QMessageBox::No
                );

            if (odp == QMessageBox::No)
                return;
        }

        int n = m_comboGracze->currentData().toInt();

        if (m_scena)
            m_scena->resetujTlo();

        m_gra.nowaGra(n);
    });

    connect(m_btnRzut, &QPushButton::clicked, this, [this](){
        obsluzRzutKlik();
    });

    connect(&m_gra, &Gra::stanZmieniony, this, [this](){

        if (!m_gra.gracze().isEmpty())
            m_lblTura->setText("Tura: " + kolorNaTekst(m_gra.aktualnyGracz().kolor()));
        else
            m_lblTura->setText("Tura: -");

        if (m_gra.czyRzucono())
            m_lblRzut->setText("Rzut: " + QString::number(m_gra.ostatniRzut()));
        else
            m_lblRzut->setText("Rzut: -");

        odswiezUiSieci();
        m_scena->odswiez();

        wyslijStanJesliHost();
    });

    connect(&m_gra, &Gra::komunikat, this, [this](const QString& t){
        statusBar()->showMessage(t, 5000);
    });

    connect(&m_gra, &Gra::koniecGry, this, [this](const QString& zwyciezca){

        if (!m_siecAktywna)
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Information);
            box.setWindowTitle("Koniec gry");
            box.setText("Koniec gry wygral: " + zwyciezca);

            auto* btnNowa = box.addButton("Rozpocznij nowa gre", QMessageBox::AcceptRole);
            QPushButton* btnKont = nullptr;

            if (m_gra.moznaKontynuowacPoWygranej())
                btnKont = box.addButton("Kontynuuj", QMessageBox::RejectRole);

            box.setWindowFlags(box.windowFlags() & ~Qt::WindowCloseButtonHint);
            box.setDefaultButton(btnNowa);
            box.setEscapeButton(btnNowa);

            box.exec();

            if (box.clickedButton() == btnNowa)
            {
                int n = m_comboGracze->currentData().toInt();
                m_scena->resetujTlo();
                m_gra.nowaGra(n);
                return;
            }

            if (btnKont && box.clickedButton() == btnKont)
                m_gra.kontynuujPoWygranej();

            return;
        }
        if (!m_jestemHostem)
            return;

        wyslijGameOverJesliHost(zwyciezca);

        QMessageBox box(this);
        box.setIcon(QMessageBox::Information);
        box.setWindowTitle("Koniec gry");
        box.setText("Koniec gry wygral: " + zwyciezca);

        auto* btnNowa = box.addButton("Rozpocznij nowa gre", QMessageBox::AcceptRole);
        QPushButton* btnKont = nullptr;

        if (m_gra.moznaKontynuowacPoWygranej())
            btnKont = box.addButton("Kontynuuj", QMessageBox::RejectRole);

        box.setWindowFlags(box.windowFlags() & ~Qt::WindowCloseButtonHint);
        box.setDefaultButton(btnNowa);
        box.setEscapeButton(btnNowa);

        box.exec();

        if (box.clickedButton() == btnNowa)
        {
            rozpocznijNowaGreSieciowaHost();
            return;
        }

        if (btnKont && box.clickedButton() == btnKont)
        {
            m_gra.kontynuujPoWygranej();
        }
    });
}

void ChinczykWindow::initTryb()
{
    m_siecAktywna = (m_config.mode == GameMode::NetHost || m_config.mode == GameMode::NetClient);
    m_lblSiec->setText("");
    m_lblLobby->setText("");
    m_btnStart->setVisible(false);

    if (m_siecAktywna)
    {
        initSiec();
        odswiezUiSieci();
        return;
    }

    m_graRozpoczeta = true;

    int n = m_comboGracze->currentData().toInt();
    m_gra.nowaGra(n);
}

void ChinczykWindow::initSiec()
{
    if (m_config.mode == GameMode::NetClient)
        m_comboGracze->setEnabled(false);

    if (m_config.mode == GameMode::NetClient)
        m_btnNowa->setEnabled(false);

    m_btnStart->setVisible(m_config.mode == GameMode::NetHost);

    if (m_config.mode == GameMode::NetHost)
    {
        m_jestemHostem = true;
        m_mojSlot = 0;

        int n = m_comboGracze->currentData().toInt();
        m_totalPlayers = n;
        m_mojKolor = KolorGracza::Czerwony;

        connect(&m_serwer, &ChinczykSerwer::log, this, [this](const QString& s){
            statusBar()->showMessage(s, 6000);
        });

        connect(&m_serwer, &ChinczykSerwer::lobbyZmienione, this, [this](const QJsonObject& lobby){
            ustawLobbyZJson(lobby);
            odswiezUiSieci();
        });

        connect(&m_serwer, &ChinczykSerwer::wiadomoscOdebrana, this, [this](int slot, const QJsonObject& msg){
            obsluzMsgHost(slot, msg);
        });

        m_serwer.start((quint16)m_config.port, n, m_config.playerName);

        m_lblSiec->setText("Siec: Host (Czerwony)");
        statusBar()->showMessage("Host: nasluch na porcie " + QString::number(m_config.port), 6000);
    }
    else
    {
        m_jestemHostem = false;

        connect(&m_klient, &ChinczykKlient::log, this, [this](const QString& s){
            statusBar()->showMessage(s, 6000);
        });

        connect(&m_klient, &ChinczykKlient::wiadomoscOdebrana, this, [this](const QJsonObject& msg){
            obsluzMsgKlient(msg);
        });

        connect(&m_klient, &ChinczykKlient::rozlaczono, this, [this](){
            statusBar()->showMessage("Klient: rozlaczono.", 6000);
        });

        m_lblSiec->setText("Siec: Klient (laczenie...)");
        m_klient.polacz(m_config.hostIp, (quint16)m_config.port, m_config.playerName);
    }
}

void ChinczykWindow::ustawLobbyZJson(const QJsonObject& lobby)
{
    m_lobby = lobby;

    m_totalPlayers = lobby.value("totalPlayers").toInt(m_totalPlayers);

    int pol = 0;
    QJsonArray players = lobby.value("players").toArray();
    for (const auto& v : players)
    {
        QJsonObject p = v.toObject();
        if (p.value("connected").toBool(false))
            pol++;
    }

    m_lblLobby->setText("Polaczeni: " + QString::number(pol) + "/" + QString::number(m_totalPlayers));

    if (m_siecAktywna && m_jestemHostem && !m_graRozpoczeta)
    {
        bool ktosJest = (pol > 1);
        if (ktosJest)
            m_comboGracze->setEnabled(false);
    }

    if (m_siecAktywna && !m_jestemHostem)
    {
        m_comboGracze->setEnabled(false);
    }
}

void ChinczykWindow::odswiezUiSieci()
{
    if (!m_siecAktywna)
    {
        m_btnStart->setVisible(false);
        m_btnNowa->setEnabled(true);
        m_btnRzut->setEnabled(!m_gra.czyRzucono());
        return;
    }

    if (!m_graRozpoczeta)
    {
        m_btnRzut->setEnabled(false);
        m_btnNowa->setEnabled(false);

        if (m_jestemHostem)
        {
            m_btnStart->setVisible(true);
            m_btnStart->setEnabled(lobbyPelne());
        }
        else
        {
            m_btnStart->setVisible(false);
        }
        return;
    }

    if (m_jestemHostem)
    {
        m_btnNowa->setEnabled(true);
    }
    else
    {
        m_btnNowa->setEnabled(false);
    }
    bool moja = czyMojaTura();
    m_btnRzut->setEnabled(moja && !m_gra.czyRzucono() && !m_gra.czyOczekujeNaDecyzje());
}

bool ChinczykWindow::czyMojaTura() const
{
    if (!m_graRozpoczeta) return false;
    if (m_gra.gracze().isEmpty()) return false;
    return m_gra.aktualnyGracz().kolor() == m_mojKolor;
}

KolorGracza ChinczykWindow::kolorDlaSlot(int slot) const
{
    if (m_totalPlayers <= 2)
    {
        if (slot == 0) return KolorGracza::Czerwony;
        return KolorGracza::Niebieski;
    }
    if (m_totalPlayers == 3)
    {
        if (slot == 0) return KolorGracza::Czerwony;
        if (slot == 1) return KolorGracza::Zielony;
        return KolorGracza::Niebieski;
    }
    if (slot == 0) return KolorGracza::Czerwony;
    if (slot == 1) return KolorGracza::Zielony;
    if (slot == 2) return KolorGracza::Niebieski;
    return KolorGracza::Zolty;
}

Pionek* ChinczykWindow::znajdzPionek(KolorGracza kolor, int id)
{
    auto& gracze = m_gra.gracze();      
    for (auto& g : gracze)                 
    {
        if (g.kolor() != kolor) continue;
        if (id < 0 || id >= g.pionki().size()) return nullptr;
        return &g.pionki()[id];            
    }
    return nullptr;
}

void ChinczykWindow::startGrySieciowejHost()
{
    if (!lobbyPelne())
    {
        QMessageBox::information(this, "Lobby", "Brakuje graczy do startu.");
        return;
    }
    m_totalPlayers = m_serwer.docelowaLiczbaGraczy();

    if (m_scena)
        m_scena->resetujTlo();

    m_gra.nowaGra(m_totalPlayers);
    m_graRozpoczeta = true;

    QJsonObject msg;
    msg["t"] = "CH_START";
    msg["totalPlayers"] = m_totalPlayers;
    msg["state"] = m_gra.stanJson();

    m_serwer.wyslijDoWszystkich(msg);

    odswiezUiSieci();
}

void ChinczykWindow::rozpocznijNowaGreSieciowaHost()
{
    if (!m_serwer.czyPelny())
    {
        QMessageBox::information(this, "Lobby", "Brakuje graczy do startu.");
        return;
    }
    if (!m_jestemHostem) return;

    if (!m_graRozpoczeta)
    {
        statusBar()->showMessage("Lobby: uzyj 'Start gry' po dolaczeniu graczy.", 4000);
        return;
    }

    if (m_scena)
        m_scena->resetujTlo();

    m_gra.nowaGra(m_totalPlayers);

    QJsonObject msg;
    msg["t"] = "CH_START";
    msg["totalPlayers"] = m_totalPlayers;
    msg["state"] = m_gra.stanJson();
    m_serwer.wyslijDoWszystkich(msg);
}

void ChinczykWindow::wyslijStanJesliHost()
{
    if (!m_siecAktywna || !m_jestemHostem) return;
    if (!m_graRozpoczeta) return;

    QJsonObject msg;
    msg["t"] = "CH_STATE";
    msg["state"] = m_gra.stanJson();
    m_serwer.wyslijDoWszystkich(msg);
}

void ChinczykWindow::wyslijGameOverJesliHost(const QString& zwyciezca)
{
    if (!m_siecAktywna || !m_jestemHostem) return;

    QJsonObject msg;
    msg["t"] = "CH_GAMEOVER";
    msg["winner"] = zwyciezca;
    msg["canContinue"] = m_gra.moznaKontynuowacPoWygranej();
    m_serwer.wyslijDoWszystkich(msg);
}

void ChinczykWindow::obsluzRzutKlik()
{
    if (!m_siecAktywna)
    {
        m_gra.rzutKostka();
        return;
    }

    if (!m_graRozpoczeta)
    {
        statusBar()->showMessage("Czekaj na start gry.", 3000);
        return;
    }

    if (!czyMojaTura())
    {
        statusBar()->showMessage("Nie twoja tura.", 2500);
        return;
    }

    if (m_gra.czyOczekujeNaDecyzje())
        return;

    if (m_jestemHostem)
    {
        m_gra.rzutKostka();
    }
    else
    {
        QJsonObject msg;
        msg["t"] = "CH_REQ_ROLL";
        msg["slot"] = m_mojSlot;
        m_klient.wyslij(msg);
    }
}

void ChinczykWindow::obsluzPionekKlik(Pionek* p)
{
    if (!p) return;

    if (!m_siecAktywna)
    {
        m_gra.wykonajRuch(p);
        return;
    }

    if (!m_graRozpoczeta)
        return;

    if (!czyMojaTura())
    {
        statusBar()->showMessage("Nie twoja tura.", 2500);
        return;
    }

    if (m_gra.czyOczekujeNaDecyzje())
        return;

    if (m_jestemHostem)
    {
        m_gra.wykonajRuch(p);
    }
    else
    {
        QJsonObject msg;
        msg["t"] = "CH_REQ_MOVE";
        msg["slot"] = m_mojSlot;
        msg["id"] = p->id();
        m_klient.wyslij(msg);
    }
}

void ChinczykWindow::obsluzMsgHost(int slot, const QJsonObject& msg)
{
    QString t = msg.value("t").toString();

    if (!m_graRozpoczeta)
    {
        return;
    }
    KolorGracza kolor = kolorDlaSlot(slot);
    if (m_gra.gracze().isEmpty()) return;
    if (m_gra.aktualnyGracz().kolor() != kolor) return;

    if (t == "CH_REQ_ROLL")
    {
        if (m_gra.czyOczekujeNaDecyzje()) return;
        if (m_gra.czyRzucono()) return;

        m_gra.rzutKostka();
        return;
    }

    if (t == "CH_REQ_MOVE")
    {
        if (m_gra.czyOczekujeNaDecyzje()) return;
        if (!m_gra.czyRzucono()) return;

        int id = msg.value("id").toInt(-1);
        Pionek* p = znajdzPionek(kolor, id);
        if (p)
            m_gra.wykonajRuch(p);

        return;
    }
}

void ChinczykWindow::obsluzMsgKlient(const QJsonObject& msg)
{
    QString t = msg.value("t").toString();

    if (t == "CH_REJECT")
    {
        QString reason = msg.value("reason").toString("Odrzucono polaczenie.");
        QMessageBox::critical(this, "Siec", reason);
        close();
        return;
    }

    if (t == "CH_WELCOME")
    {
        m_mojSlot = msg.value("slot").toInt(-1);
        int k = msg.value("kolor").toInt(0);
        m_totalPlayers = msg.value("totalPlayers").toInt(0);

        m_mojKolor = (KolorGracza)k;

        m_lblSiec->setText("Siec: Klient (" + kolorNaTekst(m_mojKolor) + ")");
        statusBar()->showMessage("Dolaczono do lobby. Slot: " + QString::number(m_mojSlot), 6000);

        QJsonObject lobby;
        lobby["t"] = "CH_LOBBY";
        lobby["totalPlayers"] = m_totalPlayers;
        lobby["players"] = msg.value("players").toArray();
        ustawLobbyZJson(lobby);
        odswiezUiSieci();
        return;
    }

    if (t == "CH_LOBBY")
    {
        ustawLobbyZJson(msg);
        odswiezUiSieci();
        return;
    }

    if (t == "CH_START")
    {
        m_totalPlayers = msg["totalPlayers"].toInt();

        m_comboGracze->setEnabled(false);
        int idx = (m_totalPlayers == 2 ? 0 : (m_totalPlayers == 3 ? 1 : 2));
        m_comboGracze->setCurrentIndex(idx);

        if (m_scena) m_scena->resetujTlo();
        m_gra.nowaGra(m_totalPlayers);
        m_gra.ustawStanJson(msg["state"].toObject());

        m_graRozpoczeta = true;

        m_scena->odswiez();
        odswiezUiSieci();
        return;
    }

    if (t == "CH_STATE")
    {
        if (!m_graRozpoczeta) return;

        QJsonObject st = msg.value("state").toObject();
        if (m_scena) m_scena->resetujTlo();

        m_gra.ustawStanJson(st);
        m_scena->odswiez();
        odswiezUiSieci();
        return;
    }
    if (t == "CH_GAMEOVER")
    {
        QString w = msg.value("winner").toString("?");
        QMessageBox::information(this, "Koniec gry", "Koniec gry wygral: " + w);
        return;
    }
}

void ChinczykWindow::closeEvent(QCloseEvent* e)
{
    if (m_siecAktywna)
    {
        if (m_jestemHostem)
            m_serwer.stop();
        else
            m_klient.rozlacz();
    }

    emit gameClosed();
    QMainWindow::closeEvent(e);
}

int ChinczykWindow::liczbaPolaczonychWLobby() const
{
    if (!m_lobby.contains("players")) return 0;

    int pol = 0;
    QJsonArray players = m_lobby.value("players").toArray();
    for (const auto& v : players)
    {
        QJsonObject p = v.toObject();
        if (p.value("connected").toBool(false))
            pol++;
    }
    return pol;
}

bool ChinczykWindow::lobbyPelne() const
{
    if (m_totalPlayers <= 0) return false;
    return liczbaPolaczonychWLobby() >= m_totalPlayers;
}


