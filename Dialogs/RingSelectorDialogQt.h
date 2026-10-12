#pragma once

#include <QDialog>
#include "GemSizeCatalog.h"

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QSlider;
class QColor;

class RingSelectorDialogQt : public QDialog
{
public:
    enum SettingType
    {
        kSettingNone = 0,
        kSettingProng = 1,
        kSettingBezel = 2
    };

    explicit RingSelectorDialogQt(QWidget* parent = nullptr);
    ~RingSelectorDialogQt() override = default;

    int profileType() const;
    double ringSize() const;
    double bandWidth() const;
    double thickness() const;

    bool hasStone() const;
    QString gemType() const;
    double gemLength() const;
    double gemWidth() const;
    double gemHeight() const;

    SettingType settingType() const;
    int prongCount() const;

    void setProfileType(int value);
    void setRingSize(double value);
    void setBandWidth(double value);
    void setThickness(double value);
    void setHasStone(bool value);
    void setGemType(const QString& value);
    void setGemDimensions(double length, double width, double height);
    void setSettingType(SettingType value);
    void setProngCount(int value);
    void setGemTypeEnabled(bool enabled);
    void setSelectedColor(const QColor& color, const QString& name);

    double recommendedSeatRadius() const;
    double recommendedProngRadius() const;
    double recommendedProngHeight() const;
    double recommendedBezelHeight() const;
    double recommendedBezelThickness() const;
    QColor selectedColor() const;

private:
    void setupUi();
    void setupConnections();
    void updateGemControls();
    void updateSettingOptions();
    void updateStoneLimits();
    void updatePreview();
    void openStoneSizeList();
    void applyStoneEntryAt(int index);
    void updateProfilePreview();
    void applyMetalColorPreset(const QColor& color);
    void updateColorPreview();
    void selectPaletteColor(const QColor& color, const QString& name, bool custom);
    void updatePreviewVisibility();

private:
    QComboBox* m_comboProfile = nullptr;
    QDoubleSpinBox* m_spinRingSize = nullptr;
    QDoubleSpinBox* m_spinBandWidth = nullptr;
    QDoubleSpinBox* m_spinThickness = nullptr;
    QSpinBox* m_spinColorR = nullptr;
    QSpinBox* m_spinColorG = nullptr;
    QSpinBox* m_spinColorB = nullptr;
    QSlider* m_sliderColorR = nullptr;
    QSlider* m_sliderColorG = nullptr;
    QSlider* m_sliderColorB = nullptr;
    QLabel* m_labelProfilePreview = nullptr;
    QLabel* m_labelColorPreview = nullptr;
    QLabel* m_labelGemPreviewTitle = nullptr;

    QCheckBox* m_checkStone = nullptr;
    QComboBox* m_comboGemType = nullptr;
    QDoubleSpinBox* m_spinGemCarat = nullptr;
    QDoubleSpinBox* m_spinGemLength = nullptr;
    QDoubleSpinBox* m_spinGemWidth = nullptr;
    QDoubleSpinBox* m_spinGemHeight = nullptr;
    QSlider* m_sliderStoneSize = nullptr;
    QPushButton* m_buttonStoneSizeList = nullptr;
    QLabel* m_labelGemTopView = nullptr;
    QLabel* m_labelGemSideView = nullptr;

    QComboBox* m_comboSetting = nullptr;
    QSpinBox* m_spinProngCount = nullptr;
    QLabel* m_labelSettingCount = nullptr;

    QLabel* m_labelInfo = nullptr;
    QLabel* m_labelLimits = nullptr;
    QLabel* m_labelPreview = nullptr;
    QLabel* m_labelGemCarat = nullptr;
    QLabel* m_labelGemLength = nullptr;
    QLabel* m_labelGemWidth = nullptr;
    QLabel* m_labelGemHeight = nullptr;

    QDialogButtonBox* m_buttonBox = nullptr;
    bool m_isUpdating = false;
    bool m_gemTypeEditable = true;
    int m_currentStoneSizeIndex = 0;
    QString m_selectedColorName;
};
