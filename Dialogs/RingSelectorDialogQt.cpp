#include "pch.h"
#include "RingSelectorDialogQt.h"
#include "GemSizeCatalog.h"
#include "StoneSizeListDialog.h"

#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSlider>
#include <QVBoxLayout>

namespace
{
double clampValue(double value, double minValue, double maxValue)
{
    if (value < minValue)
        return minValue;
    if (value > maxValue)
        return maxValue;
    return value;
}

void setupPlaceholderLabel(QLabel* label, const wchar_t* text, const QSize& size)
{
    label->setMinimumSize(size);
    label->setMaximumSize(size);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    label->setScaledContents(false);
    label->setText(QString::fromWCharArray(text));
    label->setStyleSheet(
        "QLabel {"
        " background: #ffffff;"
        " color: #4b4b55;"
        " border: 1px solid #c9dbe6;"
        " border-radius: 6px;"
        " padding: 4px;"
        "}");
}

void applyPreviewPixmap(QLabel* label, const QPixmap& sourcePixmap)
{
    if (label == nullptr)
        return;

    const QSize targetSize = label->contentsRect().isValid()
        ? label->contentsRect().size()
        : label->size();

    if (sourcePixmap.isNull() || !targetSize.isValid())
    {
        label->clear();
        return;
    }

    QImage sourceImage = sourcePixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (sourceImage.isNull())
    {
        label->clear();
        return;
    }

    QSize fittedSize = sourceImage.size();
    fittedSize.scale(targetSize, Qt::KeepAspectRatio);

    QImage scaledImage = sourceImage;
    QSize currentSize = scaledImage.size();
    while ((currentSize.width() > fittedSize.width() * 2) || (currentSize.height() > fittedSize.height() * 2))
    {
        currentSize = QSize(
            (currentSize.width() + 1) / 2,
            (currentSize.height() + 1) / 2);
        scaledImage = scaledImage.scaled(
            currentSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
    }

    if (scaledImage.size() != fittedSize)
    {
        scaledImage = scaledImage.scaled(
            fittedSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
    }

    QImage canvas(targetSize, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::white);

    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    const QPoint topLeft(
        (targetSize.width() - scaledImage.width()) / 2,
        (targetSize.height() - scaledImage.height()) / 2);
    painter.drawImage(topLeft, scaledImage);
    painter.end();

    label->setPixmap(QPixmap::fromImage(canvas));
}
}

RingSelectorDialogQt::RingSelectorDialogQt(QWidget* parent)
    : QDialog(parent)
    , m_selectedColorName(QString::fromWCharArray(L"Золотой"))
{
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    setModal(true);
    setupUi();
    updateSettingOptions();
    setupConnections();
    updateGemControls();
    updateProfilePreview();
    applyMetalColorPreset(QColor(212, 175, 55));
    updateColorPreview();
    applyStoneEntryAt(0);
    updateStoneLimits();
    updatePreview();

    setWindowTitle(QString::fromWCharArray(L"Создание ювелирного кольца"));
    setMinimumWidth(860);
    resize(900, 610);
}

void RingSelectorDialogQt::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    QHBoxLayout* contentRow = new QHBoxLayout();
    contentRow->setSpacing(12);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    QWidget* contentWidget = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(10);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* ringGroup = new QGroupBox(QString::fromWCharArray(L"Параметры кольца"));
    QGridLayout* ringLayout = new QGridLayout(ringGroup);

    QLabel* labelProfile = new QLabel(QString::fromWCharArray(L"Профиль:"));
    m_comboProfile = new QComboBox();
    m_comboProfile->addItem(QString::fromWCharArray(L"Полукруглый"), 0);
    m_comboProfile->addItem(QString::fromWCharArray(L"Плоский"), 1);
    m_comboProfile->addItem(QString::fromWCharArray(L"Овальный"), 2);
    m_comboProfile->addItem(QString::fromWCharArray(L"Вогнутый"), 3);
    m_comboProfile->setMinimumHeight(30);

    m_labelProfilePreview = new QLabel();
    setupPlaceholderLabel(m_labelProfilePreview, L"Профиль кольца", QSize(170, 120));

    QLabel* labelRingSize = new QLabel(QString::fromWCharArray(L"Размер кольца:"));
    m_spinRingSize = new QDoubleSpinBox();
    m_spinRingSize->setRange(14.0, 24.0);
    m_spinRingSize->setSingleStep(0.5);
    m_spinRingSize->setDecimals(1);
    m_spinRingSize->setValue(18.0);
    m_spinRingSize->setSuffix(QString::fromWCharArray(L" мм"));
    m_spinRingSize->setMinimumHeight(30);

    QLabel* labelBandWidth = new QLabel(QString::fromWCharArray(L"Ширина шинки:"));
    m_spinBandWidth = new QDoubleSpinBox();
    m_spinBandWidth->setRange(1.5, 12.0);
    m_spinBandWidth->setSingleStep(0.1);
    m_spinBandWidth->setDecimals(2);
    m_spinBandWidth->setValue(2.8);
    m_spinBandWidth->setSuffix(QString::fromWCharArray(L" мм"));
    m_spinBandWidth->setMinimumHeight(30);

    QLabel* labelThickness = new QLabel(QString::fromWCharArray(L"Высота профиля:"));
    m_spinThickness = new QDoubleSpinBox();
    m_spinThickness->setRange(1.0, 6.0);
    m_spinThickness->setSingleStep(0.1);
    m_spinThickness->setDecimals(2);
    m_spinThickness->setValue(2.2);
    m_spinThickness->setSuffix(QString::fromWCharArray(L" мм"));
    m_spinThickness->setMinimumHeight(30);

    QLabel* labelMetalColor = new QLabel(QString::fromWCharArray(L"Цвет металла:"));

    m_spinColorR = new QSpinBox();
    m_spinColorG = new QSpinBox();
    m_spinColorB = new QSpinBox();
    for (QSpinBox* spin : { m_spinColorR, m_spinColorG, m_spinColorB })
    {
        spin->setRange(0, 255);
        spin->setMinimumHeight(28);
    }
    m_spinColorR->setValue(212);
    m_spinColorG->setValue(175);
    m_spinColorB->setValue(55);

    m_sliderColorR = new QSlider(Qt::Horizontal);
    m_sliderColorG = new QSlider(Qt::Horizontal);
    m_sliderColorB = new QSlider(Qt::Horizontal);
    for (QSlider* slider : { m_sliderColorR, m_sliderColorG, m_sliderColorB })
    {
        slider->setRange(0, 255);
    }
    m_sliderColorR->setValue(212);
    m_sliderColorG->setValue(175);
    m_sliderColorB->setValue(55);

    QWidget* paletteWidget = new QWidget();
    QGridLayout* paletteLayout = new QGridLayout(paletteWidget);
    paletteLayout->setContentsMargins(0, 0, 0, 0);
    paletteLayout->setSpacing(6);

    const struct PalettePreset
    {
        const wchar_t* name;
        QColor color;
    } presets[] = {
        {L"Золотой", QColor(212, 175, 55)},
        {L"Серебристый", QColor(192, 192, 192)},
        {L"Розовое золото", QColor(183, 110, 121)},
        {L"Белое золото", QColor(224, 224, 229)},
        {L"Красный", QColor(180, 50, 50)},
        {L"Зелёный", QColor(65, 150, 95)},
        {L"Синий", QColor(60, 90, 170)}
    };

    const int paletteColumns = 4;
    const int presetCount = static_cast<int>(sizeof(presets) / sizeof(presets[0]));
    for (int i = 0; i < presetCount; ++i)
    {
        const PalettePreset& preset = presets[i];
        QPushButton* button = new QPushButton();
        button->setToolTip(QString::fromWCharArray(preset.name));
        button->setFixedSize(24, 24);
        button->setStyleSheet(QString(
            "QPushButton { background-color: %1; border: 1px solid #5b5b66; border-radius: 13px; }"
            "QPushButton:hover { border: 2px solid #00d5d6; }")
            .arg(preset.color.name()));
        connect(button, &QPushButton::clicked, this, [this, preset]() {
            selectPaletteColor(preset.color, QString::fromWCharArray(preset.name), false);
        });
        paletteLayout->addWidget(button, i / paletteColumns, i % paletteColumns);
    }
    m_labelColorPreview = new QLabel();
    m_labelColorPreview->setMinimumHeight(32);
    m_labelColorPreview->setAlignment(Qt::AlignCenter);
    m_labelColorPreview->setStyleSheet("QLabel { border: 1px solid #5b5b66; border-radius: 6px; padding: 6px; }");

    ringLayout->addWidget(labelProfile, 0, 0);
    ringLayout->addWidget(m_comboProfile, 0, 1, 1, 2);
    ringLayout->addWidget(labelRingSize, 1, 0);
    ringLayout->addWidget(m_spinRingSize, 1, 1, 1, 2);
    ringLayout->addWidget(labelBandWidth, 2, 0);
    ringLayout->addWidget(m_spinBandWidth, 2, 1, 1, 2);
    ringLayout->addWidget(labelThickness, 3, 0);
    ringLayout->addWidget(m_spinThickness, 3, 1, 1, 2);
    ringLayout->addWidget(labelMetalColor, 4, 0);
    ringLayout->addWidget(paletteWidget, 4, 1, 2, 2);
    ringLayout->addWidget(m_labelColorPreview, 6, 0, 1, 3);
    ringLayout->addWidget(new QLabel(QString::fromWCharArray(L"R:")), 7, 0);
    ringLayout->addWidget(m_spinColorR, 7, 1);
    ringLayout->addWidget(m_sliderColorR, 7, 2);
    ringLayout->addWidget(new QLabel(QString::fromWCharArray(L"G:")), 8, 0);
    ringLayout->addWidget(m_spinColorG, 8, 1);
    ringLayout->addWidget(m_sliderColorG, 8, 2);
    ringLayout->addWidget(new QLabel(QString::fromWCharArray(L"B:")), 9, 0);
    ringLayout->addWidget(m_spinColorB, 9, 1);
    ringLayout->addWidget(m_sliderColorB, 9, 2);
    contentLayout->addWidget(ringGroup);

    QGroupBox* stoneGroup = new QGroupBox(QString::fromWCharArray(L"Вставка и закрепка"));
    QGridLayout* stoneLayout = new QGridLayout(stoneGroup);

    m_checkStone = new QCheckBox(QString::fromWCharArray(L"Добавить или привязать камень"));
    m_checkStone->setChecked(true);

    m_labelGemTopView = new QLabel();
    setupPlaceholderLabel(m_labelGemTopView, L"Вид сверху", QSize(132, 108));

    m_labelGemSideView = new QLabel();
    setupPlaceholderLabel(m_labelGemSideView, L"Вид сбоку", QSize(132, 108));

    QLabel* labelGemType = new QLabel(QString::fromWCharArray(L"Огранка:"));
    m_comboGemType = new QComboBox();
    m_comboGemType->addItem(QString::fromWCharArray(L"Бриллиант (Round Brilliant)"), "custom");
    m_comboGemType->addItem(QString::fromWCharArray(L"Принцесса"), "princess");
    m_comboGemType->addItem(QString::fromWCharArray(L"Изумруд"), "emerald");
    m_comboGemType->addItem(QString::fromWCharArray(L"Груша"), "pear");
    m_comboGemType->addItem(QString::fromWCharArray(L"Маркиз"), "marquise");
    m_comboGemType->setMinimumHeight(30);

    m_labelGemCarat = new QLabel(QString::fromWCharArray(L"Размер, ct:"));
    m_spinGemCarat = new QDoubleSpinBox();
    m_spinGemCarat->setDecimals(3);
    m_spinGemCarat->setRange(0.001, 20.0);
    m_spinGemCarat->setReadOnly(true);
    m_spinGemCarat->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spinGemCarat->setSuffix(QString::fromWCharArray(L" ct"));
    m_spinGemCarat->setMinimumHeight(30);

    m_sliderStoneSize = new QSlider(Qt::Horizontal);
    m_buttonStoneSizeList = new QPushButton(QString::fromWCharArray(L"Список размеров"));

    m_labelGemLength = new QLabel(QString::fromWCharArray(L"Длина:"));
    m_spinGemLength = new QDoubleSpinBox();
    m_spinGemLength->setDecimals(2);
    m_spinGemLength->setSingleStep(0.1);
    m_spinGemLength->setRange(0.5, 20.0);
    m_spinGemLength->setValue(2.4);
    m_spinGemLength->setSuffix(QString::fromWCharArray(L" мм"));
    m_spinGemLength->setMinimumHeight(30);
    m_spinGemLength->setReadOnly(true);
    m_spinGemLength->setButtonSymbols(QAbstractSpinBox::NoButtons);

    m_labelGemWidth = new QLabel(QString::fromWCharArray(L"Ширина:"));
    m_spinGemWidth = new QDoubleSpinBox();
    m_spinGemWidth->setDecimals(2);
    m_spinGemWidth->setSingleStep(0.1);
    m_spinGemWidth->setRange(0.5, 20.0);
    m_spinGemWidth->setValue(2.4);
    m_spinGemWidth->setSuffix(QString::fromWCharArray(L" мм"));
    m_spinGemWidth->setMinimumHeight(30);
    m_spinGemWidth->setReadOnly(true);
    m_spinGemWidth->setButtonSymbols(QAbstractSpinBox::NoButtons);

    m_labelGemHeight = new QLabel(QString::fromWCharArray(L"Глубина:"));
    m_spinGemHeight = new QDoubleSpinBox();
    m_spinGemHeight->setDecimals(2);
    m_spinGemHeight->setSingleStep(0.1);
    m_spinGemHeight->setRange(0.5, 20.0);
    m_spinGemHeight->setValue(1.7);
    m_spinGemHeight->setSuffix(QString::fromWCharArray(L" мм"));
    m_spinGemHeight->setMinimumHeight(30);
    m_spinGemHeight->setReadOnly(true);
    m_spinGemHeight->setButtonSymbols(QAbstractSpinBox::NoButtons);

    QLabel* labelSetting = new QLabel(QString::fromWCharArray(L"Тип закрепки:"));
    m_comboSetting = new QComboBox();
    m_comboSetting->addItem(QString::fromWCharArray(L"Без закрепки"), kSettingNone);
    m_comboSetting->addItem(QString::fromWCharArray(L"Крапановая"), kSettingProng);
    m_comboSetting->addItem(QString::fromWCharArray(L"Глухая"), kSettingBezel);
    m_comboSetting->setMinimumHeight(30);

    m_labelSettingCount = new QLabel(QString::fromWCharArray(L"Количество крапанов:"));
    m_spinProngCount = new QSpinBox();
    m_spinProngCount->setRange(3, 8);
    m_spinProngCount->setValue(4);
    m_spinProngCount->setMinimumHeight(30);

    stoneLayout->addWidget(m_checkStone, 0, 0, 1, 2);
    stoneLayout->addWidget(labelGemType, 1, 0);
    stoneLayout->addWidget(m_comboGemType, 1, 1);
    stoneLayout->addWidget(m_labelGemCarat, 2, 0);
    stoneLayout->addWidget(m_spinGemCarat, 2, 1);
    stoneLayout->addWidget(m_sliderStoneSize, 3, 0, 1, 2);
    stoneLayout->addWidget(m_buttonStoneSizeList, 4, 0, 1, 2);
    stoneLayout->addWidget(m_labelGemLength, 5, 0);
    stoneLayout->addWidget(m_spinGemLength, 5, 1);
    stoneLayout->addWidget(m_labelGemWidth, 6, 0);
    stoneLayout->addWidget(m_spinGemWidth, 6, 1);
    stoneLayout->addWidget(m_labelGemHeight, 7, 0);
    stoneLayout->addWidget(m_spinGemHeight, 7, 1);
    stoneLayout->addWidget(labelSetting, 8, 0);
    stoneLayout->addWidget(m_comboSetting, 8, 1);
    stoneLayout->addWidget(m_labelSettingCount, 9, 0);
    stoneLayout->addWidget(m_spinProngCount, 9, 1);
    contentLayout->addWidget(stoneGroup);

    scrollArea->setWidget(contentWidget);
    contentRow->addWidget(scrollArea, 1);

    QWidget* sidePanel = new QWidget(this);
    sidePanel->setMinimumWidth(270);
    sidePanel->setMaximumWidth(320);
    QVBoxLayout* sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setSpacing(10);
    sideLayout->setContentsMargins(0, 0, 0, 0);

    m_labelInfo = new QLabel();
    m_labelInfo->setWordWrap(true);
    m_labelInfo->setStyleSheet(
        "QLabel { color: #13293d; background-color: #edf4f8; border: 1px solid #c9dbe6; "
        "padding: 10px; border-radius: 6px; }");
    sideLayout->addWidget(m_labelInfo);

    m_labelPreview = new QLabel();
    m_labelPreview->setWordWrap(true);
    m_labelPreview->setStyleSheet(
        "QLabel { color: #1d3526; background-color: #e9f7ef; border: 1px solid #b7dcc1; "
        "padding: 10px; border-radius: 6px; font-weight: bold; }");
    sideLayout->addWidget(m_labelPreview);

    sideLayout->addStretch();

    QGroupBox* previewGroup = new QGroupBox(QString::fromWCharArray(L"Предпросмотр"));
    QVBoxLayout* previewGroupLayout = new QVBoxLayout(previewGroup);
    previewGroupLayout->setSpacing(10);
    previewGroupLayout->setContentsMargins(10, 12, 10, 10);
    previewGroupLayout->addWidget(m_labelProfilePreview);

    m_labelGemPreviewTitle = new QLabel(QString::fromWCharArray(L"Камень"));
    previewGroupLayout->addWidget(m_labelGemPreviewTitle);

    QHBoxLayout* gemPreviewLayout = new QHBoxLayout();
    gemPreviewLayout->setContentsMargins(2, 0, 2, 0);
    gemPreviewLayout->setSpacing(12);
    gemPreviewLayout->addWidget(m_labelGemTopView);
    gemPreviewLayout->addWidget(m_labelGemSideView);
    previewGroupLayout->addLayout(gemPreviewLayout);
    sideLayout->addWidget(previewGroup);

    contentRow->addWidget(sidePanel, 0);

    contentLayout->addStretch();
    mainLayout->addLayout(contentRow, 1);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_buttonBox->button(QDialogButtonBox::Ok)->setText(QString::fromWCharArray(L"Применить"));
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(QString::fromWCharArray(L"Отмена"));
    mainLayout->addWidget(m_buttonBox);
}

void RingSelectorDialogQt::setupConnections()
{
    connect(m_comboProfile, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int) { updateProfilePreview(); updateSettingOptions(); updateGemControls(); updateStoneLimits(); updatePreview(); });
    connect(m_sliderColorR, &QSlider::valueChanged, [this](int value) {
        if (m_spinColorR->value() != value)
            m_spinColorR->setValue(value);
        selectPaletteColor(selectedColor(), QString::fromWCharArray(L"Пользовательский RGB"), true);
    });
    connect(m_sliderColorG, &QSlider::valueChanged, [this](int value) {
        if (m_spinColorG->value() != value)
            m_spinColorG->setValue(value);
        selectPaletteColor(selectedColor(), QString::fromWCharArray(L"Пользовательский RGB"), true);
    });
    connect(m_sliderColorB, &QSlider::valueChanged, [this](int value) {
        if (m_spinColorB->value() != value)
            m_spinColorB->setValue(value);
        selectPaletteColor(selectedColor(), QString::fromWCharArray(L"Пользовательский RGB"), true);
    });
    connect(m_spinColorR, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        if (m_sliderColorR->value() != value)
            m_sliderColorR->setValue(value);
        selectPaletteColor(selectedColor(), QString::fromWCharArray(L"Пользовательский RGB"), true);
    });
    connect(m_spinColorG, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        if (m_sliderColorG->value() != value)
            m_sliderColorG->setValue(value);
        selectPaletteColor(selectedColor(), QString::fromWCharArray(L"Пользовательский RGB"), true);
    });
    connect(m_spinColorB, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        if (m_sliderColorB->value() != value)
            m_sliderColorB->setValue(value);
        selectPaletteColor(selectedColor(), QString::fromWCharArray(L"Пользовательский RGB"), true);
    });
    connect(m_spinRingSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double) { updateStoneLimits(); updatePreview(); });
    connect(m_spinBandWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double) { updateStoneLimits(); updatePreview(); });
    connect(m_spinThickness, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double) { updateStoneLimits(); updatePreview(); });

    connect(m_checkStone, &QCheckBox::toggled,
        [this](bool) { updateGemControls(); updateStoneLimits(); updatePreview(); });
    connect(m_comboGemType, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int) { m_currentStoneSizeIndex = 0; applyStoneEntryAt(0); updateStoneLimits(); updatePreview(); });
    connect(m_sliderStoneSize, &QSlider::valueChanged, [this](int value) { applyStoneEntryAt(value); });
    connect(m_buttonStoneSizeList, &QPushButton::clicked, [this]() { openStoneSizeList(); });
    connect(m_comboSetting, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int) { updateGemControls(); updateStoneLimits(); updatePreview(); });
    connect(m_spinProngCount, QOverload<int>::of(&QSpinBox::valueChanged),
        [this](int) { updatePreview(); });

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void RingSelectorDialogQt::updateSettingOptions()
{
    const int currentSetting = settingType();
    const bool concaveProfile = profileType() == 3;

    m_comboSetting->blockSignals(true);
    m_comboSetting->clear();
    m_comboSetting->addItem(QString::fromWCharArray(L"Без закрепки"), kSettingNone);
    if (!concaveProfile)
    {
        m_comboSetting->addItem(QString::fromWCharArray(L"Крапановая"), kSettingProng);
        m_comboSetting->addItem(QString::fromWCharArray(L"Глухая"), kSettingBezel);
    }

    int index = m_comboSetting->findData(currentSetting);
    if (index < 0)
        index = 0;
    if (index < 0)
        index = 0;
    m_comboSetting->setCurrentIndex(index);
    m_comboSetting->blockSignals(false);
}

void RingSelectorDialogQt::updateGemControls()
{
    const bool concaveProfile = profileType() == 3;
    if (concaveProfile && m_checkStone->isChecked())
        m_checkStone->setChecked(false);

    m_checkStone->setEnabled(!concaveProfile);
    const bool stoneEnabled = !concaveProfile && m_checkStone->isChecked();
    const bool prongsEnabled = stoneEnabled && settingType() == kSettingProng;

    m_comboGemType->setEnabled(stoneEnabled && m_gemTypeEditable);
    m_spinGemCarat->setEnabled(stoneEnabled);
    m_spinGemLength->setEnabled(stoneEnabled);
    m_spinGemWidth->setEnabled(stoneEnabled);
    m_spinGemHeight->setEnabled(stoneEnabled);
    m_sliderStoneSize->setEnabled(stoneEnabled);
    m_buttonStoneSizeList->setEnabled(stoneEnabled);
    m_comboSetting->setEnabled(stoneEnabled);
    m_spinProngCount->setEnabled(prongsEnabled);
    m_labelGemTopView->setEnabled(stoneEnabled);
    m_labelGemSideView->setEnabled(stoneEnabled);

    m_labelGemCarat->setEnabled(stoneEnabled);
    m_labelGemLength->setEnabled(stoneEnabled);
    m_labelGemWidth->setEnabled(stoneEnabled);
    m_labelGemHeight->setEnabled(stoneEnabled);
    updatePreviewVisibility();

    m_labelSettingCount->setText(QString::fromWCharArray(L"Количество крапанов:"));
    m_spinProngCount->setSpecialValueText(QString());
    m_spinProngCount->setRange(3, 8);
    if (m_spinProngCount->value() < 3)
        m_spinProngCount->setValue(4);
}

void RingSelectorDialogQt::updateStoneLimits()
{
    const QVector<GemSizeEntry>& entries = GemSizeCatalog::entriesForGemType(gemType());
    if (!entries.isEmpty())
    {
        m_sliderStoneSize->setRange(0, entries.size() - 1);
        if (m_currentStoneSizeIndex >= entries.size())
            m_currentStoneSizeIndex = entries.size() - 1;
    }

    const QString type = gemType();
    m_labelGemLength->setText(QString::fromWCharArray(L"Длина:"));
    m_labelGemWidth->setVisible(true);
    m_spinGemWidth->setVisible(true);
    m_labelGemWidth->setVisible(true);

    m_spinGemLength->setMaximum(20.0);
    m_spinGemWidth->setMaximum(20.0);
    m_spinGemHeight->setMaximum(20.0);

    if (type == "custom")
    {
        m_labelGemLength->setText(QString::fromWCharArray(L"Диаметр:"));
        m_labelGemWidth->setVisible(false);
        m_spinGemWidth->setVisible(false);
        m_spinGemLength->setMaximum(20.0);
    }
    else if (type == "princess")
    {
        m_labelGemLength->setText(QString::fromWCharArray(L"Сторона квадрата:"));
        m_labelGemWidth->setVisible(false);
        m_spinGemWidth->setVisible(false);
        m_spinGemLength->setMaximum(20.0);
    }
    else
    {
        m_spinGemLength->setMaximum(20.0);
        m_spinGemWidth->setMaximum(20.0);
    }

    const QString profileInfo =
        profileType() == 0 ? QString::fromWCharArray(L"Полукруглый профиль: универсальная шинка с мягкой посадкой и классическим силуэтом.") :
        profileType() == 1 ? QString::fromWCharArray(L"Плоский профиль: строгая геометрия для обручальных и минималистичных моделей.") :
        profileType() == 2 ? QString::fromWCharArray(L"Овальный профиль: скругление внутри и снаружи, удобная посадка на палец.") :
                             QString::fromWCharArray(L"Вогнутый профиль: декоративная форма с более выразительной светотенью.");

    QString settingInfo;
    if (!hasStone() || settingType() == kSettingNone)
        settingInfo = QString::fromWCharArray(L"Закрепка: камень не закрепляется или используется только посадочное место.");
    else if (settingType() == kSettingProng)
        settingInfo = QString::fromWCharArray(L"Закрепка: крапановая, формируется мостиками из металла кольца и открывает камень для света.");
    else
        settingInfo = QString::fromWCharArray(L"Закрепка: глухая, почти на всю высоту камня с небольшим выходом вершины наружу.");

    m_labelInfo->setText(profileInfo + QString::fromWCharArray(L"\n") + settingInfo);

}

void RingSelectorDialogQt::updatePreview()
{
    QString text = QString::fromWCharArray(
        L"Кольцо: размер %1, ширина %2 мм, высота профиля %3 мм, цвет %4.")
        .arg(ringSize(), 0, 'f', 1)
        .arg(bandWidth(), 0, 'f', 2)
        .arg(thickness(), 0, 'f', 2)
        .arg(m_selectedColorName);

    if (hasStone())
    {
        if (gemType() == "custom")
        {
            text += QString::fromWCharArray(L"\nВставка: круглая, %1 ct, диаметр %2 мм, глубина %3 мм.")
                .arg(m_spinGemCarat->value(), 0, 'f', 3)
                .arg(gemLength(), 0, 'f', 2)
                .arg(gemHeight(), 0, 'f', 2);
        }
        else if (gemType() == "princess")
        {
            text += QString::fromWCharArray(L"\nВставка: принцесса, %1 ct, %2×%2×%3 мм.")
                .arg(m_spinGemCarat->value(), 0, 'f', 3)
                .arg(gemLength(), 0, 'f', 2)
                .arg(gemHeight(), 0, 'f', 2);
        }
        else
        {
            text += QString::fromWCharArray(L"\nВставка: %1, %2 ct, размеры %3×%4×%5 мм.")
                .arg(m_comboGemType->currentText())
                .arg(m_spinGemCarat->value(), 0, 'f', 3)
                .arg(gemLength(), 0, 'f', 2)
                .arg(gemWidth(), 0, 'f', 2)
                .arg(gemHeight(), 0, 'f', 2);
        }

        if (settingType() == kSettingProng)
        {
            text += QString::fromWCharArray(L"\nЗакрепка: %1 мостика(ов)/лапок, посадочный радиус %2 мм.")
                .arg(prongCount())
                .arg(recommendedSeatRadius(), 0, 'f', 2);
        }
        else if (settingType() == kSettingBezel)
        {
            text += QString::fromWCharArray(L"\nЗакрепка: глухая, высота борта %1 мм, толщина борта %2 мм.")
                .arg(recommendedBezelHeight(), 0, 'f', 2)
                .arg(recommendedBezelThickness(), 0, 'f', 2);
        }
        else
        {
            text += QString::fromWCharArray(L"\nЗакрепка не используется.");
        }
    }
    else
    {
        text += QString::fromWCharArray(L"\nВставка не используется.");
    }

    m_labelPreview->setText(text);
}

int RingSelectorDialogQt::profileType() const
{
    return m_comboProfile->currentData().toInt();
}

double RingSelectorDialogQt::ringSize() const
{
    return m_spinRingSize->value();
}

double RingSelectorDialogQt::bandWidth() const
{
    return m_spinBandWidth->value();
}

double RingSelectorDialogQt::thickness() const
{
    return m_spinThickness->value();
}

bool RingSelectorDialogQt::hasStone() const
{
    return m_checkStone->isChecked();
}

QString RingSelectorDialogQt::gemType() const
{
    return m_comboGemType->currentData().toString();
}

double RingSelectorDialogQt::gemLength() const
{
    return m_spinGemLength->value();
}

double RingSelectorDialogQt::gemWidth() const
{
    if (gemType() == "custom" || gemType() == "princess")
        return m_spinGemLength->value();
    return m_spinGemWidth->value();
}

double RingSelectorDialogQt::gemHeight() const
{
    return m_spinGemHeight->value();
}

RingSelectorDialogQt::SettingType RingSelectorDialogQt::settingType() const
{
    return static_cast<SettingType>(m_comboSetting->currentData().toInt());
}

int RingSelectorDialogQt::prongCount() const
{
    return m_spinProngCount->value();
}

void RingSelectorDialogQt::setProfileType(int value)
{
    m_comboProfile->setCurrentIndex(m_comboProfile->findData(value));
    updateProfilePreview();
    updateStoneLimits();
    updatePreview();
}

void RingSelectorDialogQt::setRingSize(double value)
{
    m_spinRingSize->setValue(value);
    updateStoneLimits();
    updatePreview();
}

void RingSelectorDialogQt::setBandWidth(double value)
{
    m_spinBandWidth->setValue(value);
    updateStoneLimits();
    updatePreview();
}

void RingSelectorDialogQt::setThickness(double value)
{
    m_spinThickness->setValue(value);
    updateStoneLimits();
    updatePreview();
}

void RingSelectorDialogQt::setHasStone(bool value)
{
    m_checkStone->setChecked(value);
    updateGemControls();
    updateStoneLimits();
    updatePreview();
}

void RingSelectorDialogQt::setGemType(const QString& value)
{
    const int index = m_comboGemType->findData(value);
    if (index >= 0)
        m_comboGemType->setCurrentIndex(index);
    m_currentStoneSizeIndex = 0;
    applyStoneEntryAt(0);
    updateStoneLimits();
    updatePreview();
}

void RingSelectorDialogQt::setGemDimensions(double length, double width, double height)
{
    const QVector<GemSizeEntry>& entries = GemSizeCatalog::entriesForGemType(gemType());
    if (!entries.isEmpty())
    {
        int bestIndex = 0;
        double bestScore = 1.0e18;
        for (int i = 0; i < entries.size(); ++i)
        {
            const GemSizeEntry& e = entries[i];
            const double score = std::abs(e.length - length) + std::abs(e.width - width) + std::abs(e.height - height);
            if (score < bestScore)
            {
                bestScore = score;
                bestIndex = i;
            }
        }
        m_currentStoneSizeIndex = bestIndex;
        applyStoneEntryAt(bestIndex);
    }
    updateStoneLimits();
    updatePreview();
}

void RingSelectorDialogQt::setSettingType(SettingType value)
{
    const int index = m_comboSetting->findData(value);
    if (index >= 0)
        m_comboSetting->setCurrentIndex(index);
    updateGemControls();
    updateStoneLimits();
    updatePreview();
}

void RingSelectorDialogQt::setProngCount(int value)
{
    m_spinProngCount->setValue(value);
    updatePreview();
}

void RingSelectorDialogQt::setGemTypeEnabled(bool enabled)
{
    m_gemTypeEditable = enabled;
    updateGemControls();
}

void RingSelectorDialogQt::setSelectedColor(const QColor& color, const QString& name)
{
    selectPaletteColor(color, name, false);
}

void RingSelectorDialogQt::openStoneSizeList()
{
    const QVector<GemSizeEntry>& entries = GemSizeCatalog::entriesForGemType(gemType());
    StoneSizeListDialog dialog(gemType(), entries, m_currentStoneSizeIndex, this);
    if (dialog.exec() == QDialog::Accepted && dialog.selectedIndex() >= 0)
    {
        m_currentStoneSizeIndex = dialog.selectedIndex();
        m_sliderStoneSize->setValue(m_currentStoneSizeIndex);
        applyStoneEntryAt(m_currentStoneSizeIndex);
    }
}

void RingSelectorDialogQt::applyStoneEntryAt(int index)
{
    const QVector<GemSizeEntry>& entries = GemSizeCatalog::entriesForGemType(gemType());
    if (entries.isEmpty())
        return;

    if (index < 0)
        index = 0;
    if (index >= entries.size())
        index = entries.size() - 1;

    m_currentStoneSizeIndex = index;
    const GemSizeEntry& entry = entries[index];
    m_spinGemCarat->setValue(entry.carat);
    m_spinGemLength->setValue(entry.length);
    m_spinGemWidth->setValue(entry.width);
    m_spinGemHeight->setValue(entry.height);
    applyPreviewPixmap(
        m_labelGemTopView,
        GemSizeCatalog::createGemTopViewPreview(gemType(), QSize(132, 108)));
    applyPreviewPixmap(
        m_labelGemSideView,
        GemSizeCatalog::createGemSideViewPreview(gemType(), QSize(132, 108)));
    updateStoneLimits();
    updatePreview();
}

void RingSelectorDialogQt::updateProfilePreview()
{
    if (m_labelProfilePreview == nullptr)
        return;

    applyPreviewPixmap(
        m_labelProfilePreview,
        GemSizeCatalog::createRingProfilePreview(profileType(), QSize(170, 120)));
    m_labelProfilePreview->setToolTip(
        profileType() == 0 ? QString::fromWCharArray(L"Полукруглый профиль") :
        profileType() == 1 ? QString::fromWCharArray(L"Плоский профиль") :
        profileType() == 2 ? QString::fromWCharArray(L"Овальный профиль") :
                             QString::fromWCharArray(L"Вогнутый профиль"));
}

void RingSelectorDialogQt::applyMetalColorPreset(const QColor& color)
{
    QSignalBlocker blockRSpin(m_spinColorR);
    QSignalBlocker blockGSpin(m_spinColorG);
    QSignalBlocker blockBSpin(m_spinColorB);
    QSignalBlocker blockRSlider(m_sliderColorR);
    QSignalBlocker blockGSlider(m_sliderColorG);
    QSignalBlocker blockBSlider(m_sliderColorB);
    m_spinColorR->setValue(color.red());
    m_spinColorG->setValue(color.green());
    m_spinColorB->setValue(color.blue());
    m_sliderColorR->setValue(color.red());
    m_sliderColorG->setValue(color.green());
    m_sliderColorB->setValue(color.blue());
}

void RingSelectorDialogQt::updateColorPreview()
{
    if (m_labelColorPreview == nullptr)
        return;

    const QColor color = selectedColor();
    m_labelColorPreview->setText(
        m_selectedColorName);
    m_labelColorPreview->setStyleSheet(QString(
        "QLabel { background: %1; color: %2; border: 1px solid #5b5b66; border-radius: 6px; padding: 6px; }")
        .arg(color.name())
        .arg((color.lightness() < 110) ? "#f5f5f5" : "#111111"));
}

void RingSelectorDialogQt::selectPaletteColor(const QColor& color, const QString& name, bool custom)
{
    if (m_isUpdating)
        return;

    m_isUpdating = true;
    applyMetalColorPreset(color);
    m_selectedColorName = name;
    if (custom)
        m_selectedColorName = QString::fromWCharArray(L"Пользовательский RGB");
    updateColorPreview();
    updatePreview();
    m_isUpdating = false;
}

void RingSelectorDialogQt::updatePreviewVisibility()
{
    const bool stoneVisible = hasStone();
    m_labelGemPreviewTitle->setVisible(stoneVisible);
    m_labelGemTopView->setVisible(stoneVisible);
    m_labelGemSideView->setVisible(stoneVisible);
}

double RingSelectorDialogQt::recommendedSeatRadius() const
{
    const double seatBase = qMin(gemLength(), gemWidth()) * 0.47;
    return clampValue(seatBase, 0.55, bandWidth() * 0.48);
}

double RingSelectorDialogQt::recommendedProngRadius() const
{
    return clampValue(qMin(bandWidth(), gemWidth()) * 0.16, 0.38, 0.95);
}

double RingSelectorDialogQt::recommendedProngHeight() const
{
    return clampValue(gemHeight() * 0.72, 0.95, 2.8);
}

double RingSelectorDialogQt::recommendedBezelHeight() const
{
    return clampValue(gemHeight() * 0.84, 0.8, 3.0);
}

double RingSelectorDialogQt::recommendedBezelThickness() const
{
    return clampValue(gemWidth() * 0.08, 0.25, 0.8);
}

QColor RingSelectorDialogQt::selectedColor() const
{
    return QColor(m_spinColorR->value(), m_spinColorG->value(), m_spinColorB->value());
}
