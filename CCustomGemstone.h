#pragma once

#include "CJewelryGemstone.h"

class CCustomGemstone : public CJewelryGemstone
{
public:
    ACRX_DECLARE_MEMBERS(CCustomGemstone);

public:
    CCustomGemstone();
    CCustomGemstone(const AcGePoint3d& center, double girdleDiameter);
    virtual ~CCustomGemstone();

public:
    double girdleDiameter() const
    {
        assertReadEnabled();
        return m_girdleDiameter;
    }

    double tablePercent() const
    {
        assertReadEnabled();
        return m_tablePercent;
    }

    double crownAngle() const
    {
        assertReadEnabled();
        return m_crownAngle;
    }

    double pavilionAngle() const
    {
        assertReadEnabled();
        return m_pavilionAngle;
    }

    double crownHeight() const
    {
        assertReadEnabled();
        return m_crownHeight;
    }

    double pavilionDepth() const
    {
        assertReadEnabled();
        return m_pavilionDepth;
    }

    double culetPercent() const
    {
        assertReadEnabled();
        return m_culetPercent;
    }

    double girdleThickness() const
    {
        assertReadEnabled();
        return m_girdleThickness;
    }

    double starFacetLength() const
    {
        assertReadEnabled();
        return m_starFacetLength;
    }

    double totalHeight() const;

public:
    Acad::ErrorStatus setGirdleDiameter(double diameter);
    Acad::ErrorStatus setTotalHeight(double height);
    virtual Nano::ErrorStatus setTablePercent(double percent) override;
    virtual Nano::ErrorStatus setCrownAngle(double degrees) override;
    virtual Nano::ErrorStatus setPavilionAngle(double degrees) override;
    virtual Nano::ErrorStatus setCrownHeight(double percent) override;
    virtual Nano::ErrorStatus setPavilionDepth(double percent) override;
    virtual Nano::ErrorStatus setCuletPercent(double percent) override;

    Acad::ErrorStatus setGirdleThickness(double percent);
    Acad::ErrorStatus setStarFacetLength(double percent);

public:
    virtual int facetCount() const override;
    virtual double volume() const override;
    virtual double caratWeight() const override;

    virtual AcString cutName() const override
    {
        return L"Round Brilliant";
    }

    virtual void buildGemGeometry(
        AcGePoint3dArray& vertices,
        AcGeVector3dArray& normals,
        AcArray<Adesk::Int32>& faceList) const override;

public:
    virtual Acad::ErrorStatus subGetGripPoints(
        AcGePoint3dArray& gripPoints,
        AcDbIntArray& osnapModes,
        AcDbIntArray& geomIds) const override;

    virtual Acad::ErrorStatus subMoveGripPointsAt(
        const AcDbIntArray& indices,
        const AcGeVector3d& offset) override;

    virtual Acad::ErrorStatus subGetGeomExtents(AcDbExtents& extents) const override;

public:
    virtual Acad::ErrorStatus dwgInFields(AcDbDwgFiler* pFiler) override;
    virtual Acad::ErrorStatus dwgOutFields(AcDbDwgFiler* pFiler) const override;
    virtual Acad::ErrorStatus dxfInFields(AcDbDxfFiler* pFiler) override;
    virtual Acad::ErrorStatus dxfOutFields(AcDbDxfFiler* pFiler) const override;

private:
    static Adesk::UInt32 kCurrentVersionNumber;

    double m_girdleDiameter;
    double m_tablePercent;
    double m_crownHeight;
    double m_crownAngle;
    double m_pavilionDepth;
    double m_pavilionAngle;
    double m_culetPercent;
    double m_girdleThickness;
    double m_starFacetLength;

    double m_absoluteCrownHeight;
    double m_absolutePavilionDepth;

private:
    void buildCrown(
        AcGePoint3dArray& vertices,
        AcGeVector3dArray& normals,
        AcArray<Adesk::Int32>& faceList,
        double girdleRadius,
        double girdleZ) const;

    void buildPavilion(
        AcGePoint3dArray& vertices,
        AcGeVector3dArray& normals,
        AcArray<Adesk::Int32>& faceList,
        double girdleRadius,
        double girdleZ) const;
};
