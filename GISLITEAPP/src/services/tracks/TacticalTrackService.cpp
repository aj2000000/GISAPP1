/**
 * @file TacticalTrackService.cpp
 * @brief Implementation of pure domain TacticalTrackService.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "TacticalTrackService.h"
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

        connect(m_trackRepo, &GISApp::Repositories::ITrackRepository::trackUpserted,
                this, [this](int trackId) {
                    auto trk = getTrackById(trackId);
                    if (trk.has_value()) {
                        emit trackUpdated(trk.value());
                    }
                });

        connect(m_trackRepo, &GISApp::Repositories::ITrackRepository::trackRemoved,
                this, &TacticalTrackService::trackRemoved);
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

bool TacticalTrackService::deleteTrack(int trackId)
{
    return m_trackRepo ? m_trackRepo->removeTrack(trackId) : false;
}

void TacticalTrackService::onRepositoryTracksUpdated()
{
    auto tracks = getAllTracks();
    qDebug() << "[TacticalTrackService] Domain tracks updated (" << tracks.size() << "tracks). Broadcasting tracksUpdated.";
    emit tracksUpdated(tracks);
}

} // namespace GISApp::Services::Tracks
