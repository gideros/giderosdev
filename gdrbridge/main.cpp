#include <QCoreApplication>
#include <gemitter.h>
#include <gplayerbridge.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    GEmitter emitter;
    GPlayerBridge bridge;

    QObject::connect(&bridge, SIGNAL(quit()), &a, SLOT(quit()), Qt::QueuedConnection);
    QObject::connect(&emitter, SIGNAL(play(const QString&)), &bridge, SLOT(play(const QString&)), Qt::QueuedConnection);

    emitter.emitPlay("C:/myprojects/gideros/samplecode/Graphics/Bird Animation/Bird Animation.gproj");

    return a.exec();
}
