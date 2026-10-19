#include "pch.h"
#include "CRingEntity.h"
#include "Gems/CJewelryGemstone.h"
#include <algorithm>
#include <cmath>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Adesk::UInt32 CRingEntity::kCurrentVersionNumber = 4;

ACRX_DXF_DEFINE_MEMBERS(
    CRingEntity,
    AcDbEntity,
    AcDb::kDHL_CURRENT,
    AcDb::kMReleaseCurrent,
    AcDbProxyEntity::kAllAllowedBits,
    RINGENTITY,
    L"VKR_NRX"
);

namespace
{
double clampPositive(double value, double fallback)
{
    return value > 1.0e-6 ? value : fallback;
}

double clampMin(double value, double minValue)
{
    return value > minValue ? value : minValue;
}

double clampValue(double value, double minValue, double maxValue)
{
    return std::max(minValue, std::min(maxValue, value));
}

void appendTriangle(
    AcArray<Adesk::Int32>& faceList,
    Adesk::Int32 a,
    Adesk::Int32 b,
    Adesk::Int32 c)
{
    faceList.append(3);
    faceList.append(a);
    faceList.append(b);
    faceList.append(c);
}

void appendProfileArc(
    AcGePoint3dArray& profilePoints,
    const AcGePoint3d& center,
    double radius,
    double startAngle,
    double endAngle,
    int segments)
{
    for (int i = 0; i <= segments; ++i)
    {
        const double t = static_cast<double>(i) / segments;
        const double angle = startAngle + (endAngle - startAngle) * t;
        profilePoints.append(AcGePoint3d(
            center.x + radius * std::cos(angle),
            center.y + radius * std::sin(angle),
            0.0));
    }
}
}

CRingEntity::CRingEntity()
    : NcDbEntity()
    , m_center(AcGePoint3d::kOrigin)
    , m_innerRadius(9.0)
    , m_bandWidth(2.8)
    , m_thickness(2.2)
    , m_profileType(kProfileHalfRound)
    , m_stoneCenter(0.0, 0.0, 10.4)
    , m_stoneSeatRadius(1.0)
    , m_hasStone(false)
    , m_settingType(kSettingNone)
    , m_prongCount(4)
    , m_prongRadius(0.48)
    , m_prongHeight(1.15)
    , m_stoneLength(2.4)
    , m_stoneWidth(2.4)
    , m_stoneHeight(1.7)
    , m_railStoneCount(0)
    , m_stoneShapeType(CJewelryGemstone::kOutlineRound)
    , m_linkedStoneId(AcDbObjectId::kNull)
{
}

CRingEntity::CRingEntity(
    const AcGePoint3d& center,
    double innerRadius,
    double bandWidth,
    double thickness)
    : NcDbEntity()
    , m_center(center)
    , m_innerRadius(clampPositive(innerRadius, 9.0))
    , m_bandWidth(clampPositive(bandWidth, 2.8))
    , m_thickness(clampPositive(thickness, 2.2))
    , m_profileType(kProfileHalfRound)
    , m_stoneCenter(center + AcGeVector3d(0.0, 0.0, innerRadius + bandWidth))
    , m_stoneSeatRadius(std::min(1.0, clampPositive(bandWidth, 2.8) * 0.35))
    , m_hasStone(false)
    , m_settingType(kSettingNone)
    , m_prongCount(4)
    , m_prongRadius(0.48)
    , m_prongHeight(1.15)
    , m_stoneLength(2.4)
    , m_stoneWidth(2.4)
    , m_stoneHeight(1.7)
    , m_railStoneCount(0)
    , m_stoneShapeType(CJewelryGemstone::kOutlineRound)
    , m_linkedStoneId(AcDbObjectId::kNull)
{
}

CRingEntity::~CRingEntity()
{
}

Acad::ErrorStatus CRingEntity::setCenter(const AcGePoint3d& center)
{
    assertWriteEnabled();
    m_center = center;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setInnerRadius(double radius)
{
    assertWriteEnabled();
    if (radius <= 0.0)
        return Acad::eInvalidInput;
    m_innerRadius = radius;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setBandWidth(double width)
{
    assertWriteEnabled();
    if (width <= 0.0)
        return Acad::eInvalidInput;
    m_bandWidth = width;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setThickness(double thickness)
{
    assertWriteEnabled();
    if (thickness <= 0.0)
        return Acad::eInvalidInput;
    m_thickness = thickness;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setProfileType(RingProfileType type)
{
    assertWriteEnabled();
    if (type < kProfileHalfRound || type > kProfileConcave)
        return Acad::eInvalidInput;
    m_profileType = type;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setStoneCenter(const AcGePoint3d& center)
{
    assertWriteEnabled();
    m_stoneCenter = center;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setStoneSeatRadius(double radius)
{
    assertWriteEnabled();
    if (radius <= 0.0)
        return Acad::eInvalidInput;
    m_stoneSeatRadius = radius;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setHasStone(bool value)
{
    assertWriteEnabled();
    m_hasStone = value;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setSettingType(SettingType type)
{
    assertWriteEnabled();

    if (type < kSettingNone || type > kSettingBezel)
        return Acad::eInvalidInput;

    m_settingType = type;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setProngCount(int count)
{
    assertWriteEnabled();
    if (count < 3 || count > 12)
        return Acad::eInvalidInput;
    m_prongCount = count;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setProngRadius(double radius)
{
    assertWriteEnabled();
    if (radius <= 0.0)
        return Acad::eInvalidInput;
    m_prongRadius = radius;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setProngHeight(double height)
{
    assertWriteEnabled();
    if (height <= 0.0)
        return Acad::eInvalidInput;
    m_prongHeight = height;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setStoneDimensions(double length, double width, double height)
{
    assertWriteEnabled();
    if (length <= 0.0 || width <= 0.0 || height <= 0.0)
        return Acad::eInvalidInput;
    m_stoneLength = length;
    m_stoneWidth = width;
    m_stoneHeight = height;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setStoneShapeType(StoneShapeType type)
{
    assertWriteEnabled();
    if (type < CJewelryGemstone::kOutlineRound || type > CJewelryGemstone::kOutlinePear)
        return Acad::eInvalidInput;
    m_stoneShapeType = type;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::setLinkedStoneId(const AcDbObjectId& id)
{
    assertWriteEnabled();
    m_linkedStoneId = id;
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::clearLinkedStoneId()
{
    assertWriteEnabled();
    m_linkedStoneId = AcDbObjectId::kNull;
    return Acad::eOk;
}

AcGePoint3d CRingEntity::sweepProfilePoint(double sweepRadius, double localX, double localZ, int segment) const
{
    const double angle = (2.0 * M_PI * segment) / kRingSegmentCount;
    return m_center + AcGeVector3d(
        (sweepRadius + localX) * std::cos(angle),
        localZ,
        (sweepRadius + localX) * std::sin(angle)
    );
}

void CRingEntity::appendQuad(
    AcArray<Adesk::Int32>& faceList,
    Adesk::Int32 a,
    Adesk::Int32 b,
    Adesk::Int32 c,
    Adesk::Int32 d) const
{
    faceList.append(4);
    faceList.append(a);
    faceList.append(b);
    faceList.append(c);
    faceList.append(d);
}

void CRingEntity::buildProfileContour(AcGePoint3dArray& profilePoints) const
{
    profilePoints.setLogicalLength(0);

    const double halfWidth = m_bandWidth * 0.5;
    const double halfHeight = m_thickness * 0.5;
    const int arcSegments = 10;

    if (m_profileType == kProfileFlat)
    {
        const double cornerRadius = clampValue(
            std::min(m_bandWidth, m_thickness) * 0.08,
            0.04,
            0.18);
        const int cornerSegments = 3;

        appendProfileArc(
            profilePoints,
            AcGePoint3d(halfWidth - cornerRadius, -halfHeight + cornerRadius, 0.0),
            cornerRadius,
            -M_PI * 0.5,
            0.0,
            cornerSegments);
        appendProfileArc(
            profilePoints,
            AcGePoint3d(halfWidth - cornerRadius, halfHeight - cornerRadius, 0.0),
            cornerRadius,
            0.0,
            M_PI * 0.5,
            cornerSegments);
        appendProfileArc(
            profilePoints,
            AcGePoint3d(-halfWidth + cornerRadius, halfHeight - cornerRadius, 0.0),
            cornerRadius,
            M_PI * 0.5,
            M_PI,
            cornerSegments);
        appendProfileArc(
            profilePoints,
            AcGePoint3d(-halfWidth + cornerRadius, -halfHeight + cornerRadius, 0.0),
            cornerRadius,
            M_PI,
            M_PI * 1.5,
            cornerSegments);
        return;
    }

    if (m_profileType == kProfileComfort)
    {
        for (int i = 0; i < 20; ++i)
        {
            const double angle = (2.0 * M_PI * i) / 20.0;
            profilePoints.append(AcGePoint3d(
                halfWidth * std::cos(angle),
                halfHeight * std::sin(angle),
                0.0));
        }
        return;
    }

    if (m_profileType == kProfileConcave)
    {
        profilePoints.append(AcGePoint3d(-halfWidth, -halfHeight, 0.0));
        profilePoints.append(AcGePoint3d(halfWidth, -halfHeight, 0.0));
        for (int i = 1; i < arcSegments; ++i)
        {
            const double t = static_cast<double>(i) / arcSegments;
            const double z = -halfHeight + 2.0 * halfHeight * t;
            const double curve = std::sin(t * M_PI);
            const double x = halfWidth - m_bandWidth * 0.28 * curve;
            profilePoints.append(AcGePoint3d(x, z, 0.0));
        }
        profilePoints.append(AcGePoint3d(halfWidth, halfHeight, 0.0));
        profilePoints.append(AcGePoint3d(-halfWidth, halfHeight, 0.0));
        return;
    }

    profilePoints.append(AcGePoint3d(-halfWidth, -halfHeight, 0.0));
    for (int i = 0; i <= arcSegments; ++i)
    {
        const double angle = -M_PI * 0.5 + (M_PI * i / arcSegments);
        const double x = -halfWidth + m_bandWidth * (0.5 + 0.5 * std::cos(angle));
        const double z = halfHeight * std::sin(angle);
        profilePoints.append(AcGePoint3d(x, z, 0.0));
    }
    profilePoints.append(AcGePoint3d(-halfWidth, halfHeight, 0.0));
}

void CRingEntity::appendProngCylinder(
    const AcGePoint3d& baseCenter,
    AcGePoint3dArray& vertices,
    AcArray<Adesk::Int32>& faceList) const
{
    const int startIndex = vertices.length();
    const double shoulderZ = baseCenter.z + m_prongHeight * 0.62;
    const double topZ = baseCenter.z + m_prongHeight;
    AcGeVector3d inward = m_stoneCenter - baseCenter;
    inward.z = 0.0;
    if (inward.length() > 1.0e-6)
        inward.normalize();

    const AcGeVector3d topOffset = inward * (m_prongRadius * 0.42);
    const double radii[3] = { m_prongRadius * 1.55, m_prongRadius * 1.18, m_prongRadius * 0.72 };
    const double zLevels[3] = { baseCenter.z, shoulderZ, topZ };
    const AcGeVector3d offsets[3] = {
        AcGeVector3d(0.0, 0.0, 0.0),
        inward * (m_prongRadius * 0.08),
        topOffset
    };

    for (int level = 0; level < 3; ++level)
    {
        for (int i = 0; i < kProngSegmentCount; ++i)
        {
            const double angle = (2.0 * M_PI * i) / kProngSegmentCount;
            const double dx = radii[level] * std::cos(angle);
            const double dy = radii[level] * std::sin(angle);
            vertices.append(AcGePoint3d(
                baseCenter.x + offsets[level].x + dx,
                baseCenter.y + offsets[level].y + dy,
                zLevels[level]));
        }
    }

    for (int level = 0; level < 2; ++level)
    {
        const int levelStart = startIndex + level * kProngSegmentCount;
        const int nextLevelStart = levelStart + kProngSegmentCount;
        for (int i = 0; i < kProngSegmentCount; ++i)
        {
            const int next = (i + 1) % kProngSegmentCount;
            appendQuad(
                faceList,
                levelStart + i,
                levelStart + next,
                nextLevelStart + next,
                nextLevelStart + i);
        }
    }

    const int bottomCenterIndex = vertices.length();
    vertices.append(AcGePoint3d(baseCenter.x, baseCenter.y, baseCenter.z));
    const int topCenterIndex = vertices.length();
    vertices.append(AcGePoint3d(
        baseCenter.x + topOffset.x,
        baseCenter.y + topOffset.y,
        topZ));

    const int bottomStart = startIndex;
    const int topStart = startIndex + 2 * kProngSegmentCount;
    for (int i = 0; i < kProngSegmentCount; ++i)
    {
        const int next = (i + 1) % kProngSegmentCount;
        appendTriangle(faceList, bottomCenterIndex, bottomStart + next, bottomStart + i);
        appendTriangle(faceList, topCenterIndex, topStart + i, topStart + next);
    }
}

void CRingEntity::appendBezelGeometry(
    AcGePoint3dArray& vertices,
    AcArray<Adesk::Int32>& faceList) const
{
    if (!m_hasStone || m_settingType != kSettingBezel)
        return;

    const int startIndex = vertices.length();
    const int segments = 48;
    const double minStoneSize = std::max(0.2, std::min(m_stoneLength, m_stoneWidth));
    const double wallThickness = clampValue(std::max(m_prongRadius * 0.78, minStoneSize * 0.045), 0.18, 0.48);
    const double stoneClearance = clampValue(minStoneSize * 0.01, 0.02, 0.06);
    const double visibleTopZ = m_stoneCenter.z + m_stoneHeight * 0.86;
    const double bezelWallHeight = clampValue(m_stoneHeight * 0.78, 0.8, 3.2);
    const double supportDepth = clampValue(bezelWallHeight * 0.9, 0.45, 1.6);
    const double supportScale = 0.58;
    const double supportZ = m_stoneCenter.z - supportDepth;
    const double bottomZ = m_stoneCenter.z + m_stoneHeight * 0.08;
    const double topZ = std::max(bottomZ + 0.2, visibleTopZ);

    auto pearContourRadius = [&](double angle, double offset) -> double
    {
        struct KeyPoint
        {
            double x;
            double y;
        };

        const double halfWidth = m_stoneWidth * 0.5;
        const KeyPoint points[12] = {
            { -m_stoneLength * 0.40, 0.0 },
            { -m_stoneLength * 0.30, halfWidth * 0.85 },
            { -m_stoneLength * 0.10, halfWidth },
            { m_stoneLength * 0.10, halfWidth * 0.90 },
            { m_stoneLength * 0.25, halfWidth * 0.65 },
            { m_stoneLength * 0.38, halfWidth * 0.35 },
            { m_stoneLength * 0.50, 0.0 },
            { m_stoneLength * 0.38, -halfWidth * 0.35 },
            { m_stoneLength * 0.25, -halfWidth * 0.65 },
            { m_stoneLength * 0.10, -halfWidth * 0.90 },
            { -m_stoneLength * 0.10, -halfWidth },
            { -m_stoneLength * 0.30, -halfWidth * 0.85 }
        };

        const double dx = std::cos(angle);
        const double dy = std::sin(angle);
        double bestT = 0.0;
        for (int i = 0; i < 12; ++i)
        {
            const KeyPoint& p = points[i];
            const KeyPoint& q = points[(i + 1) % 12];
            const double sx = q.x - p.x;
            const double sy = q.y - p.y;
            const double det = sx * dy - sy * dx;
            if (std::abs(det) <= 1.0e-8)
                continue;

            const double segmentT = -(p.x * dy - p.y * dx) / det;
            const double rayT = -(p.x * sy - p.y * sx) / det;
            if (segmentT >= -1.0e-6 && segmentT <= 1.0 + 1.0e-6 && rayT > bestT)
                bestT = rayT;
        }

        if (bestT <= 1.0e-6)
        {
            const double rx = std::max(0.2, m_stoneLength * 0.5);
            const double ry = std::max(0.2, m_stoneWidth * 0.5);
            const double denom = std::sqrt((dx * dx) / (rx * rx) + (dy * dy) / (ry * ry));
            bestT = denom > 1.0e-6 ? 1.0 / denom : std::max(rx, ry);
        }

        return std::max(0.2, bestT + offset);
    };

    auto contourPoint = [&](bool outer, double scale, double angle, double& x, double& y)
    {
        const double offset = (outer ? wallThickness : stoneClearance);
        if (m_stoneShapeType == CJewelryGemstone::kOutlineRect)
        {
            const double c = std::cos(angle);
            const double s = std::sin(angle);
            const double halfL = std::max(0.2, m_stoneLength * 0.5 + offset);
            const double halfW = std::max(0.2, m_stoneWidth * 0.5 + offset);
            const double tx = std::abs(c) > 1.0e-6 ? halfL / std::abs(c) : 1.0e9;
            const double ty = std::abs(s) > 1.0e-6 ? halfW / std::abs(s) : 1.0e9;
            const double radius = std::min(tx, ty) * scale;
            x = radius * c;
            y = radius * s;
        }
        else if (m_stoneShapeType == CJewelryGemstone::kOutlinePear)
        {
            const double radius = pearContourRadius(angle, offset) * scale;
            x = radius * std::cos(angle);
            y = radius * std::sin(angle);
        }
        else if (m_stoneShapeType == CJewelryGemstone::kOutlineOval)
        {
            const double rx = (m_stoneLength * 0.5 + offset) * scale;
            const double ry = (m_stoneWidth * 0.5 + offset) * scale;
            x = rx * std::cos(angle);
            y = ry * std::sin(angle);
        }
        else
        {
            const double radius = (std::max(m_stoneSeatRadius, minStoneSize * 0.5) + offset) * scale;
            x = radius * std::cos(angle);
            y = radius * std::sin(angle);
        }
    };

    for (int i = 0; i < segments; ++i)
    {
        const double angle = (2.0 * M_PI * i) / segments;
        double x = 0.0;
        double y = 0.0;
        contourPoint(true, supportScale, angle, x, y);
        vertices.append(AcGePoint3d(
            m_stoneCenter.x + x,
            m_stoneCenter.y + y,
            supportZ));
    }

    for (int ring = 0; ring < 4; ++ring)
    {
        const bool outer = (ring == 0 || ring == 2);
        const double z = ring < 2 ? bottomZ : topZ;
        for (int i = 0; i < segments; ++i)
        {
            const double angle = (2.0 * M_PI * i) / segments;
            double x = 0.0;
            double y = 0.0;
            contourPoint(outer, 1.0, angle, x, y);

            vertices.append(AcGePoint3d(
                m_stoneCenter.x + x,
                m_stoneCenter.y + y,
                z));
        }
    }

    const int supportStart = startIndex;
    const int outerBottomStart = startIndex + segments;
    const int innerBottomStart = startIndex + segments * 2;
    const int outerTopStart = startIndex + segments * 3;
    const int innerTopStart = startIndex + segments * 4;

    for (int i = 0; i < segments; ++i)
    {
        const int next = (i + 1) % segments;
        appendQuad(faceList, supportStart + i, supportStart + next, outerBottomStart + next, outerBottomStart + i);
    }

    for (int i = 0; i < segments; ++i)
    {
        const int next = (i + 1) % segments;
        appendQuad(faceList, outerBottomStart + i, outerBottomStart + next, outerTopStart + next, outerTopStart + i);
        appendQuad(faceList, innerBottomStart + i, innerTopStart + i, innerTopStart + next, innerBottomStart + next);
        appendQuad(faceList, outerTopStart + i, outerTopStart + next, innerTopStart + next, innerTopStart + i);
    }
}

void CRingEntity::updateSeatCenterFromCurrentRing()
{
    if (!m_hasStone)
        return;

    AcGeVector3d radial = m_stoneCenter - m_center;
    radial.y = 0.0;
    if (radial.length() <= 1.0e-6)
        radial = AcGeVector3d(0.0, 0.0, 1.0);
    else
        radial.normalize();

    m_stoneCenter = m_center + radial * (m_innerRadius + m_bandWidth);
}

void CRingEntity::transformLinkedStone(const AcGeMatrix3d& xform)
{
    if (m_linkedStoneId.isNull())
        return;

    CJewelryGemstone* pGemstone = nullptr;
    if (acdbOpenObject(pGemstone, m_linkedStoneId, AcDb::kForWrite) != Acad::eOk || pGemstone == nullptr)
        return;

    pGemstone->transformBy(xform);
    pGemstone->close();
}

void CRingEntity::syncLinkedStoneToSeat()
{
    if (m_linkedStoneId.isNull())
        return;

    CJewelryGemstone* pGemstone = nullptr;
    if (acdbOpenObject(pGemstone, m_linkedStoneId, AcDb::kForWrite) != Acad::eOk || pGemstone == nullptr)
        return;

    AcDbExtents extents;
    if (pGemstone->subGetGeomExtents(extents) != Acad::eOk)
    {
        pGemstone->close();
        return;
    }

    const AcGePoint3d currentAnchor(
        (extents.minPoint().x + extents.maxPoint().x) * 0.5,
        (extents.minPoint().y + extents.maxPoint().y) * 0.5,
        extents.minPoint().z);

    pGemstone->transformBy(AcGeMatrix3d::translation(m_stoneCenter - currentAnchor));
    pGemstone->close();
}

void CRingEntity::appendProngsGeometry(
    AcGePoint3dArray& vertices,
    AcArray<Adesk::Int32>& faceList) const
{
    if (!m_hasStone || m_settingType != kSettingProng || m_prongCount <= 0)
        return;

    const double minStoneSize = std::max(0.2, std::min(m_stoneLength, m_stoneWidth));
    const double prongClearance = clampValue(minStoneSize * 0.012, 0.025, 0.08);
    const double prongThickness = clampValue(std::max(m_prongRadius * 1.15, minStoneSize * 0.075), 0.34, 0.85);
    const double lipOverhang = clampValue(prongThickness * 0.28, 0.08, 0.22);

    auto contourRadius = [&](double angle, double offset) -> double
    {
        const double c = std::cos(angle);
        const double s = std::sin(angle);
        if (m_stoneShapeType == CJewelryGemstone::kOutlineRect)
        {
            const double halfL = std::max(0.2, m_stoneLength * 0.5 + offset);
            const double halfW = std::max(0.2, m_stoneWidth * 0.5 + offset);
            const double tx = std::abs(c) > 1.0e-6 ? halfL / std::abs(c) : 1.0e9;
            const double ty = std::abs(s) > 1.0e-6 ? halfW / std::abs(s) : 1.0e9;
            return std::min(tx, ty);
        }
        if (m_stoneShapeType == CJewelryGemstone::kOutlinePear)
        {
            struct KeyPoint
            {
                double x;
                double y;
            };

            const double halfWidth = m_stoneWidth * 0.5;
            const KeyPoint points[12] = {
                { -m_stoneLength * 0.40, 0.0 },
                { -m_stoneLength * 0.30, halfWidth * 0.85 },
                { -m_stoneLength * 0.10, halfWidth },
                { m_stoneLength * 0.10, halfWidth * 0.90 },
                { m_stoneLength * 0.25, halfWidth * 0.65 },
                { m_stoneLength * 0.38, halfWidth * 0.35 },
                { m_stoneLength * 0.50, 0.0 },
                { m_stoneLength * 0.38, -halfWidth * 0.35 },
                { m_stoneLength * 0.25, -halfWidth * 0.65 },
                { m_stoneLength * 0.10, -halfWidth * 0.90 },
                { -m_stoneLength * 0.10, -halfWidth },
                { -m_stoneLength * 0.30, -halfWidth * 0.85 }
            };

            double bestT = 0.0;
            for (int i = 0; i < 12; ++i)
            {
                const KeyPoint& p = points[i];
                const KeyPoint& q = points[(i + 1) % 12];
                const double sx = q.x - p.x;
                const double sy = q.y - p.y;
                const double det = sx * s - sy * c;
                if (std::abs(det) <= 1.0e-8)
                    continue;

                const double segmentT = -(p.x * s - p.y * c) / det;
                const double rayT = -(p.x * sy - p.y * sx) / det;
                if (segmentT >= -1.0e-6 && segmentT <= 1.0 + 1.0e-6 && rayT > bestT)
                    bestT = rayT;
            }

            if (bestT <= 1.0e-6)
            {
                const double rx = std::max(0.2, m_stoneLength * 0.5);
                const double ry = std::max(0.2, m_stoneWidth * 0.5);
                const double denom = std::sqrt((c * c) / (rx * rx) + (s * s) / (ry * ry));
                bestT = denom > 1.0e-6 ? 1.0 / denom : std::max(rx, ry);
            }

            return std::max(0.2, bestT + offset);
        }
        if (m_stoneShapeType == CJewelryGemstone::kOutlineOval)
        {
            const double rx = std::max(0.2, m_stoneLength * 0.5 + offset);
            const double ry = std::max(0.2, m_stoneWidth * 0.5 + offset);
            const double denom = std::sqrt((c * c) / (rx * rx) + (s * s) / (ry * ry));
            return denom > 1.0e-6 ? 1.0 / denom : std::max(rx, ry);
        }
        const double stoneRadius = std::max(m_stoneSeatRadius, minStoneSize * 0.5);
        return std::max(0.2, stoneRadius + offset);
    };

    const int kSamplesPerProng = 5;
    const int kLevelCount = 5;
    const double bridgeHalfStep = M_PI / static_cast<double>(m_prongCount) * 0.42;
    const double supportBaseRadius = clampValue(
        m_bandWidth * 0.36,
        minStoneSize * 0.22,
        minStoneSize * 0.42);
    const double zLevels[kLevelCount] = {
        m_stoneCenter.z - m_prongHeight * 0.62,
        m_stoneCenter.z + m_stoneHeight * 0.18,
        m_stoneCenter.z + m_stoneHeight * 0.54,
        m_stoneCenter.z + m_stoneHeight * 0.68,
        m_stoneCenter.z + m_stoneHeight * 0.76
    };
    const double halfStepScale[kLevelCount] = { 0.46, 0.82, 1.0, 0.9, 0.58 };
    const double extraOuter[kLevelCount] = {
        prongThickness * 0.72,
        prongThickness * 1.12,
        prongThickness,
        prongThickness * 0.82,
        prongThickness * 0.62
    };

    for (int i = 0; i < m_prongCount; ++i)
    {
        const double centerAngle = (2.0 * M_PI * i) / static_cast<double>(m_prongCount);
        const int levelStart = vertices.length();

        for (int level = 0; level < kLevelCount; ++level)
        {
            const double halfStep = bridgeHalfStep * halfStepScale[level];
            for (int sample = 0; sample < kSamplesPerProng; ++sample)
            {
                const double t = static_cast<double>(sample) / static_cast<double>(kSamplesPerProng - 1);
                const double angle = centerAngle - halfStep + 2.0 * halfStep * t;
                double innerRadius = supportBaseRadius;
                if (level > 0)
                {
                    double offset = prongClearance;
                    if (level == kLevelCount - 1)
                    {
                        offset = -lipOverhang;
                        if (m_stoneShapeType == CJewelryGemstone::kOutlinePear)
                            offset -= prongClearance * 1.5;
                    }
                    innerRadius = contourRadius(angle, offset);
                }

                const double outerRadius = innerRadius + extraOuter[level];
                vertices.append(AcGePoint3d(
                    m_stoneCenter.x + innerRadius * std::cos(angle),
                    m_stoneCenter.y + innerRadius * std::sin(angle),
                    zLevels[level]));
                vertices.append(AcGePoint3d(
                    m_stoneCenter.x + outerRadius * std::cos(angle),
                    m_stoneCenter.y + outerRadius * std::sin(angle),
                    zLevels[level]));
            }
        }

        const int pointsPerLevel = kSamplesPerProng * 2;
        for (int level = 0; level < kLevelCount - 1; ++level)
        {
            const int current = levelStart + level * pointsPerLevel;
            const int next = current + pointsPerLevel;
            for (int sample = 0; sample < kSamplesPerProng - 1; ++sample)
            {
                const int inner0 = current + sample * 2;
                const int outer0 = inner0 + 1;
                const int inner1 = inner0 + 2;
                const int outer1 = inner1 + 1;
                const int nextInner0 = next + sample * 2;
                const int nextOuter0 = nextInner0 + 1;
                const int nextInner1 = nextInner0 + 2;
                const int nextOuter1 = nextInner1 + 1;

                appendQuad(faceList, inner0, inner1, nextInner1, nextInner0);
                appendQuad(faceList, outer0, nextOuter0, nextOuter1, outer1);
                appendQuad(faceList, inner0, nextInner0, nextOuter0, outer0);
                appendQuad(faceList, inner1, outer1, nextOuter1, nextInner1);
            }
        }

        const int baseStart = levelStart;
        const int topStart = levelStart + (kLevelCount - 1) * pointsPerLevel;
        for (int sample = 0; sample < kSamplesPerProng - 1; ++sample)
        {
            const int baseInner0 = baseStart + sample * 2;
            const int baseOuter0 = baseInner0 + 1;
            const int baseInner1 = baseInner0 + 2;
            const int baseOuter1 = baseInner1 + 1;
            const int topInner0 = topStart + sample * 2;
            const int topOuter0 = topInner0 + 1;
            const int topInner1 = topInner0 + 2;
            const int topOuter1 = topInner1 + 1;

            appendQuad(faceList, baseInner0, baseOuter0, baseOuter1, baseInner1);
            appendQuad(faceList, topInner0, topInner1, topOuter1, topOuter0);
        }
    }
}

void CRingEntity::buildRingGeometry(
    AcGePoint3dArray& vertices,
    AcArray<Adesk::Int32>& faceList) const
{
    assertReadEnabled();

    vertices.setLogicalLength(0);
    faceList.setLogicalLength(0);

    AcGePoint3dArray profile;
    buildProfileContour(profile);
    if (profile.length() < 3)
        return;

    const int profilePointCount = profile.length();
    const double sweepRadius = m_innerRadius + m_bandWidth * 0.5;

    for (int i = 0; i < kRingSegmentCount; ++i)
    {
        for (int j = 0; j < profilePointCount; ++j)
        {
            vertices.append(sweepProfilePoint(
                sweepRadius,
                profile[j].x,
                profile[j].y,
                i));
        }
    }

    for (int i = 0; i < kRingSegmentCount; ++i)
    {
        const int nextRing = (i + 1) % kRingSegmentCount;
        const int ringStart = i * profilePointCount;
        const int nextRingStart = nextRing * profilePointCount;

        for (int j = 0; j < profilePointCount; ++j)
        {
            const int nextProfile = (j + 1) % profilePointCount;
            appendQuad(
                faceList,
                ringStart + j,
                ringStart + nextProfile,
                nextRingStart + nextProfile,
                nextRingStart + j);
        }
    }

    appendProngsGeometry(vertices, faceList);
    appendBezelGeometry(vertices, faceList);
}

Adesk::Boolean CRingEntity::subWorldDraw(AcGiWorldDraw* pWd)
{
    assertReadEnabled();

    AcGePoint3dArray vertices;
    AcArray<Adesk::Int32> faceList;
    buildRingGeometry(vertices, faceList);

    if (vertices.length() != 0 && faceList.length() != 0)
    {
        pWd->geometry().shell(
            vertices.length(),
            vertices.asArrayPtr(),
            faceList.length(),
            faceList.asArrayPtr());
    }

    if (m_hasStone && m_stoneSeatRadius > 0.0)
    {
        AcGePoint3d seatCircle[kRingSegmentCount + 1];
        for (int i = 0; i <= kRingSegmentCount; ++i)
        {
            const double angle = (2.0 * M_PI * i) / kRingSegmentCount;
            seatCircle[i] = AcGePoint3d(
                m_stoneCenter.x + m_stoneSeatRadius * std::cos(angle),
                m_stoneCenter.y + m_stoneSeatRadius * std::sin(angle),
                m_stoneCenter.z);
        }
        pWd->geometry().polyline(kRingSegmentCount + 1, seatCircle);
    }

    return Adesk::kTrue;
}

Acad::ErrorStatus CRingEntity::subTransformBy(const AcGeMatrix3d& xform)
{
    assertWriteEnabled();

    const AcGePoint3d oldSeatCenter = m_stoneCenter;
    const double sweepRadius = m_innerRadius + m_bandWidth * 0.5;
    const double halfThickness = m_thickness * 0.5;
    const AcGePoint3d oldCenter = m_center;
    const AcGePoint3d sweepPoint = oldCenter + AcGeVector3d(sweepRadius, 0.0, 0.0);
    const AcGePoint3d innerPoint = oldCenter + AcGeVector3d(m_innerRadius, 0.0, 0.0);
    const AcGePoint3d topPoint = oldCenter + AcGeVector3d(0.0, halfThickness, 0.0);

    m_center.transformBy(xform);
    m_stoneCenter.transformBy(xform);

    AcGePoint3d transformedSweep = sweepPoint;
    AcGePoint3d transformedInner = innerPoint;
    AcGePoint3d transformedTop = topPoint;
    transformedSweep.transformBy(xform);
    transformedInner.transformBy(xform);
    transformedTop.transformBy(xform);

    const double newSweepRadius = m_center.distanceTo(transformedSweep);
    const double newInnerRadius = m_center.distanceTo(transformedInner);
    const double newHalfThickness = m_center.distanceTo(transformedTop);

    if (newInnerRadius > 1.0e-6)
        m_innerRadius = newInnerRadius;
    if (newSweepRadius > m_innerRadius)
        m_bandWidth = (newSweepRadius - m_innerRadius) * 2.0;
    if (newHalfThickness > 1.0e-6)
        m_thickness = newHalfThickness * 2.0;

    transformLinkedStone(AcGeMatrix3d::translation(m_stoneCenter - oldSeatCenter));
    syncLinkedStoneToSeat();

    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::subGetGeomExtents(AcDbExtents& extents) const
{
    assertReadEnabled();

    AcGePoint3dArray vertices;
    AcArray<Adesk::Int32> faceList;
    buildRingGeometry(vertices, faceList);

    if (vertices.isEmpty())
        return Acad::eInvalidExtents;

    AcGePoint3d minPt = vertices[0];
    AcGePoint3d maxPt = vertices[0];

    for (int i = 1; i < vertices.length(); ++i)
    {
        minPt.x = std::min(minPt.x, vertices[i].x);
        minPt.y = std::min(minPt.y, vertices[i].y);
        minPt.z = std::min(minPt.z, vertices[i].z);
        maxPt.x = std::max(maxPt.x, vertices[i].x);
        maxPt.y = std::max(maxPt.y, vertices[i].y);
        maxPt.z = std::max(maxPt.z, vertices[i].z);
    }

    extents.set(minPt, maxPt);
    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::subGetGripPoints(
    AcGePoint3dArray& gripPoints,
    AcDbIntArray& osnapModes,
    AcDbIntArray& geomIds) const
{
    assertReadEnabled();

    const double outerRadius = m_innerRadius + m_bandWidth;
    const double halfThickness = m_thickness * 0.5;

    gripPoints.append(m_center);
    gripPoints.append(m_center + AcGeVector3d(outerRadius, 0.0, 0.0));
    gripPoints.append(m_center + AcGeVector3d(m_innerRadius, 0.0, 0.0));
    gripPoints.append(m_center + AcGeVector3d(0.0, halfThickness, 0.0));
    gripPoints.append(m_stoneCenter);
    gripPoints.append(m_stoneCenter + AcGeVector3d(m_stoneSeatRadius, 0.0, 0.0));

    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::subMoveGripPointsAt(
    const AcDbIntArray& indices,
    const AcGeVector3d& offset)
{
    assertWriteEnabled();

    bool ringGeometryChanged = false;
    bool seatChangedExplicitly = false;

    for (int i = 0; i < indices.length(); ++i)
    {
        switch (indices[i])
        {
        case kGripCenter:
        {
            m_center += offset;
            m_stoneCenter += offset;
            ringGeometryChanged = true;
            seatChangedExplicitly = true;
            break;
        }

        case kGripOuterRadius:
        {
            const AcGePoint3d currentGrip = m_center + AcGeVector3d(m_innerRadius + m_bandWidth, 0.0, 0.0);
            const AcGePoint3d movedGrip = currentGrip + offset;
            const AcGeVector3d radial = movedGrip - m_center;
            const double newOuterRadius = std::sqrt(radial.x * radial.x + radial.z * radial.z);
            const double newBandWidth = newOuterRadius - m_innerRadius;
            if (newBandWidth > 0.2)
            {
                m_bandWidth = newBandWidth;
                ringGeometryChanged = true;
            }
            break;
        }

        case kGripInnerRadius:
        {
            const AcGePoint3d currentGrip = m_center + AcGeVector3d(m_innerRadius, 0.0, 0.0);
            const AcGePoint3d movedGrip = currentGrip + offset;
            const AcGeVector3d radial = movedGrip - m_center;
            const double newInnerRadius = std::sqrt(radial.x * radial.x + radial.z * radial.z);
            const double outerRadius = m_innerRadius + m_bandWidth;
            if (newInnerRadius > 0.2 && newInnerRadius < outerRadius - 0.2)
            {
                m_innerRadius = newInnerRadius;
                m_bandWidth = outerRadius - newInnerRadius;
                ringGeometryChanged = true;
            }
            break;
        }

        case kGripThickness:
        {
            const AcGePoint3d currentGrip = m_center + AcGeVector3d(0.0, m_thickness * 0.5, 0.0);
            const AcGePoint3d movedGrip = currentGrip + offset;
            const double newThickness = std::abs(movedGrip.y - m_center.y) * 2.0;
            if (newThickness > 0.2)
            {
                m_thickness = newThickness;
                ringGeometryChanged = true;
            }
            break;
        }

        case kGripSeatCenter:
        {
            m_stoneCenter += offset;
            seatChangedExplicitly = true;
            break;
        }

        case kGripSeatRadius:
        {
            const AcGePoint3d currentGrip = m_stoneCenter + AcGeVector3d(m_stoneSeatRadius, 0.0, 0.0);
            const AcGePoint3d movedGrip = currentGrip + offset;
            const AcGeVector3d radial = movedGrip - m_stoneCenter;
            const double newSeatRadius = std::sqrt(radial.x * radial.x + radial.y * radial.y + radial.z * radial.z);
            if (newSeatRadius > 0.1)
            {
                m_stoneSeatRadius = newSeatRadius;
                seatChangedExplicitly = true;
            }
            break;
        }

        default:
            break;
        }
    }

    if (m_hasStone)
    {
        if (ringGeometryChanged && !seatChangedExplicitly)
            updateSeatCenterFromCurrentRing();

        if (ringGeometryChanged || seatChangedExplicitly)
            syncLinkedStoneToSeat();
    }

    return Acad::eOk;
}

Acad::ErrorStatus CRingEntity::dwgInFields(AcDbDwgFiler* pFiler)
{
    assertWriteEnabled();

    Acad::ErrorStatus es = AcDbEntity::dwgInFields(pFiler);
    if (es != Acad::eOk)
        return es;

    Adesk::UInt32 version = 0;
    if ((es = pFiler->readUInt32(&version)) != Acad::eOk)
        return es;
    if (version > kCurrentVersionNumber)
        return Acad::eMakeMeProxy;

    Adesk::Int32 profileValue = 0;
    Adesk::Int16 hasStoneValue = 0;
    Adesk::Int16 settingTypeValue = 0;
    Adesk::Int32 prongCountValue = 4;
    Adesk::Int32 railStoneCountValue = 0;
    Adesk::Int32 linkedRailStoneCountValue = 0;
    Adesk::Int16 stoneShapeValue = 0;
    AcDbSoftPointerId linkedStoneIdValue;

    if ((es = pFiler->readPoint3d(&m_center)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_innerRadius)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_bandWidth)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_thickness)) != Acad::eOk) return es;
    if (version >= 2)
    {
        if ((es = pFiler->readInt32(&profileValue)) != Acad::eOk) return es;
    }
    if ((es = pFiler->readPoint3d(&m_stoneCenter)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_stoneSeatRadius)) != Acad::eOk) return es;
    if (version >= 2)
    {
        if ((es = pFiler->readInt16(&hasStoneValue)) != Acad::eOk) return es;
        if ((es = pFiler->readInt16(&settingTypeValue)) != Acad::eOk) return es;
        if ((es = pFiler->readInt32(&prongCountValue)) != Acad::eOk) return es;
        if ((es = pFiler->readDouble(&m_prongRadius)) != Acad::eOk) return es;
        if ((es = pFiler->readDouble(&m_prongHeight)) != Acad::eOk) return es;
        if ((es = pFiler->readDouble(&m_stoneLength)) != Acad::eOk) return es;
        if ((es = pFiler->readDouble(&m_stoneWidth)) != Acad::eOk) return es;
        if ((es = pFiler->readDouble(&m_stoneHeight)) != Acad::eOk) return es;
        if (version >= 3)
        {
            if ((es = pFiler->readInt32(&railStoneCountValue)) != Acad::eOk) return es;
        }
        if ((es = pFiler->readInt16(&stoneShapeValue)) != Acad::eOk) return es;
        if ((es = pFiler->readSoftPointerId(&linkedStoneIdValue)) != Acad::eOk) return es;
        if (version >= 4)
        {
            if ((es = pFiler->readInt32(&linkedRailStoneCountValue)) != Acad::eOk) return es;
            m_linkedRailStoneIds.setLogicalLength(0);
            for (int i = 0; i < linkedRailStoneCountValue; ++i)
            {
                AcDbSoftPointerId railId;
                if ((es = pFiler->readSoftPointerId(&railId)) != Acad::eOk) return es;
                m_linkedRailStoneIds.append(railId);
            }
        }
    }

    m_profileType = static_cast<RingProfileType>(std::max(0, std::min(3, static_cast<int>(profileValue))));
    m_hasStone = hasStoneValue != 0;
    m_settingType = static_cast<SettingType>(std::max(0, std::min(2, static_cast<int>(settingTypeValue))));
    m_prongCount = std::max(3, prongCountValue);
    m_railStoneCount = std::max(0, railStoneCountValue);
    m_prongRadius = clampMin(m_prongRadius, 0.1);
    m_prongHeight = clampMin(m_prongHeight, 0.1);
    m_stoneShapeType = static_cast<StoneShapeType>(std::max(0, std::min(3, static_cast<int>(stoneShapeValue))));
    m_linkedStoneId = linkedStoneIdValue;

    return pFiler->filerStatus();
}

Acad::ErrorStatus CRingEntity::dwgOutFields(AcDbDwgFiler* pFiler) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = AcDbEntity::dwgOutFields(pFiler);
    if (es != Acad::eOk)
        return es;

    if ((es = pFiler->writeUInt32(kCurrentVersionNumber)) != Acad::eOk) return es;
    if ((es = pFiler->writePoint3d(m_center)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_innerRadius)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_bandWidth)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_thickness)) != Acad::eOk) return es;
    if ((es = pFiler->writeInt32(static_cast<Adesk::Int32>(m_profileType))) != Acad::eOk) return es;
    if ((es = pFiler->writePoint3d(m_stoneCenter)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_stoneSeatRadius)) != Acad::eOk) return es;
    if ((es = pFiler->writeInt16(m_hasStone ? 1 : 0)) != Acad::eOk) return es;
    if ((es = pFiler->writeInt16(static_cast<Adesk::Int16>(m_settingType))) != Acad::eOk) return es;
    if ((es = pFiler->writeInt32(m_prongCount)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_prongRadius)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_prongHeight)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_stoneLength)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_stoneWidth)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_stoneHeight)) != Acad::eOk) return es;
    if ((es = pFiler->writeInt32(m_railStoneCount)) != Acad::eOk) return es;
    if ((es = pFiler->writeInt16(static_cast<Adesk::Int16>(m_stoneShapeType))) != Acad::eOk) return es;
    if ((es = pFiler->writeSoftPointerId(m_linkedStoneId)) != Acad::eOk) return es;
    if ((es = pFiler->writeInt32(m_linkedRailStoneIds.length())) != Acad::eOk) return es;
    for (int i = 0; i < m_linkedRailStoneIds.length(); ++i)
    {
        if ((es = pFiler->writeSoftPointerId(m_linkedRailStoneIds[i])) != Acad::eOk) return es;
    }

    return pFiler->filerStatus();
}

Acad::ErrorStatus CRingEntity::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = AcDbEntity::dxfOutFields(pFiler);
    if (es != Acad::eOk)
        return es;

    pFiler->writeItem(AcDb::kDxfSubclass, L"CRingEntity");
    pFiler->writeUInt32(AcDb::kDxfInt32, kCurrentVersionNumber);
    pFiler->writePoint3d(AcDb::kDxfXCoord, m_center);
    pFiler->writeDouble(AcDb::kDxfReal, m_innerRadius);
    pFiler->writeDouble(AcDb::kDxfReal + 1, m_bandWidth);
    pFiler->writeDouble(AcDb::kDxfReal + 2, m_thickness);
    pFiler->writeInt32(AcDb::kDxfInt16, static_cast<Adesk::Int32>(m_profileType));
    pFiler->writePoint3d(AcDb::kDxfXCoord + 1, m_stoneCenter);
    pFiler->writeDouble(AcDb::kDxfReal + 3, m_stoneSeatRadius);
    pFiler->writeInt16(AcDb::kDxfInt16 + 1, m_hasStone ? 1 : 0);
    pFiler->writeInt16(AcDb::kDxfInt16 + 2, static_cast<Adesk::Int16>(m_settingType));
    pFiler->writeInt32(AcDb::kDxfInt16 + 3, m_prongCount);
    pFiler->writeDouble(AcDb::kDxfReal + 4, m_prongRadius);
    pFiler->writeDouble(AcDb::kDxfReal + 5, m_prongHeight);
    pFiler->writeDouble(AcDb::kDxfReal + 6, m_stoneLength);
    pFiler->writeDouble(AcDb::kDxfReal + 7, m_stoneWidth);
    pFiler->writeDouble(AcDb::kDxfReal + 8, m_stoneHeight);
    pFiler->writeInt32(AcDb::kDxfInt16 + 4, m_railStoneCount);
    pFiler->writeInt16(AcDb::kDxfInt16 + 5, static_cast<Adesk::Int16>(m_stoneShapeType));

    return pFiler->filerStatus();
}

Acad::ErrorStatus CRingEntity::dxfInFields(AcDbDxfFiler* pFiler)
{
    assertWriteEnabled();

    Acad::ErrorStatus es = AcDbEntity::dxfInFields(pFiler);
    if (es != Acad::eOk)
        return es;

    resbuf rb;
    while ((es = pFiler->readResBuf(&rb)) == Acad::eOk)
    {
        switch (rb.restype)
        {
        case AcDb::kDxfSubclass:
            if (wcscmp(rb.resval.rstring, L"CRingEntity") == 0)
                continue;
            break;

        case AcDb::kDxfXCoord:
            m_center = asPnt3d(rb.resval.rpoint);
            break;

        case AcDb::kDxfXCoord + 1:
            m_stoneCenter = asPnt3d(rb.resval.rpoint);
            break;

        case AcDb::kDxfReal:
            m_innerRadius = rb.resval.rreal;
            break;

        case AcDb::kDxfReal + 1:
            m_bandWidth = rb.resval.rreal;
            break;

        case AcDb::kDxfReal + 2:
            m_thickness = rb.resval.rreal;
            break;

        case AcDb::kDxfReal + 3:
            m_stoneSeatRadius = rb.resval.rreal;
            break;

        case AcDb::kDxfReal + 4:
            m_prongRadius = rb.resval.rreal;
            break;

        case AcDb::kDxfReal + 5:
            m_prongHeight = rb.resval.rreal;
            break;

        case AcDb::kDxfReal + 6:
            m_stoneLength = rb.resval.rreal;
            break;

        case AcDb::kDxfReal + 7:
            m_stoneWidth = rb.resval.rreal;
            break;

        case AcDb::kDxfReal + 8:
            m_stoneHeight = rb.resval.rreal;
            break;

        case AcDb::kDxfInt16:
            m_profileType = static_cast<RingProfileType>(
                std::max(0, std::min(3, static_cast<int>(rb.resval.rint))));
            break;

        case AcDb::kDxfInt16 + 1:
            m_hasStone = rb.resval.rint != 0;
            break;

        case AcDb::kDxfInt16 + 2:
            m_settingType = static_cast<SettingType>(std::max(0, std::min(2, static_cast<int>(rb.resval.rint))));
            break;

        case AcDb::kDxfInt16 + 3:
            m_prongCount = std::max(3, rb.resval.rlong);
            break;

        case AcDb::kDxfInt16 + 4:
            m_railStoneCount = std::max(0, rb.resval.rlong);
            break;

        case AcDb::kDxfInt16 + 5:
            m_stoneShapeType = static_cast<StoneShapeType>(
                std::max(0, std::min(3, static_cast<int>(rb.resval.rint))));
            break;

        default:
            pFiler->pushBackItem();
            es = Acad::eEndOfFile;
            break;
        }

        if (es == Acad::eEndOfFile)
            break;
    }

    m_prongRadius = clampMin(m_prongRadius, 0.1);
    m_prongHeight = clampMin(m_prongHeight, 0.1);
    m_railStoneCount = std::max(0, m_railStoneCount);
    return pFiler->filerStatus();
}
