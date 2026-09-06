/**
 * @file TacticalTrackService.h
 * @brief Domain service providing business logic, spatial querying, and GeoJSON serialization for tactical tracks.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef TACTICALTRACKSERVICE_H
#define TACTICALTRACKSERVICE_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <optional>
#include "ITrackRepository.h"
#include "TacticalTrack.h"

namespace GISApp::Services::Tracks {

/**
 * @class TacticalTrackService
 * @brief Pure application domain service managing tactical tracks.
 *
 * Architectural Role:
 * - Resides strictly in the Service Layer (Domain / Application Services).
 * - Subscribes to ITrackRepository track update events.
 * - Performs domain calculations, track aggregation, and standard GeoJSON serialization.
 * - Entirely UI-agnostic and presentation-independent (no dependencies on QWidget, MapWidget, or QMapLibre).
 * - Emits clean domain and data signals consumed by presentation controllers (TrackController).
 */
class TacticalTrackService : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs TacticalTrackService.
     * @param[in] trackRepo Pointer to ITrackRepository data source.
     * @param[in] parent Optional parent QObject.
     */
    explicit TacticalTrackService(GISApp::Repositories::ITrackRepository *trackRepo,
                                  QObject *parent = nullptr);
    virtual ~TacticalTrackService() override = default;

    /**
     * @brief Retrieves all active tactical tracks currently tracked in the repository.
     * @return QVector of TacticalTrack domain entities.
     */
    [[nodiscard]] QVector<GISApp::Domain::Tracks::TacticalTrack> getAllTracks() const;

    /**
     * @brief Retrieves an individual tactical track by its identifier.
     * @param[in] trackId Unique track identifier.
     * @return std::optional containing TacticalTrack if found, or std::nullopt.
     */
    [[nodiscard]] std::optional<GISApp::Domain::Tracks::TacticalTrack> getTrackById(int trackId) const;

    /**
     * @brief Returns the total number of active tactical tracks.
     * @return Track count integer.
     */
    [[nodiscard]] int trackCount() const;

    /**
     * @brief Accesses the underlying ITrackRepository instance.
     * @return Pointer to ITrackRepository.
     */
    [[nodiscard]] GISApp::Repositories::ITrackRepository* repository() const { return m_trackRepo; }

    /**
     * @brief Deletes a track from the repository by its identifier.
     * @param[in] trackId Unique track ID to remove.
     * @return True if track was deleted, false otherwise.
     */
    bool deleteTrack(int trackId);

signals:
    /**
     * @brief Emitted whenever track collection data changes, delivering domain entities to observers.
     * @param[in] tracks Snapshot of all active tactical tracks.
     */
    void tracksUpdated(const QVector<GISApp::Domain::Tracks::TacticalTrack> &tracks);

    /**
     * @brief Emitted when an individual tactical track entity is inserted or updated.
     * @param[in] track Updated tactical track domain model.
     */
    void trackUpdated(const GISApp::Domain::Tracks::TacticalTrack &track);

    /**
     * @brief Emitted when an individual tactical track entity is removed.
     * @param[in] trackId Numerical identifier of removed track.
     */
    void trackRemoved(int trackId);

public slots:
    /**
     * @brief Slot triggered when ITrackRepository emits tracksUpdated.
     * Serializes all active tracks into GeoJSON and broadcasts signals.
     */
    void onRepositoryTracksUpdated();

private:
    /// Injected repository contract for persistent and cached tracks.
    GISApp::Repositories::ITrackRepository *m_trackRepo{nullptr};
};

} // namespace GISApp::Services::Tracks

#endif // TACTICALTRACKSERVICE_H
