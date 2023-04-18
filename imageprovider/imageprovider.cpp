#include "imageprovider.h"
#include <QDebug>
static QImage sImage;

ImageProvider::ImageProvider() :
    QQuickImageProvider(QQuickImageProvider::Image) {
}

ImageProvider::~ImageProvider() {
    for (auto p : m_imagesMap) delete p.second;
}

QImage *ImageProvider::image(const QString &id) {
    auto p = m_imagesMap.find(id);
    if (p == m_imagesMap.end()) {
        m_imagesMap[id] = new QImage();
    }
    return m_imagesMap[id];
}

QImage *ImageProvider::image(const char *id) {
    QString imageId = id;
    auto p = m_imagesMap.find(imageId);
    if (p == m_imagesMap.end()) {
        m_imagesMap[imageId] = new QImage();
    }
    return m_imagesMap[imageId];
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
    Q_UNUSED(requestedSize)

    auto p = m_imagesMap.find(id);
    if (p == m_imagesMap.end()) return sImage;

    QImage *image = p->second;
    if (size) { *size = QSize(image->width(), image->height()); }

    return *image;
}
