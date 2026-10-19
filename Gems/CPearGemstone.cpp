#include "pch.h"
#include "CPearGemstone.h"

#include <cmath>

Adesk::UInt32 CPearGemstone::kCurrentVersionNumber = 1;

ACRX_DXF_DEFINE_MEMBERS(
    CPearGemstone,
    CJewelryGemstone,
    AcDb::kDHL_CURRENT,
    AcDb::kMReleaseCurrent,
    AcDbProxyEntity::kAllAllowedBits,
    CPEARGEMSTONE,
    L"VKR_NRX"
);

CPearGemstone::CPearGemstone()
    : CJewelryGemstone()
    , m_length(8.0)
    , m_width(5.0)
    , m_totalDepth(3.0)
    , m_crownHeight(0.0)
    , m_pavilionDepth(0.0)
    , m_girdleThickness(0.0)
    , m_tablePercent(TABLE_SIZE_PERCENT)
    , m_crownAngle(34.5)
    , m_pavilionAngle(40.6)
    , m_culetPercent(0.0)
{
    calculateDerivedParameters();
}

CPearGemstone::CPearGemstone(
    const AcGePoint3d& center,
    double length,
    double width,
    double totalDepth)
    : CJewelryGemstone(center)
    , m_length(length)
    , m_width(width)
    , m_totalDepth(totalDepth)
    , m_crownHeight(0.0)
    , m_pavilionDepth(0.0)
    , m_girdleThickness(0.0)
    , m_tablePercent(TABLE_SIZE_PERCENT)
    , m_crownAngle(34.5)
    , m_pavilionAngle(40.6)
    , m_culetPercent(0.0)
{
    calculateDerivedParameters();
}

CPearGemstone::~CPearGemstone()
{
}

Acad::ErrorStatus CPearGemstone::setLength(double length)
{
    assertWriteEnabled();

    if (length <= 0.0)
        return Acad::eInvalidInput;

    m_length = length;
    calculateDerivedParameters();
    return Acad::eOk;
}

Acad::ErrorStatus CPearGemstone::setWidth(double width)
{
    assertWriteEnabled();

    if (width <= 0.0)
        return Acad::eInvalidInput;

    m_width = width;
    calculateDerivedParameters();
    return Acad::eOk;
}

Acad::ErrorStatus CPearGemstone::setTotalDepth(double depth)
{
    assertWriteEnabled();

    if (depth <= 0.0)
        return Acad::eInvalidInput;

    m_totalDepth = depth;
    calculateDerivedParameters();
    return Acad::eOk;
}

void CPearGemstone::calculateDerivedParameters()
{
    m_crownHeight = m_totalDepth * (CROWN_HEIGHT_PERCENT / 100.0);
    m_pavilionDepth = m_totalDepth * (PAVILION_DEPTH_PERCENT / 100.0);
    m_girdleThickness = m_totalDepth * (GIRDLE_PERCENT / 100.0);
}

double CPearGemstone::lengthToWidthRatio() const
{
    return (m_width > 0.0) ? (m_length / m_width) : 0.0;
}

double CPearGemstone::depthPercentage() const
{
    return (m_width > 0.0) ? (m_totalDepth / m_width) * 100.0 : 0.0;
}

bool CPearGemstone::isProportionIdeal() const
{
    const double lwRatio = lengthToWidthRatio();
    const double depthPercent = depthPercentage();

    const bool ratioOK = (lwRatio >= 1.45 && lwRatio <= 1.55);
    const bool depthOK = (depthPercent >= 58.0 && depthPercent <= 64.0);

    return ratioOK && depthOK;
}

int CPearGemstone::facetCount() const
{
    assertReadEnabled();
    return 58;
}

double CPearGemstone::volume() const
{
    assertReadEnabled();
    return m_length * m_width * m_totalDepth * 0.436;
}

double CPearGemstone::caratWeight() const
{
    assertReadEnabled();

    const double volumeMM3 = volume();
    const double volumeCM3 = volumeMM3 / 1000.0;
    const double gramWeight = volumeCM3 * 3.52;

    return gramWeight * 5.0;
}

AcString CPearGemstone::cutName() const
{
    assertReadEnabled();
    return L"Pear";
}

const wchar_t* CPearGemstone::typeId() const
{
    return L"pear";
}

CJewelryGemstone::OutlineShape CPearGemstone::outlineShape() const
{
    return kOutlinePear;
}

AcCmColor CPearGemstone::defaultColor() const
{
    AcCmColor color;
    color.setRGB(220, 20, 60);
    return color;
}

void CPearGemstone::getDimensions(double& length, double& width, double& height) const
{
    assertReadEnabled();
    length = m_length;
    width = m_width;
    height = m_totalDepth;
}

Acad::ErrorStatus CPearGemstone::setDimensions(double length, double width, double height)
{
    Acad::ErrorStatus es = setLength(length);
    if (es == Acad::eOk)
        es = setWidth(width);
    if (es == Acad::eOk)
        es = setTotalDepth(height);
    return es;
}

void CPearGemstone::createPearProfile(AcGePoint3dArray& points) const
{
    points.setLogicalLength(0);

    const int numPoints = 48;
    const double halfWidth = m_width / 2.0;

    struct KeyPoint
    {
        double x;
        double y;
    };

    KeyPoint keyPoints[12];

    keyPoints[0] = { -m_length * 0.40, 0.0 };
    keyPoints[1] = { -m_length * 0.30, halfWidth * 0.85 };
    keyPoints[2] = { -m_length * 0.10, halfWidth };
    keyPoints[3] = { m_length * 0.10, halfWidth * 0.90 };
    keyPoints[4] = { m_length * 0.25, halfWidth * 0.65 };
    keyPoints[5] = { m_length * 0.38, halfWidth * 0.35 };
    keyPoints[6] = { m_length * 0.50, 0.0 };
    keyPoints[7] = { m_length * 0.38, -halfWidth * 0.35 };
    keyPoints[8] = { m_length * 0.25, -halfWidth * 0.65 };
    keyPoints[9] = { m_length * 0.10, -halfWidth * 0.90 };
    keyPoints[10] = { -m_length * 0.10, -halfWidth };
    keyPoints[11] = { -m_length * 0.30, -halfWidth * 0.85 };

    const int segments = 12;
    const int pointsPerSegment = numPoints / segments;

    for (int i = 0; i < segments; ++i)
    {
        const int i0 = (i == 0) ? (segments - 1) : (i - 1);
        const int i1 = i;
        const int i2 = (i + 1) % segments;
        const int i3 = (i + 2) % segments;

        for (int j = 0; j < pointsPerSegment; ++j)
        {
            double t = static_cast<double>(j) / pointsPerSegment;
            double t2 = t * t;
            double t3 = t2 * t;

            double x = 0.5 * (
                (2.0 * keyPoints[i1].x) +
                (-keyPoints[i0].x + keyPoints[i2].x) * t +
                (2.0 * keyPoints[i0].x - 5.0 * keyPoints[i1].x + 4.0 * keyPoints[i2].x - keyPoints[i3].x) * t2 +
                (-keyPoints[i0].x + 3.0 * keyPoints[i1].x - 3.0 * keyPoints[i2].x + keyPoints[i3].x) * t3
                );

            double y = 0.5 * (
                (2.0 * keyPoints[i1].y) +
                (-keyPoints[i0].y + keyPoints[i2].y) * t +
                (2.0 * keyPoints[i0].y - 5.0 * keyPoints[i1].y + 4.0 * keyPoints[i2].y - keyPoints[i3].y) * t2 +
                (-keyPoints[i0].y + 3.0 * keyPoints[i1].y - 3.0 * keyPoints[i2].y + keyPoints[i3].y) * t3
                );

            points.append(AcGePoint3d(m_center.x + x, m_center.y + y, m_center.z));
        }
    }
}

void CPearGemstone::buildGemGeometry(
    AcGePoint3dArray& vertices,
    AcGeVector3dArray& normals,
    AcArray<Adesk::Int32>& faceList) const
{
    assertReadEnabled();

    vertices.setLogicalLength(0);
    normals.setLogicalLength(0);
    faceList.setLogicalLength(0);

    AcGePoint3dArray girdlePoints;
    createPearProfile(girdlePoints);

    const int numMainFacets = 8;
    const int numGirdlePoints = girdlePoints.length();

    const double tableZ = m_center.z + m_crownHeight;
    const double culetZ = m_center.z - m_pavilionDepth;

    const int tableStart = vertices.length();
    for (int i = 0; i < numMainFacets; ++i)
    {
        int girdleIdx = (numGirdlePoints * i) / numMainFacets;
        AcGePoint3d gPt = girdlePoints[girdleIdx];

        AcGeVector3d dir = gPt - m_center;
        dir.z = 0.0;

        double scale = m_tablePercent / 100.0;
        AcGePoint3d tablePt = m_center + dir * scale;
        tablePt.z = tableZ;

        vertices.append(tablePt);
        normals.append(AcGeVector3d(0, 0, 1));
    }

    const int girdleStart = vertices.length();
    for (int i = 0; i < numGirdlePoints; ++i)
    {
        vertices.append(girdlePoints[i]);

        AcGeVector3d normal = girdlePoints[i] - m_center;
        normal.z = 0.0;
        normal.normalize();
        normals.append(normal);
    }

    faceList.append(numMainFacets);
    for (int i = 0; i < numMainFacets; ++i)
        faceList.append(tableStart + i);

    const int pointsPerSection = numGirdlePoints / numMainFacets;

    for (int section = 0; section < numMainFacets; ++section)
    {
        int tableIdx1 = tableStart + section;
        int tableIdx2 = tableStart + ((section + 1) % numMainFacets);

        int girdleStartIdx = girdleStart + (section * pointsPerSection);
        int girdleEndIdx = girdleStart + (((section + 1) * pointsPerSection) % numGirdlePoints);

        for (int j = 0; j < pointsPerSection; ++j)
        {
            int g1 = girdleStartIdx + j;
            int g2 = (j == pointsPerSection - 1) ? girdleEndIdx : (g1 + 1);

            if (j < pointsPerSection / 2)
            {
                faceList.append(3);
                faceList.append(tableIdx1);
                faceList.append(g1);
                faceList.append(g2);
            }
            else
            {
                faceList.append(3);
                faceList.append(tableIdx2);
                faceList.append(g1);
                faceList.append(g2);
            }
        }

        int midGirdleIdx = girdleStartIdx + pointsPerSection / 2;

        faceList.append(3);
        faceList.append(tableIdx1);
        faceList.append(midGirdleIdx);
        faceList.append(tableIdx2);
    }

    const int culetIndex = vertices.length();
    vertices.append(AcGePoint3d(m_center.x, m_center.y, culetZ));
    normals.append(AcGeVector3d(0, 0, -1));

    for (int i = 0; i < numGirdlePoints; ++i)
    {
        int next = (i + 1) % numGirdlePoints;

        faceList.append(3);
        faceList.append(girdleStart + i);
        faceList.append(culetIndex);
        faceList.append(girdleStart + next);
    }
}

Acad::ErrorStatus CPearGemstone::subGetGripPoints(
    AcGePoint3dArray& gripPoints,
    AcDbIntArray& osnapModes,
    AcDbIntArray& geomIds) const
{
    assertReadEnabled();

    const double halfWidth = m_width / 2.0;
    const double widestX = m_length * WIDEST_OFFSET;

    gripPoints.append(m_center);
    gripPoints.append(m_center + AcGeVector3d(m_length * TIP_OFFSET, 0.0, 0.0));
    gripPoints.append(m_center + AcGeVector3d(widestX, halfWidth, 0.0));
    gripPoints.append(m_center + AcGeVector3d(0.0, 0.0, m_crownHeight));
    gripPoints.append(m_center + AcGeVector3d(0.0, 0.0, -m_pavilionDepth));
    gripPoints.append(m_center + AcGeVector3d(m_length * ROUND_END_OFFSET, 0.0, 0.0));
    gripPoints.append(m_center + AcGeVector3d(widestX, -halfWidth, 0.0));

    return Acad::eOk;
}

Acad::ErrorStatus CPearGemstone::subMoveGripPointsAt(
    const AcDbIntArray& indices,
    const AcGeVector3d& offset)
{
    assertWriteEnabled();

    for (int i = 0; i < indices.length(); ++i)
    {
        switch (indices[i])
        {
        case kGripCenter:
            m_center += offset;
            break;

        case kGripTip:
        case kGripRoundEnd:
        {
            double fraction = TIP_OFFSET;
            if (indices[i] == kGripRoundEnd)
                fraction = ROUND_END_OFFSET;

            const double newLength = m_length + offset.x / fraction;
            if (newLength > 1.0 && newLength < 50.0)
                m_length = newLength;
            break;
        }

        case kGripWidthPos:
        case kGripWidthNeg:
        {
            const double newWidth = m_width + (indices[i] == kGripWidthPos ? 2.0 : -2.0) * offset.y;
            if (newWidth > 0.5 && newWidth < 30.0)
                m_width = newWidth;
            break;
        }

        case kGripCrownTop:
        {
            const double newCrownHeight = m_crownHeight + offset.z;
            if (newCrownHeight > 0.1 && newCrownHeight < 10.0)
            {
                m_totalDepth += newCrownHeight - m_crownHeight;
                m_crownHeight = newCrownHeight;
            }
            break;
        }

        case kGripCulet:
        {
            const double newPavilionDepth = m_pavilionDepth - offset.z;
            if (newPavilionDepth > 0.1 && newPavilionDepth < 20.0)
            {
                m_totalDepth += newPavilionDepth - m_pavilionDepth;
                m_pavilionDepth = newPavilionDepth;
            }
            break;
        }

        default:
            break;
        }
    }

    return Acad::eOk;
}

Acad::ErrorStatus CPearGemstone::subTransformBy(const AcGeMatrix3d& xform)
{
    assertWriteEnabled();
    m_center.transformBy(xform);
    return Acad::eOk;
}

Acad::ErrorStatus CPearGemstone::subGetGeomExtents(AcDbExtents& extents) const
{
    assertReadEnabled();

    double halfLength = m_length / 2.0;
    double halfWidth = m_width / 2.0;

    AcGePoint3d minPt(
        m_center.x - halfLength,
        m_center.y - halfWidth,
        m_center.z - m_pavilionDepth);

    AcGePoint3d maxPt(
        m_center.x + halfLength,
        m_center.y + halfWidth,
        m_center.z + m_crownHeight);

    extents.set(minPt, maxPt);
    return Acad::eOk;
}

Acad::ErrorStatus CPearGemstone::dwgOutFields(AcDbDwgFiler* pFiler) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = CJewelryGemstone::dwgOutFields(pFiler);
    if (es != Acad::eOk)
        return es;

    pFiler->writeUInt32(kCurrentVersionNumber);
    pFiler->writeDouble(m_length);
    pFiler->writeDouble(m_width);
    pFiler->writeDouble(m_totalDepth);
    pFiler->writeDouble(m_crownHeight);
    pFiler->writeDouble(m_pavilionDepth);

    return Acad::eOk;
}

Acad::ErrorStatus CPearGemstone::dwgInFields(AcDbDwgFiler* pFiler)
{
    assertWriteEnabled();

    Acad::ErrorStatus es = CJewelryGemstone::dwgInFields(pFiler);
    if (es != Acad::eOk)
        return es;

    Adesk::UInt32 version = 0;
    pFiler->readUInt32(&version);
    if (version > kCurrentVersionNumber)
        return Acad::eMakeMeProxy;

    pFiler->readDouble(&m_length);
    pFiler->readDouble(&m_width);
    pFiler->readDouble(&m_totalDepth);
    pFiler->readDouble(&m_crownHeight);
    pFiler->readDouble(&m_pavilionDepth);

    m_girdleThickness = m_totalDepth * (GIRDLE_PERCENT / 100.0);

    return Acad::eOk;
}

Acad::ErrorStatus CPearGemstone::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = CJewelryGemstone::dxfOutFields(pFiler);
    if (es != Acad::eOk)
        return es;

    pFiler->writeItem(AcDb::kDxfSubclass, L"CPearGemstone");
    pFiler->writeDouble(40, m_length);
    pFiler->writeDouble(41, m_width);
    pFiler->writeDouble(42, m_totalDepth);
    pFiler->writeDouble(43, m_crownHeight);
    pFiler->writeDouble(44, m_pavilionDepth);

    return Acad::eOk;
}

Acad::ErrorStatus CPearGemstone::dxfInFields(AcDbDxfFiler* pFiler)
{
    assertWriteEnabled();

    Acad::ErrorStatus es = CJewelryGemstone::dxfInFields(pFiler);
    if (es != Acad::eOk)
        return es;

    struct resbuf rb;
    while ((es = pFiler->readResBuf(&rb)) == Acad::eOk)
    {
        switch (rb.restype)
        {
        case AcDb::kDxfSubclass:
            if (wcscmp(rb.resval.rstring, L"CPearGemstone") == 0)
                continue;
            break;

        case 40:
            m_length = rb.resval.rreal;
            break;

        case 41:
            m_width = rb.resval.rreal;
            break;

        case 42:
            m_totalDepth = rb.resval.rreal;
            break;

        case 43:
            m_crownHeight = rb.resval.rreal;
            break;

        case 44:
            m_pavilionDepth = rb.resval.rreal;
            break;

        default:
            pFiler->pushBackItem();
            es = Acad::eEndOfFile;
            break;
        }

        if (es == Acad::eEndOfFile)
            break;
    }

    if (es != Acad::eEndOfFile)
        return Acad::eInvalidResBuf;

    calculateDerivedParameters();

    return pFiler->filerStatus();
}
