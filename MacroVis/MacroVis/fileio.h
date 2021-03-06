#ifndef FILEIO_H
#define FILEIO_H

#include <QObject>
#include <QUrl>

class FileIO : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
public:
    explicit FileIO(QObject *parent = 0);

    QUrl source() const;
    void setSource(const QUrl &source);

    Q_INVOKABLE QString read();
    Q_INVOKABLE bool write(const QString& data) const;
    Q_INVOKABLE bool fileExists(const QUrl &fileUrl) const;

signals:
    void sourceChanged();
    void error(const QString& msg);
public slots:

private:
    QUrl m_source;
};

#endif // FILEIO_H
