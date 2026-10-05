#include "pch.h"
#include <QApplication>
#include <QColor>
#include "GemstoneSelectorDialogQt.h"
#include "RingSelectorDialogQt.h"
#include "framework.h"
#include "VKR_NRX.h"
#include "CCustomGemstone.h" 
#include "CPrincessGemstone.h"
#include "CRingEntity.h"
#include "CPearGemstone.h"
#include "CEmeraldGemstone.h"
#include "CMarquiseGemstone.h"

#include <cstdio>
#include <string>



static QApplication* g_pQApp = nullptr;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CMFCLibraryNrxProjectApp, CWinApp)
END_MESSAGE_MAP()

CMFCLibraryNrxProjectApp::CMFCLibraryNrxProjectApp() {}
CMFCLibraryNrxProjectApp theApp;

BOOL CMFCLibraryNrxProjectApp::InitInstance()
{
    CWinApp::InitInstance();
    return TRUE;
}

static void addToModelSpaceNc(NcDbObjectId& objId, NcDbEntity* pEnt)
{
    NcDbBlockTable* pBT = nullptr;
    ncdbHostApplicationServices()->workingDatabase()->getBlockTable(pBT, NcDb::kForRead);

    NcDbBlockTableRecord* pMS = nullptr;
    pBT->getAt(NCDB_MODEL_SPACE, pMS, NcDb::kForWrite);
    pBT->close();

    pMS->appendNcDbEntity(objId, pEnt);
    pMS->close();
}

static bool promptEntityId(const wchar_t* prompt, AcDbObjectId& objectId)
{
    ads_name entName;
    ads_point pickPoint;
    if (acedEntSel(prompt, entName, pickPoint) != RTNORM)
        return false;
    return acdbGetObjectId(objectId, entName) == Acad::eOk;
}

static void gemstoneDimensionsFromExtents(
    CJewelryGemstone* pGemstone,
    double& length,
    double& width,
    double& height)
{
    length = 2.0;
    width = 2.0;
    height = 1.5;

    if (pGemstone == nullptr)
        return;

    AcDbExtents extents;
    if (pGemstone->subGetGeomExtents(extents) != Acad::eOk)
        return;

    const AcGePoint3d minPt = extents.minPoint();
    const AcGePoint3d maxPt = extents.maxPoint();
    length = maxPt.x - minPt.x;
    width = maxPt.y - minPt.y;
    height = maxPt.z - minPt.z;

    if (length <= 1.0e-6) length = 2.0;
    if (width <= 1.0e-6) width = length;
    if (height <= 1.0e-6) height = 1.5;
}

static AcGePoint3d recommendedSeatCenter(const AcGePoint3d& ringCenter, double innerRadius, double bandWidth, double thickness)
{
    (void)thickness;
    const double outerRadius = innerRadius + bandWidth;
    return ringCenter + AcGeVector3d(0.0, 0.0, outerRadius);
}

static CRingEntity::StoneShapeType stoneShapeFromGemType(const QString& gemType)
{
    if (gemType == "emerald" || gemType == "princess")
        return CRingEntity::kStoneRect;
    if (gemType == "pear")
        return CRingEntity::kStonePear;
    if (gemType == "marquise")
        return CRingEntity::kStoneOval;
    return CRingEntity::kStoneRound;
}

static const wchar_t* profileNameRu(CRingEntity::RingProfileType profileType)
{
    switch (profileType)
    {
    case CRingEntity::kProfileHalfRound:
        return L"Полукруглый";
    case CRingEntity::kProfileFlat:
        return L"Плоский";
    case CRingEntity::kProfileComfort:
        return L"Овальный";
    case CRingEntity::kProfileConcave:
        return L"Вогнутый";
    default:
        return L"Неизвестный";
    }
}

static const wchar_t* settingNameRu(CRingEntity::SettingType settingType)
{
    switch (settingType)
    {
    case CRingEntity::kSettingProng:
        return L"Крапановая";
    case CRingEntity::kSettingBezel:
        return L"Глухая";
    case CRingEntity::kSettingNone:
    default:
        return L"Без закрепки";
    }
}

static const wchar_t* gemTypeNameRu(const QString& gemType)
{
    if (gemType == "custom")
        return L"Бриллиант";
    if (gemType == "princess")
        return L"Принцесса";
    if (gemType == "emerald")
        return L"Изумруд";
    if (gemType == "pear")
        return L"Груша";
    if (gemType == "marquise")
        return L"Маркиз";
    return L"Неизвестная";
}

static const wchar_t* ringColorNameRu(const QColor& color)
{
    if (color == QColor(212, 175, 55))
        return L"Золотой";
    if (color == QColor(192, 192, 192))
        return L"Серебристый";
    if (color == QColor(183, 110, 121))
        return L"Розовое золото";
    if (color == QColor(224, 224, 229))
        return L"Белое золото";
    if (color == QColor(180, 50, 50))
        return L"Красный";
    if (color == QColor(65, 150, 95))
        return L"Зелёный";
    if (color == QColor(60, 90, 170))
        return L"Синий";
    return L"Пользовательский RGB";
}

static CJewelryGemstone* createGemstoneByType(
    const QString& gemType,
    const AcGePoint3d& center,
    double length,
    double width,
    double height);

static void applyGemstoneAppearance(CJewelryGemstone* pGemstone, const QString& gemType);
static void resizeGemstoneByType(
    CJewelryGemstone* pGemstone,
    double length,
    double width,
    double height);

static void orientAndPlaceGemstone(
    CJewelryGemstone* pGemstone,
    const AcGePoint3d& seatCenter,
    double gemHeight)
{
    if (pGemstone == nullptr)
        return;

    AcDbExtents extents;
    if (pGemstone->subGetGeomExtents(extents) == Acad::eOk)
    {
        const AcGePoint3d currentAnchor(
            (extents.minPoint().x + extents.maxPoint().x) * 0.5,
            (extents.minPoint().y + extents.maxPoint().y) * 0.5,
            extents.minPoint().z);
        pGemstone->transformBy(AcGeMatrix3d::translation(seatCenter - currentAnchor));
    }
    else
    {
        const AcGePoint3d fallbackTarget = seatCenter + AcGeVector3d(0.0, 0.0, gemHeight * 0.5);
        pGemstone->transformBy(AcGeMatrix3d::translation(fallbackTarget - pGemstone->center()));
    }
}

static CJewelryGemstone* createGemstoneByType(
    const QString& gemType,
    const AcGePoint3d& center,
    double length,
    double width,
    double height)
{
    if (gemType == "emerald")
        return new CEmeraldGemstone(center, length, width, height);
    if (gemType == "princess")
        return new CPrincessGemstone(center, length);
    if (gemType == "pear")
        return new CPearGemstone(center, length, width, height);
    if (gemType == "marquise")
        return new CMarquiseGemstone(center, length, width, height);
    if (gemType == "custom")
    {
        CCustomGemstone* pRound = new CCustomGemstone(center, length);
        pRound->setTotalHeight(height);
        return pRound;
    }
    return nullptr;
}

static void applyGemstoneAppearance(CJewelryGemstone* pGemstone, const QString& gemType)
{
    if (pGemstone == nullptr)
        return;

    NcCmColor color;
    if (gemType == "emerald")
        color.setRGB(80, 200, 120);
    else if (gemType == "princess")
        color.setRGB(9, 76, 144);
    else if (gemType == "pear")
        color.setRGB(220, 20, 60);
    else if (gemType == "marquise")
        color.setRGB(255, 215, 0);
    else
        color.setRGB(230, 245, 255);

    pGemstone->setColor(color);
}

static void resizeGemstoneByType(
    CJewelryGemstone* pGemstone,
    double length,
    double width,
    double height)
{
    if (pGemstone == nullptr)
        return;

    if (CCustomGemstone* pRound = dynamic_cast<CCustomGemstone*>(pGemstone))
    {
        pRound->setGirdleDiameter(length);
        pRound->setTotalHeight(height);
        return;
    }

    if (CPrincessGemstone* pPrincess = dynamic_cast<CPrincessGemstone*>(pGemstone))
    {
        pPrincess->setSquareSide(length);
        return;
    }

    if (CEmeraldGemstone* pEmerald = dynamic_cast<CEmeraldGemstone*>(pGemstone))
    {
        pEmerald->setLength(length);
        pEmerald->setWidth(width);
        pEmerald->setTotalDepth(height);
        return;
    }

    if (CPearGemstone* pPear = dynamic_cast<CPearGemstone*>(pGemstone))
    {
        pPear->setLength(length);
        pPear->setWidth(width);
        pPear->setTotalDepth(height);
        return;
    }

    if (CMarquiseGemstone* pMarquise = dynamic_cast<CMarquiseGemstone*>(pGemstone))
    {
        pMarquise->setLength(length);
        pMarquise->setWidth(width);
        pMarquise->setTotalDepth(height);
    }
}

static QString gemstoneTypeFromEntity(CJewelryGemstone* pGemstone)
{
    if (dynamic_cast<CEmeraldGemstone*>(pGemstone) != nullptr)
        return "emerald";
    if (dynamic_cast<CPrincessGemstone*>(pGemstone) != nullptr)
        return "princess";
    if (dynamic_cast<CPearGemstone*>(pGemstone) != nullptr)
        return "pear";
    if (dynamic_cast<CMarquiseGemstone*>(pGemstone) != nullptr)
        return "marquise";
    return "custom";
}

static void gemstoneDimensionsFromEntity(
    CJewelryGemstone* pGemstone,
    double& length,
    double& width,
    double& height)
{
    if (CCustomGemstone* pRound = dynamic_cast<CCustomGemstone*>(pGemstone))
    {
        length = pRound->girdleDiameter();
        width = length;
        height = pRound->totalHeight();
        return;
    }

    if (CPrincessGemstone* pPrincess = dynamic_cast<CPrincessGemstone*>(pGemstone))
    {
        length = pPrincess->squareSide();
        width = length;
        height = pPrincess->totalHeight();
        return;
    }

    if (CEmeraldGemstone* pEmerald = dynamic_cast<CEmeraldGemstone*>(pGemstone))
    {
        length = pEmerald->length();
        width = pEmerald->width();
        height = pEmerald->totalDepth();
        return;
    }

    if (CPearGemstone* pPear = dynamic_cast<CPearGemstone*>(pGemstone))
    {
        length = pPear->length();
        width = pPear->width();
        height = pPear->totalDepth();
        return;
    }

    if (CMarquiseGemstone* pMarquise = dynamic_cast<CMarquiseGemstone*>(pGemstone))
    {
        length = pMarquise->length();
        width = pMarquise->width();
        height = pMarquise->totalDepth();
        return;
    }

    gemstoneDimensionsFromExtents(pGemstone, length, width, height);
}

static void applyRingDialogColor(CRingEntity* pRing, const RingSelectorDialogQt& dialog)
{
    if (pRing == nullptr)
        return;

    NcCmColor ringColor;
    const QColor qRingColor = dialog.selectedColor();
    ringColor.setRGB(qRingColor.red(), qRingColor.green(), qRingColor.blue());
    pRing->setColor(ringColor);
}

static void updateRingStoneData(
    CRingEntity* pRing,
    const RingSelectorDialogQt& dialog,
    const AcDbObjectId& stoneId)
{
    if (pRing == nullptr)
        return;

    pRing->setStoneSeatRadius(dialog.recommendedSeatRadius());
    pRing->setStoneDimensions(dialog.gemLength(), dialog.gemWidth(), dialog.gemHeight());
    pRing->setStoneShapeType(stoneShapeFromGemType(dialog.gemType()));

    const AcGePoint3d seatCenter = recommendedSeatCenter(
        pRing->center(),
        pRing->innerRadius(),
        pRing->bandWidth(),
        pRing->thickness());
    pRing->setStoneCenter(seatCenter);
    pRing->setLinkedStoneId(stoneId);
}

static void printGemstoneSummary(
    const wchar_t* title,
    const QString& gemType,
    CJewelryGemstone* pGemstone,
    double length,
    double width,
    double height)
{
    acutPrintf(L"\n%s", title);
    acutPrintf(L"\n  Огранка: %s", gemTypeNameRu(gemType));
    if (gemType == "custom")
        acutPrintf(L"\n  Диаметр: %.2f мм", length);
    else if (gemType == "princess")
        acutPrintf(L"\n  Сторона квадрата: %.2f мм", length);
    else
        acutPrintf(L"\n  Размеры: %.2f x %.2f x %.2f мм", length, width, height);
    acutPrintf(L"\n  Высота/глубина: %.2f мм", height);

    if (pGemstone != nullptr)
    {
        acutPrintf(L"\n  Вес: %.3f карат", pGemstone->caratWeight());
        acutPrintf(L"\n  Объем: %.3f мм^3", pGemstone->volume());
        acutPrintf(L"\n  Количество граней: %d", pGemstone->facetCount());
    }
}

static void printRingSummary(
    const wchar_t* title,
    const RingSelectorDialogQt& dialog,
    CJewelryGemstone* pGemstone)
{
    const CRingEntity::RingProfileType profileType =
        static_cast<CRingEntity::RingProfileType>(dialog.profileType());
    const bool hasStone = profileType != CRingEntity::kProfileConcave && dialog.hasStone();
    const CRingEntity::SettingType settingType = hasStone
        ? static_cast<CRingEntity::SettingType>(dialog.settingType())
        : CRingEntity::kSettingNone;
    const QColor color = dialog.selectedColor();

    acutPrintf(L"\n========================================");
    acutPrintf(L"\n%s", title);
    acutPrintf(L"\n----------------------------------------");
    acutPrintf(L"\nКольцо:");
    acutPrintf(L"\n  Профиль: %s", profileNameRu(profileType));
    acutPrintf(L"\n  Размер кольца: %.1f мм", dialog.ringSize());
    acutPrintf(L"\n  Внутренний радиус: %.2f мм", dialog.ringSize() * 0.5);
    acutPrintf(L"\n  Ширина шинки: %.2f мм", dialog.bandWidth());
    acutPrintf(L"\n  Высота профиля: %.2f мм", dialog.thickness());
    acutPrintf(L"\n  Цвет металла: %s (RGB %d, %d, %d)",
        ringColorNameRu(color),
        color.red(),
        color.green(),
        color.blue());

    acutPrintf(L"\nЗакрепка:");
    acutPrintf(L"\n  Тип: %s", settingNameRu(settingType));
    if (hasStone && settingType == CRingEntity::kSettingProng)
    {
        acutPrintf(L"\n  Количество крапанов: %d", dialog.prongCount());
        acutPrintf(L"\n  Радиус/толщина крапана: %.2f мм", dialog.recommendedProngRadius());
        acutPrintf(L"\n  Высота крапана: %.2f мм", dialog.recommendedProngHeight());
        acutPrintf(L"\n  Радиус посадочного места: %.2f мм", dialog.recommendedSeatRadius());
    }
    else if (hasStone && settingType == CRingEntity::kSettingBezel)
    {
        acutPrintf(L"\n  Высота борта: %.2f мм", dialog.recommendedBezelHeight());
        acutPrintf(L"\n  Толщина борта: %.2f мм", dialog.recommendedBezelThickness());
        acutPrintf(L"\n  Радиус посадочного места: %.2f мм", dialog.recommendedSeatRadius());
    }
    else
    {
        acutPrintf(L"\n  Размеры закрепки не используются.");
    }

    if (hasStone)
    {
        printGemstoneSummary(
            L"\nВставка:",
            dialog.gemType(),
            pGemstone,
            dialog.gemLength(),
            dialog.gemWidth(),
            dialog.gemHeight());
    }
    else
    {
        acutPrintf(L"\nВставка: не используется.");
    }

    acutPrintf(L"\n========================================");
}

static std::wstring ensureStlExtension(const wchar_t* path)
{
    std::wstring result = path != nullptr ? path : L"";
    if (result.length() < 4 ||
        _wcsicmp(result.c_str() + result.length() - 4, L".stl") != 0)
    {
        result += L".stl";
    }
    return result;
}

static AcGeVector3d triangleNormal(
    const AcGePoint3d& a,
    const AcGePoint3d& b,
    const AcGePoint3d& c)
{
    AcGeVector3d normal = (b - a).crossProduct(c - a);
    if (normal.length() > 1.0e-9)
        normal.normalize();
    else
        normal = AcGeVector3d(0.0, 0.0, 1.0);
    return normal;
}

static int countStlTriangles(
    const AcGePoint3dArray& vertices,
    const AcArray<Adesk::Int32>& faceList)
{
    int triangleCount = 0;
    int pos = 0;
    while (pos < faceList.length())
    {
        const int count = faceList[pos++];
        if (count < 3 || pos + count > faceList.length())
            break;

        for (int i = 1; i < count - 1; ++i)
        {
            const int indexA = faceList[pos];
            const int indexB = faceList[pos + i];
            const int indexC = faceList[pos + i + 1];
            if (indexA >= 0 && indexA < vertices.length() &&
                indexB >= 0 && indexB < vertices.length() &&
                indexC >= 0 && indexC < vertices.length())
            {
                ++triangleCount;
            }
        }
        pos += count;
    }
    return triangleCount;
}

static bool writeBinaryStlTriangle(
    FILE* pFile,
    const AcGePoint3d& a,
    const AcGePoint3d& b,
    const AcGePoint3d& c)
{
    if (pFile == nullptr)
        return false;

    const AcGeVector3d n = triangleNormal(a, b, c);
    const float data[12] = {
        static_cast<float>(n.x), static_cast<float>(n.y), static_cast<float>(n.z),
        static_cast<float>(a.x), static_cast<float>(a.y), static_cast<float>(a.z),
        static_cast<float>(b.x), static_cast<float>(b.y), static_cast<float>(b.z),
        static_cast<float>(c.x), static_cast<float>(c.y), static_cast<float>(c.z)
    };
    const unsigned short attributeByteCount = 0;
    return fwrite(data, sizeof(float), 12, pFile) == 12 &&
        fwrite(&attributeByteCount, sizeof(attributeByteCount), 1, pFile) == 1;
}

static bool writeRingGeometryToStl(
    const std::wstring& filePath,
    const AcGePoint3dArray& vertices,
    const AcArray<Adesk::Int32>& faceList,
    int& triangleCount)
{
    triangleCount = 0;

    const int countedTriangles = countStlTriangles(vertices, faceList);
    if (countedTriangles <= 0)
        return false;

    FILE* pFile = nullptr;
    if (_wfopen_s(&pFile, filePath.c_str(), L"wb") != 0 || pFile == nullptr)
        return false;

    char header[80] = {};
    const char headerText[] = "VKR_NRX ring mesh - metal and setting only";
    memcpy(header, headerText, sizeof(headerText) - 1);
    const unsigned int stlTriangleCount = static_cast<unsigned int>(countedTriangles);
    fwrite(header, sizeof(header), 1, pFile);
    fwrite(&stlTriangleCount, sizeof(stlTriangleCount), 1, pFile);

    int pos = 0;
    while (pos < faceList.length())
    {
        const int count = faceList[pos++];
        if (count < 3 || pos + count > faceList.length())
            break;

        const int baseIndex = faceList[pos];
        for (int i = 1; i < count - 1; ++i)
        {
            const int indexA = baseIndex;
            const int indexB = faceList[pos + i];
            const int indexC = faceList[pos + i + 1];
            if (indexA >= 0 && indexA < vertices.length() &&
                indexB >= 0 && indexB < vertices.length() &&
                indexC >= 0 && indexC < vertices.length())
            {
                if (!writeBinaryStlTriangle(
                    pFile,
                    vertices[indexA],
                    vertices[indexB],
                    vertices[indexC]))
                {
                    fclose(pFile);
                    return false;
                }
                ++triangleCount;
            }
        }
        pos += count;
    }

    fclose(pFile);
    return triangleCount == countedTriangles;
}

void cmdCreateRing()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (!g_pQApp)
    {
        int argc = 0;
        g_pQApp = new QApplication(argc, nullptr);
    }

    RingSelectorDialogQt dialog;
    if (dialog.exec() != QDialog::Accepted)
    {
        acutPrintf(L"\nОперация отменена.");
        return;
    }

    ads_point centerPt;
    if (acedGetPoint(NULL, L"\nУкажите центр кольца: ", centerPt) != RTNORM)
    {
        acutPrintf(L"\nТочка вставки не выбрана.");
        return;
    }

    const double ringSize = dialog.ringSize();
    const double innerRadius = ringSize * 0.5;
    const double bandWidth = dialog.bandWidth();
    const double thickness = dialog.thickness();
    if (innerRadius <= 0.0 || bandWidth <= 0.0 || thickness <= 0.0)
    {
        acutPrintf(L"\nПараметры кольца должны быть больше нуля.");
        return;
    }

    CRingEntity* pRing = new CRingEntity(
        AcGePoint3d(centerPt[0], centerPt[1], centerPt[2]),
        innerRadius,
        bandWidth,
        thickness
    );

    const CRingEntity::RingProfileType profileType =
        static_cast<CRingEntity::RingProfileType>(dialog.profileType());
    const bool allowStoneForProfile = profileType != CRingEntity::kProfileConcave;
    const bool hasStone = allowStoneForProfile && dialog.hasStone();
    const CRingEntity::SettingType settingType = hasStone
        ? static_cast<CRingEntity::SettingType>(dialog.settingType())
        : CRingEntity::kSettingNone;

    pRing->setProfileType(profileType);
    pRing->setHasStone(hasStone);
    pRing->setSettingType(settingType);
    NcCmColor ringColor;
    const QColor qRingColor = dialog.selectedColor();
    ringColor.setRGB(qRingColor.red(), qRingColor.green(), qRingColor.blue());
    pRing->setColor(ringColor);

    const AcGePoint3d ringCenter(centerPt[0], centerPt[1], centerPt[2]);
    const AcGePoint3d seatCenter = recommendedSeatCenter(ringCenter, innerRadius, bandWidth, thickness);

    if (hasStone)
    {
        pRing->setStoneCenter(seatCenter);
        pRing->setStoneSeatRadius(dialog.recommendedSeatRadius());
        pRing->setProngCount(dialog.prongCount());
        pRing->setProngRadius(dialog.recommendedProngRadius());
        pRing->setProngHeight(
            dialog.settingType() == RingSelectorDialogQt::kSettingBezel
                ? dialog.recommendedBezelHeight()
                : dialog.recommendedProngHeight());
        pRing->setStoneDimensions(dialog.gemLength(), dialog.gemWidth(), dialog.gemHeight());
        pRing->setStoneShapeType(stoneShapeFromGemType(dialog.gemType()));
    }
    else
    {
        pRing->clearLinkedStoneId();
    }

    pRing->setDatabaseDefaults();

    NcDbObjectId objId;
    addToModelSpaceNc(objId, pRing);
    pRing->close();

    bool summaryPrinted = false;
    if (hasStone)
    {
        const QString gemType = dialog.gemType();
        const double gemLength = dialog.gemLength();
        const double gemWidth = dialog.gemWidth();
        const double gemHeight = dialog.gemHeight();

        const AcGePoint3d gemCenter = ringCenter;
        CJewelryGemstone* pGemstone = createGemstoneByType(
            gemType,
            gemCenter,
            gemLength,
            gemWidth,
            gemHeight);

        if (pGemstone != nullptr)
        {
            applyGemstoneAppearance(pGemstone, gemType);
            orientAndPlaceGemstone(pGemstone, seatCenter, gemHeight);
            pGemstone->setDatabaseDefaults();
            NcDbObjectId gemObjId;
            addToModelSpaceNc(gemObjId, pGemstone);

            CRingEntity* pRingForLink = nullptr;
            if (acdbOpenObject(pRingForLink, objId, AcDb::kForWrite) == Acad::eOk && pRingForLink != nullptr)
            {
                pRingForLink->setLinkedStoneId(gemObjId);
                pRingForLink->close();
            }

            printRingSummary(L"Кольцо создано успешно!", dialog, pGemstone);
            summaryPrinted = true;
            pGemstone->close();
        }
    }

    if (!summaryPrinted)
        printRingSummary(L"Кольцо создано успешно!", dialog, nullptr);
}

void cmdAttachStoneToRing()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (!g_pQApp)
    {
        int argc = 0;
        g_pQApp = new QApplication(argc, nullptr);
    }

    AcDbObjectId ringId;
    if (!promptEntityId(L"\nВыберите кольцо: ", ringId))
    {
        acutPrintf(L"\nКольцо не выбрано.");
        return;
    }

    CRingEntity* pRing = nullptr;
    if (acdbOpenObject(pRing, ringId, AcDb::kForWrite) != Acad::eOk || pRing == nullptr)
    {
        acutPrintf(L"\nНе удалось открыть кольцо.");
        return;
    }

    AcDbObjectId stoneId;
    if (!promptEntityId(L"\nВыберите камень: ", stoneId))
    {
        pRing->close();
        acutPrintf(L"\nКамень не выбран.");
        return;
    }

    CJewelryGemstone* pGemstone = nullptr;
    if (acdbOpenObject(pGemstone, stoneId, AcDb::kForWrite) != Acad::eOk || pGemstone == nullptr)
    {
        pRing->close();
        acutPrintf(L"\nНе удалось открыть камень.");
        return;
    }

    RingSelectorDialogQt dialog;
    dialog.setProfileType(static_cast<int>(pRing->profileType()));
    dialog.setRingSize(pRing->innerRadius() * 2.0);
    dialog.setBandWidth(pRing->bandWidth());
    dialog.setThickness(pRing->thickness());
    const bool allowStoneForProfile = pRing->profileType() != CRingEntity::kProfileConcave;
    dialog.setHasStone(allowStoneForProfile);
    dialog.setSettingType(static_cast<RingSelectorDialogQt::SettingType>(
        allowStoneForProfile ? pRing->settingType() : CRingEntity::kSettingNone));
    dialog.setProngCount(pRing->prongCount());

    QString detectedGemType = "custom";
    if (dynamic_cast<CEmeraldGemstone*>(pGemstone) != nullptr) detectedGemType = "emerald";
    else if (dynamic_cast<CPrincessGemstone*>(pGemstone) != nullptr) detectedGemType = "princess";
    else if (dynamic_cast<CPearGemstone*>(pGemstone) != nullptr) detectedGemType = "pear";
    else if (dynamic_cast<CMarquiseGemstone*>(pGemstone) != nullptr) detectedGemType = "marquise";
    dialog.setGemType(detectedGemType);

    double gemLength = 2.0;
    double gemWidth = 2.0;
    double gemHeight = 1.5;
    gemstoneDimensionsFromExtents(pGemstone, gemLength, gemWidth, gemHeight);
    dialog.setGemDimensions(gemLength, gemWidth, gemHeight);

    if (dialog.exec() != QDialog::Accepted)
    {
        pGemstone->close();
        pRing->close();
        acutPrintf(L"\nПривязка отменена.");
        return;
    }

    const CRingEntity::RingProfileType profileType =
        static_cast<CRingEntity::RingProfileType>(dialog.profileType());
    const bool allowStoneAfterDialog = profileType != CRingEntity::kProfileConcave;
    const bool hasStone = allowStoneAfterDialog && dialog.hasStone();
    const CRingEntity::SettingType settingType = hasStone
        ? static_cast<CRingEntity::SettingType>(dialog.settingType())
        : CRingEntity::kSettingNone;

    pRing->setProfileType(profileType);
    pRing->setInnerRadius(dialog.ringSize() * 0.5);
    pRing->setBandWidth(dialog.bandWidth());
    pRing->setThickness(dialog.thickness());
    pRing->setHasStone(hasStone);
    pRing->setSettingType(settingType);
    NcCmColor ringColor;
    const QColor qRingColor = dialog.selectedColor();
    ringColor.setRGB(qRingColor.red(), qRingColor.green(), qRingColor.blue());
    pRing->setColor(ringColor);
    pRing->setProngCount(dialog.prongCount());
    pRing->setProngRadius(dialog.recommendedProngRadius());
    pRing->setProngHeight(
        dialog.settingType() == RingSelectorDialogQt::kSettingBezel
            ? dialog.recommendedBezelHeight()
            : dialog.recommendedProngHeight());
    if (hasStone)
    {
        pRing->setStoneSeatRadius(dialog.recommendedSeatRadius());
        pRing->setStoneDimensions(dialog.gemLength(), dialog.gemWidth(), dialog.gemHeight());
        pRing->setStoneShapeType(stoneShapeFromGemType(dialog.gemType()));

        const AcGePoint3d seatCenter = recommendedSeatCenter(
            pRing->center(),
            pRing->innerRadius(),
            pRing->bandWidth(),
            pRing->thickness());
        pRing->setStoneCenter(seatCenter);
        pRing->setLinkedStoneId(stoneId);
        resizeGemstoneByType(pGemstone, dialog.gemLength(), dialog.gemWidth(), dialog.gemHeight());
        orientAndPlaceGemstone(pGemstone, seatCenter, dialog.gemHeight());
    }
    else
    {
        pRing->clearLinkedStoneId();
    }

    printRingSummary(L"Камень привязан к кольцу.", dialog, pGemstone);
    pGemstone->close();
    pRing->close();
}

void cmdEditGemstone()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (!g_pQApp)
    {
        int argc = 0;
        g_pQApp = new QApplication(argc, nullptr);
    }

    AcDbObjectId stoneId;
    if (!promptEntityId(L"\nВыберите камень для редактирования: ", stoneId))
    {
        acutPrintf(L"\nКамень не выбран.");
        return;
    }

    CJewelryGemstone* pGemstone = nullptr;
    if (acdbOpenObject(pGemstone, stoneId, AcDb::kForWrite) != Acad::eOk || pGemstone == nullptr)
    {
        acutPrintf(L"\nВыбранный объект не является камнем.");
        return;
    }

    QString gemType = gemstoneTypeFromEntity(pGemstone);
    double length = 2.0;
    double width = 2.0;
    double height = 1.5;
    gemstoneDimensionsFromEntity(pGemstone, length, width, height);

    GemstoneSelectorDialogQt dialog;
    dialog.setWindowTitle(QString::fromWCharArray(L"Редактирование камня"));
    dialog.setGemType(gemType);
    dialog.setGemDimensions(length, width, height);
    dialog.setQuantity(1);
    dialog.setQuantityEnabled(false);
    dialog.setGemTypeEnabled(false);

    if (dialog.exec() != QDialog::Accepted)
    {
        pGemstone->close();
        acutPrintf(L"\nРедактирование камня отменено.");
        return;
    }

    resizeGemstoneByType(pGemstone, dialog.getLength(), dialog.getWidth(), dialog.getHeight());
    applyGemstoneAppearance(pGemstone, gemType);
    printGemstoneSummary(
        L"\nКамень обновлен:",
        gemType,
        pGemstone,
        dialog.getLength(),
        dialog.getWidth(),
        dialog.getHeight());
    pGemstone->close();
}

void cmdEditRing()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (!g_pQApp)
    {
        int argc = 0;
        g_pQApp = new QApplication(argc, nullptr);
    }

    AcDbObjectId ringId;
    if (!promptEntityId(L"\nВыберите кольцо для редактирования: ", ringId))
    {
        acutPrintf(L"\nКольцо не выбрано.");
        return;
    }

    CRingEntity* pRing = nullptr;
    if (acdbOpenObject(pRing, ringId, AcDb::kForWrite) != Acad::eOk || pRing == nullptr)
    {
        acutPrintf(L"\nВыбранный объект не является кольцом.");
        return;
    }

    AcDbObjectId linkedStoneId = pRing->linkedStoneId();
    CJewelryGemstone* pLinkedStone = nullptr;
    QString gemType = "custom";
    QString originalGemType = "custom";
    double gemLength = pRing->stoneLength();
    double gemWidth = pRing->stoneWidth();
    double gemHeight = pRing->stoneHeight();

    if (!linkedStoneId.isNull() &&
        acdbOpenObject(pLinkedStone, linkedStoneId, AcDb::kForWrite) == Acad::eOk &&
        pLinkedStone != nullptr)
    {
        gemType = gemstoneTypeFromEntity(pLinkedStone);
        originalGemType = gemType;
        gemstoneDimensionsFromEntity(pLinkedStone, gemLength, gemWidth, gemHeight);
    }
    else
    {
        linkedStoneId = AcDbObjectId::kNull;
    }

    RingSelectorDialogQt dialog;
    dialog.setWindowTitle(QString::fromWCharArray(L"Редактирование кольца"));
    dialog.setProfileType(static_cast<int>(pRing->profileType()));
    dialog.setRingSize(pRing->innerRadius() * 2.0);
    dialog.setBandWidth(pRing->bandWidth());
    dialog.setThickness(pRing->thickness());
    dialog.setHasStone(pRing->hasStone() && pRing->profileType() != CRingEntity::kProfileConcave);
    dialog.setSettingType(static_cast<RingSelectorDialogQt::SettingType>(
        pRing->hasStone() ? pRing->settingType() : CRingEntity::kSettingNone));
    dialog.setProngCount(pRing->prongCount());
    dialog.setGemType(gemType);
    dialog.setGemDimensions(gemLength, gemWidth, gemHeight);

    if (dialog.exec() != QDialog::Accepted)
    {
        if (pLinkedStone != nullptr)
            pLinkedStone->close();
        pRing->close();
        acutPrintf(L"\nРедактирование кольца отменено.");
        return;
    }

    const CRingEntity::RingProfileType profileType =
        static_cast<CRingEntity::RingProfileType>(dialog.profileType());
    const bool hasStone = profileType != CRingEntity::kProfileConcave && dialog.hasStone();

    pRing->setProfileType(profileType);
    pRing->setInnerRadius(dialog.ringSize() * 0.5);
    pRing->setBandWidth(dialog.bandWidth());
    pRing->setThickness(dialog.thickness());
    pRing->setHasStone(hasStone);
    pRing->setSettingType(hasStone
        ? static_cast<CRingEntity::SettingType>(dialog.settingType())
        : CRingEntity::kSettingNone);
    applyRingDialogColor(pRing, dialog);
    pRing->setProngCount(dialog.prongCount());
    pRing->setProngRadius(dialog.recommendedProngRadius());
    pRing->setProngHeight(
        dialog.settingType() == RingSelectorDialogQt::kSettingBezel
            ? dialog.recommendedBezelHeight()
            : dialog.recommendedProngHeight());

    bool summaryPrinted = false;
    if (hasStone)
    {
        if (pLinkedStone != nullptr && dialog.gemType() != originalGemType)
        {
            pLinkedStone->erase();
            pLinkedStone->close();
            pLinkedStone = nullptr;
            linkedStoneId = AcDbObjectId::kNull;
        }

        if (pLinkedStone == nullptr)
        {
            pLinkedStone = createGemstoneByType(
                dialog.gemType(),
                pRing->center(),
                dialog.gemLength(),
                dialog.gemWidth(),
                dialog.gemHeight());

            if (pLinkedStone != nullptr)
            {
                applyGemstoneAppearance(pLinkedStone, dialog.gemType());
                orientAndPlaceGemstone(
                    pLinkedStone,
                    recommendedSeatCenter(pRing->center(), pRing->innerRadius(), pRing->bandWidth(), pRing->thickness()),
                    dialog.gemHeight());
                pLinkedStone->setDatabaseDefaults();
                addToModelSpaceNc(linkedStoneId, pLinkedStone);
            }
        }

        if (pLinkedStone != nullptr)
        {
            resizeGemstoneByType(pLinkedStone, dialog.gemLength(), dialog.gemWidth(), dialog.gemHeight());
            orientAndPlaceGemstone(
                pLinkedStone,
                recommendedSeatCenter(pRing->center(), pRing->innerRadius(), pRing->bandWidth(), pRing->thickness()),
                dialog.gemHeight());
            updateRingStoneData(pRing, dialog, linkedStoneId);
            printRingSummary(L"Кольцо обновлено!", dialog, pLinkedStone);
            summaryPrinted = true;
            pLinkedStone->close();
        }
    }
    else
    {
        pRing->clearLinkedStoneId();
        if (pLinkedStone != nullptr)
            pLinkedStone->close();
    }

    if (!summaryPrinted)
        printRingSummary(L"Кольцо обновлено!", dialog, nullptr);

    pRing->close();
}

void cmdExportRingStl()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    AcDbObjectId ringId;
    if (!promptEntityId(L"\nВыберите кольцо для экспорта STL: ", ringId))
    {
        acutPrintf(L"\nКольцо не выбрано.");
        return;
    }

    CRingEntity* pRing = nullptr;
    if (acdbOpenObject(pRing, ringId, AcDb::kForRead) != Acad::eOk || pRing == nullptr)
    {
        acutPrintf(L"\nВыбранный объект не является кольцом.");
        return;
    }

    wchar_t pathBuffer[1024] = {};
    if (acedGetString(Adesk::kTrue, L"\nУкажите путь STL-файла: ", pathBuffer) != RTNORM)
    {
        pRing->close();
        acutPrintf(L"\nЭкспорт STL отменен.");
        return;
    }

    const std::wstring filePath = ensureStlExtension(pathBuffer);
    AcGePoint3dArray vertices;
    AcArray<Adesk::Int32> faceList;
    pRing->buildRingGeometry(vertices, faceList);

    int triangleCount = 0;
    const bool ok = writeRingGeometryToStl(filePath, vertices, faceList, triangleCount);
    pRing->close();

    if (!ok)
    {
        acutPrintf(L"\nНе удалось записать STL-файл.");
        return;
    }

    acutPrintf(L"\n========================================");
    acutPrintf(L"\nSTL экспорт выполнен.");
    acutPrintf(L"\n----------------------------------------");
    acutPrintf(L"\nФайл: %s", filePath.c_str());
    acutPrintf(L"\nЭкспортирована геометрия кольца и закрепки.");
    acutPrintf(L"\nКамни не экспортировались: они используются как визуальные вставки и параметры посадки.");
    acutPrintf(L"\nКоличество треугольников: %d", triangleCount);
    acutPrintf(L"\n========================================");
}
void initCustomClasses()
{
    
    
    CCustomGemstone::rxInit();          
    CPrincessGemstone::rxInit();
    CPearGemstone::rxInit();
    CEmeraldGemstone::rxInit();
    CMarquiseGemstone::rxInit();
    CRingEntity::rxInit();
    acrxBuildClassHierarchy();

    acutPrintf(L"\n Класс CCustomGemstone зарегистрирован");
    acutPrintf(L"\n Базовый класс: CJewelryGemstone (абстрактный)");
    acutPrintf(L"\n Базовый класс: AcDbEntity");
    acutPrintf(L"\n Класс CPearGemstone зарегистрирован");
}

void uninitCustomClasses()
{
    deleteAcRxClass(CCustomGemstone::desc());  
    deleteAcRxClass(CPrincessGemstone::desc());
    deleteAcRxClass(CPearGemstone::desc());
    deleteAcRxClass(CEmeraldGemstone::desc());
    deleteAcRxClass(CMarquiseGemstone::desc());
    deleteAcRxClass(CRingEntity::desc());
    acutPrintf(L"\n Класс CCustomGemstone удален");
}
void cmdInsertGemstone()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

   
    if (!g_pQApp)
    {
        int argc = 0;
        g_pQApp = new QApplication(argc, nullptr);
    }

    GemstoneSelectorDialogQt dialog;
    if (dialog.exec() == QDialog::Accepted)
    {
        QString gemType = dialog.getGemType();
        int quantity = dialog.getQuantity();
        double length = dialog.getLength();
        double width = dialog.getWidth();
        double height = dialog.getHeight();
        wchar_t typeBuf[512];
        memset(typeBuf, 0, sizeof(typeBuf));
        int len = gemType.toWCharArray(typeBuf);
        typeBuf[len] = 0;



  
        for (int i = 0; i < quantity; i++)
        {
            // Запросить точку вставки
            ads_point pt;
            wchar_t prompt[256];
            swprintf(prompt, 256, L"\nУкажите точку вставки камня %d из %d: ", i + 1, quantity);

            if (acedGetPoint(NULL, prompt, pt) != RTNORM)
            {
                acutPrintf(L"\n✗ Отменено пользователем\n");
                break;  // Выход из цикла при отмене
            }

            AcGePoint3d center(pt[0], pt[1], pt[2]);
            CJewelryGemstone* pGemstone = createGemstoneByType(gemType, center, length, width, height);

            if (pGemstone)
            {
                applyGemstoneAppearance(pGemstone, gemType);
                double weight = pGemstone->caratWeight();
                double volume = pGemstone->volume();
                int facets = pGemstone->facetCount();
                pGemstone->setDatabaseDefaults();
                NcDbObjectId objId;
                addToModelSpaceNc(objId, pGemstone);
                acutPrintf(L"\n----------------------------------------");
                acutPrintf(L"\n✓ Камень %d создан успешно", i + 1);
                acutPrintf(L"\n  Тип: %s", typeBuf);
                acutPrintf(L"\n  Вес: %.3f карат", weight);
                acutPrintf(L"\n  Объем: %.3f мм³", volume);
                acutPrintf(L"\n  Количество граней: %d", facets);
                acutPrintf(L"\n----------------------------------------");
                pGemstone->close();
            }
        }

        acutPrintf(L"\nСоздание завершено!\n");
    }
    else
    {
        acutPrintf(L"\nОтменено пользователем.\n");
    }
}



extern "C" __declspec(dllexport) AcRx::AppRetCode
ncrxEntryPoint(AcRx::AppMsgCode msg, void* appId)
{
    switch (msg)
    {
    case AcRx::kInitAppMsg:
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        acrxDynamicLinker->unlockApplication(appId);
        acrxDynamicLinker->registerAppMDIAware(appId);

        initCustomClasses();

        acedRegCmds->addCommand(
            L"VKR_GEMSTONE",
            L"GLOBAL_GEM",
            L"GEM",
            ACRX_CMD_MODAL,
            cmdInsertGemstone
        );

        acedRegCmds->addCommand(
            L"VKR_JEWELRY_COMMANDS",
            L"RING",
            L"RING",
            ACRX_CMD_MODAL,
            cmdCreateRing
        );

        acedRegCmds->addCommand(
            L"VKR_JEWELRY_COMMANDS",
            L"ATTACHSTONE",
            L"ATTACHSTONE",
            ACRX_CMD_MODAL,
            cmdAttachStoneToRing
        );

        acedRegCmds->addCommand(
            L"VKR_GEMSTONE",
            L"EDITGEM",
            L"EDITGEM",
            ACRX_CMD_MODAL,
            cmdEditGemstone
        );

        acedRegCmds->addCommand(
            L"VKR_JEWELRY_COMMANDS",
            L"EDITRING",
            L"EDITRING",
            ACRX_CMD_MODAL,
            cmdEditRing
        );

        acedRegCmds->addCommand(
            L"VKR_JEWELRY_COMMANDS",
            L"EXPORTSTL",
            L"EXPORTSTL",
            ACRX_CMD_MODAL,
            cmdExportRingStl
        );

        acutPrintf(L"\nVKR NRX Application v4.0 загружено");
        acutPrintf(L"\nДоступные команды:");
        acutPrintf(L"\n  GEM / RING / ATTACHSTONE / EDITGEM / EDITRING / EXPORTSTL");

        break;
    }

    case AcRx::kUnloadAppMsg:
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        acedRegCmds->removeGroup(L"VKR_GEMSTONE");
        acedRegCmds->removeGroup(L"VKR_JEWELRY_COMMANDS");

        uninitCustomClasses();

        if (g_pQApp)
        {
            delete g_pQApp;
            g_pQApp = nullptr;
        }

        acutPrintf(L"\nVKR NRX Application v4.0 выгружено\n");
        break;
    }

    default:
        break;
    }

    return AcRx::kRetOK;
}

