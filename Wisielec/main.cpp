#include <QApplication>
#include "launcher.h"
#include "wisielec_window.h"
#include "kosci_window.h"
#include "chinczyk_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Launcher launcher;
    QMainWindow *currentGame = nullptr;

    QObject::connect(&launcher, &Launcher::launchGame, [&](const GameLaunchConfig &config){
        if(currentGame) currentGame->deleteLater();

        if(config.gameType == GameType::Wisielec) {
            auto *w = new WisielecWindow(config);
            QObject::connect(w, &WisielecWindow::gameClosed, [&](){
                if(currentGame) { currentGame->close(); currentGame->deleteLater(); currentGame = nullptr; }
                launcher.show();
            });
            currentGame = w;
        }
        else if(config.gameType == GameType::Kosci) {
            auto *k = new KosciWindow(config);
            QObject::connect(k, &KosciWindow::gameClosed, [&](){
                if(currentGame) { currentGame->close(); currentGame->deleteLater(); currentGame = nullptr; }
                launcher.show();
            });
            currentGame = k;
        }
        else if(config.gameType == GameType::Chinczyk) {
            auto *c = new ChinczykWindow(config);
            QObject::connect(c, &ChinczykWindow::gameClosed, [&](){
                if(currentGame) { currentGame->close(); currentGame->deleteLater(); currentGame = nullptr; }
                launcher.show();
            });
            currentGame = c;
        }

        if(currentGame) {
            launcher.hide();
            currentGame->show();
        }
    });

    launcher.show();
    return app.exec();
}
