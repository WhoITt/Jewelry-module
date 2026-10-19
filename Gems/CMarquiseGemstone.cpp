#include "pch.h"
#include "CMarquiseGemstone.h"

#include <cmath>

Adesk::UInt32 CMarquiseGemstone::kCurrentVersionNumber = 1;

ACRX_DXF_DEFINE_MEMBERS(
    CMarquiseGemstone,
    CJewelryGemstone,
    AcDb::kDHL_CURRENT,
    AcDb::kMReleaseCurrent,
    AcDbProxyEntity::kAllAllowedBits,
    MARQUISEGEMSTONE,
    L"VKR_NRX"
);

CMarquiseGemstone::CMarquiseGemstone()
    : CJewelryGemstone()
    , m_length(9.0)
    , m_width(4.5)
    , m_totalDepth(3.0)
    , m_crownHeight(0.0)
    , m_pavilionDepth(0.0)
    , m_girdleThickness(0.0)
    , m_tablePercent(DEFAULT_TABLE_PERCENT)
{
    calculateDerivedParameters();
}

CMarquiseGemstone::CMarquiseGemstone(
    const AcGePoint3d& center,
    double length,
    double width,
    double totalDepth)
    : CJewelryGemstone(center)
    , m_length(length > 0.0 ? length : 9.0)
    , m_width(width > 0.0 ? width : 4.5)
    , m_totalDepth(totalDepth > 0.0 ? totalDepth : 3.0)
    , m_crownHeight(0.0)
    , m_pavilionDepth(0.0)
    , m_girdleThickness(0.0)
    , m_tablePercent(DEFAULT_TABLE_PERCENT)
{
    calculateDerivedParameters();
}

CMarquiseGemstone::~CMarquiseGemstone()
{
}

Acad::ErrorStatus CMarquiseGemstone::setLength(double length)
{
    assertWriteEnabled();

    if (length <= 0.0)
        return Acad::eInvalidInput;

    m_length = length;
    calculateDerivedParameters();
    return Acad::eOk;
}

Acad::ErrorStatus CMarquiseGemstone::setWidth(double width)
{
    assertWriteEnabled();

    if (width <= 0.0)
        return Acad::eInvalidInput;

    m_width = width;
    calculateDerivedParameters();
    return Acad::eOk;
}

Acad::ErrorStatus CMarquiseGemstone::setTotalDepth(double depth)
{
    assertWriteEnabled();

    if (depth <= 0.0)
        return Acad::eInvalidInput;

    m_totalDepth = depth;
    calculateDerivedParameters();
    return Acad::eOk;
}

Acad::ErrorStatus CMarquiseGemstone::setTablePercent(double percent)
{
    assertWriteEnabled();

    if (percent < MIN_TABLE_PERCENT || percent > MAX_TABLE_PERCENT)
        return Acad::eInvalidInput;

    m_tablePercent = percent;
    return Acad::eOk;
}

void CMarquiseGemstone::setTablePercentClamped(double percent)
{
    if (percent < MIN_TABLE_PERCENT)
        percent = MIN_TABLE_PERCENT;
    else if (percent > MAX_TABLE_PERCENT)
        percent = MAX_TABLE_PERCENT;

    m_tablePercent = percent;
}

void CMarquiseGemstone::calculateDerivedParameters()
{
    m_crownHeight = m_totalDepth * (CROWN_HEIGHT_PERCENT / 100.0);
    m_pavilionDepth = m_totalDepth * (PAVILION_DEPTH_PERCENT / 100.0);
    m_girdleThickness = m_totalDepth * (GIRDLE_PERCENT / 100.0);
}

double CMarquiseGemstone::lengthToWidthRatio() const
{
    return (m_width > 0.0) ? (m_length / m_width) : 0.0;
}

double CMarquiseGemstone::depthPercentage() const
{
    return (m_width > 0.0) ? (m_totalDepth / m_width) * 100.0 : 0.0;
}

bool CMarquiseGemstone::isProportionIdeal() const
{
    const double lwRatio = lengthToWidthRatio();
    const double depthPercent = depthPercentage();

    const bool ratioOK = (lwRatio >= 1.75 && lwRatio <= 2.25);
    const bool depthOK = (depthPercent >= 58.0 && depthPercent <= 65.0);

    return ratioOK && depthOK;
}

int CMarquiseGemstone::facetCount() const
{
    assertReadEnabled();
    return 58;
}

double CMarquiseGemstone::volume() const
{
    assertReadEnabled();
    return m_length * m_width * m_totalDepth * 0.52;
}

double CMarquiseGemstone::caratWeight() const
{
    assertReadEnabled();
    return m_length * m_width * m_totalDepth * 0.00565;
}

AcString CMarquiseGemstone::cutName() const
{
    assertReadEnabled();
    return L"Marquise";
}

const wchar_t* CMarquiseGemstone::typeId() const
{
    return L"marquise";
}

CJewelryGemstone::OutlineShape CMarquiseGemstone::outlineShape() const
{
    return kOutlineOval;
}

AcCmColor CMarquiseGemstone::defaultColor() const
{
    AcCmColor color;
    color.setRGB(255, 215, 0);
    return color;
}

void CMarquiseGemstone::getDimensions(double& length, double& width, double& height) const
{
    assertReadEnabled();
    length = m_length;
    width = m_width;
    height = m_totalDepth;
}

Acad::ErrorStatus CMarquiseGemstone::setDimensions(double length, double width, double height)
{
    Acad::ErrorStatus es = setLength(length);
    if (es == Acad::eOk)
        es = setWidth(width);
    if (es == Acad::eOk)
        es = setTotalDepth(height);
    return es;
}

void CMarquiseGemstone::createMarquiseProfile(
    AcGePoint3dArray& points,
    double scale,
    double z) const
{
    points.setLogicalLength(0);

    const double halfWidth = (m_width * scale) / 2.0;
    const double halfLength = (m_length * scale) / 2.0;

    const int numPoints = 16;

    for (int i = 0; i < numPoints; ++i)
    {
        double t = static_cast<double>(i) / numPoints;
        double angle = t * 2.0 * 3.14159265358979323846;

        double x = halfLength * std::cos(angle);
        double y = halfWidth * std::sin(angle);

        double sharpFactor = 1.0 - 0.25 * std::fabs(std::sin(angle));
        x *= sharpFactor;

        points.append(AcGePoint3d(m_center.x + x, m_center.y + y, z));
    }
}

void CMarquiseGemstone::buildGemGeometry(
    AcGePoint3dArray& vertices,
    AcGeVector3dArray& normals,
    AcArray<Adesk::Int32>& faceList) const
{
    assertReadEnabled();

    vertices.setLogicalLength(0);
    normals.setLogicalLength(0);
    faceList.setLogicalLength(0);

    const double girdleZ = m_center.z;
    const double tableZ = m_center.z + m_crownHeight;
    const double culetZ = m_center.z - m_pavilionDepth;

    const double halfWidth = m_width / 2.0;
    const double halfLength = m_length / 2.0;
    const double tableScale = m_tablePercent / 100.0;

    const int tableStart = vertices.length();

    AcGePoint3d tablePts[8];
    tablePts[0] = AcGePoint3d(m_center.x + halfLength * tableScale * 0.9, m_center.y, tableZ);
    tablePts[1] = AcGePoint3d(m_center.x + halfLength * tableScale * 0.5, m_center.y + halfWidth * tableScale * 0.9, tableZ);
    tablePts[2] = AcGePoint3d(m_center.x, m_center.y + halfWidth * tableScale, tableZ);
    tablePts[3] = AcGePoint3d(m_center.x - halfLength * tableScale * 0.5, m_center.y + halfWidth * tableScale * 0.9, tableZ);
    tablePts[4] = AcGePoint3d(m_center.x - halfLength * tableScale * 0.9, m_center.y, tableZ);
    tablePts[5] = AcGePoint3d(m_center.x - halfLength * tableScale * 0.5, m_center.y - halfWidth * tableScale * 0.9, tableZ);
    tablePts[6] = AcGePoint3d(m_center.x, m_center.y - halfWidth * tableScale, tableZ);
    tablePts[7] = AcGePoint3d(m_center.x + halfLength * tableScale * 0.5, m_center.y - halfWidth * tableScale * 0.9, tableZ);

    for (int i = 0; i < 8; ++i)
    {
        vertices.append(tablePts[i]);
        normals.append(AcGeVector3d(0, 0, 1));
    }

    const int girdleStart = vertices.length();
    const int numGirdlePoints = 16;

    for (int i = 0; i < numGirdlePoints; ++i)
    {
        double t = static_cast<double>(i) / numGirdlePoints;
        double angle = t * 2.0 * 3.14159265358979323846;

        double ellipseX = std::cos(angle);
        double ellipseY = std::sin(angle);

        double sharpFactor = 1.0 - 0.3 * std::fabs(std::sin(angle));

        double x = halfLength * ellipseX * sharpFactor;
        double y = halfWidth * ellipseY;

        AcGePoint3d girdlePt(m_center.x + x, m_center.y + y, girdleZ);
        vertices.append(girdlePt);

        AcGeVector3d normal(x, y, 0.0);
        normal.normalize();
        normals.append(normal);
    }

    const int culetStart = vertices.length();

    const double culetScale = 0.05;
    AcGePoint3d culetPts[4];
    culetPts[0] = AcGePoint3d(m_center.x + halfLength * culetScale, m_center.y, culetZ);
    culetPts[1] = AcGePoint3d(m_center.x, m_center.y + halfWidth * culetScale, culetZ);
    culetPts[2] = AcGePoint3d(m_center.x - halfLength * culetScale, m_center.y, culetZ);
    culetPts[3] = AcGePoint3d(m_center.x, m_center.y - halfWidth * culetScale, culetZ);

    for (int i = 0; i < 4; ++i)
    {
        vertices.append(culetPts[i]);
        normals.append(AcGeVector3d(0, 0, -1));
    }

    faceList.append(8);
    for (int i = 0; i < 8; ++i)
        faceList.append(tableStart + i);

    for (int i = 0; i < 8; ++i)
    {
        int nextTable = (i + 1) % 8;

        int girdleStart1 = girdleStart + (i * 2);
        int girdleStart2 = girdleStart + (i * 2 + 1);
        int girdleEnd = girdleStart + ((i + 1) * 2) % numGirdlePoints;

        faceList.append(3);
        faceList.append(tableStart + i);
        faceList.append(girdleStart1);
        faceList.append(girdleStart2);

        faceList.append(3);
        faceList.append(tableStart + i);
        faceList.append(girdleStart2);
        faceList.append(tableStart + nextTable);

        faceList.append(3);
        faceList.append(tableStart + nextTable);
        faceList.append(girdleStart2);
        faceList.append(girdleEnd);
    }

    for (int culetIdx = 0; culetIdx < 4; ++culetIdx)
    {
        int nextCulet = (culetIdx + 1) % 4;
        int startGirdleIdx = culetIdx * 4;

        for (int j = 0; j < 4; ++j)
        {
            int g1 = girdleStart + ((startGirdleIdx + j) % numGirdlePoints);
            int g2 = girdleStart + ((startGirdleIdx + j + 1) % numGirdlePoints);

            int targetCulet = culetStart + culetIdx;

            if (j == 3)
            {
                faceList.append(4);
                faceList.append(g1);
                faceList.append(g2);
                faceList.append(culetStart + nextCulet);
                faceList.append(culetStart + culetIdx);
            }
            else
            {
                faceList.append(3);
                faceList.append(g1);
                faceList.append(g2);
                faceList.append(targetCulet);
            }
        }
    }

    faceList.append(4);
    for (int i = 3; i >= 0; --i)
        faceList.append(culetStart + i);
}

Acad::ErrorStatus CMarquiseGemstone::subGetGripPoints(
    AcGePoint3dArray& gripPoints,
    AcDbIntArray& osnapModes,
    AcDbIntArray& geomIds) const
{
    assertReadEnabled();

    const double halfLength = m_length / 2.0;
    const double halfWidth = m_width / 2.0;
    const double tableScale = m_tablePercent / 100.0;
    const double tableTipX = halfLength * tableScale * TABLE_TIP_FACTOR;

    gripPoints.append(m_center);
    gripPoints.append(m_center + AcGeVector3d(halfLength, 0.0, 0.0));
    gripPoints.append(m_center + AcGeVector3d(0.0, halfWidth, 0.0));
    gripPoints.append(m_center + AcGeVector3d(0.0, 0.0, m_crownHeight));
    gripPoints.append(m_center + AcGeVector3d(0.0, 0.0, -m_pavilionDepth));
    gripPoints.append(m_center + AcGeVector3d(-halfLength, 0.0, 0.0));
    gripPoints.append(m_center + AcGeVector3d(0.0, -halfWidth, 0.0));
    gripPoints.append(m_center + AcGeVector3d(-tableTipX, 0.0, m_crownHeight));
    gripPoints.append(m_center + AcGeVector3d(tableTipX, 0.0, m_crownHeight));
    gripPoints.append(m_center + AcGeVector3d(0.0, -halfWidth * tableScale, m_crownHeight));
    gripPoints.append(m_center + AcGeVector3d(0.0, halfWidth * tableScale, m_crownHeight));

    return Acad::eOk;
}

Acad::ErrorStatus CMarquiseGemstone::subMoveGripPointsAt(
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

        case kGripLengthPos:
        case kGripLengthNeg:
        {
            const double newLength = m_length + (indices[i] == kGripLengthPos ? 2.0 : -2.0) * offset.x;
            if (newLength > 1.0)
                m_length = newLength;
            break;
        }

        case kGripWidthPos:
        case kGripWidthNeg:
        {
            const double newWidth = m_width + (indices[i] == kGripWidthPos ? 2.0 : -2.0) * offset.y;
            if (newWidth > 0.5)
                m_width = newWidth;
            break;
        }

        case kGripCrownTop:
        {
            const double newCrownHeight = m_crownHeight + offset.z;
            if (newCrownHeight > 0.1)
            {
                m_totalDepth += newCrownHeight - m_crownHeight;
                m_crownHeight = newCrownHeight;
            }
            break;
        }

        case kGripCulet:
        {
            const double newPavilionDepth = m_pavilionDepth - offset.z;
            if (newPavilionDepth > 0.1)
            {
                m_totalDepth += newPavilionDepth - m_pavilionDepth;
                m_pavilionDepth = newPavilionDepth;
            }
            break;
        }

        case kGripTableLengthNeg:
            setTablePercentClamped(m_tablePercent - offset.x / (m_length * TABLE_TIP_FACTOR) * 200.0);
            break;

        case kGripTableLengthPos:
            setTablePercentClamped(m_tablePercent + offset.x / (m_length * TABLE_TIP_FACTOR) * 200.0);
            break;

        case kGripTableWidthNeg:
            setTablePercentClamped(m_tablePercent - offset.y / m_width * 200.0);
            break;

        case kGripTableWidthPos:
            setTablePercentClamped(m_tablePercent + offset.y / m_width * 200.0);
            break;

        default:
            break;
        }
    }

    return Acad::eOk;
}

Acad::ErrorStatus CMarquiseGemstone::subTransformBy(const AcGeMatrix3d& xform)
{
    assertWriteEnabled();
    m_center.transformBy(xform);
    return Acad::eOk;
}

Acad::ErrorStatus CMarquiseGemstone::subGetGeomExtents(AcDbExtents& extents) const
{
    assertReadEnabled();

    AcGePoint3d minPt(
        m_center.x - m_length / 2.0,
        m_center.y - m_width / 2.0,
        m_center.z - m_pavilionDepth);

    AcGePoint3d maxPt(
        m_center.x + m_length / 2.0,
        m_center.y + m_width / 2.0,
        m_center.z + m_crownHeight);

    extents.set(minPt, maxPt);
    return Acad::eOk;
}

Acad::ErrorStatus CMarquiseGemstone::dwgOutFields(AcDbDwgFiler* pFiler) const
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
    pFiler->writeDouble(m_tablePercent);

    return Acad::eOk;
}

Acad::ErrorStatus CMarquiseGemstone::dwgInFields(AcDbDwgFiler* pFiler)
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
    pFiler->readDouble(&m_tablePercent);

    m_girdleThickness = m_totalDepth * (GIRDLE_PERCENT / 100.0);

    return Acad::eOk;
}

Acad::ErrorStatus CMarquiseGemstone::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = CJewelryGemstone::dxfOutFields(pFiler);
    if (es != Acad::eOk)
        return es;

    pFiler->writeItem(AcDb::kDxfSubclass, L"CMarquiseGemstone");
    pFiler->writeDouble(40, m_length);
    pFiler->writeDouble(41, m_width);
    pFiler->writeDouble(42, m_totalDepth);
    pFiler->writeDouble(43, m_crownHeight);
    pFiler->writeDouble(44, m_pavilionDepth);
    pFiler->writeDouble(45, m_tablePercent);

    return Acad::eOk;
}

Acad::ErrorStatus CMarquiseGemstone::dxfInFields(AcDbDxfFiler* pFiler)
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
            if (wcscmp(rb.resval.rstring, L"CMarquiseGemstone") == 0)
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

        case 45:
            m_tablePercent = rb.resval.rreal;
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
