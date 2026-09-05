/**
 * @file TrackTableModel.h
 * @brief Qt Abstract Table Model adapting tactical track domain entities for tabular views.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef TRACKTABLEMODEL_H
#define TRACKTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QColor>
#include "TacticalTrack.h"

namespace GISApp::Repositories {
class ITrackRepository;
}

namespace GISApp::UIModels::Tracks {

/**
 * @class TrackTableModel
 * @brief Qt Table Model providing tabular representation and formatting of tactical track data.
 *
 * Architectural Role & Responsibilities:
 * - Resides in the UI Models layer (Presentation / View-Model tier in Clean Architecture).
 * - Adapts raw domain entities (GISApp::Domain::Tracks::TacticalTrack) into a structured 2D table grid
 *   suitable for consumption by QTableView, QSortFilterProxyModel, and custom delegates.
 * - Subscribes to ITrackRepository::tracksUpdated() to ensure live telemetry changes reflect immediately
 *   in the UI without blocking the rendering or network threads.
 * - Enforces tactical military aesthetics (MIL-STD-2525 color-coding for Hostile, Friendly, and Neutral tracks).
 * - Formats geodetic coordinates, altitudes, speeds, and headings with standard aeronautical/marine units.
 */
class TrackTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * @brief Logical columns represented by the model.
     */
    enum Column {
        ColumnId = 0,          ///< Unique system track identifier
        ColumnCallsign,        ///< Operator or transponder callsign
        ColumnAffiliation,     ///< Hostile, Friendly, Neutral, Unknown
        ColumnDomain,          ///< Air, Surface, Land, Subsurface
        ColumnLatitude,        ///< Geodetic latitude in decimal degrees
        ColumnLongitude,       ///< Geodetic longitude in decimal degrees
        ColumnAltitude,        ///< Altitude in meters above mean sea level
        ColumnSpeed,           ///< Ground speed in km/h
        ColumnHeading,         ///< True heading in degrees
        ColumnReportTime,      ///< Timestamp of the most recent telemetry packet
        ColumnRemarks,         ///< Free-text operational notes or remarks
        ColumnCount            ///< Total number of columns
    };

    /**
     * @brief Custom user roles for programmatic extraction of raw, unformatted values.
     */
    enum TrackModelRoles {
        TrackIdRole = Qt::UserRole + 1,    ///< Raw integer track ID
        RawLatitudeRole,                   ///< Raw double latitude in degrees
        RawLongitudeRole,                  ///< Raw double longitude in degrees
        RawAltitudeRole,                   ///< Raw double altitude in meters
        RawSpeedRole,                      ///< Raw double ground speed in km/h
        RawHeadingRole,                    ///< Raw double heading in degrees
        TrackIdentityRole,                 ///< Integer enum representing TrackIdentity
        TrackObjectRole                    ///< Full TacticalTrack instance via QVariant
    };

    /**
     * @brief Constructs TrackTableModel with an optional track repository.
     * @param[in] repo Optional pointer to ITrackRepository.
     * @param[in] parent Optional parent QObject for lifecycle management.
     */
    explicit TrackTableModel(GISApp::Repositories::ITrackRepository *repo = nullptr, QObject *parent = nullptr);

    /**
     * @brief Virtual destructor releasing model resources.
     */
    virtual ~TrackTableModel() override = default;

    /**
     * @brief Attaches a new track repository and establishes update connections.
     * @param[in] repo Pointer to ITrackRepository.
     */
    void setTrackRepository(GISApp::Repositories::ITrackRepository *repo);

    /**
     * @brief Retrieves the attached track repository.
     * @return Pointer to ITrackRepository, or nullptr if none attached.
     */
    [[nodiscard]] GISApp::Repositories::ITrackRepository* trackRepository() const { return m_repo; }

    /**
     * @brief Retrieves the domain track entity located at a specific row index.
     * @param[in] row Zero-based row index in the model.
     * @return Copy of the TacticalTrack entity, or an empty default track if out of bounds.
     */
    [[nodiscard]] GISApp::Domain::Tracks::TacticalTrack getTrackAt(int row) const;

    /**
     * @brief Retrieves read-only access to all active track entities cached in the model.
     * @return Const reference to internal QVector of TacticalTrack.
     */
    [[nodiscard]] const QVector<GISApp::Domain::Tracks::TacticalTrack>& tracks() const { return m_tracks; }

    // QAbstractTableModel interface overrides
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public slots:
    /**
     * @brief Refetches tracks from the attached repository and updates the model.
     */
    void reloadTracks();

    /**
     * @brief Slot triggered when the attached repository emits tracksUpdated.
     */
    void onTracksUpdated();

private:
    /**
     * @brief Establishes Qt signal-slot bindings with the attached repository.
     */
    void setupRepositoryConnections();

    /// Attached domain track repository supplying telemetry entities.
    GISApp::Repositories::ITrackRepository *m_repo{nullptr};

    /// Local cached snapshot of tactical tracks displayed by the table.
    QVector<GISApp::Domain::Tracks::TacticalTrack> m_tracks;
};

} // namespace GISApp::UIModels::Tracks

#endif // TRACKTABLEMODEL_H
