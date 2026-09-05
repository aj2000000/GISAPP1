/**
 * @file BaseTablePanelDialog.h
 * @brief Reusable base wrapper implementing standard tactical table dialog UI and behavior.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef BASETABLEPANELDIALOG_H
#define BASETABLEPANELDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>

#include "ITablePanelDialog.h"

namespace GISApp::Core::Wrappers {

/**
 * @class GenericTableFilterProxyModel
 * @brief Default proxy model supporting case-insensitive multi-column text search and custom hooks.
 */
class GenericTableFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit GenericTableFilterProxyModel(QObject *parent = nullptr);
    virtual ~GenericTableFilterProxyModel() override = default;

    /**
     * @brief Sets the global search text filter.
     * @param[in] text Search string.
     */
    void setSearchText(const QString &text);

    /**
     * @brief Retrieves the active search text.
     * @return Search query string.
     */
    [[nodiscard]] QString searchText() const { return m_searchText; }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_searchText;
};

/**
 * @class BaseTablePanelDialog
 * @brief Reusable modeless tactical dialog establishing the concrete layout and interactions for all table panels.
 *
 * Architectural Role & Responsibilities:
 * - Resides in the Core Wrappers layer (`src/core/wrappers/`).
 * - Implements GISApp::Core::Interfaces::ITablePanelDialog and inherits QDialog.
 * - Standardizes modeless windowing, tactical dark theme styles, toolbars, and QTableView behaviors.
 * - Employs the Template Method design pattern: provides ready-to-use search, proxy sorting, and table
 *   views, while offering virtual hooks for derived classes to inject domain-specific filter dropdowns,
 *   custom row filtering algorithms, metrics formatting, and click handlers.
 * - Acts as the base class for specialized dialogs such as TrackTablePanelDialog, SensorTablePanelDialog, etc.
 */
class BaseTablePanelDialog : public QDialog, public GISApp::Core::Interfaces::ITablePanelDialog
{
    Q_OBJECT
    friend class GenericTableFilterProxyModel;

public:
    /**
     * @brief Constructs BaseTablePanelDialog with an optional source model and parent widget.
     * @param[in] model Optional pointer to QAbstractItemModel.
     * @param[in] parent Optional parent QWidget.
     */
    explicit BaseTablePanelDialog(QAbstractItemModel *model = nullptr, QWidget *parent = nullptr);

    /**
     * @brief Virtual destructor releasing table views, layouts, and proxy models.
     */
    virtual ~BaseTablePanelDialog() override = default;

    // ITablePanelDialog interface implementation
    void showDialog() override;
    void closeDialog() override;
    [[nodiscard]] bool isDialogVisible() const override;
    void setModel(QAbstractItemModel *model) override;
    [[nodiscard]] QAbstractItemModel* model() const override;
    void setSearchFilter(const QString &filterText) override;
    void updateStatistics() override;
    void refreshData() override;

    /**
     * @brief Retrieves the underlying QTableView.
     * @return Pointer to QTableView.
     */
    [[nodiscard]] QTableView* tableView() const { return m_tableView; }

    /**
     * @brief Retrieves the active QSortFilterProxyModel.
     * @return Pointer to QSortFilterProxyModel.
     */
    [[nodiscard]] QSortFilterProxyModel* proxyModel() const { return m_proxyModel; }

    /**
     * @brief Retrieves the search line edit control.
     * @return Pointer to QLineEdit.
     */
    [[nodiscard]] QLineEdit* searchEdit() const { return m_searchEdit; }

    /**
     * @brief Retrieves the metrics status badge label.
     * @return Pointer to QLabel.
     */
    [[nodiscard]] QLabel* statusBadge() const { return m_statusBadge; }

signals:
    /**
     * @brief Emitted when a row is clicked by the user.
     * @param[in] sourceRow 0-based row index in the underlying source model.
     */
    void rowSelected(int sourceRow);

    /**
     * @brief Emitted when a row is double-clicked by the user.
     * @param[in] sourceRow 0-based row index in the underlying source model.
     */
    void rowDoubleClicked(int sourceRow);

protected:
    /**
     * @brief Virtual hook allowing derived classes to insert custom filter widgets (e.g. combo boxes)
     * into the top toolbar directly adjacent to the search box.
     * @param[in] customFilterLayout Target horizontal layout inside the toolbar.
     */
    virtual void setupCustomFilters(QHBoxLayout *customFilterLayout);

    /**
     * @brief Virtual hook allowing derived dialogs to provide custom row filtering logic.
     * @param[in] sourceRow Row index in the source model.
     * @param[in] sourceParent Parent model index.
     * @return True if the row should be displayed, false to filter out.
     */
    virtual bool customFilterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

    /**
     * @brief Virtual hook called when an operator single-clicks a table row.
     * @param[in] sourceRow Row index in the source model.
     */
    virtual void handleRowSelected(int sourceRow);

    /**
     * @brief Virtual hook called when an operator double-clicks a table row.
     * @param[in] sourceRow Row index in the source model.
     */
    virtual void handleRowDoubleClicked(int sourceRow);

    /**
     * @brief Initializes the base UI controls, tactical dark stylesheet, and layouts.
     */
    virtual void setupBaseUi();

    /**
     * @brief Establishes core signal-slot connections for filtering, searching, and table clicks.
     */
    virtual void setupBaseConnections();

    /// Source data model bound to the view.
    QAbstractItemModel *m_sourceModel{nullptr};

    /// Sorting and filtering proxy model.
    GenericTableFilterProxyModel *m_proxyModel{nullptr};

    /// Primary tabular view.
    QTableView *m_tableView{nullptr};

    /// Top search box.
    QLineEdit *m_searchEdit{nullptr};

    /// Informational telemetry badge.
    QLabel *m_statusBadge{nullptr};

    /// Manual refresh trigger button.
    QPushButton *m_refreshBtn{nullptr};

    /// Toolbar container for custom filter widgets added by derived classes.
    QHBoxLayout *m_customFilterLayout{nullptr};

private slots:
    void onSearchTextChanged(const QString &text);
    void onTableCellClicked(const QModelIndex &index);
    void onTableCellDoubleClicked(const QModelIndex &index);
};

} // namespace GISApp::Core::Wrappers

#endif // BASETABLEPANELDIALOG_H
