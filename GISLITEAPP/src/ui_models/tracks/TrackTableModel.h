/**
 * @file TrackTableModel.h
 * @brief Table model for rendering tactical track lists using true STRUCT_TRACK properties.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef TRACKTABLEMODEL_H
#define TRACKTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QColor>
#include <QFont>

#include "TacticalTrack.h"
#include "ITrackRepository.h"

namespace GISApp::UIModels::Tracks {

/**
 * @class TrackTableModel
 * @brief High-performance table model displaying tactical tracks based on canonical STRUCT_TRACK properties.
 *
 * Architectural Role & Design Patterns:
 * - Resides in the UI Models layer (`src/ui_models/tracks/`).
 * - Implements the **Qt Model-View Architecture** (`QAbstractTableModel`).
 * - Employs `FieldKeyValueMapper` as the single source of truth for converting protocol/attribute
 *   integers into human-readable tactical terminology.
 */
class TrackTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * @brief Logical columns represented by the model directly reflecting STRUCT_TRACK properties.
     */
    enum Column {
        ColumnId = 0,          ///< Unique system track identifier (track_id)
        ColumnName,            ///< Designated track callsign/name (track_name)
        ColumnIdentity,        ///< Track identity (HOSTILE, FRIENDLY, NEUTRAL, UNKNOWN)
        ColumnType,            ///< Domain type (AIR, SURFACE, SUBSURFACE, LAND)
        ColumnSubType,         ///< Track specific subtype
        ColumnClassification,  ///< Security/IFF classification
        ColumnStrength,        ///< Target formation count / strength
        ColumnActivity,        ///< Activity type
        ColumnLatitude,        ///< Geodetic latitude in decimal degrees
        ColumnLongitude,       ///< Geodetic longitude in decimal degrees
        ColumnHeight,          ///< Elevation / height above MSL in meters
        ColumnDirection,       ///< Bearing / direction in degrees
        ColumnSystemType,      ///< System track designation (SYSTEM 1, SYSTEM 2/FUSED)
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
        RawHeightRole,                     ///< Raw double height in meters
        RawDirectionRole,                  ///< Raw double direction in degrees
        TrackIdentityRole,                 ///< Integer code representing IDENTITY
        TrackObjectRole                    ///< Full TacticalTrack instance via QVariant
    };

    /**
     * @brief Constructs TrackTableModel with an optional track repository.
     * @param[in] repo Optional pointer to ITrackRepository.
     * @param[in] parent Optional parent QObject for lifecycle management.
     */
    explicit TrackTableModel(GISApp::Repositories::ITrackRepository *repo = nullptr, QObject *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~TrackTableModel() override = default;

    /**
     * @brief Sets or replaces the track repository.
     * @param[in] repo Pointer to ITrackRepository.
     */
    void setRepository(GISApp::Repositories::ITrackRepository *repo);

    /**
     * @brief Retrieves the active track repository.
     * @return Pointer to active ITrackRepository.
     */
    [[nodiscard]] GISApp::Repositories::ITrackRepository* repository() const { return m_repo; }

    /**
     * @brief Returns the TacticalTrack instance at the specified model row.
     * @param[in] row 0-based row index.
     * @return TacticalTrack instance or empty track if invalid index.
     */
    [[nodiscard]] GISApp::Domain::Tracks::TacticalTrack getTrackAt(int row) const;

    /**
     * @brief Retrieves all active tactical tracks currently loaded in the model.
     * @return Const reference to track vector.
     */
    [[nodiscard]] const QVector<GISApp::Domain::Tracks::TacticalTrack>& tracks() const { return m_tracks; }

    // --- QAbstractTableModel Interface ---
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public slots:
    /**
     * @brief Slot triggered when the repository signals tracks have been updated.
     */
    void onTracksUpdated();

    /**
     * @brief Re-queries all tracks from the repository and resets the model.
     */
    void reloadTracks();

private:
    void setupRepositoryConnections();

    GISApp::Repositories::ITrackRepository *m_repo{nullptr};
    QVector<GISApp::Domain::Tracks::TacticalTrack> m_tracks;
};

} // namespace GISApp::UIModels::Tracks

#endif // TRACKTABLEMODEL_H
