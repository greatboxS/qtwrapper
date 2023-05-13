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
    size_t size = 1024 * 1000;
    char *buffer = new char[size];

    const char *images[] = {"image1.jpg", "image2.jpg", "image3.jpg", "image4.jpg"};

    for (int i = 0; i < 4; i++) {
        qDebug() << "open Image " << images[i];
        int fd = open(images[i], O_RDONLY);
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
        close(fd);

        QByteArray arr(buffer, bytes_read);
        QString imageStr = "data:image/;base64," + arr.toBase64();
        std::string buf(buffer, bytes_read);
        imageProvider->updateImage(images[i], buf.data(), buf.size());
    }

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
