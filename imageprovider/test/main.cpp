#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "../imageprovider.h"
#include <QDebug>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    ImageProvider provider;
    auto image = provider.image("image1");

    // read file data into buffer
    size_t size = 1024 * 100;
    char* buffer = new char[size];

    int fd = open("icon.png", O_RDONLY);
    if (fd == -1) {
       perror("open");
       exit(EXIT_FAILURE);
    }

    int bytes_read = read(fd, buffer, size);
    if (bytes_read == -1) {
       perror("read");
       exit(EXIT_FAILURE);
    }

    qDebug() << "read bytes" << (int)bytes_read;

    image->loadFromData((const uchar *)buffer, bytes_read);

    QQmlApplicationEngine engine;
    engine.addImageProvider("imageProvider", &provider);

    const QUrl url(u"qrc:/imageprovider/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
