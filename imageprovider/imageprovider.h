/**
 * @file imageprovider.h
 * @author greatboxs (greatboxs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-05-5
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __IMAGEPROVIDER_H__
#define __IMAGEPROVIDER_H__

#include <QQuickImageProvider>
#include <QQuickPaintedItem>
#include <QGradient>
#include <QImage>
#include <map>

/**
 * @fn ImageProvider
 * @brief
 *
 */
class ImageProvider : public QQuickImageProvider
{
    Q_OBJECT
    std::map<QString, QImage *> m_imagesMap;

private:
    static ImageProvider *m_instance;
    static int m_state;
    ImageProvider();
    ~ImageProvider();

    ImageProvider(const ImageProvider &) = delete;
    ImageProvider(const ImageProvider &&) = delete;
    ImageProvider &operator=(const ImageProvider &) = delete;

public:
    static ImageProvider *instance();
    void destroy();
    QImage *image(const QString &id);
    QImage *image(const char *id);

    /**
     * @fn getImage
     * @brief Get the Image object
     *
     * @param id    Image id (this value is mapping with "source" in qml)
     * @return QImage
     */
    QImage getImage(const QString &id);

    /**
     * @fn updateImage
     * @brief Add if not exists or update the current image in provider resource
     *
     * @param id    Image id (this value is mapping with "source" in qml)
     * @param buf   Image binary data
     * @param size  Image size
     */
    void updateImage(const char *id, const char *buf, size_t size);

    /**
     * @fn updateImage
     * @brief
     *
     * @param id    Image id (this value is mapping with "source" in qml)
     * @param path  Image file path (note: "qrc:/" prefix is replaced with ":/")
     */
    void updateImage(const char *id, const QString &path);

    /**
     * @fn updateImage
     * @brief
     *
     * @param id    Image id (this value is mapping with "source" in qml)
     * @param img   Image object to copy from
     */
    void updateImage(const char *id, const QImage &img);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

signals:
    void imageChanged(const QString id);
};

/**
 * @fn OpacityImage
 * @brief A simple opacity mask image using QQuickPaintedItem to handle the painting job.
 * Users can register OpacityImage to the QML engine and use it in a QML file.
 * The image is provided by the ImageProvider or from a resource URL.
 * Users have to set the "source" (works with ImageProvider) or "url" in the QML file.
 * Currently, OpacityImage supports LinearGradient (Gradient), RadialGradient, and ConicalGradient.
 */
class OpacityImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString source READ getSource WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QString url READ getURL WRITE setURL NOTIFY urlChanged)
    Q_PROPERTY(qreal radius READ getRadius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(qreal border READ getBorder WRITE setBorder NOTIFY borderChanged)
    Q_PROPERTY(QString borderColor READ getBorderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(ResizeMode resizemode READ getResizeMode WRITE setResizeMode NOTIFY resizeModeChanged)
    Q_PROPERTY(QJSValue gradient READ getGradientJSValue WRITE setGradientJSValue NOTIFY gradientJSValueChanged)
    Q_PROPERTY(bool xMirror READ getxMirror WRITE setxMirror NOTIFY xMirrorChanged)
    Q_PROPERTY(bool yMirror READ getyMirror WRITE setyMirror NOTIFY yMirrorChanged)

public:
    enum class ResizeMode {
        Scaled = 0,
        Fit,
        Fixed,
    };
    Q_ENUM(ResizeMode)

private:
    QString m_source;
    QImage m_image;
    qreal m_radius;
    ResizeMode m_resizemode;
    QJSValue m_gradientJsValue;
    QVariantMap m_gradientObject;
    QGradient *m_gradient;
    QLinearGradient m_linearGradient;
    QRadialGradient m_radialGradient;
    QConicalGradient m_conicalGradient;

public:
    OpacityImage();
    ~OpacityImage();
    virtual void paint(QPainter *painter) override;

    QString getSource() const { return m_source; }
    void setSource(const QString image);

    qreal getRadius() const { return m_radius; }
    void setRadius(const qreal &newRadius);

    ResizeMode getResizeMode() const { return m_resizemode; }
    void setResizeMode(ResizeMode newResizemode);

    QJSValue getGradientJSValue() const { return m_gradientJsValue; }
    void setGradientJSValue(QJSValue &value);

    qreal getBorder() const;
    void setBorder(qreal newBorder);

    QString getBorderColor() const;
    void setBorderColor(const QString &newBorderColor);

    bool getxMirror() const;
    void setxMirror(bool newXMirror);

    bool getyMirror() const;
    void setyMirror(bool newYMirror);

    QString getURL() const;
    void setURL(const QString &newUrl);

private:
    void setGradient(const QVariantMap &map);
    qreal m_border;
    QString m_borderColor;
    bool m_xMirror;
    bool m_yMirror;
    QString m_url;

signals:
    void sourceChanged();
    void radiusChanged();
    void resizeModeChanged();
    void gradientJSValueChanged();
    void borderChanged();
    void borderColorChanged();
    void mirrorChanged();
    void xMirrorChanged();
    void yMirrorChanged();
    void urlChanged();
};
#endif // __IMAGEPROVIDER_H__
