#include "pch.h"
#include "CJewelryGemstone.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Adesk::UInt32 CJewelryGemstone::kCurrentVersionNumber = 1;

ACRX_DEFINE_MEMBERS(
    CJewelryGemstone,
    ACRX_DEFINE_MEMBERS_ONLY
);


// Конструкторы CJewelryGemstone
CJewelryGemstone::CJewelryGemstone()
    : AcDbEntity()
{
    m_center = AcGePoint3d::kOrigin;
    m_material = L"Diamond";
    m_refractiveIndex = 2.417;
    m_specificGravity = 3.52;
}

CJewelryGemstone::CJewelryGemstone(const AcGePoint3d& center)
    : AcDbEntity()
{
    m_center = center;
    m_material = L"Diamond";
    m_refractiveIndex = 2.417;
    m_specificGravity = 3.52;
}

CJewelryGemstone::~CJewelryGemstone()
{
}

Acad::ErrorStatus CJewelryGemstone::setPosition(const AcGePoint3d& center)
{
    return setCenter(center);
}

Acad::ErrorStatus CJewelryGemstone::setCenter(const AcGePoint3d& center)
{
    assertWriteEnabled();
    m_center = center;
    return Acad::eOk;
}

Acad::ErrorStatus CJewelryGemstone::setMaterial(const AcString& material)
{
    assertWriteEnabled();
    m_material = material;

    if (material == L"Diamond")
    {
        m_refractiveIndex = 2.417;
        m_specificGravity = 3.52;
    }
    else if (material == L"Ruby" || material == L"Sapphire")
    {
        m_refractiveIndex = 1.762;
        m_specificGravity = 4.00;
    }
    else if (material == L"Emerald")
    {
        m_refractiveIndex = 1.577;
        m_specificGravity = 2.72;
    }
    else if (material == L"Topaz")
    {
        m_refractiveIndex = 1.619;
        m_specificGravity = 3.53;
    }

    return Acad::eOk;
}

Acad::ErrorStatus CJewelryGemstone::setRefractiveIndex(double ri)
{
    assertWriteEnabled();
    if (ri < 1.0 || ri > 3.0)
        return Acad::eInvalidInput;
    m_refractiveIndex = ri;
    return Acad::eOk;
}

Acad::ErrorStatus CJewelryGemstone::setSpecificGravity(double sg)
{
    assertWriteEnabled();
    if (sg < 1.0 || sg > 10.0)
        return Acad::eInvalidInput;
    m_specificGravity = sg;
    return Acad::eOk;
}

Adesk::Boolean CJewelryGemstone::subWorldDraw(AcGiWorldDraw* pWd)
{
    assertReadEnabled();

    // 1. Создаем массивы
    AcGePoint3dArray vertices;
    AcGeVector3dArray normals;
    AcArray<int> faceList;

    // 2. Генерируем геометрию
    buildGemGeometry(vertices, normals, faceList);

    // 3. Проверка на пустоту 
    if (vertices.length() == 0 || faceList.length() == 0)
        return Adesk::kTrue;

    // 4. Отрисовка оболочки (Shell)
    
    pWd->geometry().shell(
        vertices.length(),      // Количество вершин
        vertices.asArrayPtr(),  // Указатель на массив вершин
        faceList.length(),      // Размер массива индексов (общее кол-во чисел в faceList)
        (const Adesk::Int32*)faceList.asArrayPtr() 
    );

    return Adesk::kTrue;
}

Acad::ErrorStatus CJewelryGemstone::subGetOsnapPoints(
    AcDb::OsnapMode osnapMode,
    Adesk::GsMarker gsSelectionMark,
    const AcGePoint3d& pickPoint,
    const AcGePoint3d& lastPoint,
    const AcGeMatrix3d& viewXform,
    AcGePoint3dArray& snapPoints,
    AcDbIntArray& geomIds) const
{
    assertReadEnabled();

    if (osnapMode == AcDb::kOsModeCen)
    {
        snapPoints.append(m_center);
    }

    return Acad::eOk;
}

Acad::ErrorStatus CJewelryGemstone::subTransformBy(const AcGeMatrix3d& xform)
{
    assertWriteEnabled();
    m_center.transformBy(xform);
    return Acad::eOk;
}

AcGePoint3d CJewelryGemstone::getPointOnCircle(double radius, int index, int totalPoints, double z) const
{
    double angle = (2.0 * M_PI * index) / totalPoints;
    return m_center + AcGeVector3d(
        radius * cos(angle),
        radius * sin(angle),
        z
    );
}

AcGeVector3d CJewelryGemstone::calculateFacetNormal(
    const AcGePoint3d& p1,
    const AcGePoint3d& p2,
    const AcGePoint3d& p3) const
{
    AcGeVector3d v1 = p2 - p1;
    AcGeVector3d v2 = p3 - p1;
    AcGeVector3d normal = v1.crossProduct(v2);
    normal.normalize();
    return normal;
}

Acad::ErrorStatus CJewelryGemstone::dwgInFields(AcDbDwgFiler* pFiler)
{
    assertWriteEnabled();

    Acad::ErrorStatus es = AcDbEntity::dwgInFields(pFiler);
    if (es != Acad::eOk) return es;

    Adesk::UInt32 version;
    if ((es = pFiler->readUInt32(&version)) != Acad::eOk) return es;
    if (version > kCurrentVersionNumber) return Acad::eMakeMeProxy;

    if ((es = pFiler->readPoint3d(&m_center)) != Acad::eOk) return es;

    Adesk::Int32 materialCode = 0;
    if ((es = pFiler->readInt32(&materialCode)) != Acad::eOk) return es;

    switch (materialCode)
    {
    case 0: m_material = L"Diamond"; break;
    case 1: m_material = L"Ruby"; break;
    case 2: m_material = L"Sapphire"; break;
    case 3: m_material = L"Emerald"; break;
    case 4: m_material = L"Topaz"; break;
    default: m_material = L"Diamond"; break;
    }

    if ((es = pFiler->readDouble(&m_refractiveIndex)) != Acad::eOk) return es;
    if ((es = pFiler->readDouble(&m_specificGravity)) != Acad::eOk) return es;

    return pFiler->filerStatus();
}

Acad::ErrorStatus CJewelryGemstone::dwgOutFields(AcDbDwgFiler* pFiler) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = AcDbEntity::dwgOutFields(pFiler);
    if (es != Acad::eOk) return es;

    if ((es = pFiler->writeUInt32(kCurrentVersionNumber)) != Acad::eOk) return es;
    if ((es = pFiler->writePoint3d(m_center)) != Acad::eOk) return es;

    Adesk::Int32 materialCode = 0;
    if (m_material == L"Diamond") materialCode = 0;
    else if (m_material == L"Ruby") materialCode = 1;
    else if (m_material == L"Sapphire") materialCode = 2;
    else if (m_material == L"Emerald") materialCode = 3;
    else if (m_material == L"Topaz") materialCode = 4;

    if ((es = pFiler->writeInt32(materialCode)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_refractiveIndex)) != Acad::eOk) return es;
    if ((es = pFiler->writeDouble(m_specificGravity)) != Acad::eOk) return es;

    return pFiler->filerStatus();
}

Acad::ErrorStatus CJewelryGemstone::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = AcDbEntity::dxfOutFields(pFiler);
    if (es != Acad::eOk) return es;

    pFiler->writeItem(AcDb::kDxfSubclass, L"CJewelryGemstone");
    pFiler->writeUInt32(AcDb::kDxfInt32, kCurrentVersionNumber);
    pFiler->writePoint3d(AcDb::kDxfXCoord, m_center);
    pFiler->writeString(AcDb::kDxfText, m_material);
    pFiler->writeDouble(AcDb::kDxfReal, m_refractiveIndex);
    pFiler->writeDouble(AcDb::kDxfReal + 1, m_specificGravity);

    return pFiler->filerStatus();
}

Acad::ErrorStatus CJewelryGemstone::dxfInFields(AcDbDxfFiler* pFiler)
{
    assertWriteEnabled();

    Acad::ErrorStatus es = AcDbEntity::dxfInFields(pFiler);
    if (es != Acad::eOk) return es;

    resbuf rb;

    while ((es = pFiler->readResBuf(&rb)) == Acad::eOk)
    {
        switch (rb.restype)
        {
        case AcDb::kDxfSubclass:
            if (wcscmp(rb.resval.rstring, L"CJewelryGemstone") == 0)
                continue;
            break;

        case AcDb::kDxfXCoord:
            m_center = asPnt3d(rb.resval.rpoint);
            break;

        case AcDb::kDxfText:
            m_material = rb.resval.rstring;
            break;

        case AcDb::kDxfReal:
            m_refractiveIndex = rb.resval.rreal;
            break;

        case AcDb::kDxfReal + 1:
            m_specificGravity = rb.resval.rreal;
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

Nano::ErrorStatus CJewelryGemstone::setTablePercent(double percent)
{

    return Nano::eInvalidInput;
}

Nano::ErrorStatus CJewelryGemstone::setCrownHeight(double height)
{
    return Nano::eInvalidInput;
}

Nano::ErrorStatus CJewelryGemstone::setPavilionDepth(double depth)
{
    return Nano::eInvalidInput;
}

Nano::ErrorStatus CJewelryGemstone::setCrownAngle(double angle)
{
    return Nano::eInvalidInput;
}

Nano::ErrorStatus CJewelryGemstone::setPavilionAngle(double angle)
{
    return Nano::eInvalidInput;
}

Nano::ErrorStatus CJewelryGemstone::setCuletPercent(double percent)
{
    return Nano::eInvalidInput;
}