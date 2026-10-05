#include "pch.h"
#include "StoneSizeListDialog.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "GemSizeCatalog.h"

StoneSizeListDialog::StoneSizeListDialog(
    const QString& gemType,
    const QVector<GemSizeEntry>& entries,
    int currentIndex,
    QWidget* parent)
    : QDialog(parent)
{
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    setModal(true);
    setupUi(gemType, entries, currentIndex);
}

int StoneSizeListDialog::selectedIndex() const
{
    return m_selectedIndex;
}

void StoneSizeListDialog::setupUi(const QString& gemType, const QVector<GemSizeEntry>& entries, int currentIndex)
{
    setWindowTitle(QString::fromWCharArray(L"Список размеров"));
    resize(620, 540);

    setStyleSheet(
        "QDialog { background-color: #f2f2f5; color: #000000; }"
        "QLabel { color: #000000; }"
        "QPushButton { background-color: %1; border: 1px solid #5b5b66; border-radius: 13px; }"
        "QPushButton:hover { border: 2px solid #00d5d6; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    QLabel* title = new QLabel(
        QString::fromWCharArray(L"Стандартизованный список размеров: ") +
        GemSizeCatalog::gemDisplayName(gemType));
    title->setStyleSheet(
        "QLabel { color: #000000; background-color: #f2f2f5; border: 1px solid #c9dbe6; "
        "padding: 8px; border-radius: 6px; font-size: 15px; font-weight: bold; }");
    mainLayout->addWidget(title);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({
        QString::fromWCharArray(L"Карат"),
        QString::fromWCharArray(L"Длина, мм"),
        QString::fromWCharArray(L"Ширина, мм"),
        QString::fromWCharArray(L"Высота, мм")
        });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(true);
    m_table->setStyleSheet(
        "QTableWidget {"
        " background: #f2f2f5;"
        " alternate-background-color: #c9c9d1;"
        " color: #000000;"
        " gridline-color: #55555c;"
        " border: 1px solid #55555c;"
        "}"
        "QHeaderView::section {"
        " background: #3d3d44;"
        " color: #e7e7eb;"
        " padding: 6px;"
        " border: 1px solid #55555c;"
        "}"
        "QTableWidget::item {"
        " background: transparent;"
        " padding: 4px;"
        "}"
        "QTableWidget::item:selected {"
        " background: #8fd7eb;"
        " color: #111111;"
        "}");
    populateRows(entries, currentIndex);
    mainLayout->addWidget(m_table, 1);

    m_footer = new QLabel();
    m_footer->setWordWrap(true);
    m_footer->setStyleSheet(
        "QLabel { color: #1d3526; background-color: #e9f7ef; border: 1px solid #b7dcc1; "
        "padding: 8px; border-radius: 6px; font-weight: bold; }");
    mainLayout->addWidget(m_footer);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(QString::fromWCharArray(L"Применить"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QString::fromWCharArray(L"Отмена"));
    mainLayout->addWidget(buttons);

    connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        m_selectedIndex = row;
        accept();
        });
    connect(m_table, &QTableWidget::currentCellChanged, this, [this, entries](int row, int, int, int) {
        if (row >= 0 && row < entries.size())
        {
            const GemSizeEntry& e = entries[row];
            m_selectedIndex = row;
            m_footer->setText(
                QString::fromWCharArray(L"Выбран размер: %1 ct | %2 × %3 × %4 мм")
                .arg(e.carat, 0, 'f', 3)
                .arg(e.length, 0, 'f', 2)
                .arg(e.width, 0, 'f', 2)
                .arg(e.height, 0, 'f', 2));
        }
        });
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (m_table->currentRow() >= 0)
            m_selectedIndex = m_table->currentRow();
        accept();
        });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (currentIndex >= 0 && currentIndex < entries.size())
    {
        m_table->selectRow(currentIndex);
        m_table->setCurrentCell(currentIndex, 0);
    }
    else if (!entries.isEmpty())
    {
        m_table->selectRow(0);
        m_table->setCurrentCell(0, 0);
    }
}

void StoneSizeListDialog::populateRows(const QVector<GemSizeEntry>& entries, int currentIndex)
{
    Q_UNUSED(currentIndex);

    m_table->setRowCount(entries.size());
    for (int row = 0; row < entries.size(); ++row)
    {
        const GemSizeEntry& e = entries[row];
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(e.carat, 'f', 3)));
        m_table->setItem(row, 1, new QTableWidgetItem(QString::number(e.length, 'f', 2)));
        m_table->setItem(row, 2, new QTableWidgetItem(QString::number(e.width, 'f', 2)));
        m_table->setItem(row, 3, new QTableWidgetItem(QString::number(e.height, 'f', 2)));
    }
}
