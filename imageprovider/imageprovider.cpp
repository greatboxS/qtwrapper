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

void ImageProvider::addImage(const char *id, std::string &buf) {
    QWriteLocker locker(&sImageProviderLock);
    auto img = this->image(id);
    if (img == NULL) {
        m_imagesMap[id] = new QImage();
        img = m_imagesMap[id];
    }
    img->loadFromData((const uchar *)buf.data(), buf.size());
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
    m_resizemode(ResizeMode::AutoResize),
    m_gradient(NULL),
    m_border(0.0),
    m_borderColor("black"),
    m_xMirror(false),
    m_yMirror(false) {
}
OpacityImage::~OpacityImage() {
}

void OpacityImage::paint(QPainter *painter) {
    auto imageProvider = ImageProvider::instance();
    const QSizeF size = boundingRect().size();
    const qreal radius = this->getRadius();
    m_image = imageProvider->getImage(getSource());
    QSizeF sSize = QSizeF(m_image.width(), m_image.height());
    QSizeF dSize = size;

    QPainterPath borderPath;

    switch (m_resizemode) {
    case ResizeMode::AutoResize: {

    } break;
    case ResizeMode::AutoSizeByWidth: {

    } break;
    case ResizeMode::AutoSizeByHeight: {

    } break;
    case ResizeMode::Fitting: {

    } break;
    case ResizeMode::Fixed: {
    } break;

    default:
        break;
    }

    if (getxMirror() || getyMirror()) {
        m_image.mirror(getyMirror(), getxMirror());
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    borderPath.addRoundedRect(QRectF(QPointF(0, 0), size), radius, radius);
    painter->setClipPath(borderPath, Qt::IntersectClip);

    if (getBorder() <= 0) {
        painter->setPen(Qt::NoPen);
    } else {
        QColor color(getBorderColor());
        QPen pen(color);
        pen.setWidth(getBorder());
        painter->setPen(pen);
        painter->drawPath(borderPath);
    }

    if (m_gradient) {
        painter->setBrush(*m_gradient);
        painter->setCompositionMode(QPainter::CompositionMode_Multiply);
    }

    QRectF sRect(QPoint(0, 0), sSize);
    QRectF dRect(QPoint(0, 0), dSize);
    painter->drawImage(dRect, m_image, sRect);
    if (m_gradient) {
        painter->fillPath(borderPath, *m_gradient);
    }
}

void OpacityImage::setSource(const QString image) {
    if (m_source == image) return;
    m_source = image;
    emit imageChanged();
}

void OpacityImage::setRadius(const qreal &newRadius) {
    if (m_radius == newRadius) return;
    m_radius = newRadius;
    emit radiusChanged();
}

void OpacityImage::setResizeMode(ResizeMode newResizemode) {
    if (m_resizemode == newResizemode) return;
    m_resizemode = newResizemode;
    emit resizeModeChanged();
}

void OpacityImage::setGradientJSValue(QJSValue &value) {
    if (m_gradientJsValue.equals(value)) return;
    if (!value.isObject()) return;

    QJSValue prop;
    QVariantList stopsList;
    m_gradientJsValue = value;
    m_gradientObject.clear();

    prop = value.property(GRADIENT_TYPE_TEXT);
    if (prop.isString()) {
        m_gradientObject[GRADIENT_TYPE_TEXT] = prop.toString();
    }

    if (prop.toString() == "linear") {
        m_gradientObject["xStart"] = 0.0;
        m_gradientObject["xStop"] = 0.0;
        m_gradientObject["yStart"] = 0.0;
        m_gradientObject["yStop"] = 0.0;

        prop = value.property("xStart");
        if (prop.isNumber()) {
            m_gradientObject["xStart"] = prop.toNumber();
        }
        prop = value.property("xStopx");
        if (prop.isNumber()) {
            m_gradientObject["xStop"] = prop.toNumber();
        }
        prop = value.property("yStart");
        if (prop.isNumber()) {
            m_gradientObject["yStart"] = prop.toNumber();
        }
        prop = value.property("yStop");
        if (prop.isNumber()) {
            m_gradientObject["yStop"] = prop.toNumber();
        }
    } else if (prop.toString() == "radial") {

        m_gradientObject["Cx"] = 0.0;
        m_gradientObject["Cy"] = 0.0;
        m_gradientObject["Fx"] = 0.0;
        m_gradientObject["Fy"] = 0.0;
        m_gradientObject["cradius"] = 0.0;
        m_gradientObject["fradius"] = 0.0;

    } else if (prop.toString() == "conical") {
        m_gradientObject["Cx"] = 0.0;
        m_gradientObject["Cy"] = 0.0;
        m_gradientObject["angle"] = 0.0;

    } else {
        m_gradientObject["xStart"] = 0.0;
        m_gradientObject["xStop"] = 0.0;
        m_gradientObject["yStart"] = 0.0;
        m_gradientObject["yStop"] = 0.0;
    }

    prop = value.property(GRADIENT_STOPS_TEXT);
    if (prop.isArray()) {
        int length = prop.property("length").toInt();
        for (int i = 0; i < length; i++) {
            QJSValue stopValue = prop.property(i);
            QVariantMap stopMap;
            if (stopValue.isObject()) {
                QJSValue positionValue = stopValue.property(GRADIENT_POSITION_TEXT);
                QJSValue colorValue = stopValue.property(GRADIENT_COLOR_TEXT);
                if (positionValue.isNumber() && colorValue.isString()) {
                    stopMap[GRADIENT_POSITION_TEXT] = positionValue.toNumber();
                    stopMap[GRADIENT_COLOR_TEXT] = colorValue.toString();
                    stopsList.append(stopMap);
                }
            }
        }
        m_gradientObject[GRADIENT_STOPS_TEXT] = stopsList;
    }
    setGradient(m_gradientObject);
}

void OpacityImage::setGradient(const QVariantMap &map) {

    QString typeStr = map[GRADIENT_TYPE_TEXT].toString();
    QVariantList stopsList = map[GRADIENT_STOPS_TEXT].toList();
    QGradient::Type type = (typeStr == "conical" ? QGradient::Type::ConicalGradient
                                                 : (typeStr == "radial" ? QGradient::Type::RadialGradient
                                                                        : QGradient::Type::LinearGradient));
    QGradient *gradient = NULL;
    switch (type) {
    case QGradient::LinearGradient: {
        gradient = new QLinearGradient;
        QPointF startP(map["xStart"].toDouble(), map["yStart"].toDouble());
        QPointF stopP(map["xStart"].toDouble(), map["yStart"].toDouble());
        reinterpret_cast<QLinearGradient *>(gradient)->setStart(startP);
        reinterpret_cast<QLinearGradient *>(gradient)->setFinalStop(stopP);
    }
    case QGradient::RadialGradient: {
        QPointF cP(map["Cx"].toDouble(), map["Cy"].toDouble());
        QPointF fP(map["Fx"].toDouble(), map["Fy"].toDouble());

        reinterpret_cast<QRadialGradient *>(gradient)->setCenter(cP);
        reinterpret_cast<QRadialGradient *>(gradient)->setCenterRadius(map["cradius"].toDouble());

        reinterpret_cast<QRadialGradient *>(gradient)->setFocalPoint(fP);
        reinterpret_cast<QRadialGradient *>(gradient)->setFocalRadius(map["fradius"].toDouble());
        gradient = new QRadialGradient;
    }
    case QGradient::ConicalGradient: {
        QPointF cP(map["Cx"].toDouble(), map["Cy"].toDouble());
        reinterpret_cast<QConicalGradient *>(gradient)->setCenter(cP);
        reinterpret_cast<QConicalGradient *>(gradient)->setAngle(map["angle"].toDouble());
        gradient = new QConicalGradient;

    } break;
    default:
        gradient = new QLinearGradient;
        break;
    }

    for (auto stopVariant : stopsList) {
        QVariantMap stopMap = stopVariant.toMap();
        double position = stopMap["position"].toDouble();
        QColor color(stopMap["color"].toString());
        gradient->setColorAt(position, color);
    }

    gradient->setCoordinateMode(QGradient::StretchToDeviceMode);

    if (m_gradient) delete m_gradient;
    m_gradient = gradient;
}

qreal OpacityImage::getBorder() const {
    return m_border;
}

void OpacityImage::setBorder(qreal newBorder) {
    if (qFuzzyCompare(m_border, newBorder))
        return;
    m_border = newBorder;
    emit borderChanged();
}

QString OpacityImage::getBorderColor() const {
    return m_borderColor;
}

void OpacityImage::setBorderColor(const QString &newBorderColor) {
    if (m_borderColor == newBorderColor)
        return;
    m_borderColor = newBorderColor;
    emit borderColorChanged();
}

bool OpacityImage::getxMirror() const {
    return m_xMirror;
}

void OpacityImage::setxMirror(bool newXMirror) {
    if (m_xMirror == newXMirror)
        return;
    m_xMirror = newXMirror;
    emit xMirrorChanged();
}

bool OpacityImage::getyMirror() const {
    return m_yMirror;
}

void OpacityImage::setyMirror(bool newYMirror) {
    if (m_yMirror == newYMirror)
        return;
    m_yMirror = newYMirror;
    emit yMirrorChanged();
}
