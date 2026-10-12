#include "pch.h"
#include "GemstoneSelectorDialogQt.h"

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
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

#include <cmath>

#include "GemSizeCatalog.h"
#include "StoneSizeListDialog.h"

namespace
{
QString gemInfoText(const QString& gemType)
{
    if (gemType == "custom")
        return QString::fromWCharArray(L"Круглый бриллиант.");
    if (gemType == "princess")
        return QString::fromWCharArray(L"Огранка Принцесса.");
    if (gemType == "emerald")
        return QString::fromWCharArray(L"Изумрудная огранка.");
    if (gemType == "pear")
        return QString::fromWCharArray(L"Огранка Груша.");
    return QString::fromWCharArray(L"Огранка Маркиз.");
}

void setupPreviewPlaceholder(QLabel* label, const wchar_t* title)
{
    label->setMinimumSize(220, 160);
    label->setMaximumSize(220, 160);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    label->setScaledContents(false);
    label->setText(QString::fromWCharArray(title));
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

GemstoneSelectorDialogQt::GemstoneSelectorDialogQt(QWidget* parent)
    : QDialog(parent)
{
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    setModal(true);
    setupUI();
    setupConnections();
    loadGemTypes();

    setWindowTitle(QString::fromWCharArray(L"Параметры вставки"));
    setMinimumWidth(760);
    resize(840, 560);
}

void GemstoneSelectorDialogQt::setupUI()
{
    setStyleSheet(
        "QDialog { background-color: #f2f2f5; color: #000000; }"
        "QGroupBox { border: 1px solid #4b4b55; border-radius: 6px; margin-top: 10px; padding-top: 12px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; color: #000000; }"
        "QLabel { color: #000000; }"
        "QPushButton { background-color: %1; border: 1px solid #5b5b66; border-radius: 13px; }"
        "QPushButton:hover { border: 2px solid #00d5d6; }"
        "QComboBox, QSpinBox, QDoubleSpinBox { background: #f2f2f5; color: #000000; border: 1px solid #5b5b66; border-radius: 4px; padding: 5px; min-height: 28px; }"
        "QSlider::groove:horizontal { height: 6px; background: #565662; border-radius: 3px; }"
        "QSlider::handle:horizontal { background: #00d5d6; width: 14px; margin: -5px 0; border-radius: 7px; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(14, 14, 14, 14);

    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->setSpacing(12);

    QGroupBox* leftPanel = new QGroupBox(QString::fromWCharArray(L"Параметры камня"));
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(10);

    leftLayout->addWidget(new QLabel(QString::fromWCharArray(L"Огранка")));
    m_comboGemType = new QComboBox();
    leftLayout->addWidget(m_comboGemType);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #4b4b55;");
    leftLayout->addWidget(line);

    leftLayout->addWidget(new QLabel(QString::fromWCharArray(L"Размер")));
    m_spinCarat = new QDoubleSpinBox();
    m_spinCarat->setRange(0.001, 20.0);
    m_spinCarat->setDecimals(3);
    m_spinCarat->setReadOnly(true);
    m_spinCarat->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spinCarat->setSuffix(QString::fromWCharArray(L" ct"));
    leftLayout->addWidget(m_spinCarat);

    m_sliderSize = new QSlider(Qt::Horizontal);
    leftLayout->addWidget(m_sliderSize);

    m_buttonSizeList = new QPushButton(QString::fromWCharArray(L"Список размеров"));
    leftLayout->addWidget(m_buttonSizeList);

    m_labelCurrentSize = new QLabel();
    m_labelCurrentSize->setWordWrap(true);
    m_labelCurrentSize->setStyleSheet(
        "QLabel { color: #13293d; background-color: #edf4f8; border: 1px solid #c9dbe6; "
        "padding: 8px; border-radius: 6px; }");
    leftLayout->addWidget(m_labelCurrentSize);

    leftLayout->addWidget(new QLabel(QString::fromWCharArray(L"Количество камней")));
    m_spinQuantity = new QSpinBox();
    m_spinQuantity->setMinimum(1);
    m_spinQuantity->setMaximum(1000);
    m_spinQuantity->setValue(1);
    m_spinQuantity->setSuffix(QString::fromWCharArray(L" шт."));
    leftLayout->addWidget(m_spinQuantity);

    leftLayout->addStretch();
    topRow->addWidget(leftPanel, 0);

    QGroupBox* rightPanel = new QGroupBox(QString::fromWCharArray(L"Предпросмотр и размеры"));
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(10);

    QHBoxLayout* previewRow = new QHBoxLayout();
    previewRow->setSpacing(10);
    m_labelTopView = new QLabel();
    setupPreviewPlaceholder(m_labelTopView, L"Вид сверху\nPNG добавите позже");
    previewRow->addWidget(m_labelTopView);

    m_labelSideView = new QLabel();
    setupPreviewPlaceholder(m_labelSideView, L"Вид сбоку\nPNG добавите позже");
    previewRow->addWidget(m_labelSideView);
    previewRow->addStretch();
    rightLayout->addLayout(previewRow);

    QGridLayout* sizeGrid = new QGridLayout();
    sizeGrid->setHorizontalSpacing(4);
    sizeGrid->setVerticalSpacing(8);
    sizeGrid->addWidget(new QLabel(QString::fromWCharArray(L"Длина")), 0, 0);
    m_valueLength = new QLabel();
    sizeGrid->addWidget(m_valueLength, 0, 1);
    sizeGrid->addWidget(new QLabel(QString::fromWCharArray(L"Ширина")), 1, 0);
    m_valueWidth = new QLabel();
    sizeGrid->addWidget(m_valueWidth, 1, 1);
    sizeGrid->addWidget(new QLabel(QString::fromWCharArray(L"Высота")), 2, 0);
    m_valueHeight = new QLabel();
    sizeGrid->addWidget(m_valueHeight, 2, 1);
    rightLayout->addLayout(sizeGrid);

    m_labelInfo = new QLabel();
    m_labelInfo->setWordWrap(true);
    m_labelInfo->setStyleSheet(
        "QLabel { color: #13293d; background-color: #edf4f8; border: 1px solid #c9dbe6; "
        "padding: 10px; border-radius: 6px; }");
    rightLayout->addWidget(m_labelInfo);

    m_labelPreview = new QLabel();
    m_labelPreview->setWordWrap(true);
    m_labelPreview->setStyleSheet(
        "QLabel { color: #1d3526; background-color: #e9f7ef; border: 1px solid #b7dcc1; "
        "padding: 10px; border-radius: 6px; font-weight: bold; }");
    rightLayout->addWidget(m_labelPreview);

    rightLayout->addStretch();
    topRow->addWidget(rightPanel, 1);

    mainLayout->addLayout(topRow, 1);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_buttonBox->button(QDialogButtonBox::Ok)->setText(QString::fromWCharArray(L"Применить"));
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(QString::fromWCharArray(L"Отмена"));
    mainLayout->addWidget(m_buttonBox);
}

void GemstoneSelectorDialogQt::setupConnections()
{
    connect(m_comboGemType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GemstoneSelectorDialogQt::onGemTypeChanged);
    connect(m_spinQuantity, QOverload<int>::of(&QSpinBox::valueChanged), this, &GemstoneSelectorDialogQt::onQuantityChanged);
    connect(m_sliderSize, &QSlider::valueChanged, this, &GemstoneSelectorDialogQt::applyEntryAt);
    connect(m_buttonSizeList, &QPushButton::clicked, this, &GemstoneSelectorDialogQt::openSizeList);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void GemstoneSelectorDialogQt::loadGemTypes()
{
    m_comboGemType->addItem(QString::fromWCharArray(L"Бриллиант"), "custom");
    m_comboGemType->addItem(QString::fromWCharArray(L"Принцесса"), "princess");
    m_comboGemType->addItem(QString::fromWCharArray(L"Изумруд"), "emerald");
    m_comboGemType->addItem(QString::fromWCharArray(L"Груша"), "pear");
    m_comboGemType->addItem(QString::fromWCharArray(L"Маркиз"), "marquise");

    updateCurrentCutUi();
}

void GemstoneSelectorDialogQt::updateCurrentCutUi()
{
    const QVector<GemSizeEntry>& entries = GemSizeCatalog::entriesForGemType(getGemType());

    m_sliderSize->blockSignals(true);
    m_sliderSize->setRange(0, entries.isEmpty() ? 0 : entries.size() - 1);
    if (m_currentSizeIndex >= entries.size())
        m_currentSizeIndex = entries.isEmpty() ? 0 : entries.size() - 1;
    m_sliderSize->setValue(m_currentSizeIndex);
    m_sliderSize->blockSignals(false);

    applyEntryAt(m_currentSizeIndex);
}

void GemstoneSelectorDialogQt::onGemTypeChanged(int index)
{
    Q_UNUSED(index);
    m_currentSizeIndex = 0;
    updateCurrentCutUi();
}

void GemstoneSelectorDialogQt::onQuantityChanged(int value)
{
    Q_UNUSED(value);
    updatePreviewText();
}

void GemstoneSelectorDialogQt::openSizeList()
{
    const QVector<GemSizeEntry>& entries = GemSizeCatalog::entriesForGemType(getGemType());
    StoneSizeListDialog dialog(getGemType(), entries, m_currentSizeIndex, this);
    if (dialog.exec() == QDialog::Accepted && dialog.selectedIndex() >= 0)
    {
        m_currentSizeIndex = dialog.selectedIndex();
        m_sliderSize->setValue(m_currentSizeIndex);
        applyEntryAt(m_currentSizeIndex);
    }
}

void GemstoneSelectorDialogQt::applyEntryAt(int index)
{
    const QVector<GemSizeEntry>& entries = GemSizeCatalog::entriesForGemType(getGemType());
    if (entries.isEmpty())
        return;

    index = qBound(0, index, entries.size() - 1);
    m_currentSizeIndex = index;

    const GemSizeEntry& entry = entries[index];
    m_spinCarat->setValue(entry.carat);
    m_valueLength->setText(QString::number(entry.length, 'f', 2) + QString::fromWCharArray(L" мм"));
    m_valueWidth->setText(QString::number(entry.width, 'f', 2) + QString::fromWCharArray(L" мм"));
    m_valueHeight->setText(QString::number(entry.height, 'f', 2) + QString::fromWCharArray(L" мм"));
    m_labelCurrentSize->setText(
        QString::fromWCharArray(L"Строка списка: %1 из %2\n%3 ct | %4 × %5 × %6 мм")
        .arg(index + 1)
        .arg(entries.size())
        .arg(entry.carat, 0, 'f', 3)
        .arg(entry.length, 0, 'f', 2)
        .arg(entry.width, 0, 'f', 2)
        .arg(entry.height, 0, 'f', 2));

    m_labelInfo->setText(gemInfoText(getGemType()));
    applyPreviewPixmap(
        m_labelTopView,
        GemSizeCatalog::createGemTopViewPreview(getGemType(), QSize(220, 160)));
    applyPreviewPixmap(
        m_labelSideView,
        GemSizeCatalog::createGemSideViewPreview(getGemType(), QSize(220, 160)));
    updatePreviewText();
}

void GemstoneSelectorDialogQt::updatePreviewText()
{
    const QVector<GemSizeEntry>& entries = GemSizeCatalog::entriesForGemType(getGemType());
    if (entries.isEmpty())
        return;

    const GemSizeEntry& entry = entries[m_currentSizeIndex];
    m_labelPreview->setText(
        QString::fromWCharArray(L"Выбрано: %1 шт.\nОгранка: %2\nРазмер: %3 ct\nГабариты: %4 × %5 × %6 мм")
        .arg(m_spinQuantity->value())
        .arg(GemSizeCatalog::gemDisplayName(getGemType()))
        .arg(entry.carat, 0, 'f', 3)
        .arg(entry.length, 0, 'f', 2)
        .arg(entry.width, 0, 'f', 2)
        .arg(entry.height, 0, 'f', 2));
}

QString GemstoneSelectorDialogQt::getGemType() const
{
    return m_comboGemType->currentData().toString();
}

int GemstoneSelectorDialogQt::getQuantity() const
{
    return m_spinQuantity->value();
}

double GemstoneSelectorDialogQt::getLength() const
{
    const QVector<GemSizeEntry>& entries = GemSizeCatalog::entriesForGemType(getGemType());
    return entries.isEmpty() ? 0.0 : entries[m_currentSizeIndex].length;
}

double GemstoneSelectorDialogQt::getWidth() const
{
    const QVector<GemSizeEntry>& entries = GemSizeCatalog::entriesForGemType(getGemType());
    return entries.isEmpty() ? 0.0 : entries[m_currentSizeIndex].width;
}

double GemstoneSelectorDialogQt::getHeight() const
{
    const QVector<GemSizeEntry>& entries = GemSizeCatalog::entriesForGemType(getGemType());
    return entries.isEmpty() ? 0.0 : entries[m_currentSizeIndex].height;
}

void GemstoneSelectorDialogQt::setGemType(const QString& value)
{
    const int index = m_comboGemType->findData(value);
    if (index >= 0)
    {
        m_comboGemType->setCurrentIndex(index);
        m_currentSizeIndex = 0;
        updateCurrentCutUi();
    }
}

void GemstoneSelectorDialogQt::setGemDimensions(double length, double width, double height)
{
    const QVector<GemSizeEntry>& entries = GemSizeCatalog::entriesForGemType(getGemType());
    if (entries.isEmpty())
        return;

    int bestIndex = 0;
    double bestScore = 1.0e18;
    for (int i = 0; i < entries.size(); ++i)
    {
        const GemSizeEntry& entry = entries[i];
        const double score =
            std::abs(entry.length - length) +
            std::abs(entry.width - width) +
            std::abs(entry.height - height);
        if (score < bestScore)
        {
            bestScore = score;
            bestIndex = i;
        }
    }

    m_currentSizeIndex = bestIndex;
    m_sliderSize->setValue(bestIndex);
    applyEntryAt(bestIndex);
}

void GemstoneSelectorDialogQt::setQuantity(int value)
{
    m_spinQuantity->setValue(value);
    updatePreviewText();
}

void GemstoneSelectorDialogQt::setGemTypeEnabled(bool enabled)
{
    m_comboGemType->setEnabled(enabled);
}

void GemstoneSelectorDialogQt::setQuantityEnabled(bool enabled)
{
    m_spinQuantity->setEnabled(enabled);
}
