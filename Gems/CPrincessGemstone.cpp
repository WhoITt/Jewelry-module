#include "pch.h"
#include "CPrincessGemstone.h"

namespace
{
const double kCornerDirs[4][2] = { { 1.0, 1.0 }, { -1.0, 1.0 }, { -1.0, -1.0 }, { 1.0, -1.0 } };
}

Adesk::UInt32 CPrincessGemstone::kCurrentVersionNumber = 1;

ACRX_DXF_DEFINE_MEMBERS(
    CPrincessGemstone,
    CJewelryGemstone,
    AcDb::kDHL_CURRENT,
    AcDb::kMReleaseCurrent,
    AcDbProxyEntity::kAllAllowedBits,
    PRINCESSGEMSTONE,
    L"VKR_NRX"
);

CPrincessGemstone::CPrincessGemstone()
    : CJewelryGemstone()
    , m_squareSide(6.5)
    , m_crownHeight(10.0)
    , m_pavilionDepth(68.0)
{
}

CPrincessGemstone::CPrincessGemstone(const AcGePoint3d& center, double squareSide)
    : CJewelryGemstone(center)
    , m_squareSide(squareSide > 0.0 ? squareSide : 6.5)
    , m_crownHeight(10.0)
    , m_pavilionDepth(68.0)
{
}

CPrincessGemstone::~CPrincessGemstone()
{
}

Acad::ErrorStatus CPrincessGemstone::setSquareSide(double side)
{
    assertWriteEnabled();

    if (side <= 0.0)
        return Acad::eInvalidInput;

    m_squareSide = side;
    return Acad::eOk;
}

Acad::ErrorStatus CPrincessGemstone::setCrownHeight(double percent)
{
    assertWriteEnabled();

    if (percent < 8.0 || percent > 12.0)
        return Acad::eInvalidInput;

    m_crownHeight = percent;
    return Acad::eOk;
}

Acad::ErrorStatus CPrincessGemstone::setPavilionDepth(double percent)
{
    assertWriteEnabled();

    if (percent < 60.0 || percent > 75.0)
        return Acad::eInvalidInput;

    m_pavilionDepth = percent;
    return Acad::eOk;
}

Nano::ErrorStatus CPrincessGemstone::setTablePercent(double percent)
{
    assertWriteEnabled();

    if (percent < 40.0 || percent > 70.0)
        return Nano::eInvalidInput;

    return Nano::eOk;
}

Nano::ErrorStatus CPrincessGemstone::setCrownAngle(double angle)
{
    assertWriteEnabled();

    if (angle < 25.0 || angle > 40.0)
        return Nano::eInvalidInput;

    return Nano::eOk;
}

Nano::ErrorStatus CPrincessGemstone::setPavilionAngle(double angle)
{
    assertWriteEnabled();

    if (angle < 38.0 || angle > 43.0)
        return Nano::eInvalidInput;

    return Nano::eOk;
}

Nano::ErrorStatus CPrincessGemstone::setCuletPercent(double percent)
{
    assertWriteEnabled();

    if (percent < 0.0 || percent > 5.0)
        return Nano::eInvalidInput;

    return Nano::eOk;
}

int CPrincessGemstone::facetCount() const
{
    assertReadEnabled();
    return 76;
}

double CPrincessGemstone::totalHeight() const
{
    assertReadEnabled();
    return m_squareSide * (m_crownHeight + m_pavilionDepth) / 100.0;
}

double CPrincessGemstone::volume() const
{
    assertReadEnabled();

    const double a = m_squareSide;
    const double crownH = a * m_crownHeight / 100.0;
    const double pavilionH = a * m_pavilionDepth / 100.0;

    const double crownVol = (a * a * crownH) / 3.0;
    const double pavilionVol = (a * a * pavilionH) / 3.0;

    return crownVol + pavilionVol;
}

double CPrincessGemstone::caratWeight() const
{
    assertReadEnabled();

    const double side = m_squareSide;
    const double h = totalHeight();

    return side * side * h * 0.0095;
}

const wchar_t* CPrincessGemstone::typeId() const
{
    return L"princess";
}

CJewelryGemstone::OutlineShape CPrincessGemstone::outlineShape() const
{
    return kOutlineRect;
}

AcCmColor CPrincessGemstone::defaultColor() const
{
    AcCmColor color;
    color.setRGB(9, 76, 144);
    return color;
}

void CPrincessGemstone::getDimensions(double& length, double& width, double& height) const
{
    assertReadEnabled();
    length = m_squareSide;
    width = m_squareSide;
    height = totalHeight();
}

Acad::ErrorStatus CPrincessGemstone::setDimensions(double length, double width, double height)
{
    // The height follows from the side and the crown and pavilion proportions.
    (void)width;
    (void)height;
    return setSquareSide(length);
}

void CPrincessGemstone::buildGemGeometry(
    AcGePoint3dArray& vertices,
    AcGeVector3dArray& normals,
    AcArray<Adesk::Int32>& faceList) const
{
    assertReadEnabled();

    vertices.setLogicalLength(0);
    normals.setLogicalLength(0);
    faceList.setLogicalLength(0);

    const double halfSide = m_squareSide / 2.0;
    const double rundistZ = 0.0;

    buildCrown(vertices, normals, faceList, halfSide, rundistZ);
    buildPavilion(vertices, normals, faceList, halfSide, rundistZ);
}

void CPrincessGemstone::buildCrown(
    AcGePoint3dArray& vertices,
    AcGeVector3dArray& normals,
    AcArray<Adesk::Int32>& faceList,
    double halfSide,
    double rundistZ) const
{
    const double crownH = m_squareSide * m_crownHeight / 100.0;
    const double tableSize = halfSide * 0.6;
    const double tableZ = rundistZ + crownH;

    const int tableStart = vertices.length();
    vertices.append(m_center + AcGeVector3d(-tableSize, -tableSize, tableZ));
    vertices.append(m_center + AcGeVector3d(tableSize, -tableSize, tableZ));
    vertices.append(m_center + AcGeVector3d(tableSize, tableSize, tableZ));
    vertices.append(m_center + AcGeVector3d(-tableSize, tableSize, tableZ));

    for (int i = 0; i < 4; ++i)
        normals.append(AcGeVector3d(0, 0, 1));

    const int rundistStart = vertices.length();
    vertices.append(m_center + AcGeVector3d(-halfSide, -halfSide, rundistZ));
    vertices.append(m_center + AcGeVector3d(halfSide, -halfSide, rundistZ));
    vertices.append(m_center + AcGeVector3d(halfSide, halfSide, rundistZ));
    vertices.append(m_center + AcGeVector3d(-halfSide, halfSide, rundistZ));

    for (int i = 0; i < 4; ++i)
    {
        AcGeVector3d normal = vertices[rundistStart + i] - m_center;
        normal.z = 0.0;
        normal.normalize();
        normals.append(normal);
    }

    faceList.append(4);
    for (int i = 0; i < 4; ++i)
        faceList.append(tableStart + i);

    for (int i = 0; i < 4; ++i)
    {
        int next = (i + 1) % 4;

        faceList.append(4);
        faceList.append(tableStart + i);
        faceList.append(tableStart + next);
        faceList.append(rundistStart + next);
        faceList.append(rundistStart + i);
    }

    for (int i = 0; i < 4; ++i)
    {
        int next = (i + 1) % 4;

        faceList.append(3);
        faceList.append(tableStart + i);
        faceList.append(tableStart + next);
        faceList.append(rundistStart + i);

        faceList.append(3);
        faceList.append(tableStart + next);
        faceList.append(rundistStart + next);
        faceList.append(rundistStart + i);
    }
}

void CPrincessGemstone::buildPavilion(
    AcGePoint3dArray& vertices,
    AcGeVector3dArray& normals,
    AcArray<Adesk::Int32>& faceList,
    double halfSide,
    double rundistZ) const
{
    const double pavilionH = m_squareSide * m_pavilionDepth / 100.0;
    const double culetZ = rundistZ - pavilionH;

    const int rundistStart = vertices.length();
    vertices.append(m_center + AcGeVector3d(-halfSide, -halfSide, rundistZ));
    vertices.append(m_center + AcGeVector3d(halfSide, -halfSide, rundistZ));
    vertices.append(m_center + AcGeVector3d(halfSide, halfSide, rundistZ));
    vertices.append(m_center + AcGeVector3d(-halfSide, halfSide, rundistZ));

    for (int i = 0; i < 4; ++i)
    {
        AcGeVector3d normal = vertices[rundistStart + i] - m_center;
        normal.z = 0.0;
        normal.normalize();
        normals.append(normal);
    }

    const int culetStart = vertices.length();
    vertices.append(m_center + AcGeVector3d(0, 0, culetZ));
    normals.append(AcGeVector3d(0, 0, -1));

    for (int i = 0; i < 4; ++i)
    {
        int next = (i + 1) % 4;

        faceList.append(3);
        faceList.append(rundistStart + i);
        faceList.append(rundistStart + next);
        faceList.append(culetStart);
    }
}

Acad::ErrorStatus CPrincessGemstone::subGetGripPoints(
    AcGePoint3dArray& gripPoints,
    AcDbIntArray& osnapModes,
    AcDbIntArray& geomIds) const
{
    assertReadEnabled();

    const double halfSide = m_squareSide / 2.0;

    gripPoints.append(m_center);
    gripPoints.append(m_center + AcGeVector3d(0.0, 0.0, m_squareSide * m_crownHeight / 100.0));
    gripPoints.append(m_center + AcGeVector3d(0.0, 0.0, -m_squareSide * m_pavilionDepth / 100.0));

    for (int i = 0; i < 4; ++i)
        gripPoints.append(m_center + AcGeVector3d(kCornerDirs[i][0] * halfSide, kCornerDirs[i][1] * halfSide, 0.0));

    return Acad::eOk;
}

Acad::ErrorStatus CPrincessGemstone::subMoveGripPointsAt(
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
            const double newCrownHeight = m_squareSide * m_crownHeight / 100.0 + offset.z;
            if (newCrownHeight > 0.0)
                m_crownHeight = (newCrownHeight / m_squareSide) * 100.0;
        }
        else if (grip == kGripCulet)
        {
            const double newPavilionDepth = m_squareSide * m_pavilionDepth / 100.0 - offset.z;
            if (newPavilionDepth > 0.0)
                m_pavilionDepth = (newPavilionDepth / m_squareSide) * 100.0;
        }
        else if (grip >= kGripCornerFirst && grip <= kGripCornerLast)
        {
            const double* dir = kCornerDirs[grip - kGripCornerFirst];
            const double newSide = m_squareSide + dir[0] * offset.x + dir[1] * offset.y;
            if (newSide > 0.1)
                m_squareSide = newSide;
        }
    }

    return Acad::eOk;
}

Acad::ErrorStatus CPrincessGemstone::subGetGeomExtents(AcDbExtents& extents) const
{
    assertReadEnabled();

    const double halfSide = m_squareSide / 2.0;
    const double crownH = m_squareSide * m_crownHeight / 100.0;
    const double pavilionH = m_squareSide * m_pavilionDepth / 100.0;

    AcGePoint3d minPt = m_center + AcGeVector3d(-halfSide, -halfSide, -pavilionH);
    AcGePoint3d maxPt = m_center + AcGeVector3d(halfSide, halfSide, crownH);

    extents.set(minPt, maxPt);
    return Acad::eOk;
}

Acad::ErrorStatus CPrincessGemstone::dwgInFields(AcDbDwgFiler* pFiler)
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

    if ((es = pFiler->readDouble(&m_squareSide)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_crownHeight)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_pavilionDepth)) != Acad::eOk) return es;

    return pFiler->filerStatus();
}

Acad::ErrorStatus CPrincessGemstone::dwgOutFields(AcDbDwgFiler* pFiler) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = CJewelryGemstone::dwgOutFields(pFiler);
    if (es != Acad::eOk)
        return es;

    if ((es = pFiler->writeUInt32(kCurrentVersionNumber)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_squareSide)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_crownHeight)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_pavilionDepth)) != Acad::eOk) return es;

    return pFiler->filerStatus();
}

Acad::ErrorStatus CPrincessGemstone::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = CJewelryGemstone::dxfOutFields(pFiler);
    if (es != Acad::eOk)
        return es;

    pFiler->writeItem(AcDb::kDxfSubclass, L"CPrincessGemstone");
    pFiler->writeUInt32(AcDb::kDxfInt32, kCurrentVersionNumber);
    pFiler->writeDouble(AcDb::kDxfReal, m_squareSide);
    pFiler->writeDouble(AcDb::kDxfReal + 1, m_crownHeight);
    pFiler->writeDouble(AcDb::kDxfReal + 2, m_pavilionDepth);

    return pFiler->filerStatus();
}

Acad::ErrorStatus CPrincessGemstone::dxfInFields(AcDbDxfFiler* pFiler)
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
            if (wcscmp(rb.resval.rstring, L"CPrincessGemstone") == 0)
                continue;
            break;

        case AcDb::kDxfReal:
            if (realCount == 0)
                m_squareSide = rb.resval.rreal;
            ++realCount;
            break;

        case AcDb::kDxfReal + 1:
            m_crownHeight = rb.resval.rreal;
            break;

        case AcDb::kDxfReal + 2:
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

    return pFiler->filerStatus();
}
