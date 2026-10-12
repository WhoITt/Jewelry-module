#pragma once

#include <QDialog>
#include "GemSizeCatalog.h"

class QComboBox;
class QLabel;
class QListWidget;
class QPushButton;
class QSlider;
class QDoubleSpinBox;
class QSpinBox;
class QDialogButtonBox;

class GemstoneSelectorDialogQt : public QDialog
{
public:
    explicit GemstoneSelectorDialogQt(QWidget* parent = nullptr);
    ~GemstoneSelectorDialogQt() override = default;

    QString getGemType() const;
    int getQuantity() const;
    double getLength() const;
    double getWidth() const;
    double getHeight() const;

    void setGemType(const QString& value);
    void setGemDimensions(double length, double width, double height);
    void setQuantity(int value);
    void setGemTypeEnabled(bool enabled);
    void setQuantityEnabled(bool enabled);

private:
    void onGemTypeChanged(int index);
    void onQuantityChanged(int value);
    void openSizeList();
    void applyEntryAt(int index);
    void updatePreviewText();

    void setupUI();
    void setupConnections();
    void loadGemTypes();
    void updateCurrentCutUi();

private:
    QComboBox* m_comboGemType = nullptr;
    QSpinBox* m_spinQuantity = nullptr;
    QDoubleSpinBox* m_spinCarat = nullptr;
    QSlider* m_sliderSize = nullptr;
    QPushButton* m_buttonSizeList = nullptr;
    QLabel* m_labelTopView = nullptr;
    QLabel* m_labelSideView = nullptr;
    QLabel* m_valueLength = nullptr;
    QLabel* m_valueWidth = nullptr;
    QLabel* m_valueHeight = nullptr;

    QLabel* m_labelPreview = nullptr;
    QLabel* m_labelInfo = nullptr;
    QLabel* m_labelCurrentSize = nullptr;

    QDialogButtonBox* m_buttonBox = nullptr;
    int m_currentSizeIndex = 0;
};
