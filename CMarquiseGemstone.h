#pragma once

#include "CJewelryGemstone.h"

class CMarquiseGemstone : public CJewelryGemstone
{
public:
    ACRX_DECLARE_MEMBERS(CMarquiseGemstone);

    CMarquiseGemstone();
    CMarquiseGemstone(const AcGePoint3d& center, double length, double width, double totalDepth);
    virtual ~CMarquiseGemstone();

    double length() const { assertReadEnabled(); return m_length; }
    double width() const { assertReadEnabled(); return m_width; }
    double totalDepth() const { assertReadEnabled(); return m_totalDepth; }
    double crownHeight() const { assertReadEnabled(); return m_crownHeight; }
    double pavilionDepth() const { assertReadEnabled(); return m_pavilionDepth; }
    double tablePercent() const { assertReadEnabled(); return m_tablePercent; }

    Acad::ErrorStatus setLength(double length);
    Acad::ErrorStatus setWidth(double width);
    Acad::ErrorStatus setTotalDepth(double depth);
    Acad::ErrorStatus setTablePercent(double percent);

    virtual int facetCount() const override;
    virtual double volume() const override;
    virtual double caratWeight() const override;
    virtual AcString cutName() const override;

    double lengthToWidthRatio() const;
    double depthPercentage() const;
    bool isProportionIdeal() const;

protected:
    virtual void buildGemGeometry(
        AcGePoint3dArray& vertices,
        AcGeVector3dArray& normals,
        AcArray<Adesk::Int32>& faceList) const override;

    void calculateDerivedParameters();
    void createMarquiseProfile(AcGePoint3dArray& points, double scale, double z) const;

    virtual Acad::ErrorStatus subGetGripPoints(
        AcGePoint3dArray& gripPoints,
        AcDbIntArray& osnapModes,
        AcDbIntArray& geomIds) const override;

    virtual Acad::ErrorStatus subMoveGripPointsAt(
        const AcDbIntArray& indices,
        const AcGeVector3d& offset) override;

    virtual Acad::ErrorStatus subTransformBy(const AcGeMatrix3d& xform) override;
    virtual Acad::ErrorStatus subGetGeomExtents(AcDbExtents& extents) const override;

    virtual Acad::ErrorStatus dwgOutFields(AcDbDwgFiler* pFiler) const override;
    virtual Acad::ErrorStatus dwgInFields(AcDbDwgFiler* pFiler) override;
    virtual Acad::ErrorStatus dxfOutFields(AcDbDxfFiler* pFiler) const override;
    virtual Acad::ErrorStatus dxfInFields(AcDbDxfFiler* pFiler) override;

private:
    double m_length;
    double m_width;
    double m_totalDepth;
    double m_crownHeight;
    double m_pavilionDepth;
    double m_girdleThickness;
    double m_tablePercent;

    static constexpr double CROWN_HEIGHT_PERCENT = 14.0;
    static constexpr double PAVILION_DEPTH_PERCENT = 85.0;
    static constexpr double GIRDLE_PERCENT = 1.0;
    static constexpr double DEFAULT_TABLE_PERCENT = 58.0;

    static Adesk::UInt32 kCurrentVersionNumber;
};