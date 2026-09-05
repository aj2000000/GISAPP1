/**
 * @file TrackRepository.cpp
 * @brief Thread-safe in-memory TrackRepository implementation.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#include "TrackRepository.h"
#include "DatabaseManager.h"
#include <QWriteLocker>
#include <QReadLocker>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QSqlDatabase>
#include <QDebug>

namespace GISApp::Repositories::Tracks {

TrackRepository::TrackRepository(QObject *parent)
    : ITrackRepository(parent)
{
    // Restore persisted tracks from SQLite database
    loadFromDatabase();

    // If database was empty (e.g. first application run), seed default tactical tracks
    if (m_tracks.isEmpty()) {

    }
}

void TrackRepository::upsertTrack(const Domain::Tracks::TacticalTrack &track)
{
    int id = track.trackId();
    {
        QWriteLocker locker(&m_lock);
        m_tracks.insert(id, track);
    }
    saveTrackToDatabase(track);
    emit trackUpserted(id);
    emit tracksUpdated();
}

void TrackRepository::upsertTracks(const QVector<Domain::Tracks::TacticalTrack> &tracks)
{
    if (tracks.isEmpty()) return;

    {
        QWriteLocker locker(&m_lock);
        for (const auto &track : tracks) {
            m_tracks.insert(track.trackId(), track);
        }
    }
    saveTracksToDatabase(tracks);
    emit tracksUpdated();
}

QVector<Domain::Tracks::TacticalTrack> TrackRepository::getAllTracks() const
{
    QReadLocker locker(&m_lock);
    return m_tracks.values().toVector();
}

std::optional<Domain::Tracks::TacticalTrack> TrackRepository::getTrackById(int trackId) const
{
    QReadLocker locker(&m_lock);
    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        return it.value();
    }
    return std::nullopt;
}

bool TrackRepository::removeTrack(int trackId)
{
    bool removed = false;
    {
        QWriteLocker locker(&m_lock);
        removed = (m_tracks.remove(trackId) > 0);
    }

    if (removed) {
        deleteTrackFromDatabase(trackId);
        emit trackRemoved(trackId);
        emit tracksUpdated();
    }
    return removed;
}

void TrackRepository::clearTracks()
{
    {
        QWriteLocker locker(&m_lock);
        m_tracks.clear();
    }
    clearDatabaseTracks();
    emit tracksUpdated();
}

int TrackRepository::count() const
{
    QReadLocker locker(&m_lock);
    return m_tracks.size();
}

void TrackRepository::loadFromDatabase()
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) {
        qDebug() << "[TrackRepository] Database not open, skipping loadFromDatabase";
        return;
    }

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral("SELECT track_id, callsign, latitude, longitude, altitude, heading, speed, "
                                   "identity, domain, symbol_code, remarks, report_time FROM tracks;"))) {
        qWarning() << "[TrackRepository] Failed to query tracks:" << query.lastError().text();
        return;
    }

    QWriteLocker locker(&m_lock);
    while (query.next()) {
        int id = query.value(0).toInt();
        QString callsign = query.value(1).toString();
        double lat = query.value(2).toDouble();
        double lon = query.value(3).toDouble();
        double alt = query.value(4).toDouble();
        double heading = query.value(5).toDouble();
        double speed = query.value(6).toDouble();
        int identityInt = query.value(7).toInt();
        int domainInt = query.value(8).toInt();
        QString symbol = query.value(9).toString();
        QString remarks = query.value(10).toString();
        QDateTime reportTime = query.value(11).toDateTime();

        Domain::Tracks::TacticalTrack trk(id, callsign, lat, lon, alt, heading);
        trk.setSpeed(speed);
        trk.setIdentity(static_cast<Domain::Tracks::TrackIdentity>(identityInt));
        trk.setDomain(static_cast<Domain::Tracks::TrackDomain>(domainInt));
        trk.setSymbolCode(symbol);
        trk.setRemarks(remarks);
        if (reportTime.isValid()) {
            trk.setReportTime(reportTime);
        }
        m_tracks.insert(id, trk);
    }

    qInfo() << "[TrackRepository] Restored" << m_tracks.size() << "tracks from SQLite database.";
}

void TrackRepository::saveTrackToDatabase(const Domain::Tracks::TacticalTrack &track)
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO tracks ("
        "  track_id, callsign, latitude, longitude, altitude, heading, speed, "
        "  identity, domain, symbol_code, remarks, report_time, updated_at"
        ") VALUES ("
        "  :track_id, :callsign, :latitude, :longitude, :altitude, :heading, :speed, "
        "  :identity, :domain, :symbol_code, :remarks, :report_time, CURRENT_TIMESTAMP"
        ") ON CONFLICT(track_id) DO UPDATE SET "
        "  callsign = excluded.callsign, "
        "  latitude = excluded.latitude, "
        "  longitude = excluded.longitude, "
        "  altitude = excluded.altitude, "
        "  heading = excluded.heading, "
        "  speed = excluded.speed, "
        "  identity = excluded.identity, "
        "  domain = excluded.domain, "
        "  symbol_code = excluded.symbol_code, "
        "  remarks = excluded.remarks, "
        "  report_time = excluded.report_time, "
        "  updated_at = CURRENT_TIMESTAMP;"
    ));

    query.bindValue(":track_id", track.trackId());
    query.bindValue(":callsign", track.callsign());
    query.bindValue(":latitude", track.latitude());
    query.bindValue(":longitude", track.longitude());
    query.bindValue(":altitude", track.altitude());
    query.bindValue(":heading", track.heading());
    query.bindValue(":speed", track.speed());
    query.bindValue(":identity", static_cast<int>(track.identity()));
    query.bindValue(":domain", static_cast<int>(track.domain()));
    query.bindValue(":symbol_code", track.symbolCode());
    query.bindValue(":remarks", track.remarks());
    query.bindValue(":report_time", track.reportTime().toString(Qt::ISODate));

    if (!query.exec()) {
        qWarning() << "[TrackRepository] Failed to upsert track" << track.trackId() << "into database:" << query.lastError().text();
    }
}

void TrackRepository::saveTracksToDatabase(const QVector<Domain::Tracks::TacticalTrack> &tracks)
{
    if (tracks.isEmpty()) return;

    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    db.transaction();
    for (const auto &track : tracks) {
        saveTrackToDatabase(track);
    }
    db.commit();
}

void TrackRepository::deleteTrackFromDatabase(int trackId)
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    query.prepare(QStringLiteral("DELETE FROM tracks WHERE track_id = :id;"));
    query.bindValue(":id", trackId);
    if (!query.exec()) {
        qWarning() << "[TrackRepository] Failed to delete track" << trackId << "from database:" << query.lastError().text();
    }
}

void TrackRepository::clearDatabaseTracks()
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral("DELETE FROM tracks;"))) {
        qWarning() << "[TrackRepository] Failed to clear tracks from database:" << query.lastError().text();
    }
}

} // namespace GISApp::Repositories::Tracks
