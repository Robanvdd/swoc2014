#ifndef PLAYERMODEL_H
#define PLAYERMODEL_H

#include <QAbstractListModel>
#include "player.h"

class PlayersModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int length READ rowCount NOTIFY lengthChanged)
public:
    enum PlayerRoles {
        NameRole = Qt::UserRole + 1,
        CreditsRole,
        UfosRole,
        HueRole,
        ColorRole,
        IncomeRole,
        PlanetsOwnedRole,
    };
    explicit PlayersModel(QObject *parent = nullptr);

    void CreatePlayer(int playerId);
    void Clear();
    bool PlayerExists(int playerId) const;
    Player* GetPlayer(int playerId) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;
    QHash<int, QByteArray> roleNames() const;

signals:
    void lengthChanged();

private:
    QList<Player*> m_players;
};

#endif // PLAYERMODEL_H
