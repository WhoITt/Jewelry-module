#pragma once

#include <QPixmap>
#include <QString>
#include <QVector>

struct GemSizeEntry
{
    double carat = 0.0;
    double length = 0.0;
    double width = 0.0;
    double height = 0.0;
};

namespace GemSizeCatalog
{
const QVector<GemSizeEntry>& entriesForGemType(const QString& gemType);
QString gemDisplayName(const QString& gemType);
QPixmap createGemTopViewPreview(const QString& gemType, const QSize& size);
QPixmap createGemSideViewPreview(const QString& gemType, const QSize& size);
QPixmap createRingProfilePreview(int profileType, const QSize& size);
}
