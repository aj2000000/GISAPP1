/**
 * @file ThemeManager.cpp
 * @brief Implementation of ThemeManager stylesheet generation.
 */

#include "ThemeManager.h"
#include "DatabaseManager.h"
#include <QApplication>

namespace GISApp::UI {

/**
 * @brief Retrieves the global ThemeManager singleton instance.
 * @return Reference to the persistent ThemeManager singleton.
 */
ThemeManager& ThemeManager::instance()
{
    static ThemeManager s_instance;
    return s_instance;
}

/**
 * @brief Constructs the ThemeManager singleton and defaults to TacticalDark theme.
 * @param[in] parent Optional QObject parent pointer.
 */
ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent), m_currentTheme(ThemeType::TacticalDark)
{
}

/**
 * @brief Converts a ThemeType enum value to a human-readable display string.
 * @param[in] theme ThemeType to format.
 * @return QString representing the descriptive name of the theme.
 */
QString ThemeManager::themeName(ThemeType theme)
{
    switch (theme) {
        case ThemeType::CyberEmerald:     return "Cyber Emerald";
        case ThemeType::MidnightBlue:     return "Midnight Blue";
        case ThemeType::HighContrastDark: return "High-Contrast Dark";
        case ThemeType::LightOps:         return "Light Operations";
        case ThemeType::TacticalDark:
        default:                          return "Tactical Dark (Default)";
    }
}

/**
 * @brief Converts a ThemeType enum to a database key string.
 * @param[in] theme ThemeType to serialize.
 * @return Unique string key representation (e.g. "TacticalDark").
 */
QString ThemeManager::themeToKey(ThemeType theme)
{
    switch (theme) {
        case ThemeType::CyberEmerald:     return QStringLiteral("CyberEmerald");
        case ThemeType::MidnightBlue:     return QStringLiteral("MidnightBlue");
        case ThemeType::HighContrastDark: return QStringLiteral("HighContrastDark");
        case ThemeType::LightOps:         return QStringLiteral("LightOps");
        case ThemeType::TacticalDark:
        default:                          return QStringLiteral("TacticalDark");
    }
}

/**
 * @brief Parses a database key string to a ThemeType enum value.
 * @param[in] key Key string from persistent settings.
 * @return Matching ThemeType value, defaulting to TacticalDark if unknown.
 */
ThemeType ThemeManager::themeFromKey(const QString &key)
{
    if (key == QStringLiteral("CyberEmerald"))     return ThemeType::CyberEmerald;
    if (key == QStringLiteral("MidnightBlue"))     return ThemeType::MidnightBlue;
    if (key == QStringLiteral("HighContrastDark")) return ThemeType::HighContrastDark;
    if (key == QStringLiteral("LightOps"))         return ThemeType::LightOps;
    return ThemeType::TacticalDark;
}

/**
 * @brief Loads the saved theme from persistent SQLite storage, or returns default if not set.
 * @return Persisted ThemeType enum value.
 */
ThemeType ThemeManager::loadSavedThemeOrDefault() const
{
    QString savedKey = GISApp::Database::DatabaseManager::instance().getSetting(
        QStringLiteral("app_theme"), QStringLiteral("TacticalDark"));
    return themeFromKey(savedKey);
}

/**
 * @brief Applies the selected visual theme across the entire application stylesheet.
 * @param[in] theme The ThemeType to apply.
 * @param[in] saveToDb If true, records the selected theme in the SQLite database.
 * @note Re-renders all styled Qt widgets and emits themeChanged signal.
 */
void ThemeManager::applyTheme(ThemeType theme, bool saveToDb)
{
    m_currentTheme = theme;
    QString qss = getStyleSheet(theme);
    if (qApp) {
        qApp->setStyleSheet(qss);
    }
    if (saveToDb) {
        GISApp::Database::DatabaseManager::instance().setSetting(
            QStringLiteral("app_theme"), themeToKey(theme));
    }
    emit themeChanged(theme);
}

/**
 * @brief Builds the comprehensive QSS stylesheet for the given theme.
 * @param[in] theme Target ThemeType.
 * @return Complete Qt StyleSheet string with fully resolved colors, borders, and icons.
 */
QString ThemeManager::getStyleSheet(ThemeType theme) const
{
    QString bgPrimary, bgPanel, bgFloating, accent, border, textPrimary, textMuted;
    QString iconCheck, iconIndeterminate;

    switch (theme) {
        case ThemeType::CyberEmerald:
            bgPrimary   = "#040b07"; bgPanel   = "#08170e"; bgFloating = "rgba(8, 23, 14, 0.88)";
            accent      = "#00ff88"; border    = "#10331f"; textPrimary= "#e6fffa"; textMuted  = "#62a884";
            iconCheck   = ":/icons/checkbox_checked_dark.svg";
            iconIndeterminate = ":/icons/checkbox_indeterminate_dark.svg";
            break;
        case ThemeType::MidnightBlue:
            bgPrimary   = "#080d14"; bgPanel   = "#0f1724"; bgFloating = "rgba(15, 23, 36, 0.88)";
            accent      = "#38bdf8"; border    = "#1e293b"; textPrimary= "#f0f9ff"; textMuted  = "#8497b0";
            iconCheck   = ":/icons/checkbox_checked_white.svg";
            iconIndeterminate = ":/icons/checkbox_indeterminate_white.svg";
            break;
        case ThemeType::HighContrastDark:
            bgPrimary   = "#000000"; bgPanel   = "#121212"; bgFloating = "rgba(18, 18, 18, 0.92)";
            accent      = "#eab308"; border    = "#27272a"; textPrimary= "#ffffff"; textMuted  = "#d4d4d8";
            iconCheck   = ":/icons/checkbox_checked_dark.svg";
            iconIndeterminate = ":/icons/checkbox_indeterminate_dark.svg";
            break;
        case ThemeType::LightOps:
            bgPrimary   = "#f1f5f9"; bgPanel   = "#ffffff"; bgFloating = "rgba(255, 255, 255, 0.95)";
            accent      = "#2563eb"; border    = "#cbd5e1"; textPrimary= "#0f172a"; textMuted  = "#64748b";
            iconCheck   = ":/icons/checkbox_checked_white.svg";
            iconIndeterminate = ":/icons/checkbox_indeterminate_white.svg";
            break;
        case ThemeType::TacticalDark:
        default:
            bgPrimary   = "#080a0c"; bgPanel   = "#0f1317"; bgFloating = "rgba(15, 19, 23, 0.88)";
            accent      = "#10b981"; border    = "#1a222a"; textPrimary= "#f3f4f6"; textMuted  = "#9ca3af";
            iconCheck   = ":/icons/checkbox_checked_white.svg";
            iconIndeterminate = ":/icons/checkbox_indeterminate_white.svg";
            break;
    }

    return QString(R"(
        QMainWindow {
            background-color: %1;
            color: %6;
        }
        QMenuBar {
            background-color: %2;
            color: %6;
            border-bottom: 1px solid %5;
            font-size: 11px;
            font-weight: bold;
        }
        QMenuBar::item {
            padding: 6px 10px;
            background: transparent;
        }
        QMenuBar::item:selected {
            background: %5;
            color: %4;
        }
        QMenu {
            background-color: %2;
            color: %6;
            border: 1px solid %5;
        }
        QMenu::item:selected {
            background-color: %5;
            color: %4;
        }
        QFrame#HeaderBar {
            background-color: %2;
            border-bottom: 1px solid %5;
        }
        QPushButton#HeaderActionButton {
            background-color: rgba(255, 255, 255, 0.05);
            color: %6;
            border: 1px solid %5;
            border-radius: 4px;
            padding: 4px 10px;
            font-size: 11px;
        }
        QPushButton#HeaderActionButton:hover {
            background-color: %5;
            color: %4;
        }
        QWidget#LeftSidebar {
            background-color: %2;
            border-right: 1px solid %5;
        }
        QFrame#RightToolPanel, QFrame#ZoomControlsWidget {
            background-color: %3;
            border: 1px solid %5;
            border-radius: 8px;
        }
        QFrame#RightToolPanel QToolButton, QFrame#ZoomControlsWidget QToolButton {
            background-color: transparent;
            color: %6;
            border: none;
            border-radius: 4px;
            font-size: 14px;
        }
        QFrame#RightToolPanel QToolButton:hover, QFrame#ZoomControlsWidget QToolButton:hover {
            background-color: rgba(255, 255, 255, 0.15);
            color: %4;
        }
        /* QToolBar Complete Modern Styling */
        QToolBar {
            background-color: %2;
            border: none;
            border-bottom: 1px solid %5;
            spacing: 4px;
            padding: 3px 6px;
        }
        QToolBar::handle:horizontal {
            width: 4px;
            margin: 4px 3px;
            background-color: %5;
            border-radius: 2px;
        }
        QToolBar::handle:vertical {
            height: 4px;
            margin: 3px 4px;
            background-color: %5;
            border-radius: 2px;
        }
        QToolBar::separator {
            background-color: %5;
            width: 1px;
            margin: 4px 6px;
        }
        QToolBar QToolButton {
            background-color: transparent;
            color: %6;
            border: 1px solid transparent;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 12px;
            font-weight: 500;
        }
        QToolBar QToolButton:hover {
            background-color: rgba(255, 255, 255, 0.08);
            border: 1px solid %5;
            color: %4;
        }
        QToolBar QToolButton:pressed {
            background-color: rgba(255, 255, 255, 0.15);
            color: %4;
        }
        QToolBar QToolButton:checked {
            background-color: rgba(255, 255, 255, 0.12);
            border: 1px solid %4;
            color: %4;
        }
        QToolBar QToolButton::menu-indicator {
            subcontrol-origin: padding;
            subcontrol-position: center right;
            right: 2px;
            width: 8px;
        }

        /* QStatusBar & Status Labels */
        QStatusBar {
            background-color: %2;
            color: %6;
            border-top: 1px solid %5;
        }
        QStatusBar QLabel {
            color: %6;
            background: transparent;
            font-size: 11px;
        }
        QLabel#StatusBarLabel {
            color: %4;
            font-weight: bold;
            font-family: 'Monospace', 'Courier New', monospace;
            padding: 0px 14px 0px 8px;
            font-size: 12px;
            background: transparent;
        }
        QLabel#InfoLabel {
            color: %7;
            font-size: 11px;
            font-family: monospace;
        }
        QLabel#CoordLabel {
            color: %4;
            font-family: 'Monospace', 'Courier New', monospace;
            font-weight: bold;
            font-size: 12px;
        }
        QToolButton {
            background-color: transparent;
            color: %6;
            border: none;
            border-radius: 4px;
        }
        QToolButton:hover {
            background-color: rgba(255, 255, 255, 0.10);
            color: %4;
        }
        QToolButton:checked {
            background-color: rgba(16, 185, 129, 0.15);
            color: %4;
            border: 1px solid %4;
        }
        QToolButton#HomeButton {
            background-color: rgba(16, 185, 129, 0.20);
            color: #10b981;
            border: 1px solid rgba(16, 185, 129, 0.5);
            border-radius: 6px;
        }
        QToolButton#HomeButton:hover {
            background-color: rgba(16, 185, 129, 0.30);
            color: #34d399;
        }

        /* Global Dialog & MessageBox Styling for High-Contrast Text Visibility */
        QDialog, QMessageBox, QInputDialog, QFileDialog {
            background-color: %2;
            color: %6;
            border: 1px solid %5;
            border-radius: 8px;
        }
        QMessageBox QLabel, QDialog QLabel, QInputDialog QLabel, QFileDialog QLabel {
            color: %6;
            font-size: 13px;
            font-weight: 500;
            background: transparent;
        }
        QMessageBox QPushButton, QDialogButtonBox QPushButton, QDialog QPushButton {
            background-color: %5;
            color: %6;
            border: 1px solid rgba(255, 255, 255, 0.2);
            border-radius: 6px;
            padding: 6px 16px;
            font-size: 12px;
            font-weight: bold;
            min-width: 75px;
        }
        QMessageBox QPushButton:hover, QDialogButtonBox QPushButton:hover, QDialog QPushButton:hover {
            background-color: %4;
            color: #ffffff;
            border-color: %4;
        }
        QMessageBox QPushButton:pressed, QDialogButtonBox QPushButton:pressed, QDialog QPushButton:pressed {
            background-color: %4;
            color: #ffffff;
        }
        QMessageBox QPushButton:disabled, QDialogButtonBox QPushButton:disabled, QDialog QPushButton:disabled {
            background-color: rgba(255, 255, 255, 0.05);
            color: %7;
            border-color: %5;
        }
        QLineEdit, QTextEdit, QPlainTextEdit {
            background-color: %1;
            color: %6;
            border: 1px solid %5;
            border-radius: 6px;
            padding: 5px 8px;
            selection-background-color: %4;
            selection-color: #ffffff;
        }

        /* =======================================================
           QCheckBox & Checkbox Indicators (Application-Wide)
           ======================================================= */
        QCheckBox {
            spacing: 8px;
            color: %6;
            font-size: 12px;
            font-weight: 500;
            background: transparent;
        }
        QCheckBox:hover {
            color: %4;
        }
        QCheckBox:disabled {
            color: %7;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid %5;
            border-radius: 4px;
            background-color: %1;
        }
        QCheckBox::indicator:unchecked:hover {
            border: 1px solid %4;
            background-color: rgba(255, 255, 255, 0.08);
        }
        QCheckBox::indicator:unchecked:pressed {
            border: 1px solid %4;
            background-color: rgba(255, 255, 255, 0.15);
        }
        QCheckBox::indicator:checked {
            background-color: %4;
            border: 1px solid %4;
            image: url(@ICON_CHECK@);
        }
        QCheckBox::indicator:checked:hover {
            background-color: %4;
            border: 1px solid #ffffff;
            image: url(@ICON_CHECK@);
        }
        QCheckBox::indicator:indeterminate {
            background-color: %4;
            border: 1px solid %4;
            image: url(@ICON_INDET@);
        }
        QCheckBox::indicator:indeterminate:hover {
            background-color: %4;
            border: 1px solid #ffffff;
            image: url(@ICON_INDET@);
        }
        QCheckBox::indicator:disabled {
            border: 1px solid %5;
            background-color: rgba(255, 255, 255, 0.03);
        }

        /* QSpinBox & QDoubleSpinBox Modern High-Contrast Styling */
        QSpinBox, QDoubleSpinBox {
            background-color: %1;
            color: %6;
            border: 1px solid %5;
            border-radius: 6px;
            padding: 4px 24px 4px 10px;
            min-height: 24px;
            selection-background-color: %4;
            selection-color: #ffffff;
            font-weight: 500;
        }
        QSpinBox:hover, QSpinBox:focus, QDoubleSpinBox:hover, QDoubleSpinBox:focus {
            border-color: %4;
        }
        QSpinBox::up-button, QDoubleSpinBox::up-button {
            subcontrol-origin: border;
            subcontrol-position: top right;
            width: 20px;
            border-left: 1px solid %5;
            border-bottom: 1px solid %5;
            background-color: %2;
            border-top-right-radius: 5px;
        }
        QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover {
            background-color: %4;
        }
        QSpinBox::up-button:pressed, QDoubleSpinBox::up-button:pressed {
            background-color: %3;
        }
        QSpinBox::up-arrow, QDoubleSpinBox::up-arrow {
            width: 0;
            height: 0;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-bottom: 5px solid %6;
        }
        QSpinBox::up-button:hover QSpinBox::up-arrow, QDoubleSpinBox::up-button:hover QDoubleSpinBox::up-arrow {
            border-bottom-color: #ffffff;
        }
        QSpinBox::down-button, QDoubleSpinBox::down-button {
            subcontrol-origin: border;
            subcontrol-position: bottom right;
            width: 20px;
            border-left: 1px solid %5;
            background-color: %2;
            border-bottom-right-radius: 5px;
        }
        QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {
            background-color: %4;
        }
        QSpinBox::down-button:pressed, QDoubleSpinBox::down-button:pressed {
            background-color: %3;
        }
        QSpinBox::down-arrow, QDoubleSpinBox::down-arrow {
            width: 0;
            height: 0;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid %6;
        }
        QSpinBox::down-button:hover QSpinBox::down-arrow, QDoubleSpinBox::down-button:hover QDoubleSpinBox::down-arrow {
            border-top-color: #ffffff;
        }

        /* QComboBox Complete High-Contrast & Visible Text Styling */
        QComboBox {
            background-color: %1;
            color: %6;
            border: 1px solid %5;
            border-radius: 6px;
            padding: 4px 28px 4px 10px;
            min-height: 24px;
            selection-background-color: %4;
            selection-color: #ffffff;
            font-weight: 500;
        }
        QComboBox:hover, QComboBox:focus {
            border-color: %4;
        }
        QComboBox:on {
            border-color: %4;
            background-color: %2;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 24px;
            border-left: none;
            background-color: transparent;
        }
        QComboBox::down-arrow {
            width: 0;
            height: 0;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid %4;
            margin-right: 8px;
        }
        QComboBox QAbstractItemView {
            background-color: %2;
            color: %6;
            border: 1px solid %4;
            border-radius: 6px;
            padding: 4px;
            selection-background-color: %4;
            selection-color: #ffffff;
            outline: 0px;
        }
        QComboBox QAbstractItemView::item {
            min-height: 26px;
            padding: 4px 10px;
            color: %6;
            background-color: transparent;
        }
        QComboBox QAbstractItemView::item:hover, QComboBox QAbstractItemView::item:selected {
            background-color: %4;
            color: #ffffff;
            border-radius: 4px;
        }
        QToolTip {
            background-color: %2;
            color: %6;
            border: 1px solid %4;
            padding: 4px 8px;
            border-radius: 4px;
        }

        /* QTableWidget & QTableView Premium Styling */
        QTableWidget, QTableView {
            background-color: %1;
            alternate-background-color: %2;
            color: %6;
            gridline-color: %5;
            border: 1px solid %5;
            border-radius: 8px;
            selection-background-color: %4;
            selection-color: #ffffff;
            outline: 0px;
            font-size: 12px;
        }
        QTableWidget::item, QTableView::item {
            color: %6;
            padding: 6px 10px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
        }
        QTableWidget::item:hover, QTableView::item:hover {
            background-color: rgba(56, 189, 248, 0.15);
            color: #ffffff;
        }
        QTableWidget::item:selected, QTableView::item:selected {
            background-color: %4;
            color: #ffffff;
            font-weight: bold;
        }
        QHeaderView {
            background-color: %2;
            border: none;
            border-bottom: 2px solid %4;
        }
        QHeaderView::section {
            background-color: %2;
            color: %4;
            padding: 8px 12px;
            font-weight: bold;
            font-size: 12px;
            border: none;
            border-right: 1px solid %5;
            border-bottom: 2px solid %4;
        }
        QHeaderView::section:hover {
            background-color: %1;
            color: #ffffff;
        }
        QTableCornerButton::section {
            background-color: %2;
            border: 1px solid %5;
        }

        /* QScrollBar Vertical & Horizontal Custom High-Contrast Styling */
        QScrollBar:vertical {
            background-color: %1;
            width: 10px;
            margin: 0px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical {
            background-color: %5;
            min-height: 25px;
            border-radius: 4px;
            margin: 2px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: %4;
        }
        QScrollBar::handle:vertical:pressed {
            background-color: %4;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
            background: none;
            border: none;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }

        QScrollBar:horizontal {
            background-color: %1;
            height: 10px;
            margin: 0px;
            border-radius: 5px;
        }
        QScrollBar::handle:horizontal {
            background-color: %5;
            min-width: 25px;
            border-radius: 4px;
            margin: 2px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: %4;
        }
        QScrollBar::handle:horizontal:pressed {
            background-color: %4;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
            background: none;
            border: none;
        }
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
            background: none;
        }

        /* =======================================================
           Floating Layer Tree Panel & Components (Theme-Synced)
           ======================================================= */
        QFrame#LayerTreePanel {
            background-color: %3;
            border: 1px solid %5;
            border-radius: 8px;
        }
        QWidget#LayerTreeHeader {
            background-color: %2;
            border-bottom: 1px solid %5;
            border-top-left-radius: 7px;
            border-top-right-radius: 7px;
        }
        QLabel#LayerTreeTitle {
            color: %4;
            font-weight: bold;
            font-size: 13px;
            background: transparent;
        }
        QLabel#LayerTreeBadge {
            color: %7;
            font-size: 11px;
            background: transparent;
        }
        QToolButton#LayerTreeCloseButton {
            background-color: transparent;
            color: %7;
            border: none;
            border-radius: 4px;
            font-size: 13px;
            padding: 2px;
        }
        QToolButton#LayerTreeCloseButton:hover {
            background-color: rgba(239, 68, 68, 0.20);
            color: #ef4444;
        }
        QWidget#LayerTreeToolbar {
            background: transparent;
        }
        QToolButton#LayerTreeActionBtn {
            background-color: rgba(255, 255, 255, 0.05);
            color: %6;
            border: 1px solid %5;
            border-radius: 4px;
            font-size: 13px;
        }
        QToolButton#LayerTreeActionBtn:hover {
            background-color: %5;
            border: 1px solid %4;
            color: %4;
        }
        QToolButton#LayerTreeActionBtn:pressed {
            background-color: %4;
            color: #ffffff;
        }
        QLineEdit#LayerFilterEdit {
            background-color: %1;
            color: %6;
            border: 1px solid %5;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 11px;
        }
        QLineEdit#LayerFilterEdit:focus {
            border: 1px solid %4;
        }
        QTreeView#GISLayerTreeView {
            background-color: %1;
            color: %6;
            border: 1px solid %5;
            border-radius: 4px;
            outline: 0px;
            font-size: 12px;
            padding: 4px;
        }
        QTreeView#GISLayerTreeView::item {
            color: %6;
            padding: 4px 6px;
            min-height: 26px;
            border-radius: 4px;
            border: 1px solid transparent;
        }
        QTreeView#GISLayerTreeView::item:hover {
            background-color: rgba(255, 255, 255, 0.08);
            color: %4;
            border: 1px solid rgba(255, 255, 255, 0.12);
        }
        QTreeView#GISLayerTreeView::item:selected {
            background-color: rgba(255, 255, 255, 0.14);
            border: 1px solid %4;
            color: #ffffff;
            font-weight: bold;
        }
        QTreeView#GISLayerTreeView::item:selected:hover {
            background-color: rgba(255, 255, 255, 0.20);
            border: 1px solid %4;
            color: #ffffff;
        }
        QTreeView#GISLayerTreeView::branch {
            background: transparent;
        }
        QTreeView#GISLayerTreeView::branch:has-children:!has-siblings:closed,
        QTreeView#GISLayerTreeView::branch:closed:has-children:has-siblings {
            image: url(:/icons/branch_closed.svg);
        }
        QTreeView#GISLayerTreeView::branch:open:has-children:!has-siblings,
        QTreeView#GISLayerTreeView::branch:open:has-children:has-siblings {
            image: url(:/icons/branch_open.svg);
        }

        /* QTreeView Checkbox Indicators (Unified High-Contrast Styling) */
        QTreeView::indicator,
        QTreeView#GISLayerTreeView::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid %5;
            border-radius: 4px;
            background-color: %2;
            margin-right: 6px;
        }
        QTreeView::indicator:unchecked,
        QTreeView#GISLayerTreeView::indicator:unchecked {
            background-color: %2;
            border: 1px solid %5;
        }
        QTreeView::indicator:unchecked:hover,
        QTreeView#GISLayerTreeView::indicator:unchecked:hover {
            border: 1px solid %4;
            background-color: rgba(255, 255, 255, 0.08);
        }
        QTreeView::indicator:unchecked:pressed,
        QTreeView#GISLayerTreeView::indicator:unchecked:pressed {
            border: 1px solid %4;
            background-color: rgba(255, 255, 255, 0.15);
        }
        QTreeView::indicator:checked,
        QTreeView#GISLayerTreeView::indicator:checked {
            background-color: %4;
            border: 1px solid %4;
            image: url(@ICON_CHECK@);
        }
        QTreeView::indicator:checked:hover,
        QTreeView#GISLayerTreeView::indicator:checked:hover {
            background-color: %4;
            border: 1px solid #ffffff;
            image: url(@ICON_CHECK@);
        }
        QTreeView::indicator:indeterminate,
        QTreeView#GISLayerTreeView::indicator:indeterminate {
            background-color: %4;
            border: 1px solid %4;
            image: url(@ICON_INDET@);
        }
        QTreeView::indicator:indeterminate:hover,
        QTreeView#GISLayerTreeView::indicator:indeterminate:hover {
            background-color: %4;
            border: 1px solid #ffffff;
            image: url(@ICON_INDET@);
        }
        QTreeView::indicator:disabled,
        QTreeView#GISLayerTreeView::indicator:disabled {
            border: 1px solid %5;
            background-color: rgba(255, 255, 255, 0.03);
        }
        QMenu#LayerContextMenu {
            background-color: %2;
            color: %6;
            border: 1px solid %5;
            border-radius: 6px;
            padding: 4px;
        }
        QMenu#LayerContextMenu::item {
            padding: 6px 14px;
            border-radius: 4px;
            color: %6;
        }
        QMenu#LayerContextMenu::item:selected {
            background-color: %4;
            color: #ffffff;
        }

        /* =======================================================
           Tactical Table Panels & Dialog Components
           ======================================================= */
        QDialog#BaseTablePanelDialog,
        QDialog#TrackTablePanelDialog {
            background-color: %2;
            color: %6;
            border: 1px solid %5;
            border-radius: 8px;
        }
        QLabel#StatusBadge {
            color: %7;
            font-size: 12px;
            padding: 4px 10px;
            background-color: %1;
            border: 1px solid %5;
            border-radius: 4px;
        }
        QLabel#TableFilterLabel {
            color: %7;
            font-size: 11px;
            font-weight: bold;
            padding-right: 4px;
            background: transparent;
        }

        /* =======================================================
           Tactical Track Detail & Inspector Dialogs
           ======================================================= */
        QDialog#TrackDetailDialog,
        QDialog#TrackEditDialog {
            background-color: %2;
            color: %6;
            border: 1px solid %5;
            border-radius: 8px;
        }
        QDialog#TrackDetailDialog QScrollArea,
        QDialog#TrackDetailDialog QScrollArea > QWidget > QWidget,
        QDialog#TrackDetailDialog QWidget#scrollContent {
            background-color: %2;
            border: none;
        }
        QDialog#TrackDetailDialog QGroupBox,
        QDialog#TrackEditDialog QGroupBox {
            background-color: %1;
            font-size: 11px;
            font-weight: bold;
            color: %4;
            border: 1px solid %5;
            border-radius: 6px;
            margin-top: 16px;
            padding-top: 16px;
            padding-bottom: 10px;
        }
        QDialog#TrackDetailDialog QGroupBox::title,
        QDialog#TrackEditDialog QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 12px;
            padding: 2px 6px;
            background-color: %2;
            color: %4;
            border: 1px solid %5;
            border-radius: 3px;
        }
        QLabel#TrackDetailCaption {
            color: %7;
            font-size: 10px;
            font-weight: bold;
            letter-spacing: 0.5px;
            background: transparent;
        }
        QLabel#TrackDetailValue {
            color: %6;
            background-color: %2;
            border: 1px solid %5;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 11px;
            font-weight: 600;
        }
        QLabel#TrackDetailTitle {
            font-size: 18px;
            font-weight: bold;
            color: %6;
            background: transparent;
        }
        QLabel#TrackDetailIdentityBadge {
            font-weight: bold;
            padding: 2px 12px;
            border-radius: 4px;
            font-size: 11px;
        }
        QLabel#TrackDetailIdentityBadge[identity="HOSTILE"] {
            background-color: #7f1d1d;
            color: #fca5a5;
            border: 1px solid #ef4444;
        }
        QLabel#TrackDetailIdentityBadge[identity="FRIENDLY"] {
            background-color: #0c4a6e;
            color: #7dd3fc;
            border: 1px solid #00d2ff;
        }
        QLabel#TrackDetailIdentityBadge[identity="NEUTRAL"] {
            background-color: #14532d;
            color: #86efac;
            border: 1px solid #22c55e;
        }
        QLabel#TrackDetailIdentityBadge[identity="UNKNOWN"] {
            background-color: #713f12;
            color: #fde047;
            border: 1px solid #eab308;
        }
        QPushButton#TrackDetailCloseBtn {
            background-color: %1;
            color: %6;
            border: 1px solid %5;
            padding: 8px 24px;
            border-radius: 4px;
            font-size: 12px;
            font-weight: bold;
        }
        QPushButton#TrackDetailCloseBtn:hover {
            background-color: %5;
            color: %4;
            border-color: %4;
        }

        /* =======================================================
           Authentication Window & Auth Pages (Login & Register)
           ======================================================= */
        QWidget#AuthWindow {
            background-color: %2;
            color: %6;
        }
        QWidget#LoginPage,
        QWidget#RegisterPage {
            background-color: transparent;
            font-family: 'Segoe UI', Arial, sans-serif;
        }
        QLabel#loginBrandTitle,
        QLabel#registerTitle {
            font-size: 26px;
            font-weight: 700;
            color: %4;
            letter-spacing: 1px;
            background: transparent;
        }
        QLabel#loginSubtitle,
        QLabel#registerSubtitle {
            font-size: 13px;
            color: %7;
            margin-bottom: 6px;
            background: transparent;
        }
        QLabel#fieldLabel {
            font-size: 12px;
            font-weight: 600;
            color: %6;
            margin-top: 4px;
            background: transparent;
        }
        QLabel#mutedNoticeLabel {
            color: %7;
            font-size: 13px;
            background: transparent;
        }
        QLabel#loginErrorLabel,
        QLabel#registerErrorLabel {
            background-color: rgba(239, 68, 68, 0.15);
            color: #f87171;
            border: 1px solid rgba(239, 68, 68, 0.4);
            border-radius: 6px;
            padding: 8px;
            font-size: 12px;
        }
        QPushButton#primaryButton {
            background-color: %4;
            color: #ffffff;
            font-size: 14px;
            font-weight: 600;
            padding: 10px;
            border-radius: 6px;
            border: none;
        }
        QPushButton#primaryButton:hover {
            background-color: %4;
            border: 1px solid #ffffff;
        }
        QPushButton#primaryButton:pressed {
            background-color: %5;
        }
        QPushButton#secondaryButton {
            background-color: %1;
            color: %6;
            font-size: 13px;
            font-weight: 500;
            padding: 8px;
            border-radius: 6px;
            border: 1px solid %5;
        }
        QPushButton#secondaryButton:hover {
            background-color: %5;
            color: %4;
            border-color: %4;
        }
        QPushButton#linkButton {
            color: %4;
            font-size: 13px;
            font-weight: 600;
            text-decoration: none;
            border: none;
            padding: 0;
            background: transparent;
        }
        QPushButton#linkButton:hover {
            color: #ffffff;
            text-decoration: underline;
        }
    )")
    .arg(bgPrimary, bgPanel, bgFloating, accent, border, textPrimary, textMuted)
    .replace("@ICON_CHECK@", iconCheck)
    .replace("@ICON_INDET@", iconIndeterminate);
}

} // namespace GISApp::UI
