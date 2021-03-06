#include "MicroGame.h"

#include <QJsonDocument>
#include <QDebug>
#include <QDir>

#include <iostream>

MicroGame::MicroGame(QString executablePathMicroEngine,
                     MicroGameInput input,
                     QString tickFolder,
                     QObject *parent)
    : GameObject(parent)
    , m_executablePathMicroEngine(executablePathMicroEngine)
    , m_input(input)
    , m_process(new QProcess(this))
    , m_dataAvailable(false)
    , m_tickFolder(tickFolder)
{
}

void MicroGame::startProcess()
{
    m_process->setReadChannel(QProcess::StandardOutput);

    QObject::connect(m_process, &QProcess::readyReadStandardOutput, this, [this]()
    {
        m_dataAvailable = true;
        emit dataAvailable();
    });

    QObject::connect(m_process, &QProcess::readyReadStandardError, this, [this]()
    {
        std::cerr << m_process->readAllStandardError().toStdString();
    });

    QObject::connect(m_process, &QProcess::stateChanged, this, [this]()
    {
        if (m_process->state() == QProcess::Running)
        {
            QJsonObject jsonInput;
            jsonInput["gameId"] = m_id;
            jsonInput["ticks"] = m_tickFolder + "/";
            m_input.writePlayerJson(jsonInput);
            QJsonDocument doc(jsonInput);
            m_process->write(doc.toJson(QJsonDocument::Compact) + "\n");
            std::cerr << doc.toJson(QJsonDocument::Compact).toStdString() + "\n";
        }
    });

    QObject::connect(m_process, &QProcess::errorOccurred, this, [this]()
    {
        std::cerr << "Error occured " << m_process->errorString().toStdString();
    });

    m_process->start(m_executablePathMicroEngine);

    m_process->waitForStarted(1000);

    if (m_process->state() != QProcess::Running)
    {
        std::cerr << m_process->errorString().toStdString();
        throw std::runtime_error("Could not start micro.jar");
    }
    else
    {
        std::cerr << "Succesfully started micro.jar";
    }
}

void MicroGame::stopProcess()
{
    m_process->disconnect();
    m_process->kill();
    m_process->waitForFinished(500);
}

bool MicroGame::running()
{
    return m_process->state() == QProcess::Running;
}

bool MicroGame::getFinished() const
{
    return m_dataAvailable;
}

bool MicroGame::canReadLine() const
{
    return m_process->canReadLine();
}

QString MicroGame::readLine() const
{
    if (m_process->canReadLine())
    {
        return QString::fromLocal8Bit(m_process->readLine());
    }
    return QString();
}
