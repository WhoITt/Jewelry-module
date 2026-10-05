#include "pch.h"
#include "GemSizeCatalog.h"

#include <QBrush>
#include <QColor>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPolygonF>
#include <cmath>

namespace
{
const QVector<GemSizeEntry> kRoundEntries = {
    {0.002, 0.80, 0.80, 0.50}, {0.003, 0.90, 0.90, 0.56}, {0.004, 1.00, 1.00, 0.62},
    {0.005, 1.10, 1.10, 0.68}, {0.006, 1.20, 1.20, 0.74}, {0.008, 1.30, 1.30, 0.81},
    {0.010, 1.40, 1.40, 0.87}, {0.013, 1.50, 1.50, 0.93}, {0.015, 1.60, 1.60, 0.99},
    {0.018, 1.70, 1.70, 1.05}, {0.020, 1.75, 1.75, 1.09}, {0.022, 1.80, 1.80, 1.12},
    {0.026, 1.90, 1.90, 1.18}, {0.030, 2.00, 2.00, 1.24}, {0.035, 2.10, 2.10, 1.30},
    {0.040, 2.20, 2.20, 1.36}, {0.043, 2.25, 2.25, 1.40}, {0.052, 2.40, 2.40, 1.49},
    {0.058, 2.50, 2.50, 1.55}, {0.073, 2.70, 2.70, 1.67}, {0.078, 2.75, 2.75, 1.71},
    {0.082, 2.80, 2.80, 1.74}, {0.091, 2.90, 2.90, 1.80}, {0.101, 3.00, 3.00, 1.86},
    {0.111, 3.10, 3.10, 1.92}, {0.122, 3.20, 3.20, 1.98}, {0.134, 3.30, 3.30, 2.05},
    {0.147, 3.40, 3.40, 2.11}, {0.160, 3.50, 3.50, 2.17}, {0.174, 3.60, 3.60, 2.23},
    {0.189, 3.70, 3.70, 2.29}, {0.197, 3.75, 3.75, 2.33}, {0.205, 3.80, 3.80, 2.36},
    {0.221, 3.90, 3.90, 2.42}, {0.239, 4.00, 4.00, 2.48}, {0.257, 4.10, 4.10, 2.54},
    {0.277, 4.20, 4.20, 2.60}, {0.318, 4.40, 4.40, 2.73}, {0.340, 4.50, 4.50, 2.79},
    {0.363, 4.60, 4.60, 2.85}, {0.413, 4.80, 4.80, 2.98}, {0.467, 5.00, 5.00, 3.10},
    {0.525, 5.20, 5.20, 3.22}, {0.588, 5.40, 5.40, 3.35}, {0.621, 5.50, 5.50, 3.41},
    {0.656, 5.60, 5.60, 3.47}, {0.728, 5.80, 5.80, 3.60}, {0.806, 6.00, 6.00, 3.72},
    {0.979, 6.40, 6.40, 3.97}, {1.025, 6.50, 6.50, 4.03}, {1.073, 6.60, 6.60, 4.09},
    {1.174, 6.80, 6.80, 4.22}, {1.280, 7.00, 7.00, 4.34}, {1.393, 7.20, 7.20, 4.46},
    {1.513, 7.40, 7.40, 4.59}, {1.575, 7.50, 7.50, 4.65}, {1.639, 7.60, 7.60, 4.71},
    {1.771, 7.80, 7.80, 4.84}, {1.911, 8.00, 8.00, 4.96}, {2.058, 8.20, 8.20, 5.08},
    {2.212, 8.40, 8.40, 5.21}, {2.374, 8.60, 8.60, 5.33}, {2.544, 8.80, 8.80, 5.46},
    {2.721, 9.00, 9.00, 5.58}, {2.907, 9.20, 9.20, 5.70}, {3.100, 9.40, 9.40, 5.83},
    {3.200, 9.50, 9.50, 5.89}, {3.303, 9.60, 9.60, 5.95}, {3.513, 9.80, 9.80, 6.08},
    {3.733, 10.00, 10.00, 6.20}, {3.961, 10.20, 10.20, 6.32}, {4.199, 10.40, 10.40, 6.45},
    {4.446, 10.60, 10.60, 6.57}, {4.702, 10.80, 10.80, 6.70}, {4.968, 11.00, 11.00, 6.82},
    {5.244, 11.20, 11.20, 6.94}, {6.450, 12.00, 12.00, 7.44}, {8.201, 13.00, 13.00, 8.06},
    {10.243, 14.00, 14.00, 8.68}, {12.598, 15.00, 15.00, 9.30}
};

const QVector<GemSizeEntry> kPearEntries = {
    {0.015, 2.50, 1.30, 0.83}, {0.044, 3.00, 2.00, 1.28}, {0.068, 3.00, 2.50, 1.60},
    {0.091, 4.00, 2.50, 1.60}, {0.131, 4.00, 3.00, 1.92}, {0.147, 4.50, 3.00, 1.92},
    {0.163, 5.00, 3.00, 1.92}, {0.245, 5.50, 3.50, 2.24}, {0.304, 5.80, 3.80, 2.43},
    {0.349, 6.00, 4.00, 2.56}, {0.635, 7.00, 5.00, 3.20}, {0.726, 8.00, 5.00, 3.20},
    {0.934, 8.50, 5.50, 3.52}, {1.046, 8.00, 6.00, 3.84}, {1.177, 9.00, 6.00, 3.84},
    {1.779, 10.00, 7.00, 4.48}, {2.135, 12.00, 7.00, 4.48}, {2.789, 12.00, 8.00, 5.12},
    {3.021, 13.00, 8.00, 5.12}, {3.254, 14.00, 8.00, 5.12}, {4.265, 14.50, 9.00, 5.76},
    {4.412, 15.00, 9.00, 5.76}
};

const QVector<GemSizeEntry> kEmeraldEntries = {
    {0.043, 3.00, 1.50, 0.98}, {0.050, 3.50, 1.50, 0.98}, {0.073, 3.75, 1.75, 1.14},
    {0.102, 4.00, 2.00, 1.30}, {0.149, 3.75, 2.50, 1.63}, {0.258, 4.50, 3.00, 1.95},
    {0.287, 5.00, 3.00, 1.95}, {0.368, 5.30, 3.30, 2.15}, {0.534, 5.80, 3.80, 2.47},
    {0.612, 6.00, 4.00, 2.60}, {0.840, 6.50, 4.50, 2.93}, {1.116, 7.00, 5.00, 3.25},
    {1.447, 7.50, 5.50, 3.58}, {1.837, 8.00, 6.00, 3.90}, {2.291, 8.50, 6.50, 4.23},
    {2.813, 9.00, 7.00, 4.55}, {4.082, 10.00, 8.00, 5.20}, {5.683, 11.00, 9.00, 5.85},
    {7.654, 12.00, 10.00, 6.50}, {8.930, 14.00, 10.00, 6.50}, {11.577, 15.00, 11.00, 7.15},
    {14.696, 16.00, 12.00, 7.80}
};

const QVector<GemSizeEntry> kMarquiseEntries = {
    {0.013, 2.50, 1.25, 0.70}, {0.022, 3.00, 1.50, 0.84}, {0.035, 3.50, 1.75, 0.98},
    {0.045, 3.50, 2.00, 1.12}, {0.052, 4.00, 2.00, 1.12}, {0.070, 4.25, 2.25, 1.26},
    {0.091, 4.50, 2.50, 1.40}, {0.101, 5.00, 2.50, 1.40}, {0.146, 5.00, 3.00, 1.68},
    {0.175, 6.00, 3.00, 1.68}, {0.238, 6.00, 3.50, 1.96}, {0.278, 7.00, 3.50, 1.96},
    {0.414, 8.00, 4.00, 2.24}, {0.590, 9.00, 4.50, 2.52}, {0.810, 10.00, 5.00, 2.80},
    {0.937, 10.50, 5.25, 2.94}, {1.078, 11.00, 5.50, 3.08}, {1.399, 12.00, 6.00, 3.36},
    {1.779, 13.00, 6.50, 3.64}, {2.221, 14.00, 7.00, 3.92}, {2.550, 14.00, 7.50, 4.20},
    {3.316, 16.00, 8.00, 4.48}, {3.743, 16.00, 8.50, 4.76}
};

const QVector<GemSizeEntry> kPrincessEntries = {
    {0.012, 1.25, 1.25, 0.94}, {0.021, 1.50, 1.50, 1.13}, {0.034, 1.75, 1.75, 1.31},
    {0.050, 2.00, 2.00, 1.50}, {0.072, 2.25, 2.25, 1.69}, {0.098, 2.50, 2.50, 1.88},
    {0.131, 2.75, 2.75, 2.06}, {0.170, 3.00, 3.00, 2.25}, {0.216, 3.25, 3.25, 2.44},
    {0.270, 3.50, 3.50, 2.63}, {0.332, 3.75, 3.75, 2.81}, {0.403, 4.00, 4.00, 3.00},
    {0.501, 4.30, 4.30, 3.23}, {0.574, 4.50, 4.50, 3.38}, {0.675, 4.75, 4.75, 3.56},
    {0.787, 5.00, 5.00, 3.75}, {0.912, 5.25, 5.25, 3.94}, {1.048, 5.50, 5.50, 4.13},
    {1.361, 6.00, 6.00, 4.50}, {1.730, 6.50, 6.50, 4.88}, {1.895, 6.70, 6.70, 5.03},
    {2.161, 7.00, 7.00, 5.25}, {2.658, 7.50, 7.50, 5.63}, {3.225, 8.00, 8.00, 6.00},
    {3.869, 8.50, 8.50, 6.38}, {4.592, 9.00, 9.00, 6.75}, {5.401, 9.50, 9.50, 7.13},
    {5.839, 9.75, 9.75, 7.31}, {6.300, 10.00, 10.00, 7.50}, {7.293, 10.50, 10.50, 7.88}
};

QColor kStrokeColor(214, 214, 220);
QColor kAccentColor(39, 213, 214);
QColor kFillColor(123, 177, 255);
QColor kDarkFill(59, 59, 66);
constexpr int kMinimumPreviewEdge = 800;

QSize normalizedPreviewSize(const QSize& requestedSize)
{
    const int width = requestedSize.width() > 1 ? requestedSize.width() : 1;
    const int height = requestedSize.height() > 1 ? requestedSize.height() : 1;
    return QSize(width, height);
}

QSize highResolutionRenderSize(const QSize& requestedSize)
{
    const QSize normalized = normalizedPreviewSize(requestedSize);
    const int longestEdge = normalized.width() > normalized.height()
        ? normalized.width()
        : normalized.height();
    if (longestEdge >= kMinimumPreviewEdge)
        return normalized;

    const double scale = static_cast<double>(kMinimumPreviewEdge) / static_cast<double>(longestEdge);
    return QSize(
        static_cast<int>(std::round(normalized.width() * scale)),
        static_cast<int>(std::round(normalized.height() * scale)));
}

QPixmap downscalePreviewToTarget(const QPixmap& source, const QSize& requestedSize)
{
    if (source.isNull())
        return QPixmap();

    const QSize normalized = normalizedPreviewSize(requestedSize);
    if (source.width() == normalized.width() && source.height() == normalized.height())
        return source;

    return source.scaled(
        normalized,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
}

QString gemImagePath(const QString& gemType, bool topView)
{
    const QString baseDir = QString::fromWCharArray(L"C:/Users/Mark/OneDrive/Рабочий стол/ВКР КАРТИНКИ/");
    const QString suffix = topView ? " top.png" : " side.png";

    if (gemType == "emerald")
        return baseDir + "Emerald" + suffix;
    if (gemType == "princess")
        return baseDir + "Princess" + suffix;
    if (gemType == "pear")
        return baseDir + "Pear" + suffix;
    if (gemType == "marquise")
        return baseDir + "Marquise" + suffix;
    return baseDir + "Brilliant" + suffix;
}

QString profileImagePath(int profileType)
{
    const QString baseDir = QString::fromWCharArray(L"C:/Users/Mark/OneDrive/Рабочий стол/ВКР КАРТИНКИ/");
    if (profileType == 0)
        return baseDir + "HalfRound.png";
    if (profileType == 1)
        return baseDir + "Flat.png";
    if (profileType == 2)
        return baseDir + "Oval.png";
    return baseDir + "Concave.png";
}

QPixmap loadExternalGemPreview(const QString& gemType, bool topView, const QSize& size)
{
    Q_UNUSED(size);

    const QString path = gemImagePath(gemType, topView);
    if (!QFileInfo::exists(path))
        return QPixmap();

    QImage image(path);
    if (image.isNull())
        return QPixmap();

    return QPixmap::fromImage(image);
}

QPixmap loadExternalProfilePreview(int profileType, const QSize& size)
{
    Q_UNUSED(size);

    const QString path = profileImagePath(profileType);
    if (!QFileInfo::exists(path))
        return QPixmap();

    QImage image(path);
    if (image.isNull())
        return QPixmap();

    return QPixmap::fromImage(image);
}

void preparePainter(QPainter& painter, const QSize& size)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.fillRect(QRect(QPoint(0, 0), size), QColor(48, 48, 54));
}

QPixmap drawRoundTop(const QSize& size)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    preparePainter(painter, size);

    QRectF outer(6.0, 6.0, size.width() - 12.0, size.height() - 12.0);
    painter.setPen(QPen(kStrokeColor, 2.0));
    painter.setBrush(QBrush(kDarkFill));
    painter.drawEllipse(outer);

    painter.setPen(QPen(kAccentColor, 2.0));
    painter.drawLine(QPointF(size.width() * 0.5, 8.0), QPointF(size.width() * 0.5, size.height() - 8.0));
    painter.drawLine(QPointF(8.0, size.height() * 0.5), QPointF(size.width() - 8.0, size.height() * 0.5));

    painter.setPen(QPen(QColor(205, 205, 210), 1.2));
    for (int i = 0; i < 8; ++i)
    {
        const double angle = i * 3.14159265358979323846 / 4.0;
        painter.drawLine(
            QPointF(size.width() * 0.5, size.height() * 0.5),
            QPointF(size.width() * 0.5 + std::cos(angle) * outer.width() * 0.44,
                size.height() * 0.5 + std::sin(angle) * outer.height() * 0.44));
    }
    return pixmap;
}

QPixmap drawPrincessTop(const QSize& size)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    preparePainter(painter, size);

    QRectF rect(8.0, 8.0, size.width() - 16.0, size.height() - 16.0);
    painter.setPen(QPen(kStrokeColor, 2.0));
    painter.setBrush(QBrush(kDarkFill));
    painter.drawRoundedRect(rect, 4.0, 4.0);

    painter.setPen(QPen(kAccentColor, 2.0));
    painter.drawLine(rect.topLeft(), rect.bottomRight());
    painter.drawLine(rect.topRight(), rect.bottomLeft());
    painter.drawRect(rect.adjusted(rect.width() * 0.18, rect.height() * 0.18, -rect.width() * 0.18, -rect.height() * 0.18));
    return pixmap;
}

QPixmap drawEmeraldTop(const QSize& size)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    preparePainter(painter, size);

    QRectF rect(10.0, 6.0, size.width() - 20.0, size.height() - 12.0);
    QPainterPath path;
    path.moveTo(rect.left() + 8.0, rect.top());
    path.lineTo(rect.right() - 8.0, rect.top());
    path.lineTo(rect.right(), rect.top() + 8.0);
    path.lineTo(rect.right(), rect.bottom() - 8.0);
    path.lineTo(rect.right() - 8.0, rect.bottom());
    path.lineTo(rect.left() + 8.0, rect.bottom());
    path.lineTo(rect.left(), rect.bottom() - 8.0);
    path.lineTo(rect.left(), rect.top() + 8.0);
    path.closeSubpath();

    painter.setPen(QPen(kStrokeColor, 2.0));
    painter.setBrush(QBrush(kDarkFill));
    painter.drawPath(path);
    painter.setPen(QPen(kAccentColor, 2.0));
    painter.drawRect(rect.adjusted(8.0, 8.0, -8.0, -8.0));
    return pixmap;
}

QPixmap drawPearTop(const QSize& size)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    preparePainter(painter, size);

    QPainterPath path;
    const QPointF top(size.width() * 0.52, 8.0);
    const QPointF left(size.width() * 0.22, size.height() * 0.38);
    const QPointF bottom(size.width() * 0.50, size.height() - 8.0);
    const QPointF right(size.width() * 0.78, size.height() * 0.38);
    path.moveTo(top);
    path.cubicTo(QPointF(size.width() * 0.18, 10.0), QPointF(size.width() * 0.10, size.height() * 0.52), bottom);
    path.cubicTo(QPointF(size.width() * 0.90, size.height() * 0.52), QPointF(size.width() * 0.86, 10.0), top);

    painter.setPen(QPen(kStrokeColor, 2.0));
    painter.setBrush(QBrush(kDarkFill));
    painter.drawPath(path);
    painter.setPen(QPen(kAccentColor, 2.0));
    painter.drawLine(top, bottom);
    painter.drawLine(left, right);
    return pixmap;
}

QPixmap drawMarquiseTop(const QSize& size)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    preparePainter(painter, size);

    QPainterPath path;
    const QRectF rect(8.0, 10.0, size.width() - 16.0, size.height() - 20.0);
    path.moveTo(rect.left(), rect.center().y());
    path.quadTo(rect.center().x(), rect.top(), rect.right(), rect.center().y());
    path.quadTo(rect.center().x(), rect.bottom(), rect.left(), rect.center().y());

    painter.setPen(QPen(kStrokeColor, 2.0));
    painter.setBrush(QBrush(kDarkFill));
    painter.drawPath(path);
    painter.setPen(QPen(kAccentColor, 2.0));
    painter.drawLine(QPointF(rect.left(), rect.center().y()), QPointF(rect.right(), rect.center().y()));
    painter.drawLine(QPointF(rect.center().x(), rect.top()), QPointF(rect.center().x(), rect.bottom()));
    return pixmap;
}

QPixmap drawSidePreview(const QString& gemType, const QSize& size)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    preparePainter(painter, size);
    painter.setPen(QPen(kStrokeColor, 2.0));
    painter.setBrush(QBrush(kDarkFill));

    QPolygonF poly;
    if (gemType == "custom")
    {
        poly << QPointF(6.0, size.height() * 0.48)
             << QPointF(size.width() * 0.26, size.height() * 0.28)
             << QPointF(size.width() * 0.74, size.height() * 0.28)
             << QPointF(size.width() - 6.0, size.height() * 0.48)
             << QPointF(size.width() * 0.5, size.height() - 8.0);
    }
    else if (gemType == "princess")
    {
        poly << QPointF(10.0, size.height() * 0.34)
             << QPointF(size.width() - 10.0, size.height() * 0.34)
             << QPointF(size.width() * 0.78, size.height() * 0.62)
             << QPointF(size.width() * 0.50, size.height() - 8.0)
             << QPointF(size.width() * 0.22, size.height() * 0.62);
    }
    else if (gemType == "emerald")
    {
        poly << QPointF(10.0, size.height() * 0.30)
             << QPointF(size.width() - 10.0, size.height() * 0.30)
             << QPointF(size.width() * 0.82, size.height() * 0.68)
             << QPointF(size.width() * 0.18, size.height() * 0.68);
    }
    else if (gemType == "pear")
    {
        poly << QPointF(12.0, size.height() * 0.34)
             << QPointF(size.width() * 0.70, size.height() * 0.30)
             << QPointF(size.width() - 10.0, size.height() * 0.48)
             << QPointF(size.width() * 0.42, size.height() - 8.0)
             << QPointF(12.0, size.height() * 0.60);
    }
    else
    {
        poly << QPointF(8.0, size.height() * 0.50)
             << QPointF(size.width() * 0.28, size.height() * 0.28)
             << QPointF(size.width() * 0.72, size.height() * 0.28)
             << QPointF(size.width() - 8.0, size.height() * 0.50)
             << QPointF(size.width() * 0.50, size.height() - 8.0);
    }

    painter.drawPolygon(poly);
    painter.setPen(QPen(kAccentColor, 2.0));
    painter.drawLine(QPointF(6.0, size.height() * 0.34), QPointF(size.width() - 6.0, size.height() * 0.34));
    return pixmap;
}

QPixmap drawProfilePreview(int profileType, const QSize& size)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    preparePainter(painter, size);
    painter.setPen(QPen(kStrokeColor, 2.0));
    painter.setBrush(QBrush(kFillColor));

    QPainterPath path;
    const QRectF box(10.0, 10.0, size.width() - 20.0, size.height() - 20.0);
    if (profileType == 0)
    {
        path.moveTo(box.left(), box.bottom());
        path.lineTo(box.right(), box.bottom());
        path.quadTo(box.right(), box.top(), box.left(), box.top() + box.height() * 0.35);
        path.closeSubpath();
    }
    else if (profileType == 1)
    {
        path.addRoundedRect(box, 4.0, 4.0);
    }
    else if (profileType == 2)
    {
        path.moveTo(box.left() + 8.0, box.bottom());
        path.quadTo(box.left(), box.center().y(), box.left() + 8.0, box.top() + 6.0);
        path.lineTo(box.right() - 8.0, box.top() + 6.0);
        path.quadTo(box.right(), box.center().y(), box.right() - 8.0, box.bottom());
        path.closeSubpath();
    }
    else
    {
        path.moveTo(box.left(), box.bottom());
        path.lineTo(box.right(), box.bottom());
        path.lineTo(box.right() - 6.0, box.top());
        path.quadTo(box.center().x(), box.center().y(), box.left() + 6.0, box.top());
        path.closeSubpath();
    }

    painter.drawPath(path);
    painter.setPen(QPen(kAccentColor, 2.0));
    painter.drawLine(QPointF(box.left(), box.bottom() + 2.0), QPointF(box.right(), box.bottom() + 2.0));
    return pixmap;
}
}

namespace GemSizeCatalog
{
const QVector<GemSizeEntry>& entriesForGemType(const QString& gemType)
{
    if (gemType == "emerald")
        return kEmeraldEntries;
    if (gemType == "princess")
        return kPrincessEntries;
    if (gemType == "pear")
        return kPearEntries;
    if (gemType == "marquise")
        return kMarquiseEntries;
    return kRoundEntries;
}

QString gemDisplayName(const QString& gemType)
{
    if (gemType == "emerald")
        return QString::fromWCharArray(L"Изумруд");
    if (gemType == "princess")
        return QString::fromWCharArray(L"Принцесса");
    if (gemType == "pear")
        return QString::fromWCharArray(L"Груша");
    if (gemType == "marquise")
        return QString::fromWCharArray(L"Маркиз");
    return QString::fromWCharArray(L"Бриллиант");
}

QPixmap createGemTopViewPreview(const QString& gemType, const QSize& size)
{
    const QPixmap externalPreview = loadExternalGemPreview(gemType, true, size);
    if (!externalPreview.isNull())
        return externalPreview;

    const QSize renderSize = highResolutionRenderSize(size);
    QPixmap rendered;
    if (gemType == "emerald")
        rendered = drawEmeraldTop(renderSize);
    else if (gemType == "princess")
        rendered = drawPrincessTop(renderSize);
    else if (gemType == "pear")
        rendered = drawPearTop(renderSize);
    else if (gemType == "marquise")
        rendered = drawMarquiseTop(renderSize);
    else
        rendered = drawRoundTop(renderSize);

    return downscalePreviewToTarget(rendered, size);
}

QPixmap createGemSideViewPreview(const QString& gemType, const QSize& size)
{
    const QPixmap externalPreview = loadExternalGemPreview(gemType, false, size);
    if (!externalPreview.isNull())
        return externalPreview;

    const QSize renderSize = highResolutionRenderSize(size);
    return downscalePreviewToTarget(drawSidePreview(gemType, renderSize), size);
}

QPixmap createRingProfilePreview(int profileType, const QSize& size)
{
    const QPixmap externalPreview = loadExternalProfilePreview(profileType, size);
    if (!externalPreview.isNull())
        return externalPreview;

    const QSize renderSize = highResolutionRenderSize(size);
    return downscalePreviewToTarget(drawProfilePreview(profileType, renderSize), size);
}
}
