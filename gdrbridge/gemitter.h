#ifndef GEMITTER_H
#define GEMITTER_H

#include <QObject>

class GEmitter : public QObject
{
    Q_OBJECT
public:
    explicit GEmitter(QObject *parent = 0);

    void emitPlay(const QString &fileName);

signals:
    void play(const QString &fileName);

public slots:

};

#endif // GEMITTER_H
