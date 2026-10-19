#include "pch.h"
#include "CCustomGemstone.h"

#include <cmath>

namespace
{
    constexpr double PI_VALUE = 3.14159265358979323846;

    const double kGripAxisDirs[4][2] = { { 1.0, 0.0 }, { -1.0, 0.0 }, { 0.0, 1.0 }, { 0.0, -1.0 } };

    double movedGripRadius(int axisIndex, double radius, const AcGeVector3d& offset)
    {
        const double x = kGripAxisDirs[axisIndex][0] * radius + offset.x;
        const double y = kGripAxisDirs[axisIndex][1] * radius + offset.y;
        return std::sqrt(x * x + y * y);
    }
}

Adesk::UInt32 CCustomGemstone::kCurrentVersionNumber = 1;

ACRX_DXF_DEFINE_MEMBERS(
    CCustomGemstone,
    CJewelryGemstone,
    AcDb::kDHL_CURRENT,
    AcDb::kMReleaseCurrent,
    AcDbProxyEntity::kAllAllowedBits,
    CUSTOMGEMSTONE,
    L"VKR_NRX"
);

CCustomGemstone::CCustomGemstone()
    : CJewelryGemstone()
    , m_girdleDiameter(6.5)
    , m_tablePercent(57.0)
    , m_crownHeight(16.2)
    , m_crownAngle(34.5)
    , m_pavilionDepth(43.1)
    , m_pavilionAngle(40.75)
    , m_culetPercent(0.0)
    , m_girdleThickness(3.0)
    , m_starFacetLength(55.0)
{
    m_absoluteCrownHeight = m_girdleDiameter * m_crownHeight / 100.0;
    m_absolutePavilionDepth = m_girdleDiameter * m_pavilionDepth / 100.0;
}

CCustomGemstone::CCustomGemstone(const AcGePoint3d& center, double girdleDiameter)
    : CJewelryGemstone(center)
    , m_girdleDiameter(girdleDiameter > 0.0 ? girdleDiameter : 6.5)
    , m_tablePercent(57.0)
    , m_crownHeight(16.2)
    , m_crownAngle(34.5)
    , m_pavilionDepth(43.1)
    , m_pavilionAngle(40.75)
    , m_culetPercent(0.0)
    , m_girdleThickness(3.0)
    , m_starFacetLength(55.0)
{
    m_absoluteCrownHeight = m_girdleDiameter * m_crownHeight / 100.0;
    m_absolutePavilionDepth = m_girdleDiameter * m_pavilionDepth / 100.0;
}

CCustomGemstone::~CCustomGemstone()
{
}

Acad::ErrorStatus CCustomGemstone::setGirdleDiameter(double diameter)
{
    assertWriteEnabled();

    if (diameter <= 0.0)
        return Acad::eInvalidInput;

    m_girdleDiameter = diameter;

    if (m_absoluteCrownHeight > 0.0)
        m_crownHeight = (m_absoluteCrownHeight / m_girdleDiameter) * 100.0;

    if (m_absolutePavilionDepth > 0.0)
        m_pavilionDepth = (m_absolutePavilionDepth / m_girdleDiameter) * 100.0;

    return Acad::eOk;
}

Acad::ErrorStatus CCustomGemstone::setTotalHeight(double height)
{
    assertWriteEnabled();

    if (height <= 0.0)
        return Acad::eInvalidInput;

    double crownRatio = m_absoluteCrownHeight;
    double pavilionRatio = m_absolutePavilionDepth;
    const double currentTotal = crownRatio + pavilionRatio;

    if (currentTotal <= 1.0e-6)
    {
        crownRatio = 16.2;
        pavilionRatio = 43.1;
    }

    const double ratioTotal = crownRatio + pavilionRatio;
    m_absoluteCrownHeight = height * crownRatio / ratioTotal;
    m_absolutePavilionDepth = height * pavilionRatio / ratioTotal;

    m_crownHeight = (m_absoluteCrownHeight / m_girdleDiameter) * 100.0;
    m_pavilionDepth = (m_absolutePavilionDepth / m_girdleDiameter) * 100.0;

    return Acad::eOk;
}

Nano::ErrorStatus CCustomGemstone::setTablePercent(double percent)
{
    assertWriteEnabled();

    if (percent < 40.0 || percent > 70.0)
        return Nano::eInvalidInput;

    m_tablePercent = percent;
    return Nano::eOk;
}

Nano::ErrorStatus CCustomGemstone::setCrownAngle(double degrees)
{
    assertWriteEnabled();

    if (degrees < 25.0 || degrees > 40.0)
        return Nano::eInvalidInput;

    m_crownAngle = degrees;
    return Nano::eOk;
}

Nano::ErrorStatus CCustomGemstone::setPavilionAngle(double degrees)
{
    assertWriteEnabled();

    if (degrees < 37.0 || degrees > 44.0)
        return Nano::eInvalidInput;

    m_pavilionAngle = degrees;
    return Nano::eOk;
}

Nano::ErrorStatus CCustomGemstone::setCrownHeight(double percent)
{
    assertWriteEnabled();

    if (percent < 10.0 || percent > 20.0)
        return Nano::eInvalidInput;

    m_crownHeight = percent;
    m_absoluteCrownHeight = m_girdleDiameter * m_crownHeight / 100.0;

    return Nano::eOk;
}

Nano::ErrorStatus CCustomGemstone::setPavilionDepth(double percent)
{
    assertWriteEnabled();

    if (percent < 38.0 || percent > 48.0)
        return Nano::eInvalidInput;

    m_pavilionDepth = percent;
    m_absolutePavilionDepth = m_girdleDiameter * m_pavilionDepth / 100.0;

    return Nano::eOk;
}

Nano::ErrorStatus CCustomGemstone::setCuletPercent(double percent)
{
    assertWriteEnabled();

    if (percent < 0.0 || percent > 5.0)
        return Nano::eInvalidInput;

    m_culetPercent = percent;
    return Nano::eOk;
}

Acad::ErrorStatus CCustomGemstone::setGirdleThickness(double percent)
{
    assertWriteEnabled();

    if (percent < 0.5 || percent > 6.0)
        return Acad::eInvalidInput;

    m_girdleThickness = percent;
    return Acad::eOk;
}

Acad::ErrorStatus CCustomGemstone::setStarFacetLength(double percent)
{
    assertWriteEnabled();

    if (percent < 40.0 || percent > 70.0)
        return Acad::eInvalidInput;

    m_starFacetLength = percent;
    return Acad::eOk;
}

int CCustomGemstone::facetCount() const
{
    assertReadEnabled();
    return (m_culetPercent > 0.001) ? 58 : 57;
}

double CCustomGemstone::totalHeight() const
{
    assertReadEnabled();
    return m_absoluteCrownHeight + m_absolutePavilionDepth;
}

double CCustomGemstone::volume() const
{
    assertReadEnabled();

    const double r = m_girdleDiameter / 2.0;
    const double crownH = m_absoluteCrownHeight;
    const double pavilionH = m_absolutePavilionDepth;
    const double tableR = r * m_tablePercent / 100.0;
    const double culetR = r * m_culetPercent / 100.0;

    const double crownVol = (PI_VALUE * crownH / 3.0) * (r * r + r * tableR + tableR * tableR);
    const double pavilionVol = (PI_VALUE * pavilionH / 3.0) * (r * r + r * culetR + culetR * culetR);

    return crownVol + pavilionVol;
}

double CCustomGemstone::caratWeight() const
{
    assertReadEnabled();

    const double d = m_girdleDiameter;
    const double h = totalHeight();

    return d * d * h * 0.00612;
}

const wchar_t* CCustomGemstone::typeId() const
{
    return L"custom";
}

CJewelryGemstone::OutlineShape CCustomGemstone::outlineShape() const
{
    return kOutlineRound;
}

AcCmColor CCustomGemstone::defaultColor() const
{
    AcCmColor color;
    color.setRGB(230, 245, 255);
    return color;
}

void CCustomGemstone::getDimensions(double& length, double& width, double& height) const
{
    assertReadEnabled();
    length = m_girdleDiameter;
    width = m_girdleDiameter;
    height = totalHeight();
}

Acad::ErrorStatus CCustomGemstone::setDimensions(double length, double width, double height)
{
    (void)width;
    Acad::ErrorStatus es = setGirdleDiameter(length);
    if (es == Acad::eOk)
        es = setTotalHeight(height);
    return es;
}

void CCustomGemstone::buildGemGeometry(
    AcGePoint3dArray& vertices,
    AcGeVector3dArray& normals,
    AcArray<Adesk::Int32>& faceList) const
{
    assertReadEnabled();

    vertices.setLogicalLength(0);
    normals.setLogicalLength(0);
    faceList.setLogicalLength(0);

    const double girdleRadius = m_girdleDiameter / 2.0;
    const double girdleZ = 0.0;

    buildCrown(vertices, normals, faceList, girdleRadius, girdleZ);
    buildPavilion(vertices, normals, faceList, girdleRadius, girdleZ);
}

void CCustomGemstone::buildCrown(
    AcGePoint3dArray& vertices,
    AcGeVector3dArray& normals,
    AcArray<Adesk::Int32>& faceList,
    double girdleRadius,
    double girdleZ) const
{
    const double crownH = m_absoluteCrownHeight;
    const double tableR = girdleRadius * m_tablePercent / 100.0;
    const double tableZ = girdleZ + crownH;

    const int tableStart = vertices.length();
    for (int i = 0; i < 8; ++i)
    {
        vertices.append(getPointOnCircle(tableR, i, 8, tableZ));
        normals.append(AcGeVector3d(0, 0, 1));
    }

    const int girdleMainStart = vertices.length();
    for (int i = 0; i < 8; ++i)
    {
        AcGePoint3d pt = getPointOnCircle(girdleRadius, i, 8, girdleZ);
        vertices.append(pt);

        AcGeVector3d dir = pt - m_center;
        dir.z = 0.0;
        dir.normalize();
        normals.append(dir);
    }

    const int girdleStarStart = vertices.length();
    for (int i = 0; i < 8; ++i)
    {
        double angle = (2.0 * PI_VALUE * i) / 8.0 + PI_VALUE / 8.0;
        AcGePoint3d pt = m_center + AcGeVector3d(
            girdleRadius * std::cos(angle),
            girdleRadius * std::sin(angle),
            girdleZ);

        vertices.append(pt);

        AcGeVector3d dir = pt - m_center;
        dir.z = 0.0;
        dir.normalize();
        normals.append(dir);
    }

    faceList.append(8);
    for (int i = 0; i < 8; ++i)
        faceList.append(tableStart + i);

    for (int i = 0; i < 8; ++i)
    {
        int next = (i + 1) % 8;

        faceList.append(3);
        faceList.append(tableStart + i);
        faceList.append(tableStart + next);
        faceList.append(girdleStarStart + i);
    }

    for (int i = 0; i < 8; ++i)
    {
        int prev = (i - 1 + 8) % 8;

        faceList.append(4);
        faceList.append(tableStart + i);
        faceList.append(girdleStarStart + i);
        faceList.append(girdleMainStart + i);
        faceList.append(girdleStarStart + prev);
    }

    for (int i = 0; i < 8; ++i)
    {
        int next = (i + 1) % 8;

        faceList.append(3);
        faceList.append(girdleMainStart + i);
        faceList.append(girdleStarStart + i);
        faceList.append(girdleMainStart + next);
    }
}

void CCustomGemstone::buildPavilion(
    AcGePoint3dArray& vertices,
    AcGeVector3dArray& normals,
    AcArray<Adesk::Int32>& faceList,
    double girdleRadius,
    double girdleZ) const
{
    const double pavilionH = m_absolutePavilionDepth;
    const double culetR = girdleRadius * m_culetPercent / 100.0;
    const double culetZ = girdleZ - pavilionH;

    const int girdleMainStart = vertices.length();
    for (int i = 0; i < 8; ++i)
    {
        AcGePoint3d pt = getPointOnCircle(girdleRadius, i, 8, girdleZ);
        vertices.append(pt);

        AcGeVector3d dir = pt - m_center;
        dir.z = 0.0;
        dir.normalize();
        normals.append(dir);
    }

    const int girdleStarStart = vertices.length();
    for (int i = 0; i < 8; ++i)
    {
        double angle = (2.0 * PI_VALUE * i) / 8.0 + PI_VALUE / 8.0;
        AcGePoint3d pt = m_center + AcGeVector3d(
            girdleRadius * std::cos(angle),
            girdleRadius * std::sin(angle),
            girdleZ);

        vertices.append(pt);

        AcGeVector3d dir = pt - m_center;
        dir.z = 0.0;
        dir.normalize();
        normals.append(dir);
    }

    const int culetStart = vertices.length();

    if (m_culetPercent < 0.001)
    {
        vertices.append(m_center + AcGeVector3d(0, 0, culetZ));
        normals.append(AcGeVector3d(0, 0, -1));
    }
    else
    {
        for (int i = 0; i < 8; ++i)
        {
            vertices.append(getPointOnCircle(culetR, i, 8, culetZ));
            normals.append(AcGeVector3d(0, 0, -1));
        }
    }

    if (m_culetPercent < 0.001)
    {
        for (int i = 0; i < 8; ++i)
        {
            int next = (i + 1) % 8;

            faceList.append(3);
            faceList.append(girdleMainStart + i);
            faceList.append(girdleMainStart + next);
            faceList.append(culetStart);
        }

        for (int i = 0; i < 8; ++i)
        {
            int next = (i + 1) % 8;

            faceList.append(3);
            faceList.append(girdleMainStart + i);
            faceList.append(culetStart);
            faceList.append(girdleStarStart + i);

            faceList.append(3);
            faceList.append(girdleStarStart + i);
            faceList.append(culetStart);
            faceList.append(girdleMainStart + next);
        }
    }
    else
    {
        for (int i = 0; i < 8; ++i)
        {
            int next = (i + 1) % 8;

            faceList.append(4);
            faceList.append(girdleMainStart + i);
            faceList.append(girdleMainStart + next);
            faceList.append(culetStart + next);
            faceList.append(culetStart + i);
        }

        for (int i = 0; i < 8; ++i)
        {
            int next = (i + 1) % 8;

            faceList.append(3);
            faceList.append(girdleMainStart + i);
            faceList.append(culetStart + i);
            faceList.append(girdleStarStart + i);

            faceList.append(3);
            faceList.append(girdleStarStart + i);
            faceList.append(culetStart + next);
            faceList.append(girdleMainStart + next);
        }
    }
}

Acad::ErrorStatus CCustomGemstone::subGetGripPoints(
    AcGePoint3dArray& gripPoints,
    AcDbIntArray& osnapModes,
    AcDbIntArray& geomIds) const
{
    assertReadEnabled();

    const double r = m_girdleDiameter / 2.0;
    const double tableR = r * m_tablePercent / 100.0;
    const double crownH = m_absoluteCrownHeight;

    gripPoints.append(m_center);
    gripPoints.append(m_center + AcGeVector3d(0.0, 0.0, crownH));
    gripPoints.append(m_center + AcGeVector3d(0.0, 0.0, -m_absolutePavilionDepth));

    for (int i = 0; i < 4; ++i)
        gripPoints.append(m_center + AcGeVector3d(kGripAxisDirs[i][0] * r, kGripAxisDirs[i][1] * r, 0.0));

    for (int i = 0; i < 4; ++i)
        gripPoints.append(m_center + AcGeVector3d(kGripAxisDirs[i][0] * tableR, kGripAxisDirs[i][1] * tableR, crownH));

    return Acad::eOk;
}

Acad::ErrorStatus CCustomGemstone::subMoveGripPointsAt(
    const AcDbIntArray& indices,
    const AcGeVector3d& offset)
{
    assertWriteEnabled();

    for (int i = 0; i < indices.length(); ++i)
    {
        const int grip = indices[i];

        if (grip == kGripCenter)
        {
            m_center += offset;
        }
        else if (grip == kGripCrownTop)
        {
            const double newCrownHeight = m_absoluteCrownHeight + offset.z;
            if (newCrownHeight >= 0.05 && newCrownHeight <= 50.0)
            {
                m_absoluteCrownHeight = newCrownHeight;
                m_crownHeight = (m_absoluteCrownHeight / m_girdleDiameter) * 100.0;
            }
        }
        else if (grip == kGripCulet)
        {
            const double newPavilionDepth = m_absolutePavilionDepth - offset.z;
            if (newPavilionDepth >= 0.05 && newPavilionDepth <= 60.0)
            {
                m_absolutePavilionDepth = newPavilionDepth;
                m_pavilionDepth = (m_absolutePavilionDepth / m_girdleDiameter) * 100.0;
            }
        }
        else if (grip >= kGripGirdleFirst && grip <= kGripGirdleLast)
        {
            const double newDiameter =
                2.0 * movedGripRadius(grip - kGripGirdleFirst, m_girdleDiameter / 2.0, offset);
            if (newDiameter >= 0.5 && newDiameter <= 100.0)
                setGirdleDiameter(newDiameter);
        }
        else if (grip >= kGripTableFirst && grip <= kGripTableLast)
        {
            const double r = m_girdleDiameter / 2.0;
            const double newTablePercent =
                movedGripRadius(grip - kGripTableFirst, r * m_tablePercent / 100.0, offset) / r * 100.0;
            if (newTablePercent >= 10.0 && newTablePercent <= 90.0)
                m_tablePercent = newTablePercent;
        }
    }

    return Acad::eOk;
}

Acad::ErrorStatus CCustomGemstone::subGetGeomExtents(AcDbExtents& extents) const
{
    assertReadEnabled();

    const double r = m_girdleDiameter / 2.0;
    const double crownH = m_absoluteCrownHeight;
    const double pavilionH = m_absolutePavilionDepth;

    AcGePoint3d minPt = m_center + AcGeVector3d(-r, -r, -pavilionH);
    AcGePoint3d maxPt = m_center + AcGeVector3d(r, r, crownH);

    extents.set(minPt, maxPt);
    return Acad::eOk;
}

Acad::ErrorStatus CCustomGemstone::dwgInFields(AcDbDwgFiler* pFiler)
{
    assertWriteEnabled();

    Acad::ErrorStatus es = CJewelryGemstone::dwgInFields(pFiler);
    if (es != Acad::eOk)
        return es;

    Adesk::UInt32 version = 0;
    if ((es = pFiler->readUInt32(&version)) != Acad::eOk)
        return es;

    if (version > kCurrentVersionNumber)
        return Acad::eMakeMeProxy;

    if ((es = pFiler->readDouble(&m_girdleDiameter)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_tablePercent)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_crownAngle)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_pavilionAngle)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_crownHeight)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_pavilionDepth)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_culetPercent)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_girdleThickness)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_starFacetLength)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_absoluteCrownHeight)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_absolutePavilionDepth)) != Acad::eOk) return es;

    return pFiler->filerStatus();
}

Acad::ErrorStatus CCustomGemstone::dwgOutFields(AcDbDwgFiler* pFiler) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = CJewelryGemstone::dwgOutFields(pFiler);
    if (es != Acad::eOk)
        return es;

    if ((es = pFiler->writeUInt32(kCurrentVersionNumber)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_girdleDiameter)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_tablePercent)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_crownAngle)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_pavilionAngle)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_crownHeight)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_pavilionDepth)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_culetPercent)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_girdleThickness)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_starFacetLength)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_absoluteCrownHeight)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_absolutePavilionDepth)) != Acad::eOk) return es;

    return pFiler->filerStatus();
}

Acad::ErrorStatus CCustomGemstone::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = CJewelryGemstone::dxfOutFields(pFiler);
    if (es != Acad::eOk)
        return es;

    pFiler->writeItem(AcDb::kDxfSubclass, L"CCustomGemstone");
    pFiler->writeUInt32(AcDb::kDxfInt32, kCurrentVersionNumber);
    pFiler->writeDouble(AcDb::kDxfReal, m_girdleDiameter);
    pFiler->writeDouble(AcDb::kDxfReal + 1, m_tablePercent);
    pFiler->writeDouble(AcDb::kDxfReal + 2, m_crownAngle);
    pFiler->writeDouble(AcDb::kDxfReal + 3, m_pavilionAngle);
    pFiler->writeDouble(AcDb::kDxfReal + 4, m_crownHeight);
    pFiler->writeDouble(AcDb::kDxfReal + 5, m_pavilionDepth);
    pFiler->writeDouble(AcDb::kDxfReal + 6, m_culetPercent);
    pFiler->writeDouble(AcDb::kDxfReal + 7, m_girdleThickness);
    pFiler->writeDouble(AcDb::kDxfReal + 8, m_starFacetLength);

    return pFiler->filerStatus();
}

Acad::ErrorStatus CCustomGemstone::dxfInFields(AcDbDxfFiler* pFiler)
{
    assertWriteEnabled();

    Acad::ErrorStatus es = CJewelryGemstone::dxfInFields(pFiler);
    if (es != Acad::eOk)
        return es;

    resbuf rb;
    int realCount = 0;

    while ((es = pFiler->readResBuf(&rb)) == Acad::eOk)
    {
        switch (rb.restype)
        {
        case AcDb::kDxfSubclass:
            if (wcscmp(rb.resval.rstring, L"CCustomGemstone") == 0)
                continue;
            break;

        case AcDb::kDxfReal:
            if (realCount == 0)
                m_girdleDiameter = rb.resval.rreal;
            ++realCount;
            break;

        case AcDb::kDxfReal + 1: m_tablePercent = rb.resval.rreal; break;
        case AcDb::kDxfReal + 2: m_crownAngle = rb.resval.rreal; break;
        case AcDb::kDxfReal + 3: m_pavilionAngle = rb.resval.rreal; break;
        case AcDb::kDxfReal + 4: m_crownHeight = rb.resval.rreal; break;
        case AcDb::kDxfReal + 5: m_pavilionDepth = rb.resval.rreal; break;
        case AcDb::kDxfReal + 6: m_culetPercent = rb.resval.rreal; break;
        case AcDb::kDxfReal + 7: m_girdleThickness = rb.resval.rreal; break;
        case AcDb::kDxfReal + 8: m_starFacetLength = rb.resval.rreal; break;

        default:
            pFiler->pushBackItem();
            es = Acad::eEndOfFile;
            break;
        }

        if (es == Acad::eEndOfFile)
            break;
    }

    return pFiler->filerStatus();
}
