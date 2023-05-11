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
    void addImage(const char *id, std::string &buf);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
};

class OpacityImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString source READ getSource WRITE setSource NOTIFY imageChanged)
    Q_PROPERTY(qreal radius READ getRadius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(qreal border READ getBorder WRITE setBorder NOTIFY borderChanged)
    Q_PROPERTY(QString borderColor READ getBorderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(ResizeMode resizemode READ getResizeMode WRITE setResizeMode NOTIFY resizeModeChanged)
    Q_PROPERTY(QJSValue gradient READ getGradientJSValue WRITE setGradientJSValue NOTIFY gradientChanged)
    Q_PROPERTY(bool xMirror READ getxMirror WRITE setxMirror NOTIFY xMirrorChanged)
    Q_PROPERTY(bool yMirror READ getyMirror WRITE setyMirror NOTIFY yMirrorChanged)

public:
    enum class ResizeMode {
        AutoResize = 0,
        AutoSizeByWidth,
        AutoSizeByHeight,
        Fitting,
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

    bool getMirror() const;
    void setMirror(bool newMirror);

    bool getxMirror() const;
    void setxMirror(bool newXMirror);

    bool getyMirror() const;
    void setyMirror(bool newYMirror);

private:
    void setGradient(const QVariantMap &map);
    qreal m_border;
    QString m_borderColor;
    bool m_xMirror;
    bool m_yMirror;

signals:
    void imageChanged();
    void radiusChanged();
    void resizeModeChanged();
    void gradientChanged();
    void borderChanged();
    void borderColorChanged();
    void mirrorChanged();
    void xMirrorChanged();
    void yMirrorChanged();
};
#endif // __IMAGEPROVIDER_H__
