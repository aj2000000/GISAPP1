/**
 * @file ITrackRepository.h
 * @brief Abstract repository interface contract for tactical track persistence and caching.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef ITRACKREPOSITORY_H
#define ITRACKREPOSITORY_H

#include <QObject>
#include <QVector>
#include <optional>
#include "TacticalTrack.h"

namespace GISApp::Repositories {

/**
 * @class ITrackRepository
 * @brief Abstract repository interface defining persistence and live cache contracts for tactical tracks.
 *
 * Implements the Repository pattern to decouple track ingestion (UDP handlers, simulated feeds)
 * from track consumers (MapLibre rendering engine, Track Table UI views).
 */
class ITrackRepository : public QObject
{
    Q_OBJECT

public:
    explicit ITrackRepository(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ITrackRepository() override = default;

    /**
     * @brief Inserts or updates an individual tactical track in the store.
     * @param[in] track TacticalTrack domain entity.
     */
    virtual void upsertTrack(const Domain::Tracks::TacticalTrack &track) = 0;

    /**
     * @brief Batch inserts or updates a collection of tactical tracks.
     * @param[in] tracks Vector of TacticalTrack domain entities.
     */
    virtual void upsertTracks(const QVector<Domain::Tracks::TacticalTrack> &tracks) = 0;

    /**
     * @brief Retrieves all active tactical tracks currently held in the repository.
     * @return QVector of TacticalTrack domain entities.
     */
    [[nodiscard]] virtual QVector<Domain::Tracks::TacticalTrack> getAllTracks() const = 0;

    /**
     * @brief Retrieves an individual track by its unique track identifier.
     * @param[in] trackId Numerical track ID.
     * @return std::optional containing TacticalTrack if found, std::nullopt otherwise.
     */
    [[nodiscard]] virtual std::optional<Domain::Tracks::TacticalTrack> getTrackById(int trackId) const = 0;

    /**
     * @brief Removes a track from the repository by ID.
     * @param[in] trackId Numerical track ID to remove.
     * @return True if track was found and removed, false otherwise.
     */
    virtual bool removeTrack(int trackId) = 0;

    /**
     * @brief Purges all tracks from the repository.
     */
    virtual void clearTracks() = 0;

    /**
     * @brief Returns current total active track count.
     * @return Integer count of active tracks.
     */
    [[nodiscard]] virtual int count() const = 0;

signals:
    /**
     * @brief Emitted whenever one or more tracks are inserted, updated, or purged.
     */
    void tracksUpdated();

    /**
     * @brief Emitted when an individual track is inserted or updated.
     * @param[in] trackId Numerical identifier of upserted track.
     */
    void trackUpserted(int trackId);

    /**
     * @brief Emitted when an individual track is removed.
     * @param[in] trackId Numerical identifier of removed track.
     */
    void trackRemoved(int trackId);
};

} // namespace GISApp::Repositories

#endif // ITRACKREPOSITORY_H
