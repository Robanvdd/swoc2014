#ifndef PLAYER_H
#define PLAYER_H

#include "gameobject.h"
#include "ufo.h"

#include <QColor>

class Player : public GameObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name NOTIFY nameChanged)
    Q_PROPERTY(int credits MEMBER m_credits NOTIFY creditsChanged)
    Q_PROPERTY(QList<QObject*> ufos MEMBER m_ufos NOTIFY ufosChanged)
    Q_PROPERTY(double hue MEMBER m_hue NOTIFY hueChanged)
    Q_PROPERTY(QColor color READ getColor WRITE setColor NOTIFY colorChanged)
public:
    explicit Player(QObject *parent = nullptr);
    explicit Player(int playerId, QObject* parent = nullptr);
    ~Player();

    Q_INVOKABLE bool ufoExists(int ufoId) const;
    Q_INVOKABLE void createUfo(int ufoId);
    Q_INVOKABLE Ufo* getUfo(int ufoId) const;
    Q_INVOKABLE void destroyUfo(int ufoId);

    double getHue() const;

    QColor getColor() const;
    void setColor(const QColor& color);

    QString getName() const;

signals:
    void nameChanged();
    void creditsChanged();
    void ufosChanged();
    void hueChanged();
    void colorChanged();

public slots:
private:
    QString m_name;
    int m_credits;
    double m_hue;
    QColor m_color;
    QList<QObject*> m_ufos;
};

#endif // PLAYER_H
