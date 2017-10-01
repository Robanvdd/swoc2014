#ifndef SOLARSYSTEM_H
#define SOLARSYSTEM_H

#include "Planet.h"
#include "Ufo.h"

#include <QMap>
#include <QObject>
#include <QPoint>

class SolarSystem : public GameObject
{
    Q_OBJECT
public:
    explicit SolarSystem(QString name, QPoint coord, QMap<int, Planet*> planets, QObject *parent = nullptr);
    void writeState(QJsonObject& gameState) const;
    void applyTick(double durationInSeconds);
    QMap<int, Planet*> getPlanets();
    QPointF getPlanetLocation(const Planet&planet) const;
    QList<Ufo*> getUfosNearLocation(const QPointF& location, const Player& player);

signals:

public slots:
private:
    QString m_name;
    QPoint m_coord;
    QMap<int, Planet*> m_planets;
};

#endif // SOLARSYSTEM_H
