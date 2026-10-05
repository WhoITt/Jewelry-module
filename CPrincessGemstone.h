#pragma once

#include "CJewelryGemstone.h"

class CPrincessGemstone : public CJewelryGemstone
{
public:
    ACRX_DECLARE_MEMBERS(CPrincessGemstone);

public:
    CPrincessGemstone();
    CPrincessGemstone(const AcGePoint3d& center, double squareSide);
    virtual ~CPrincessGemstone();

public:
    double squareSide() const
    {
        assertReadEnabled();
        return m_squareSide;
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

    double totalHeight() const;

public:
    Acad::ErrorStatus setSquareSide(double side);
    Acad::ErrorStatus setCrownHeight(double percent);
    Acad::ErrorStatus setPavilionDepth(double percent);

    virtual Nano::ErrorStatus setTablePercent(double percent) override;
    virtual Nano::ErrorStatus setCrownAngle(double angle) override;
    virtual Nano::ErrorStatus setPavilionAngle(double angle) override;
    virtual Nano::ErrorStatus setCuletPercent(double percent) override;

public:
    virtual int facetCount() const override;
    virtual double volume() const override;
    virtual double caratWeight() const override;

    virtual AcString cutName() const override
    {
        return L"Princess Cut";
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

    double m_squareSide;
    double m_crownHeight;
    double m_pavilionDepth;

private:
    void buildCrown(
        AcGePoint3dArray& vertices,
        AcGeVector3dArray& normals,
        AcArray<Adesk::Int32>& faceList,
        double halfSide,
        double rundistZ) const;

    void buildPavilion(
        AcGePoint3dArray& vertices,
        AcGeVector3dArray& normals,
        AcArray<Adesk::Int32>& faceList,
        double halfSide,
        double rundistZ) const;
};