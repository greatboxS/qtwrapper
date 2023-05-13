#include "imageprovider.h"
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QJsonValue>
#include <QJSValueIterator>
#include <QReadWriteLock>

#define GRADIENT_TYPE_TEXT     "type"
#define GRADIENT_STOPS_TEXT    "stops"
#define GRADIENT_POSITION_TEXT "position"
#define GRADIENT_COLOR_TEXT    "color"
#define INITIAL_UPDATE_TRY     100000

#define js_parse_number(value, obj, name)              \
    if (value.property(#name).isNumber()) {            \
        obj[#name] = value.property(#name).toNumber(); \
    }

#define js_parse_string(value, obj, name)              \
    if (value.property(#name).isString()) {            \
        obj[#name] = value.property(#name).toString(); \
    }

static QReadWriteLock sImageProviderLock;
ImageProvider *ImageProvider::m_instance = NULL;
int ImageProvider::m_state = -1;

ImageProvider::ImageProvider() :
    QQuickImageProvider(QQuickImageProvider::Image) {
}

ImageProvider::~ImageProvider() {
    for (auto p : m_imagesMap) delete p.second;
}

ImageProvider *ImageProvider::instance() {
    if (m_state == -1) {
        m_instance = new ImageProvider();
        m_state++;
    }
    return m_instance;
}

void ImageProvider::destroy() {
    if (m_state == 0) {
        delete ImageProvider::m_instance;
        ImageProvider::m_instance = NULL;
        m_state = -1;
    }
}

QImage *ImageProvider::image(const QString &id) {
    auto p = m_imagesMap.find(id);
    if (p == m_imagesMap.end()) return NULL;
    return m_imagesMap[id];
}

QImage *ImageProvider::image(const char *id) {
    QString imageId = id;
    auto p = m_imagesMap.find(imageId);
    if (p == m_imagesMap.end()) return NULL;
    return m_imagesMap[imageId];
}

QImage ImageProvider::getImage(const QString &id) {
    QReadLocker locker(&sImageProviderLock);
    auto p = m_imagesMap.find(id);
    if (p == m_imagesMap.end()) return QImage();
    return *m_imagesMap[id];
}

void ImageProvider::updateImage(const char *id, const char *buf, size_t size) {
    QWriteLocker locker(&sImageProviderLock);
    auto img = this->image(id);
    if (img == NULL) {
        m_imagesMap[id] = new QImage();
        img = m_imagesMap[id];
    }
    img->loadFromData((const uchar *)buf, size);
    emit imageChanged(id);
}

void ImageProvider::updateImage(const char *id, const QString path) {
    QWriteLocker locker(&sImageProviderLock);
    auto img = this->image(id);
    if (img == NULL) {
        m_imagesMap[id] = new QImage();
        img = m_imagesMap[id];
    }
    if (img->load(path)) {
        emit imageChanged(id);
    }
}

void ImageProvider::updateImage(const char *id, const QImage &image) {
    QWriteLocker locker(&sImageProviderLock);
    auto img = this->image(id);
    if (img == NULL) {
        m_imagesMap[id] = new QImage();
        img = m_imagesMap[id];
    }
    *img = image;
    emit imageChanged(id);
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
    Q_UNUSED(requestedSize)
    QReadLocker locker(&sImageProviderLock);

    auto p = m_imagesMap.find(id);
    if (p == m_imagesMap.end()) return QImage();

    QImage *image = p->second;
    if (size) { *size = QSize(image->width(), image->height()); }

    return *image;
}

OpacityImage::OpacityImage() :
    m_source(""),
    m_image(NULL),
    m_radius(0),
    m_resizemode(ResizeMode::Fit),
    m_gradient(NULL),
    m_border(0.0),
    m_borderColor("black"),
    m_xMirror(false),
    m_yMirror(false),
    m_updateCount(INITIAL_UPDATE_TRY) {

    auto provider = ImageProvider::instance();
    QObject::connect(provider, &ImageProvider::imageChanged, this, &OpacityImage::imageChanged, Qt::QueuedConnection);
}
OpacityImage::~OpacityImage() {
}

void OpacityImage::paint(QPainter *painter) {
    QPainterPath borderPath;
    auto imageProvider = ImageProvider::instance();
    const qreal radius = this->getRadius();
    const QSizeF dSize = boundingRect().size();
    QColor borderColor(getBorderColor());
    QPen pen(borderColor);

    m_image = imageProvider->getImage(getSource());
    /* set the default value */
    if (m_gradient) {
        if (m_gradient->type() == QGradient::LinearGradient) {
            if (m_linearGradient.start() == QPointF(0, 0) && m_linearGradient.finalStop() == QPointF(0, 0)) {
                m_linearGradient.setStart(QPointF(0, 0));
                m_linearGradient.setFinalStop(QPointF(0, height()));
            }
        } else if (m_gradient->type() == QGradient::RadialGradient) {
        } else if (m_gradient->type() == QGradient::ConicalGradient) {
        }
    }

    switch (m_resizemode) {
    case ResizeMode::Scaled: {
        m_image = m_image.scaled(dSize.width(), dSize.height(), Qt::KeepAspectRatioByExpanding);
    } break;
    case ResizeMode::Fit: {
        m_image = m_image.scaled(dSize.width(), dSize.height(), Qt::IgnoreAspectRatio);
    } break;
    case ResizeMode::Fixed: {
    } break;
    default:
        m_image = m_image.scaled(dSize.width(), dSize.height(), Qt::IgnoreAspectRatio);
        break;
    }

    if (getxMirror() || getyMirror()) {
        m_image.mirror(getyMirror(), getxMirror());
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    borderPath.addRoundedRect(QRectF(QPointF(0, 0), dSize), radius, radius);
    painter->setClipPath(borderPath, Qt::IntersectClip);

    if (getBorder() <= 0) {
        painter->setPen(Qt::NoPen);
    } else {
        pen.setWidth(getBorder());
        painter->setPen(pen);
        painter->drawPath(borderPath);
    }

    if (m_gradient) {
        if (m_gradient->type() == QGradient::LinearGradient) {
            painter->fillRect(boundingRect(), m_linearGradient);
        } else if (m_gradient->type() == QGradient::RadialGradient) {
            painter->fillRect(boundingRect(), m_radialGradient);
        } else if (m_gradient->type() == QGradient::ConicalGradient) {
            painter->fillRect(boundingRect(), m_conicalGradient);
        }
    }

    if (m_gradient) {
        QImage opacityMask(QSize(dSize.width(), dSize.height()), QImage::Format_Alpha8);
        QPainter opacityPainter(&opacityMask);
        QImage outputImage(QSize(dSize.width(), dSize.height()), QImage::Format_ARGB32);
        QPainter gradientPainter(&outputImage);
        QPixmap opacityPixmap = QPixmap::fromImage(opacityMask);

        if (m_gradient->type() == QGradient::LinearGradient)
            opacityPainter.fillRect(opacityMask.rect(), m_linearGradient);
        else if (m_gradient->type() == QGradient::RadialGradient)
            opacityPainter.fillRect(opacityMask.rect(), m_radialGradient);
        else if (m_gradient->type() == QGradient::ConicalGradient)
            opacityPainter.fillRect(opacityMask.rect(), m_conicalGradient);

        opacityPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        opacityPainter.drawImage(opacityMask.rect(), opacityMask.createAlphaMask());
        opacityPainter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
        opacityPainter.drawImage(opacityMask.rect(), opacityMask);

        outputImage.fill(Qt::transparent);
        gradientPainter.setCompositionMode(QPainter::CompositionMode_Source);
        gradientPainter.drawImage(0, 0, m_image);
        gradientPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);

        if (m_gradient->type() == QGradient::LinearGradient)
            gradientPainter.fillRect(outputImage.rect(), m_linearGradient);
        else if (m_gradient->type() == QGradient::RadialGradient)
            gradientPainter.fillRect(outputImage.rect(), m_radialGradient);
        else if (m_gradient->type() == QGradient::ConicalGradient)
            gradientPainter.fillRect(outputImage.rect(), m_conicalGradient);

        gradientPainter.setBackgroundMode(Qt::BGMode::TransparentMode);
        gradientPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        gradientPainter.drawPixmap(0, 0, opacityPixmap);

        opacityPainter.end();
        gradientPainter.end();

        painter->setCompositionMode(QPainter::CompositionMode_DestinationAtop);
        painter->drawImage(0, 0, outputImage);

        if (m_updateCount > 0) {
            m_updateCount--;
            update();
        }
    } else {
        painter->drawImage(0, 0, m_image);
    }
}

void OpacityImage::setSource(const QString image) {
    if (m_source == image) return;
    m_source = image;
    update();
    emit sourceChanged();
}

void OpacityImage::setRadius(const qreal &newRadius) {
    if (m_radius == newRadius) return;
    m_radius = newRadius;
    update();
    emit radiusChanged();
}

void OpacityImage::setResizeMode(ResizeMode newResizemode) {
    if (m_resizemode == newResizemode) return;
    m_resizemode = newResizemode;
    update();
    emit resizeModeChanged();
}

void OpacityImage::setGradientJSValue(QJSValue &value) {
    m_gradient = NULL;
    if (m_gradientJsValue.equals(value)) return;
    if (!value.isObject()) return;

    QJSValue prop;
    QVariantList stopsList;
    m_gradientJsValue = value;
    m_gradientObject.clear();

    m_gradientObject["type"] = "linear";
    js_parse_string(value, m_gradientObject, type);
    auto type = m_gradientObject["type"].toString();

    /* linear gradient props*/
    m_gradientObject["xStart"] = 0.0;
    m_gradientObject["xStop"] = 0.0;
    m_gradientObject["yStart"] = 0.0;
    m_gradientObject["yStop"] = 0.0;
    /* radial gradient props*/
    m_gradientObject["Cx"] = 0.0;
    m_gradientObject["Cy"] = 0.0;
    m_gradientObject["Fx"] = 0.0;
    m_gradientObject["Fy"] = 0.0;
    m_gradientObject["cradius"] = 0.0;
    m_gradientObject["fradius"] = 0.0;
    /* conical gradient props*/
    m_gradientObject["Cx"] = 0.0;
    m_gradientObject["Cy"] = 0.0;
    m_gradientObject["angle"] = 0.0;

    if (type == "linear") {
        js_parse_number(value, m_gradientObject, xStart);
        js_parse_number(value, m_gradientObject, yStart);
        js_parse_number(value, m_gradientObject, xStop);
        js_parse_number(value, m_gradientObject, yStop);
    } else if (type == "radial") {

        js_parse_number(value, m_gradientObject, Cx);
        js_parse_number(value, m_gradientObject, Cy);
        js_parse_number(value, m_gradientObject, Fx);
        js_parse_number(value, m_gradientObject, Fy);
        js_parse_number(value, m_gradientObject, fradius);
        js_parse_number(value, m_gradientObject, cradius);
        js_parse_number(value, m_gradientObject, radius);
    } else if (type == "conical") {
        js_parse_number(value, m_gradientObject, Cx);
        js_parse_number(value, m_gradientObject, Cy);
        js_parse_number(value, m_gradientObject, angle);
    } else {
    }

    prop = value.property(GRADIENT_STOPS_TEXT);
    if (prop.isArray()) {
        int length = prop.property("length").toInt();
        for (int i = 0; i < length; i++) {
            QJSValue stopValue = prop.property(i);
            QVariantMap stopMap;
            if (stopValue.isObject()) {
                js_parse_number(stopValue, stopMap, position);
                js_parse_string(stopValue, stopMap, color);
                stopsList.append(stopMap);
            }
        }
        m_gradientObject[GRADIENT_STOPS_TEXT] = stopsList;
    }
    setGradient(m_gradientObject);
    update();
    emit gradientJSValueChanged();
}

void OpacityImage::setGradient(const QVariantMap &map) {

    QString typeStr = map[GRADIENT_TYPE_TEXT].toString();
    QVariantList stopsList = map[GRADIENT_STOPS_TEXT].toList();
    QGradient::Type type = (typeStr == "conical" ? QGradient::Type::ConicalGradient
                                                 : (typeStr == "radial" ? QGradient::Type::RadialGradient
                                                                        : QGradient::Type::LinearGradient));
    switch (type) {
    case QGradient::LinearGradient: {
        QPointF startP(map["xStart"].toDouble(), map["yStart"].toDouble());
        QPointF stopP(map["xStop"].toDouble(), map["yStop"].toDouble());
        m_linearGradient.setStart(startP);
        m_linearGradient.setFinalStop(stopP);
        m_gradient = &m_linearGradient;
    } break;
    case QGradient::RadialGradient: {
        QPointF cP(map["Cx"].toDouble(), map["Cy"].toDouble());
        QPointF fP(map["Fx"].toDouble(), map["Fy"].toDouble());
        qreal cradius = map["cradius"].toDouble();
        qreal fradius = map["fradius"].toDouble();
        qreal radius = map["radius"].toDouble();
        m_radialGradient = QRadialGradient(cP, cradius, fP, fradius);
        m_radialGradient.setRadius(radius);
        m_gradient = &m_radialGradient;
    } break;
    case QGradient::ConicalGradient: {
        QPointF cP(map["Cx"].toDouble(), map["Cy"].toDouble());
        m_conicalGradient.setCenter(cP);
        m_conicalGradient.setAngle(map["angle"].toDouble());
        m_gradient = &m_conicalGradient;
    } break;
    default:
        break;
    }

    for (auto stopVariant : stopsList) {
        QVariantMap stopMap = stopVariant.toMap();
        double position = stopMap["position"].toDouble();
        QColor color(stopMap["color"].toString());
        if (m_gradient)
            m_gradient->setColorAt(position, color);
    }
}

qreal OpacityImage::getBorder() const {
    return m_border;
}

void OpacityImage::setBorder(qreal newBorder) {
    if (qFuzzyCompare(m_border, newBorder))
        return;
    m_border = newBorder;
    update();
    emit borderChanged();
}

QString OpacityImage::getBorderColor() const {
    return m_borderColor;
}

void OpacityImage::setBorderColor(const QString &newBorderColor) {
    if (m_borderColor == newBorderColor)
        return;
    m_borderColor = newBorderColor;
    update();
    emit borderColorChanged();
}

bool OpacityImage::getxMirror() const {
    return m_xMirror;
}

void OpacityImage::setxMirror(bool newXMirror) {
    if (m_xMirror == newXMirror)
        return;
    m_xMirror = newXMirror;
    update();
    emit xMirrorChanged();
}

bool OpacityImage::getyMirror() const {
    return m_yMirror;
}

void OpacityImage::setyMirror(bool newYMirror) {
    if (m_yMirror == newYMirror)
        return;
    m_yMirror = newYMirror;
    update();
    emit yMirrorChanged();
}

void OpacityImage::imageChanged(const QString id) {
    if (id == getSource()) {
        update();
    }
}