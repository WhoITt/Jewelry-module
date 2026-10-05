#pragma once

#include "CJewelryGemstone.h"

class CPearGemstone : public CJewelryGemstone
{
public:
    ACRX_DECLARE_MEMBERS(CPearGemstone);

public:
    CPearGemstone();
    CPearGemstone(const AcGePoint3d& center, double length, double width, double totalDepth);
    virtual ~CPearGemstone();

public:
    Acad::ErrorStatus setLength(double length);
    Acad::ErrorStatus setWidth(double width);
    Acad::ErrorStatus setTotalDepth(double depth);

public:
    double length() const { return m_length; }
    double width() const { return m_width; }
    double totalDepth() const { return m_totalDepth; }

public:
    virtual int facetCount() const override;
    virtual double volume() const override;
    virtual double caratWeight() const override;
    virtual AcString cutName() const override;

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
    double lengthToWidthRatio() const;
    double depthPercentage() const;
    bool isProportionIdeal() const;

public:
    virtual Acad::ErrorStatus dwgInFields(AcDbDwgFiler* pFiler) override;
    virtual Acad::ErrorStatus dwgOutFields(AcDbDwgFiler* pFiler) const override;
    virtual Acad::ErrorStatus dxfInFields(AcDbDxfFiler* pFiler) override;
    virtual Acad::ErrorStatus dxfOutFields(AcDbDxfFiler* pFiler) const override;

protected:
    virtual Acad::ErrorStatus subTransformBy(const AcGeMatrix3d& xform) override;

private:
    static Adesk::UInt32 kCurrentVersionNumber;

    double m_length;
    double m_width;
    double m_totalDepth;

    double m_crownHeight;
    double m_pavilionDepth;
    double m_girdleThickness;
    double m_tablePercent;

    double m_crownAngle;
    double m_pavilionAngle;
    double m_culetPercent;

private:
    void calculateDerivedParameters();
    void createPearProfile(AcGePoint3dArray& points) const;

    static constexpr double DEFAULT_LENGTH_TO_WIDTH = 1.50;
    static constexpr double DEFAULT_DEPTH_PERCENT = 61.0;
    static constexpr double CROWN_HEIGHT_PERCENT = 14.0;
    static constexpr double PAVILION_DEPTH_PERCENT = 85.0;
    static constexpr double GIRDLE_PERCENT = 1.0;
    static constexpr double TABLE_SIZE_PERCENT = 58.0;
};