#pragma once

#include "dbmain.h"
#include "dbents.h"
#include "gepnt3d.h"
#include "gevec3d.h"
#include "gemat3d.h"

class CJewelryGemstone : public NcDbEntity
{
public:
    ACRX_DECLARE_MEMBERS(CJewelryGemstone);

public:
    CJewelryGemstone();
    CJewelryGemstone(const AcGePoint3d& center);
    virtual ~CJewelryGemstone();

public:
    AcGePoint3d center() const
    {
        assertReadEnabled();
        return m_center;
    }

    AcGePoint3d position() const
    {
        assertReadEnabled();
        return m_center;
    }

    AcString material() const
    {
        assertReadEnabled();
        return m_material;
    }

    double refractiveIndex() const
    {
        assertReadEnabled();
        return m_refractiveIndex;
    }

    double specificGravity() const
    {
        assertReadEnabled();
        return m_specificGravity;
    }

public:
    Acad::ErrorStatus setCenter(const AcGePoint3d& center);
    Acad::ErrorStatus setPosition(const AcGePoint3d& center);
    Acad::ErrorStatus setMaterial(const AcString& material);
    Acad::ErrorStatus setRefractiveIndex(double ri);
    Acad::ErrorStatus setSpecificGravity(double sg);

public:
    virtual int facetCount() const = 0;
    virtual double volume() const = 0;
    virtual double caratWeight() const = 0;
    virtual AcString cutName() const = 0;

    virtual void buildGemGeometry(
        AcGePoint3dArray& vertices,
        AcGeVector3dArray& normals,
        AcArray<Adesk::Int32>& faceList) const = 0;

public:
    virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw* pWd) override;

    virtual Acad::ErrorStatus subGetGripPoints(
        AcGePoint3dArray& gripPoints,
        AcDbIntArray& osnapModes,
        AcDbIntArray& geomIds) const override = 0;

    virtual Acad::ErrorStatus subMoveGripPointsAt(
        const AcDbIntArray& indices,
        const AcGeVector3d& offset) override = 0;

    virtual Acad::ErrorStatus subGetOsnapPoints(
        AcDb::OsnapMode osnapMode,
        Adesk::GsMarker gsSelectionMark,
        const AcGePoint3d& pickPoint,
        const AcGePoint3d& lastPoint,
        const AcGeMatrix3d& viewXform,
        AcGePoint3dArray& snapPoints,
        AcDbIntArray& geomIds) const override;

    virtual Acad::ErrorStatus subGetGeomExtents(AcDbExtents& extents) const override = 0;
    virtual Acad::ErrorStatus subTransformBy(const AcGeMatrix3d& xform) override;

public:
    virtual Nano::ErrorStatus setTablePercent(double percent);
    virtual Nano::ErrorStatus setCrownHeight(double height);
    virtual Nano::ErrorStatus setPavilionDepth(double depth);
    virtual Nano::ErrorStatus setCrownAngle(double angle);
    virtual Nano::ErrorStatus setPavilionAngle(double angle);
    virtual Nano::ErrorStatus setCuletPercent(double percent);

public:
    virtual Acad::ErrorStatus dwgInFields(AcDbDwgFiler* pFiler) override;
    virtual Acad::ErrorStatus dwgOutFields(AcDbDwgFiler* pFiler) const override;
    virtual Acad::ErrorStatus dxfInFields(AcDbDxfFiler* pFiler) override;
    virtual Acad::ErrorStatus dxfOutFields(AcDbDxfFiler* pFiler) const override;

protected:
    static Adesk::UInt32 kCurrentVersionNumber;

    AcGePoint3d m_center;
    AcString m_material;
    double m_refractiveIndex;
    double m_specificGravity;

protected:
    AcGePoint3d getPointOnCircle(double radius, int index, int totalPoints, double z) const;

    AcGeVector3d calculateFacetNormal(
        const AcGePoint3d& p1,
        const AcGePoint3d& p2,
        const AcGePoint3d& p3) const;
};