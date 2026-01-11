#include "kosci_window.h"
#include "ui_oknogry.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QRandomGenerator>

static const std::vector<Kategoria> KAT_LISTA = {
    Kategoria::Jedynki, Kategoria::Dwojki, Kategoria::Trojki, Kategoria::Czworki, Kategoria::Piatki, Kategoria::Szostki,
    Kategoria::Trojka, Kategoria::Czworka, Kategoria::Full, Kategoria::MalyStrit, Kategoria::DuzyStrit, Kategoria::Yahtzee, Kategoria::Szansa
};
static const QStringList KAT_NAZWY = {
    "Jedynki", "Dwójki", "Trójki", "Czwórki", "Piątki", "Szóstki",
    "3 Jednakowe", "4 Jednakowe", "Full", "Mały Strit", "Duży Strit", "YAHTZEE", "Szansa"
};

KosciWindow::KosciWindow(const GameLaunchConfig &config, QWidget* parent)
    : QMainWindow(parent), ui(new Ui::OknoGry)
{
    ui->setupUi(this);
    logic = new KosciLogic(this);

    // Wymuszenie stylu dla tabeli (naprawa widoczności w trybie ciemnym)
    ui->tabela->setStyleSheet(
        "QTableWidget { background-color: #f0f0f0; color: black; gridline-color: #aaa; font-size: 14px; }"
        "QHeaderView::section { background-color: #d0d0d0; color: black; padding: 4px; border: 1px solid #aaa; }"
        "QTableWidget::item { padding: 5px; }"
        "QTableWidget::item:selected { background-color: #aaddff; color: black; }"
        );

    // Wymuszenie stylu dla kości
    QString diceStyle = "QToolButton { background-color: white; border: 2px solid #555; border-radius: 10px; }";
    ui->kostka0->setStyleSheet(diceStyle); ui->kostka1->setStyleSheet(diceStyle);
    ui->kostka2->setStyleSheet(diceStyle); ui->kostka3->setStyleSheet(diceStyle);
    ui->kostka4->setStyleSheet(diceStyle);

    if(ui->editIp) ui->editIp->setVisible(false);
    if(ui->editPort) ui->editPort->setVisible(false);
    if(ui->btnHost) ui->btnHost->setVisible(false);
    if(ui->btnPolacz) ui->btnPolacz->setVisible(false);

    QPushButton *btnExit = new QPushButton("Wyjście", this);
    if(ui->topLayout) ui->topLayout->addWidget(btnExit);
    connect(btnExit, &QPushButton::clicked, this, &KosciWindow::onBackToMenu);

    connect(logic, &KosciLogic::zmianaStanu, this, &KosciWindow::odswiez);

    if(config.mode == GameMode::Solo) {
        logic->startBot("Gracz");
    }
    else if(config.mode == GameMode::LocalDuo) {
        logic->startLokalnie("Gracz 1", "Gracz 2");
    }
    else if(config.mode == GameMode::NetHost) {
        logic->startHost("Host");
    }
    else if(config.mode == GameMode::NetClient) {
        logic->startKlient(config.hostIp, "Klient");
    }

    auto setupK = [&](QToolButton* b, int i){
        connect(b, &QToolButton::clicked, [=](){ logic->przelaczBlokade(i); });
    };
    setupK(ui->kostka0,0); setupK(ui->kostka1,1); setupK(ui->kostka2,2); setupK(ui->kostka3,3); setupK(ui->kostka4,4);

    connect(ui->btnRzut, &QPushButton::clicked, [=](){
        logic->rzuc();
        if(logic->czyMojaTura() && logic->rzutNr() < 3) {
            animKroki = 10; animTimer.start(50);
        }
    });
    connect(&animTimer, &QTimer::timeout, this, &KosciWindow::onAnimacja);

    ui->tabela->setRowCount((int)KAT_LISTA.size() + 4);
    connect(ui->tabela, &QTableWidget::cellClicked, [=](int r, int c){
        if(c > 0 && r < (int)KAT_LISTA.size()) logic->wybierz(KAT_LISTA[r]);
    });

    // Inicjalne odświeżenie (ważne!)
    odswiez();
}

KosciWindow::~KosciWindow() { delete ui; }

void KosciWindow::closeEvent(QCloseEvent *event) {
    emit gameClosed();
    QMainWindow::closeEvent(event);
}

void KosciWindow::onBackToMenu() {
    emit gameClosed();
}

void KosciWindow::onAnimacja() {
    animKroki--;
    for(int i=0; i<5; i++) {
        if(!logic->blokady()[i]) ustawKosc(i, QRandomGenerator::global()->bounded(1,7), false);
    }
    if(animKroki <= 0) {
        animTimer.stop();
        odswiez();
    }
}

void KosciWindow::odswiez() {
    if(animTimer.isActive()) return;

    auto k = logic->kosci(); auto b = logic->blokady();
    for(int i=0; i<5; i++) ustawKosc(i, k[i], b[i]);
    ui->labelRzuty->setText("Rzut: " + QString::number(logic->rzutNr()) + "/3");

    const auto& gracze = logic->gracze();
    ui->tabela->setColumnCount((int)gracze.size() + 1);

    QStringList naglowki;
    naglowki << "Kategoria";
    for(const auto& g : gracze) naglowki << g.nazwa;
    ui->tabela->setHorizontalHeaderLabels(naglowki);

    for(int r=0; r<(int)KAT_LISTA.size(); r++) {
        // Kolumna 0: Nazwy kategorii
        auto *headerItem = ui->tabela->item(r, 0);
        if(!headerItem) {
            headerItem = new QTableWidgetItem(KAT_NAZWY[r]);
            ui->tabela->setItem(r, 0, headerItem);
        } else {
            headerItem->setText(KAT_NAZWY[r]);
        }
        headerItem->setFlags(Qt::ItemIsEnabled); // Nieedytowalne
        headerItem->setBackground(QColor(220, 220, 220));

        Kategoria cat = KAT_LISTA[r];

        // Kolumny graczy
        for(int i=0; i<(int)gracze.size(); i++) {
            auto* item = ui->tabela->item(r, i+1);
            if(!item) { item = new QTableWidgetItem(); ui->tabela->setItem(r, i+1, item); }

            item->setTextAlignment(Qt::AlignCenter);
            // Reset stylu dla zwykłych komórek
            item->setBackground(Qt::white);
            item->setForeground(Qt::black);

            if(gracze[i].zajete.value(cat)) {
                item->setText(QString::number(gracze[i].wynik[cat]));
                QFont f = item->font(); f.setBold(true); item->setFont(f);
                item->setBackground(QColor(200, 255, 200)); // Jasnozielony dla zajętych
            } else {
                QFont f = item->font(); f.setBold(false); item->setFont(f);

                // Podgląd wyniku (tylko dla aktywnego gracza)
                if(i == logic->tura() && logic->czyMojaTura() && logic->rzutNr() > 0) {
                    int preview = logic->obliczPunkty(cat, k);
                    item->setText(QString::number(preview));
                    item->setForeground(Qt::gray);
                } else {
                    item->setText("");
                }
            }
        }
    }

    // Wiersze sum
    int rowSum = (int)KAT_LISTA.size();
    QColor kolorTla(255, 250, 205);

    auto setSum = [&](int r, int i, int v, QString txt){
        QTableWidgetItem* item = ui->tabela->item(r, i+1);
        // Header sumy
        if(i == -1) {
            auto* h = ui->tabela->item(r, 0);
            if(!h) { h = new QTableWidgetItem(); ui->tabela->setItem(r, 0, h); }
            h->setText(txt);
            h->setBackground(kolorTla);
            h->setFlags(Qt::ItemIsEnabled);
            return;
        }

        if(!item) { item = new QTableWidgetItem(); ui->tabela->setItem(r, i+1, item); }
        item->setText(QString::number(v));
        item->setBackground(kolorTla);
        item->setForeground(Qt::black);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::ItemIsEnabled);
    };

    for(int i=-1; i<(int)gracze.size(); i++) {
        setSum(rowSum+0, i, (i>=0?gracze[i].sumaGor():0), "Suma Góra");
        setSum(rowSum+1, i, (i>=0?gracze[i].bonus():0), "Bonus (+35)");
        setSum(rowSum+2, i, (i>=0?gracze[i].sumaDol():0), "Suma Dół");
        setSum(rowSum+3, i, (i>=0?gracze[i].total():0), "SUMA ŁĄCZNA");
    }

    if(!gracze.empty()) {
        ui->labelTura->setText("Tura: " + gracze[logic->tura()].nazwa);
        ui->labelTura->setStyleSheet(logic->czyMojaTura() ? "color: #4CAF50; font-weight: bold; font-size: 16px;" : "color: #ccc; font-size: 16px;");
        ui->btnRzut->setEnabled(logic->czyMojaTura() && logic->rzutNr() < 3);
    }
}

void KosciWindow::ustawKosc(int idx, int val, bool blocked) {
    QToolButton* b = nullptr;
    if(idx==0) b=ui->kostka0; else if(idx==1) b=ui->kostka1; else if(idx==2) b=ui->kostka2;
    else if(idx==3) b=ui->kostka3; else if(idx==4) b=ui->kostka4;

    if(b) {
        // Próba ustawienia ikony z zasobów
        QString iconPath = QString(":/kosci/d%1.png").arg(val);
        QIcon icon(iconPath);

        // Zawsze ustawiamy tekst jako fallback
        b->setText(QString::number(val));

        if (!icon.isNull()) {
            b->setIcon(icon);
            b->setIconSize(QSize(64, 64)); // Wymuszenie rozmiaru ikony
        }

        // Stylizacja (Blokada = Zielona ramka)
        if(blocked) {
            b->setStyleSheet("background-color: #dcedc8; border: 3px solid #4CAF50; border-radius: 10px; font-size: 24px; font-weight: bold;");
        } else {
            b->setStyleSheet("background-color: white; border: 2px solid #555; border-radius: 10px; font-size: 24px; font-weight: bold;");
        }
    }
}
