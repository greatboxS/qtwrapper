/**
 * @file imageprovider.cpp
 * @author greatboxs (greatboxs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-05-5
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "imageprovider.h"
#include <QPainter>
#include <QPainterPath>
#include <QJSValueIterator>
#include <QReadWriteLock>

#define js_parse_number(value, obj, name)              \
    if (value.property(#name).isNumber()) {            \
        obj[#name] = value.property(#name).toNumber(); \
    }

#define js_parse_string(value, obj, name)              \
    if (!value.property(#name).isNull()) {             \
        obj[#name] = value.property(#name).toString(); \
    }

static QReadWriteLock sImageProviderLock;
ImageProvider *ImageProvider::m_instance = NULL;
int ImageProvider::m_state = -1;

/**
 * @fn ImageProvider
 * @brief Construct a new Image Provider:: Image Provider object
 *
 */
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
        m_state++;
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

void ImageProvider::updateImage(const char *id, const QString &path) {
    QWriteLocker locker(&sImageProviderLock);
    QString file = path;
    auto img = this->image(id);
    if (img == NULL) {
        m_imagesMap[id] = new QImage();
        img = m_imagesMap[id];
    }

    /* Check if the url is passed to remove the qrc:/ prefix */
    if (file.contains("qrc:/") && file.indexOf("qrc:/") == 0) {
        file.replace("qrc:/", ":/");
    }

    if (img->load(file)) {
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

/**
 * @fn OpacityImage
 * @brief Construct a new Opacity Image:: Opacity Image object
 *
 */
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
    m_url("") {

    QObject::connect(
        ImageProvider::instance(), &ImageProvider::imageChanged, this, [this](const QString id) {
            if (id == getSource()) {
                m_image = ImageProvider::instance()->getImage(getSource());
                update();
            }
        },
        Qt::QueuedConnection);

    QObject::connect(this, &OpacityImage::sourceChanged, this, [this]() {
        m_image = ImageProvider::instance()->getImage(getSource());
        update();
    });

    QObject::connect(this, &OpacityImage::urlChanged, this, [this]() {
        QString url = getURL();
        if (url.contains("qrc:/") && url.indexOf("qrc:/") == 0) {
            url.replace("qrc:/", ":/");
        }
        m_image = QImage(url);
        update();
    });
}

OpacityImage::~OpacityImage() {
}

void OpacityImage::paint(QPainter *painter) {

    if (m_image.isNull()) return;

    QPainterPath borderPath;
    const qreal radius = this->getRadius();
    const QSizeF dSize = boundingRect().size();
    QColor borderColor(getBorderColor());
    QPen pen(borderColor);

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

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    borderPath.addRoundedRect(QRectF(QPointF(0, 0), dSize), radius, radius);
    painter->setClipPath(borderPath, Qt::IntersectClip);
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    if (getBorder() <= 0) {
        painter->setPen(Qt::NoPen);
    } else {
        pen.setWidth(getBorder());
        painter->setPen(pen);
        painter->drawPath(borderPath);
    }

    if (m_gradient) {
        QImage opacityMask(QSize(dSize.toSize()), QImage::Format_Alpha8);
        QPainter oPainter(&opacityMask);
        QImage resultImage(dSize.toSize(), QImage::Format_ARGB32_Premultiplied);
        QPainter iPainter(&resultImage);

        oPainter.setCompositionMode(QPainter::CompositionMode_Source);
        if (m_gradient->type() == QGradient::LinearGradient) {
            oPainter.fillRect(opacityMask.rect(), m_linearGradient);
        } else if (m_gradient->type() == QGradient::RadialGradient) {
            oPainter.fillRect(opacityMask.rect(), m_radialGradient);
        } else if (m_gradient->type() == QGradient::ConicalGradient) {
            oPainter.fillRect(opacityMask.rect(), m_conicalGradient);
        } else {
        }
        oPainter.end();

        iPainter.setCompositionMode(QPainter::CompositionMode_Source);
        iPainter.fillRect(resultImage.rect(), Qt::transparent);
        iPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        iPainter.drawImage(0, 0, opacityMask);
        iPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        iPainter.drawImage(0, 0, m_image);
        iPainter.end();

        painter->drawImage(0, 0, resultImage);
    } else {
        painter->drawImage(0, 0, m_image);
    }

    if (m_gradient) {
        painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
        if (m_gradient->type() == QGradient::LinearGradient) {
            painter->fillRect(boundingRect(), m_linearGradient);
        } else if (m_gradient->type() == QGradient::RadialGradient) {
            painter->fillRect(boundingRect(), m_radialGradient);
        } else if (m_gradient->type() == QGradient::ConicalGradient) {
            painter->fillRect(boundingRect(), m_conicalGradient);
        }
    }
    painter->restore();
}

void OpacityImage::setSource(const QString image) {
    if (m_source == image) return;
    m_source = image;
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
    if (!value.isObject() || value.isNull()) return;
    if (m_gradientJsValue.equals(value)) return;

    QJSValue prop;
    QVariantList stopsList;
    m_gradientJsValue = value;
    m_gradientObject.clear();

    /* Indicate what type of gradient it is*/
    js_parse_number(value, m_gradientObject, angle);
    js_parse_number(value, m_gradientObject, focalX);
    js_parse_number(value, m_gradientObject, x1);
    if (m_gradientObject.find("angle") != m_gradientObject.end()) {
        m_gradientObject["type"] = QGradient::Type::ConicalGradient;
    } else if (m_gradientObject.find("focalX") != m_gradientObject.end()) {
        m_gradientObject["type"] = QGradient::Type::RadialGradient;
    } else if (m_gradientObject.find("x1") != m_gradientObject.end()) {
        m_gradientObject["type"] = QGradient::Type::LinearGradient;
    } else {
        m_gradientObject["type"] = QGradient::Type::LinearGradient;
    }

    QGradient::Type type = static_cast<QGradient::Type>(m_gradientObject["type"].toInt());
    /* linear gradient props*/
    m_gradientObject["x1"] = 0.0;
    m_gradientObject["y1"] = 0.0;
    m_gradientObject["x2"] = 0.0;
    m_gradientObject["y2"] = 0.0;
    /* radial gradient props*/
    m_gradientObject["centerX"] = 0.0;
    m_gradientObject["centerY"] = 0.0;
    m_gradientObject["focalX"] = 0.0;
    m_gradientObject["focalY"] = 0.0;
    m_gradientObject["centerRadius"] = 0.0;
    m_gradientObject["focalRadius"] = 0.0;
    /* conical gradient props*/
    m_gradientObject["centerX"] = 0.0;
    m_gradientObject["centerY"] = 0.0;
    m_gradientObject["angle"] = 0.0;

    m_gradientObject["spread"] = 0.0;
    m_gradientObject["orientation"] = 0.0;
    js_parse_number(value, m_gradientObject, spread);
    js_parse_number(value, m_gradientObject, orientation);

    if (type == QGradient::LinearGradient) {
        js_parse_number(value, m_gradientObject, x1);
        js_parse_number(value, m_gradientObject, y1);
        js_parse_number(value, m_gradientObject, x2);
        js_parse_number(value, m_gradientObject, y2);
    } else if (type == QGradient::RadialGradient) {
        js_parse_number(value, m_gradientObject, centerX);
        js_parse_number(value, m_gradientObject, centerY);
        js_parse_number(value, m_gradientObject, focalX);
        js_parse_number(value, m_gradientObject, focalY);
        js_parse_number(value, m_gradientObject, focalRadius);
        js_parse_number(value, m_gradientObject, centerRadius);
    } else if (type == QGradient::ConicalGradient) {
        js_parse_number(value, m_gradientObject, centerX);
        js_parse_number(value, m_gradientObject, centerY);
        js_parse_number(value, m_gradientObject, angle);
    } else {
    }

    prop = value.property("stops");
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
        m_gradientObject["stops"] = stopsList;
    } else {
        QJSValueIterator it(prop);
        while (it.hasNext()) {
            it.next();
            QJSValue stopValue = it.value();
            QVariantMap stopMap;
            if (stopValue.isObject()) {
                js_parse_number(stopValue, stopMap, position);
                js_parse_string(stopValue, stopMap, color);
                stopsList.append(stopMap);
            }
        }
        m_gradientObject["stops"] = stopsList;
    }
    setGradient(m_gradientObject);
    update();
    emit gradientJSValueChanged();
}

void OpacityImage::setGradient(const QVariantMap &map) {

    QVariantList stopsList = map["stops"].toList();
    QGradient::Type type = static_cast<QGradient::Type>(map["type"].toInt());
    switch (type) {
    case QGradient::LinearGradient: {
        QPointF startP(map["x1"].toDouble(), map["y1"].toDouble());
        QPointF stopP(map["x2"].toDouble(), map["y2"].toDouble());
        m_linearGradient.setStart(startP);
        m_linearGradient.setFinalStop(stopP);
        m_gradient = &m_linearGradient;
    } break;
    case QGradient::RadialGradient: {
        QPointF cp(map["centerX"].toDouble(), map["centerY"].toDouble());
        QPointF fp(map["focalX"].toDouble(), map["focalY"].toDouble());
        qreal centerRadius = map["centerRadius"].toDouble();
        qreal focalRadius = map["focalRadius"].toDouble();

        if (focalRadius != 0)
            m_radialGradient = QRadialGradient(cp, centerRadius, fp, focalRadius);
        else
            m_radialGradient = QRadialGradient(cp, centerRadius);

        m_gradient = &m_radialGradient;
    } break;
    case QGradient::ConicalGradient: {
        QPointF cP(map["centerX"].toDouble(), map["centerY"].toDouble());
        m_conicalGradient.setCenter(cP);
        m_conicalGradient.setAngle(map["angle"].toDouble());
        m_gradient = &m_conicalGradient;
    } break;
    default:
        break;
    }

    m_gradient->setCoordinateMode(QGradient::LogicalMode);
    for (auto stopVariant : stopsList) {
        QVariantMap stopMap = stopVariant.toMap();
        double position = stopMap["position"].toDouble();
        QColor color(stopMap["color"].toString());
        if (m_gradient)
            m_gradient->setColorAt(position, color);
    }
}

qreal OpacityImage::getBorder() const { return m_border; }

void OpacityImage::setBorder(qreal newBorder) {
    if (qFuzzyCompare(m_border, newBorder))
        return;
    m_border = newBorder;
    update();
    emit borderChanged();
}

QString OpacityImage::getBorderColor() const { return m_borderColor; }

void OpacityImage::setBorderColor(const QString &newBorderColor) {
    if (m_borderColor == newBorderColor)
        return;
    m_borderColor = newBorderColor;
    update();
    emit borderColorChanged();
}

bool OpacityImage::getxMirror() const { return m_xMirror; }

void OpacityImage::setxMirror(bool newXMirror) {
    if (m_xMirror == newXMirror)
        return;
    m_xMirror = newXMirror;
    update();
    emit xMirrorChanged();
}

bool OpacityImage::getyMirror() const { return m_yMirror; }

void OpacityImage::setyMirror(bool newYMirror) {
    if (m_yMirror == newYMirror)
        return;
    m_yMirror = newYMirror;
    update();
    emit yMirrorChanged();
}

QString OpacityImage::getURL() const { return m_url; }

void OpacityImage::setURL(const QString &newUrl) {
    if (m_url == newUrl)
        return;
    m_url = newUrl;
    emit urlChanged();
}