/**
 * @file ThemeManager.h
 * @brief Centralized tactical theme manager and QSS stylesheet generator.
 *
 * Provides a thread-safe singleton managing the visual styling system across the GIS application,
 * generating dynamic CSS/QSS tailored to tactical operational environments (tactical dark, cyber
 * emerald, midnight blue, high-contrast dark, and light ops).
 */

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>

namespace GISApp::UI {

/**
 * @enum ThemeType
 * @brief Identifies available tactical visual themes.
 */
enum class ThemeType {
    TacticalDark,     ///< Default tactical operations theme (deep charcoal, emerald accents).
    CyberEmerald,     ///< High-tech night-ops cybernetic theme (dark forest, neon lime accents).
    MidnightBlue,     ///< Naval/maritime command theme (deep navy, sky blue accents).
    HighContrastDark, ///< Maximum readability theme for outdoor HUDs (pure black, amber accents).
    LightOps          ///< Daylight / sunlit environment theme (clean light grey, blue accents).
};

/**
 * @class ThemeManager
 * @brief Singleton managing application-wide visual themes and QSS generation.
 *
 * Architectural Role:
 * - Serves as the single source of truth for UI color schemes, typography, and widget styling.
 * - Broadcasts theme switch events to observing panels and views via the Qt signals/slots mechanism.
 * - Produces comprehensive QSS stylesheets including controls, buttons, toolbars, tree views,
 *   checkboxes, indicators, and dialogs.
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Retrieves the global ThemeManager singleton instance.
     * @return Reference to the persistent ThemeManager singleton.
     */
    static ThemeManager& instance();

    /**
     * @brief Gets the currently active tactical theme.
     * @return Current ThemeType enum value.
     */
    [[nodiscard]] ThemeType currentTheme() const { return m_currentTheme; }

    /**
     * @brief Converts a ThemeType enum to a human-readable display string.
     * @param[in] theme ThemeType to format.
     * @return User-facing localized theme name.
     */
    static QString themeName(ThemeType theme);

    /**
     * @brief Converts a ThemeType enum to a database key string.
     * @param[in] theme ThemeType to serialize.
     * @return Unique string key representation (e.g. "TacticalDark").
     */
    static QString themeToKey(ThemeType theme);

    /**
     * @brief Parses a database key string to a ThemeType enum value.
     * @param[in] key Key string from persistent settings.
     * @return Matching ThemeType value, defaulting to TacticalDark if unknown.
     */
    static ThemeType themeFromKey(const QString &key);

    /**
     * @brief Loads the saved theme from persistent SQLite storage, or returns default if not set.
     * @return Persisted ThemeType enum value.
     */
    [[nodiscard]] ThemeType loadSavedThemeOrDefault() const;

    /**
     * @brief Sets and applies a new theme across the entire QApplication.
     * @param[in] theme The new ThemeType to activate.
     * @param[in] saveToDb If true, stores the selected theme in the SQLite database.
     * @note Applies the stylesheet directly to `qApp` and notifies all subscribers via `themeChanged`.
     */
    void applyTheme(ThemeType theme, bool saveToDb = true);

    /**
     * @brief Generates the full CSS/QSS stylesheet string for the specified theme.
     * @param[in] theme Target theme type.
     * @return Complete, formatted Qt stylesheet string.
     */
    [[nodiscard]] QString getStyleSheet(ThemeType theme) const;

signals:
    /**
     * @brief Emitted whenever the active application theme is modified.
     * @param[in] theme Newly activated ThemeType.
     */
    void themeChanged(ThemeType theme);

private:
    /**
     * @brief Private constructor enforcing the singleton design pattern.
     * @param[in] parent Optional QObject parent.
     */
    explicit ThemeManager(QObject *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~ThemeManager() override = default;

    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    /**
     * @brief Currently active tactical visual theme.
     */
    ThemeType m_currentTheme;
};

} // namespace GISApp::UI

#endif // THEMEMANAGER_H
