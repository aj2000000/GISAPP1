/**
 * @file TrackRepository.cpp
 * @brief Thread-safe in-memory TrackRepository implementation.
 * @author GISLITE Development Team
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
    if (!query.exec(QStringLiteral("SELECT track_id, track_name, latitude, longitude, height, dir, "
                                   "track_identity, attr_type, attr_sub_type, attr_classification, attr_strength, "
                                   "attr_act_type, attr_act_sub_type, attr_act_classification, sys_track_type, "
                                   "symbol_code, remarks, report_time FROM tracks;"))) {
        qWarning() << "[TrackRepository] Failed to query tracks:" << query.lastError().text();
        return;
    }

    QWriteLocker locker(&m_lock);
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        double lat = query.value(2).toDouble();
        double lon = query.value(3).toDouble();
        double height = query.value(4).toDouble();
        double dir = query.value(5).toDouble();
        IDENTITY identity = static_cast<IDENTITY>(query.value(6).toUInt());

        STRUCT_TRACK_ATTRIBUTES attr;
        attr.type = static_cast<UINT_8>(query.value(7).toUInt());
        attr.sub_type = static_cast<UINT_8>(query.value(8).toUInt());
        attr.classification = static_cast<UINT_8>(query.value(9).toUInt());
        attr.strength = static_cast<UINT_8>(query.value(10).toUInt());
        attr.act_type = static_cast<UINT_8>(query.value(11).toUInt());
        attr.act_sub_type = static_cast<UINT_8>(query.value(12).toUInt());
        attr.act_classification = static_cast<UINT_8>(query.value(13).toUInt());

        SYSTEM_TRACK_TYPE sysType = static_cast<SYSTEM_TRACK_TYPE>(query.value(14).toUInt());
        QString symbol = query.value(15).toString();
        QString remarks = query.value(16).toString();
        QDateTime reportTime = query.value(17).toDateTime();

        Domain::Tracks::TacticalTrack trk(id, name, lat, lon, height, dir);
        trk.setIdentity(identity);
        trk.setAttributes(attr);
        trk.setSystemTrackType(sysType);
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
        "  track_id, track_name, latitude, longitude, height, dir, "
        "  track_identity, attr_type, attr_sub_type, attr_classification, attr_strength, "
        "  attr_act_type, attr_act_sub_type, attr_act_classification, sys_track_type, "
        "  symbol_code, remarks, report_time, updated_at"
        ") VALUES ("
        "  :track_id, :track_name, :latitude, :longitude, :height, :dir, "
        "  :track_identity, :attr_type, :attr_sub_type, :attr_classification, :attr_strength, "
        "  :attr_act_type, :attr_act_sub_type, :attr_act_classification, :sys_track_type, "
        "  :symbol_code, :remarks, :report_time, CURRENT_TIMESTAMP"
        ") ON CONFLICT(track_id) DO UPDATE SET "
        "  track_name = excluded.track_name, "
        "  latitude = excluded.latitude, "
        "  longitude = excluded.longitude, "
        "  height = excluded.height, "
        "  dir = excluded.dir, "
        "  track_identity = excluded.track_identity, "
        "  attr_type = excluded.attr_type, "
        "  attr_sub_type = excluded.attr_sub_type, "
        "  attr_classification = excluded.attr_classification, "
        "  attr_strength = excluded.attr_strength, "
        "  attr_act_type = excluded.attr_act_type, "
        "  attr_act_sub_type = excluded.attr_act_sub_type, "
        "  attr_act_classification = excluded.attr_act_classification, "
        "  sys_track_type = excluded.sys_track_type, "
        "  symbol_code = excluded.symbol_code, "
        "  remarks = excluded.remarks, "
        "  report_time = excluded.report_time, "
        "  updated_at = CURRENT_TIMESTAMP;"
    ));

    query.bindValue(":track_id", track.trackId());
    query.bindValue(":track_name", track.trackName());
    query.bindValue(":latitude", track.latatitude());
    query.bindValue(":longitude", track.longitude());
    query.bindValue(":height", track.height());
    query.bindValue(":dir", track.dir());
    query.bindValue(":track_identity", static_cast<int>(track.identity()));
    query.bindValue(":attr_type", static_cast<int>(track.type()));
    query.bindValue(":attr_sub_type", static_cast<int>(track.subType()));
    query.bindValue(":attr_classification", static_cast<int>(track.classification()));
    query.bindValue(":attr_strength", static_cast<int>(track.strength()));
    query.bindValue(":attr_act_type", static_cast<int>(track.actType()));
    query.bindValue(":attr_sub_type", static_cast<int>(track.actSubType()));
    query.bindValue(":attr_act_classification", static_cast<int>(track.actClassification()));
    query.bindValue(":sys_track_type", static_cast<int>(track.systemTrackType()));
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
