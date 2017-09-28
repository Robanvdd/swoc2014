#include "PlayerBotFolders.h"

#include <QCoreApplication>
#include <QTimer>
#include <iostream>

#include <Engine.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    if ((argc - 1) % 3 != 0)
    {
        std::cout << "Need multiple of three arguments: <player1> <macrobot1> <microbot1> <player2> <macrobot2> <microbot2> ..." << std::endl;
        return -1;
    }

    QList<PlayerBotFolders*> playerBotFolders;
    for (int i = 0; i < (argc - 1) / 3; i++)
    {
        PlayerBotFolders* player = new PlayerBotFolders();
        player->setPlayerName(argv[i*3 + 1]);
        player->setMacroBotFolder(argv[i*3 + 2]);
        player->setMicroBotFolder(argv[i*3 + 3]);
        playerBotFolders.append(player);
    }

    Engine* engine = new Engine(playerBotFolders, &a);

    QObject::connect(engine, &Engine::finished, &a, [&a]() { a.quit(); });
    QObject::connect(engine, &Engine::errorOccured, &a, [&a]() { a.quit(); });
    QObject::connect(engine, &Engine::destroyed, &a, [&a]() { a.quit(); });
    QTimer::singleShot(0, engine, SLOT(startNewMacroGame()));

    return a.exec();
}