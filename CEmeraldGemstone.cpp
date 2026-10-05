#include "pch.h"
#include "CEmeraldGemstone.h"

#include <cmath>

Adesk::UInt32 CEmeraldGemstone::kCurrentVersionNumber = 1;

ACRX_DXF_DEFINE_MEMBERS(
    CEmeraldGemstone,
    AcDbEntity,
    AcDb::kDHL_CURRENT,
    AcDb::kMReleaseCurrent,
    AcDbProxyEntity::kAllAllowedBits,
    EMERALDGEMSTONE,
    L"VKR_NRX"
);

CEmeraldGemstone::CEmeraldGemstone()
    : CJewelryGemstone()
    , m_length(7.0)
    , m_width(5.0)
    , m_totalDepth(3.5)
    , m_crownHeight(0.0)
    , m_pavilionDepth(0.0)
    , m_girdleThickness(0.0)
    , m_cornerCutPercent(DEFAULT_CORNER_CUT)
    , m_tablePercent(DEFAULT_TABLE_PERCENT)
    , m_crownSteps(3)
    , m_pavilionSteps(3)
{
    calculateDerivedParameters();
}

CEmeraldGemstone::CEmeraldGemstone(
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
    , m_cornerCutPercent(DEFAULT_CORNER_CUT)
    , m_tablePercent(DEFAULT_TABLE_PERCENT)
    , m_crownSteps(3)
    , m_pavilionSteps(3)
{
    calculateDerivedParameters();
}

CEmeraldGemstone::~CEmeraldGemstone()
{
}

Acad::ErrorStatus CEmeraldGemstone::setLength(double length)
{
    assertWriteEnabled();

    if (length <= 0.0)
        return Acad::eInvalidInput;

    m_length = length;
    calculateDerivedParameters();
    return Acad::eOk;
}

Acad::ErrorStatus CEmeraldGemstone::setWidth(double width)
{
    assertWriteEnabled();

    if (width <= 0.0)
        return Acad::eInvalidInput;

    m_width = width;
    calculateDerivedParameters();
    return Acad::eOk;
}

Acad::ErrorStatus CEmeraldGemstone::setTotalDepth(double depth)
{
    assertWriteEnabled();

    if (depth <= 0.0)
        return Acad::eInvalidInput;

    m_totalDepth = depth;
    calculateDerivedParameters();
    return Acad::eOk;
}

Acad::ErrorStatus CEmeraldGemstone::setCornerCut(double percent)
{
    assertWriteEnabled();

    if (percent < 10.0 || percent > 40.0)
        return Acad::eInvalidInput;

    m_cornerCutPercent = percent;
    return Acad::eOk;
}

Nano::ErrorStatus CEmeraldGemstone::setTablePercent(double percent)
{
    assertWriteEnabled();

    if (percent < 55.0 || percent > 75.0)
        return Nano::eInvalidInput;

    m_tablePercent = percent;
    return Nano::eOk;
}

void CEmeraldGemstone::calculateDerivedParameters()
{
    m_crownHeight = m_totalDepth * (CROWN_HEIGHT_PERCENT / 100.0);
    m_pavilionDepth = m_totalDepth * (PAVILION_DEPTH_PERCENT / 100.0);
    m_girdleThickness = m_totalDepth * (GIRDLE_PERCENT / 100.0);
}

double CEmeraldGemstone::lengthToWidthRatio() const
{
    return (m_width > 0.0) ? (m_length / m_width) : 0.0;
}

double CEmeraldGemstone::depthPercentage() const
{
    return (m_width > 0.0) ? (m_totalDepth / m_width) * 100.0 : 0.0;
}

bool CEmeraldGemstone::isProportionIdeal() const
{
    const double lwRatio = lengthToWidthRatio();
    const double depthPercent = depthPercentage();

    const bool ratioOK = (lwRatio >= 1.30 && lwRatio <= 1.50);
    const bool depthOK = (depthPercent >= 60.0 && depthPercent <= 70.0);

    return ratioOK && depthOK;
}

int CEmeraldGemstone::facetCount() const
{
    assertReadEnabled();
    return 50;
}

double CEmeraldGemstone::volume() const
{
    assertReadEnabled();
    return m_length * m_width * m_totalDepth * 0.82;
}

double CEmeraldGemstone::caratWeight() const
{
    assertReadEnabled();
    return m_length * m_width * m_totalDepth * 0.0092;
}

AcString CEmeraldGemstone::cutName() const
{
    assertReadEnabled();
    return L"Emerald";
}

void CEmeraldGemstone::createEmeraldProfile(
    AcGePoint3dArray& points,
    double width,
    double length,
    double z) const
{
    points.setLogicalLength(0);

    const double halfWidth = width / 2.0;
    const double halfLength = length / 2.0;
    const double cornerSize = halfWidth * (m_cornerCutPercent / 100.0);

    points.append(AcGePoint3d(m_center.x + halfLength - cornerSize, m_center.y + halfWidth, z));
    points.append(AcGePoint3d(m_center.x + halfLength, m_center.y + halfWidth - cornerSize, z));
    points.append(AcGePoint3d(m_center.x + halfLength, m_center.y - halfWidth + cornerSize, z));
    points.append(AcGePoint3d(m_center.x + halfLength - cornerSize, m_center.y - halfWidth, z));
    points.append(AcGePoint3d(m_center.x - halfLength + cornerSize, m_center.y - halfWidth, z));
    points.append(AcGePoint3d(m_center.x - halfLength, m_center.y - halfWidth + cornerSize, z));
    points.append(AcGePoint3d(m_center.x - halfLength, m_center.y + halfWidth - cornerSize, z));
    points.append(AcGePoint3d(m_center.x - halfLength + cornerSize, m_center.y + halfWidth, z));
}

void CEmeraldGemstone::createTableProfile(AcGePoint3dArray& points) const
{
    const double tableWidth = m_width * (m_tablePercent / 100.0);
    const double tableLength = m_length * (m_tablePercent / 100.0);
    const double tableZ = m_center.z + m_crownHeight;

    createEmeraldProfile(points, tableWidth, tableLength, tableZ);
}

void CEmeraldGemstone::createGirdleProfile(AcGePoint3dArray& points) const
{
    createEmeraldProfile(points, m_width, m_length, m_center.z);
}

void CEmeraldGemstone::buildGemGeometry(
    AcGePoint3dArray& vertices,
    AcGeVector3dArray& normals,
    AcArray<Adesk::Int32>& faceList) const
{
    assertReadEnabled();

    vertices.setLogicalLength(0);
    normals.setLogicalLength(0);
    faceList.setLogicalLength(0);

    const double girdleZ = m_center.z;
    const double culetZ = m_center.z - m_pavilionDepth;

    AcGePoint3dArray tablePoints;
    createTableProfile(tablePoints);

    const int tableStart = vertices.length();
    for (int i = 0; i < tablePoints.length(); ++i)
    {
        vertices.append(tablePoints[i]);
        normals.append(AcGeVector3d(0, 0, 1));
    }

    AcGePoint3dArray girdlePoints;
    createGirdleProfile(girdlePoints);

    const int girdleStart = vertices.length();
    for (int i = 0; i < girdlePoints.length(); ++i)
    {
        vertices.append(girdlePoints[i]);

        AcGeVector3d normal = girdlePoints[i] - m_center;
        normal.z = 0.0;
        normal.normalize();
        normals.append(normal);
    }

    faceList.append(8);
    for (int i = 0; i < 8; ++i)
        faceList.append(tableStart + i);

    for (int i = 0; i < 8; ++i)
    {
        int next = (i + 1) % 8;

        faceList.append(4);
        faceList.append(tableStart + i);
        faceList.append(tableStart + next);
        faceList.append(girdleStart + next);
        faceList.append(girdleStart + i);
    }

    const int culetStart = vertices.length();

    const double culetSize = m_width * 0.05;
    AcGePoint3dArray culetPoints;
    createEmeraldProfile(culetPoints, culetSize, culetSize * (m_length / m_width), culetZ);

    for (int i = 0; i < culetPoints.length(); ++i)
    {
        vertices.append(culetPoints[i]);
        normals.append(AcGeVector3d(0, 0, -1));
    }

    for (int i = 0; i < 8; ++i)
    {
        int next = (i + 1) % 8;

        faceList.append(4);
        faceList.append(girdleStart + i);
        faceList.append(girdleStart + next);
        faceList.append(culetStart + next);
        faceList.append(culetStart + i);
    }

    faceList.append(8);
    for (int i = 0; i < 8; ++i)
        faceList.append(culetStart + i);
}

Acad::ErrorStatus CEmeraldGemstone::subGetGripPoints(
    AcGePoint3dArray& gripPoints,
    AcDbIntArray& osnapModes,
    AcDbIntArray& geomIds) const
{
    assertReadEnabled();
    gripPoints.append(m_center);
    return Acad::eOk;
}

Acad::ErrorStatus CEmeraldGemstone::subMoveGripPointsAt(
    const AcDbIntArray& indices,
    const AcGeVector3d& offset)
{
    assertWriteEnabled();

    for (int i = 0; i < indices.length(); ++i)
    {
        if (indices[i] == 0)
        {
            m_center += offset;
        }
    }

    return Acad::eOk;
}

Acad::ErrorStatus CEmeraldGemstone::subTransformBy(const AcGeMatrix3d& xform)
{
    assertWriteEnabled();
    m_center.transformBy(xform);
    return Acad::eOk;
}

Acad::ErrorStatus CEmeraldGemstone::subGetGeomExtents(AcDbExtents& extents) const
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

Acad::ErrorStatus CEmeraldGemstone::dwgOutFields(AcDbDwgFiler* pFiler) const
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
    pFiler->writeDouble(m_cornerCutPercent);
    pFiler->writeDouble(m_tablePercent);

    return Acad::eOk;
}

Acad::ErrorStatus CEmeraldGemstone::dwgInFields(AcDbDwgFiler* pFiler)
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
    pFiler->readDouble(&m_cornerCutPercent);
    pFiler->readDouble(&m_tablePercent);

    calculateDerivedParameters();

    return Acad::eOk;
}

Acad::ErrorStatus CEmeraldGemstone::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = CJewelryGemstone::dxfOutFields(pFiler);
    if (es != Acad::eOk)
        return es;

    pFiler->writeItem(AcDb::kDxfSubclass, L"CEmeraldGemstone");
    pFiler->writeDouble(40, m_length);
    pFiler->writeDouble(41, m_width);
    pFiler->writeDouble(42, m_totalDepth);
    pFiler->writeDouble(43, m_crownHeight);
    pFiler->writeDouble(44, m_pavilionDepth);
    pFiler->writeDouble(45, m_cornerCutPercent);
    pFiler->writeDouble(46, m_tablePercent);

    return Acad::eOk;
}

Acad::ErrorStatus CEmeraldGemstone::dxfInFields(AcDbDxfFiler* pFiler)
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
            if (wcscmp(rb.resval.rstring, L"CEmeraldGemstone") == 0)
                continue;
            break;
        case 40: m_length = rb.resval.rreal; break;
        case 41: m_width = rb.resval.rreal; break;
        case 42: m_totalDepth = rb.resval.rreal; break;
        case 43: m_crownHeight = rb.resval.rreal; break;
        case 44: m_pavilionDepth = rb.resval.rreal; break;
        case 45: m_cornerCutPercent = rb.resval.rreal; break;
        case 46: m_tablePercent = rb.resval.rreal; break;
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
