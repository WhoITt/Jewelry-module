#pragma once

#include <QDialog>
#include "GemSizeCatalog.h"

class QLabel;
class QPushButton;
class QTableWidget;

class StoneSizeListDialog : public QDialog
{
public:
    StoneSizeListDialog(const QString& gemType, const QVector<GemSizeEntry>& entries, int currentIndex, QWidget* parent = nullptr);

    int selectedIndex() const;

private:
    void setupUi(const QString& gemType, const QVector<GemSizeEntry>& entries, int currentIndex);
    void populateRows(const QVector<GemSizeEntry>& entries, int currentIndex);

private:
    QTableWidget* m_table = nullptr;
    QLabel* m_footer = nullptr;
    int m_selectedIndex = -1;
};
