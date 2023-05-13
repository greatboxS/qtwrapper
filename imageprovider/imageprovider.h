#ifndef __IMAGEPROVIDER_H__
#define __IMAGEPROVIDER_H__

#include <QQuickImageProvider>
#include <QQuickPaintedItem>
#include <QGradient>
#include <QImage>
#include <map>
#include <memory>

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
    QImage getImage(const QString &id);
    void updateImage(const char *id, const char *buf, size_t size);
    void updateImage(const char *id, const QString path);
    void updateImage(const char *id, const QImage &img);
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

signals:
    void imageChanged(const QString id);
};

class OpacityImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString source READ getSource WRITE setSource NOTIFY sourceChanged)
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
    int m_updateCount;

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

public slots:
    void imageChanged(const QString id);

private:
    void setGradient(const QVariantMap &map);
    qreal m_border;
    QString m_borderColor;
    bool m_xMirror;
    bool m_yMirror;

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
};
#endif // __IMAGEPROVIDER_H__
