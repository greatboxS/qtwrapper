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
#include <QQmlContext>
#include <QObject>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    auto imageProvider = ImageProvider::instance();
    // read file data into buffer
    size_t size = 1024 * 100;
    char *buffer = new char[size];

    int fd = open("ai-image-enlarger-1-after-2.jpg", O_RDONLY);
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

    QByteArray arr(buffer, bytes_read);
    QString imageStr = "data:image/;base64," + arr.toBase64();
    // qDebug() << imageStr;

    std::string buf(buffer, bytes_read);
    imageProvider->addImage("image1", buf);

    qmlRegisterType<OpacityImage>("opacityimage", 1, 0, "OpacityImage");

    QQmlApplicationEngine engine;
    engine.addImageProvider("imageProvider", imageProvider);

    auto root = engine.rootContext();

    const QUrl url(u"qrc:/imageprovider/main.qml"_qs);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
