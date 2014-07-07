#include "gemitter.h"

GEmitter::GEmitter(QObject *parent) :
    QObject(parent)
{
}

void GEmitter::emitPlay(const QString &fileName)
{
    emit play(fileName);
}
