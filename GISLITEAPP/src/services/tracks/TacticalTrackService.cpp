/**
 * @file TacticalTrackService.cpp
 * @brief Implementation of pure domain TacticalTrackService.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#include "TacticalTrackService.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace GISApp::Services::Tracks {

TacticalTrackService::TacticalTrackService(GISApp::Repositories::ITrackRepository *trackRepo,
                                           QObject *parent)
    : QObject(parent)
    , m_trackRepo(trackRepo)
{
    if (m_trackRepo) {
        connect(m_trackRepo, &GISApp::Repositories::ITrackRepository::tracksUpdated,
                this, &TacticalTrackService::onRepositoryTracksUpdated);
    }
}

QVector<GISApp::Domain::Tracks::TacticalTrack> TacticalTrackService::getAllTracks() const
{
    return m_trackRepo ? m_trackRepo->getAllTracks() : QVector<GISApp::Domain::Tracks::TacticalTrack>{};
}

std::optional<GISApp::Domain::Tracks::TacticalTrack> TacticalTrackService::getTrackById(int trackId) const
{
    return m_trackRepo ? m_trackRepo->getTrackById(trackId) : std::nullopt;
}

int TacticalTrackService::trackCount() const
{
    return m_trackRepo ? m_trackRepo->count() : 0;
}

QByteArray TacticalTrackService::getTracksAsGeoJson() const
{
    QVector<GISApp::Domain::Tracks::TacticalTrack> tracks = getAllTracks();

    QJsonArray featuresArray;
    for (const auto &track : tracks) {
        featuresArray.append(track.toGeoJsonFeature());
    }

    QJsonObject featureCollection;
    featureCollection[QStringLiteral("type")] = QStringLiteral("FeatureCollection");
    featureCollection[QStringLiteral("features")] = featuresArray;

    QJsonDocument doc(featureCollection);
    return doc.toJson(QJsonDocument::Compact);
}

void TacticalTrackService::onRepositoryTracksUpdated()
{
    QByteArray geoJson = getTracksAsGeoJson();
    auto tracks = getAllTracks();

    qDebug() << "[TacticalTrackService] Domain tracks updated (" << tracks.size() << "tracks). Broadcasting GeoJSON.";
    emit geoJsonUpdated(geoJson);
    emit tracksUpdated(tracks);
}

} // namespace GISApp::Services::Tracks
