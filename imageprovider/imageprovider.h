#ifndef __IMAGEPROVIDER_H__
#define __IMAGEPROVIDER_H__

#include <QQuickImageProvider>
#include <QImage>
#include <map>

class ImageProvider : public QQuickImageProvider
{
    std::map<QString, QImage *> m_imagesMap;

public:
    ImageProvider();
    ~ImageProvider();

    QImage *image(const QString &id);
    QImage *image(const char *id);
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
};
#endif // __IMAGEPROVIDER_H__
