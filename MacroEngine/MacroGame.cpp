#include "BuyCommand.h"
#include "CommandBase.h"
#include "ConquerCommand.h"
#include "MacroGame.h"
#include "MicroGameInput.h"
#include "MicroGameInputPlayer.h"
#include "MicroGameOutput.h"
#include "MoveToCoordCommand.h"
#include "MoveToPlanetCommand.h"
#include <QFile>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextStream>
#include <iostream>
#include <exception>
#include <QDir>

//#ifdef __linux__
//#define RUN_FILE "/run.sh"
//#endif
//#ifdef __WIN32__
//#define RUN_FILE "\\run.bat"
//#endif
//#ifdef __WIN64__
//#define RUN_FILE "\\run.bat"
//#endif
//#ifdef _WIN32_
//#define RUN_FILE "\\run.bat"
//#endif
//#ifdef _WIN64
//#define RUN_FILE "\\run.bat"
//#endif
#define RUN_FILE "runCommand.txt"

MacroGame::MacroGame(QList<PlayerBotFolders*> playerBotFolders, Universe* universe, QObject *parent)
    : GameObject(parent)
    , m_playerBotFolders(playerBotFolders)
    , m_universe(universe)
    , m_tickTimer(new QTimer(this))
    , m_currentTick(1)
    , m_tickDurationInSeconds(0.25)
{
    m_universe->setParent(this);
    setNameAndLogDir();

    int hue = 0;
    int hueJump = 0;
    if (m_playerBotFolders.size() > 0)
        hueJump = 255 / m_playerBotFolders.size();
    foreach (auto playerBotFolder, m_playerBotFolders)
    {
        auto player = new Player(playerBotFolder->getPlayerName(), hue, this);
        m_ufoShop.giveUfo(player, m_universe);
        m_ufoShop.giveUfo(player, m_universe);
        m_ufoShop.giveUfo(player, m_universe);
        m_universe->addPlayer(player);
        auto runCommandFilename = playerBotFolder->getMacroBotFolder() + "/" + RUN_FILE;
        QFile runCommandFile(runCommandFilename);
        QString command;
        if (runCommandFile.open(QFile::ReadWrite))
        {
            command = runCommandFile.readLine();
        }
        else
        {
            throw std::runtime_error("Can't open bot runCommand.txt");
        }
        auto bot = new MacroBot(command, "", this);
        m_macroBots << bot;
        m_playerBotMap[player] = bot;
        m_botPlayerMap[bot] = player;
        m_playerMicroBotFolder[player] = playerBotFolder->getMicroBotFolder();
        hue += hueJump;
    }

    connect(m_tickTimer, &QTimer::timeout, this, [this]() { handleTick(); });
}

void MacroGame::setNameAndLogDir()
{
    QDir dir;
    auto previousDirs = dir.entryList(QStringList("Match*"), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    auto matchN = 1;
    foreach (auto previousDir, previousDirs)
    {
        previousDir.remove("Match");
        bool sane = false;
        auto previousMatch = previousDir.toInt(&sane);
        if (sane && previousMatch >= matchN)
            matchN = previousMatch + 1;
    }
    m_gameId = matchN;
    m_name = "Match" + QString::number(matchN);

    if (dir.mkpath(m_name + "/MacroTicks"))
    {
        m_tickDir = QDir(m_name + "/MacroTicks");
        m_matchDir = QDir(m_name);
    }

    else
        throw std::runtime_error("Can't create log dir");
}

void MacroGame::run()
{
    startBots();
    m_elapsedTimer.start();
    m_tickTimer->start(static_cast<int>(m_tickDurationInSeconds * 1000));
}

void MacroGame::startBots()
{
    for (auto it = m_macroBots.begin(); it != m_macroBots.end(); it++)
    {
        auto macroBot = *it;
        macroBot->startProcess();
    }
}

void MacroGame::killBots()
{
    std::cerr << "Killing Bots" << std::endl;
    for (auto it = m_macroBots.begin(); it != m_macroBots.end(); it++)
    {
        auto macroBot = *it;
        macroBot->stopProcess();
        std::cerr << "Killed " << m_botPlayerMap[macroBot]->getName().toStdString() << std::endl;
    }
}

void MacroGame::killMicroGames()
{
    std::cerr << "Killing MicroGames" << std::endl;
    for (auto it = m_microGames.begin(); it != m_microGames.end(); it++)
    {
        auto microGame = *it;
        microGame->stopProcess();
    }
}

void MacroGame::stopMacroGame()
{
    std::cerr << "Stopping MacroGame" << std::endl;
    m_tickTimer->stop();
    killBots();
    killMicroGames();

    std::cerr << "Reporting game results" << std::endl;
    std::cout << m_gameId << " " << m_matchDir.absolutePath().toStdString() << " ";
    foreach (auto player, m_universe->getPlayers())
    {
        std::cout << player->getName().toStdString() << " " << player->getCredits() << " ";
    }
    std::cout << std::endl;

    deleteLater();
}

bool MacroGame::gameTimeOver()
{
    return m_elapsedTimer.elapsed() > 600e3;
}

void MacroGame::handleTick()
{
    if (gameTimeOver())
    {
        stopMacroGame();
        return;
    }

    m_universe->applyTick(m_tickDurationInSeconds);

    QJsonDocument gameStateDoc(generateGameState());
    writeGameState(gameStateDoc);
    communicateWithBots(gameStateDoc);

    m_currentTick++;
}

void MacroGame::writeGameState(QJsonDocument gameStateDoc)
{
    auto gameStateJson = gameStateDoc.toJson(QJsonDocument::Indented);
    QFile file(m_tickDir.filePath("tick" + QString::number(m_currentTick) + ".json"));
    if (file.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&file);
        stream << gameStateJson;
    }
}

void MacroGame::communicateWithBot(Player* player, QJsonDocument gameStateDoc)
{
    auto macroBot = m_playerBotMap[player];
    macroBot->sendGameState(gameStateDoc.toJson(QJsonDocument::Compact));

    // Collect all commands
    QStringList commands = macroBot->receiveCommands();
    for (auto commandString : commands)
    {
        // Parse command
        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(commandString.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError)
        {
            auto object = doc.object();
            std::unique_ptr<CommandBase> command = createCommand(object);
            handleCommand(player, command);
        }
    }
}

void MacroGame::handleBuyCommand(Player* player, BuyCommand* buyCommand)
{
    m_ufoShop.buyUfos(player, m_universe->getPlanet(buyCommand->getPlanetId()), m_universe, buyCommand->getAmount());
}


void MacroGame::handleMoveToPlanetCommand(Player* player, MoveToPlanetCommand* moveToPlanetCommand)
{
    if (player == nullptr || moveToPlanetCommand == nullptr)
        throw std::logic_error("Got nullptr player or moveToPlanetCommand");

    auto planetId = moveToPlanetCommand->getPlanetId();
    auto planet = m_universe->getPlanet(planetId);
    if (planet == nullptr)
        return;

    foreach (auto ufoId, moveToPlanetCommand->getUfos())
    {
        if (player->hasUfo(ufoId))
        {
            auto ufo = player->getUfo(ufoId);
            ufo->setFlyingToPlanet(true);
            ufo->setTargetPlanet(planet);
        }
    }
}

void MacroGame::handleMoveToCoordCommand(Player* player, MoveToCoordCommand* moveToCoordCommand)
{
    if (player == nullptr || moveToCoordCommand == nullptr)
        throw std::logic_error("Got nullptr player or moveToCoordCommand");

    foreach (auto ufoId, moveToCoordCommand->getUfos())
    {
        if (player->hasUfo(ufoId))
        {
            auto ufo = player->getUfo(ufoId);
            ufo->setFlyingToCoord(true);
            ufo->setTargetCoord(moveToCoordCommand->getCoords());
        }
    }
}

void MacroGame::handleConquerCommand(Player* player, ConquerCommand* conquerCommand)
{
    auto planet = m_universe->getPlanet(conquerCommand->getPlanetId());
    if (planet == nullptr || player == nullptr)
        return;
    // Planet not yet claimed
    if (planet->getOwnedBy() == -1)
    {
        planet->takeOverBy(player);
        return;
    }

    // Prepare fight
    Player* currentOwner = m_universe->getPlayers().value(planet->getOwnedBy(), nullptr);
    if (currentOwner == nullptr)
    {
        throw std::logic_error("Planet is owned, but not by an existing player");
    }

    if (currentOwner == player)
        return;

    SolarSystem* solarSystem = m_universe->getCorrespondingSolarSystem(planet);
    QPointF location = solarSystem->getPlanetLocation(*planet);
    QList<Ufo*> nearbyUfosPlayer = m_universe->getUfosNearLocation(location, *player);
    QList<Ufo*> nearbyUfosCurrentOwner = m_universe->getUfosNearLocation(location, *currentOwner);
    if (nearbyUfosPlayer.size() == 0)
        return;
    if (nearbyUfosCurrentOwner.size() == 0)
    {
        planet->takeOverBy(player);
        return;
    }
    startMicroGame(player, location, planet);
}

void MacroGame::startMicroGame(Player* player, const QPointF& location, Planet* planet)
{
    if (m_universe->getUfosNearLocation(location, *player).size() <= 0)
        return;

    QMap<Player*, QList<Ufo*>> playerUfos;
    QList<MicroGameInputPlayer> microGameInputs;
    foreach (auto player, m_universe->getPlayers())
    {
        QList<Ufo*> nearbyUfos = m_universe->getUfosNearLocation(location, *player);
        if (nearbyUfos.size() > 0)
            microGameInputs.append(MicroGameInputPlayer(player, nearbyUfos, m_playerMicroBotFolder[player]));
    }
    if (microGameInputs.size() >= 2)
        startMicroGame(MicroGameInput(microGameInputs), planet);
}

void MacroGame::startMicroGame(const MicroGameInput& input, Planet* planet)
{
    std::cerr << "Starting MicroGame" << std::endl;

    foreach (auto playerInput, input.m_microGameInputPlayers)
    {
        foreach (auto ufo, playerInput.ufos)
        {
            ufo->setInFight(true);
        }
    }

    static int nextMicroGame = 0;
    QDir microLogFolder(m_tickDir.filePath("../MicroGame" + QString::number(++nextMicroGame)));
    m_tickDir.mkpath(microLogFolder.absolutePath());
    MicroGame* microGame = new MicroGame("java -jar micro.jar", input, microLogFolder.absolutePath());
    microGame->startProcess();

    QObject::connect(microGame, &MicroGame::dataAvailable, this, [this, microGame, planet]() {
        if (microGame->canReadLine())
        {
            auto result = microGame->readLine();

            std::cerr << result.toStdString() << std::endl;

            MicroGameOutput parsedOutput;
            parsedOutput.readOutput(result);
            if (parsedOutput.getGameId() < 0)
            {
                throw std::runtime_error("Parsing microgame output went horribly wrong.");
            }

            if (parsedOutput.getWinner() > 0)
                planet->takeOverBy(m_universe->getPlayers()[parsedOutput.getWinner()]);
            foreach (auto playerOutput, parsedOutput.getPlayers())
            {
                auto player = m_universe->getPlayers()[playerOutput.getId()];
                foreach (auto ufoId, playerOutput.getCasualties())
                {
                    player->removeUfo(player->getUfo(ufoId));
                }
                foreach (auto ufoId, playerOutput.getSurvivors())
                {
                    player->getUfo(ufoId)->setInFight(false);
                }
            }

            QFile file("microOutput_" + QString::number(microGame->getId()) + ".json");
            if (file.open(QIODevice::ReadWrite))
            {
                QTextStream stream(&file);
                stream << result;
            }

            microGame->stopProcess();
            m_microGames.removeAll(microGame);
            microGame->deleteLater();
        }
    });

    m_microGames << microGame;
}

void MacroGame::handleCommand(Player* player, std::unique_ptr<CommandBase>& command)
{
    command->printCommand();
    BuyCommand* buyCommand = dynamic_cast<BuyCommand*>(command.get());
    if (buyCommand)
    {
        handleBuyCommand(player, buyCommand);
    }
    ConquerCommand* conquerCommand = dynamic_cast<ConquerCommand*>(command.get());
    if (conquerCommand)
    {
        handleConquerCommand(player, conquerCommand);
    }
    MoveToPlanetCommand* moveToPlanetCommand = dynamic_cast<MoveToPlanetCommand*>(command.get());
    if (moveToPlanetCommand)
    {
        handleMoveToPlanetCommand(player, moveToPlanetCommand);
    }
    MoveToCoordCommand* moveToCoordCommand = dynamic_cast<MoveToCoordCommand*>(command.get());
    if (moveToCoordCommand)
    {
        handleMoveToCoordCommand(player, moveToCoordCommand);
    }
}

void MacroGame::communicateWithBots(QJsonDocument gameStateDoc)
{
    for (auto player : m_universe->getPlayers())
    {
        communicateWithBot(player, gameStateDoc);
    }
}

QJsonObject MacroGame::generateGameState()
{
    QJsonObject gameState;
    gameState["id"] = m_id;
    gameState["name"] = m_name;
    gameState["tick"] = m_currentTick;
    m_universe->writeState(gameState);
    return gameState;
}

std::unique_ptr<CommandBase> MacroGame::createCommand(const QJsonObject object)
{
    auto commandString = object["Command"].toString();
    std::unique_ptr<CommandBase> command;
    if (commandString == "moveToPlanet")
    {
        command = std::make_unique<MoveToPlanetCommand>();
    }
    else if (commandString == "moveToCoord")
    {
        command = std::make_unique<MoveToCoordCommand>();
    }
    else if (commandString == "buy")
    {
        command = std::make_unique<BuyCommand>();
    }
    else if (commandString == "conquer")
    {
        command = std::make_unique<ConquerCommand>();
    }
    else {
        throw std::exception();
    }
    command->readCommand(object);
    return command;
}
