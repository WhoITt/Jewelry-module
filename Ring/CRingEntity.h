#pragma once

#include "dbmain.h"
#include "dbents.h"
#include "dbid.h"
#include "gepnt3d.h"
#include "gept2dar.h"
#include "gevec3d.h"
#include "gemat3d.h"

class CRingEntity : public NcDbEntity
{
public:
    ACRX_DECLARE_MEMBERS(CRingEntity);

    enum RingProfileType
    {
        kProfileHalfRound = 0,
        kProfileFlat = 1,
        kProfileComfort = 2,
        kProfileConcave = 3
    };

    enum SettingType
    {
        kSettingNone = 0,
        kSettingProng = 1,
        kSettingBezel = 2
    };

    enum StoneShapeType
    {
        kStoneRound = 0,
        kStoneRect = 1,
        kStoneOval = 2,
        kStonePear = 3
    };

public:
    CRingEntity();
    CRingEntity(
        const AcGePoint3d& center,
        double innerRadius,
        double bandWidth,
        double thickness);
    virtual ~CRingEntity();

public:
    AcGePoint3d center() const
    {
        assertReadEnabled();
        return m_center;
    }

    double innerRadius() const
    {
        assertReadEnabled();
        return m_innerRadius;
    }

    double bandWidth() const
    {
        assertReadEnabled();
        return m_bandWidth;
    }

    double thickness() const
    {
        assertReadEnabled();
        return m_thickness;
    }

    RingProfileType profileType() const
    {
        assertReadEnabled();
        return m_profileType;
    }

    AcGePoint3d stoneCenter() const
    {
        assertReadEnabled();
        return m_stoneCenter;
    }

    double stoneSeatRadius() const
    {
        assertReadEnabled();
        return m_stoneSeatRadius;
    }

    bool hasStone() const
    {
        assertReadEnabled();
        return m_hasStone;
    }

    SettingType settingType() const
    {
        assertReadEnabled();
        return m_settingType;
    }

    int prongCount() const
    {
        assertReadEnabled();
        return m_prongCount;
    }

    double prongRadius() const
    {
        assertReadEnabled();
        return m_prongRadius;
    }

    double prongHeight() const
    {
        assertReadEnabled();
        return m_prongHeight;
    }

    double stoneLength() const
    {
        assertReadEnabled();
        return m_stoneLength;
    }

    double stoneWidth() const
    {
        assertReadEnabled();
        return m_stoneWidth;
    }

    double stoneHeight() const
    {
        assertReadEnabled();
        return m_stoneHeight;
    }

    StoneShapeType stoneShapeType() const
    {
        assertReadEnabled();
        return m_stoneShapeType;
    }

    AcDbObjectId linkedStoneId() const
    {
        assertReadEnabled();
        return m_linkedStoneId;
    }

public:
    Acad::ErrorStatus setCenter(const AcGePoint3d& center);
    Acad::ErrorStatus setInnerRadius(double radius);
    Acad::ErrorStatus setBandWidth(double width);
    Acad::ErrorStatus setThickness(double thickness);
    Acad::ErrorStatus setProfileType(RingProfileType type);
    Acad::ErrorStatus setStoneCenter(const AcGePoint3d& center);
    Acad::ErrorStatus setStoneSeatRadius(double radius);
    Acad::ErrorStatus setHasStone(bool value);
    Acad::ErrorStatus setSettingType(SettingType type);
    Acad::ErrorStatus setProngCount(int count);
    Acad::ErrorStatus setProngRadius(double radius);
    Acad::ErrorStatus setProngHeight(double height);
    Acad::ErrorStatus setStoneDimensions(double length, double width, double height);
    Acad::ErrorStatus setStoneShapeType(StoneShapeType type);
    Acad::ErrorStatus setLinkedStoneId(const AcDbObjectId& id);
    Acad::ErrorStatus clearLinkedStoneId();

public:
    void buildRingGeometry(
        AcGePoint3dArray& vertices,
        AcArray<Adesk::Int32>& faceList) const;

public:
    virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw* pWd) override;
    virtual Acad::ErrorStatus subTransformBy(const AcGeMatrix3d& xform) override;
    virtual Acad::ErrorStatus subGetGeomExtents(AcDbExtents& extents) const override;

    virtual Acad::ErrorStatus subGetGripPoints(
        AcGePoint3dArray& gripPoints,
        AcDbIntArray& osnapModes,
        AcDbIntArray& geomIds) const override;

    virtual Acad::ErrorStatus subMoveGripPointsAt(
        const AcDbIntArray& indices,
        const AcGeVector3d& offset) override;

public:
    virtual Acad::ErrorStatus dwgInFields(AcDbDwgFiler* pFiler) override;
    virtual Acad::ErrorStatus dwgOutFields(AcDbDwgFiler* pFiler) const override;
    virtual Acad::ErrorStatus dxfInFields(AcDbDxfFiler* pFiler) override;
    virtual Acad::ErrorStatus dxfOutFields(AcDbDxfFiler* pFiler) const override;

private:
    static Adesk::UInt32 kCurrentVersionNumber;
    static const int kRingSegmentCount = 64;
    static const int kProngSegmentCount = 10;

    enum GripIndex
    {
        kGripCenter = 0,
        kGripOuterRadius = 1,
        kGripInnerRadius = 2,
        kGripThickness = 3,
        kGripSeatCenter = 4,
        kGripSeatRadius = 5
    };

private:
    AcGePoint3d sweepProfilePoint(double sweepRadius, double localX, double localZ, int segment) const;
    void appendQuad(
        AcArray<Adesk::Int32>& faceList,
        Adesk::Int32 a,
        Adesk::Int32 b,
        Adesk::Int32 c,
        Adesk::Int32 d) const;
    void buildProfileContour(AcGePoint3dArray& profilePoints) const;
    void appendProngsGeometry(
        AcGePoint3dArray& vertices,
        AcArray<Adesk::Int32>& faceList) const;
    void appendBezelGeometry(
        AcGePoint3dArray& vertices,
        AcArray<Adesk::Int32>& faceList) const;
    void appendProngCylinder(
        const AcGePoint3d& baseCenter,
        AcGePoint3dArray& vertices,
        AcArray<Adesk::Int32>& faceList) const;
    void updateSeatCenterFromCurrentRing();
    void transformLinkedStone(const AcGeMatrix3d& xform);
    void syncLinkedStoneToSeat();

private:
    AcGePoint3d m_center;
    double m_innerRadius;
    double m_bandWidth;
    double m_thickness;
    RingProfileType m_profileType;
    AcGePoint3d m_stoneCenter;
    double m_stoneSeatRadius;
    bool m_hasStone;
    SettingType m_settingType;
    int m_prongCount;
    double m_prongRadius;
    double m_prongHeight;
    double m_stoneLength;
    double m_stoneWidth;
    double m_stoneHeight;
    int m_railStoneCount;
    StoneShapeType m_stoneShapeType;
    AcDbSoftPointerId m_linkedStoneId;
    AcArray<AcDbSoftPointerId> m_linkedRailStoneIds;
};
