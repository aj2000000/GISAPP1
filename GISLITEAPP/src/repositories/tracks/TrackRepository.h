/**
 * @file TrackRepository.h
 * @brief Header definition for in-memory thread-safe TrackRepository.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef TRACKREPOSITORY_H
#define TRACKREPOSITORY_H

#include "ITrackRepository.h"
#include <QHash>
#include <QReadWriteLock>

namespace GISApp::Repositories::Tracks {

/**
 * @class TrackRepository
 * @brief High-performance, thread-safe in-memory tactical track repository.
 *
 * Implements ITrackRepository using a synchronized hash map (QReadWriteLock) to allow
 * concurrent read access by the GUI/Map rendering threads and write access by the UDP ingestion thread.
 */
class TrackRepository : public ITrackRepository
{
    Q_OBJECT

public:
    /**
     * @brief Constructs TrackRepository.
     * @param[in] parent Optional parent QObject.
     */
    explicit TrackRepository(QObject *parent = nullptr);
    virtual ~TrackRepository() override = default;

    void upsertTrack(const Domain::Tracks::TacticalTrack &track) override;
    void upsertTracks(const QVector<Domain::Tracks::TacticalTrack> &tracks) override;
    [[nodiscard]] QVector<Domain::Tracks::TacticalTrack> getAllTracks() const override;
    [[nodiscard]] std::optional<Domain::Tracks::TacticalTrack> getTrackById(int trackId) const override;
    bool removeTrack(int trackId) override;
    void clearTracks() override;
    [[nodiscard]] int count() const override;

private:
    /**
     * @brief Restores all persisted tactical tracks from SQLite database into memory cache.
     */
    void loadFromDatabase();

    /**
     * @brief Persists or updates a single tactical track entity in the SQLite database.
     * @param[in] track TacticalTrack domain entity.
     */
    void saveTrackToDatabase(const Domain::Tracks::TacticalTrack &track);

    /**
     * @brief Batch persists or updates a collection of tactical tracks in a single SQLite transaction.
     * @param[in] tracks Collection of TacticalTrack domain entities.
     */
    void saveTracksToDatabase(const QVector<Domain::Tracks::TacticalTrack> &tracks);

    /**
     * @brief Removes a track record from SQLite database by ID.
     * @param[in] trackId Numerical track ID.
     */
    void deleteTrackFromDatabase(int trackId);

    /**
     * @brief Purges all records from the SQLite tracks table.
     */
    void clearDatabaseTracks();

    mutable QReadWriteLock m_lock;
    QHash<int, Domain::Tracks::TacticalTrack> m_tracks;
};

} // namespace GISApp::Repositories::Tracks

#endif // TRACKREPOSITORY_H
