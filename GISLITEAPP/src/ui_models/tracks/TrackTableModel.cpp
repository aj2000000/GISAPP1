/**
 * @file TrackTableModel.cpp
 * @brief Implementation of TrackTableModel for tabular tactical track presentation.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#include "TrackTableModel.h"
#include "ITrackRepository.h"

#include <QFont>
#include <QDateTime>
#include <QDebug>

namespace GISApp::UIModels::Tracks {

TrackTableModel::TrackTableModel(GISApp::Repositories::ITrackRepository *repo, QObject *parent)
    : QAbstractTableModel(parent)
    , m_repo(nullptr)
{
    setTrackRepository(repo);
}

void TrackTableModel::setTrackRepository(GISApp::Repositories::ITrackRepository *repo)
{
    if (m_repo == repo) {
        return;
    }

    if (m_repo) {
        disconnect(m_repo, &GISApp::Repositories::ITrackRepository::tracksUpdated,
                   this, &TrackTableModel::onTracksUpdated);
    }

    m_repo = repo;
    setupRepositoryConnections();
    reloadTracks();
}

void TrackTableModel::setupRepositoryConnections()
{
    if (!m_repo) {
        return;
    }

    connect(m_repo, &GISApp::Repositories::ITrackRepository::tracksUpdated,
            this, &TrackTableModel::onTracksUpdated);
}

void TrackTableModel::reloadTracks()
{
    beginResetModel();
    if (m_repo) {
        m_tracks = m_repo->getAllTracks();
    } else {
        m_tracks.clear();
    }
    endResetModel();
}

void TrackTableModel::onTracksUpdated()
{
    reloadTracks();
}

GISApp::Domain::Tracks::TacticalTrack TrackTableModel::getTrackAt(int row) const
{
    if (row >= 0 && row < m_tracks.size()) {
        return m_tracks.at(row);
    }
    return GISApp::Domain::Tracks::TacticalTrack();
}

int TrackTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_tracks.size();
}

int TrackTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColumnCount;
}

QVariant TrackTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ColumnId:          return tr("ID");
        case ColumnCallsign:    return tr("Callsign");
        case ColumnAffiliation: return tr("Affiliation");
        case ColumnDomain:      return tr("Domain");
        case ColumnLatitude:    return tr("Latitude");
        case ColumnLongitude:   return tr("Longitude");
        case ColumnAltitude:    return tr("Altitude");
        case ColumnSpeed:       return tr("Speed");
        case ColumnHeading:     return tr("Heading");
        case ColumnReportTime:  return tr("Time (UTC)");
        case ColumnRemarks:     return tr("Remarks");
        default: break;
        }
    } else if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        return section + 1;
    } else if (role == Qt::TextAlignmentRole) {
        return QVariant(Qt::AlignCenter);
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant TrackTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tracks.size()) {
        return QVariant();
    }

    const auto &track = m_tracks.at(index.row());
    const int col = index.column();

    // Raw value extraction via custom user roles
    if (role == TrackIdRole) {
        return track.trackId();
    } else if (role == RawLatitudeRole) {
        return track.latitude();
    } else if (role == RawLongitudeRole) {
        return track.longitude();
    } else if (role == RawAltitudeRole) {
        return track.altitude();
    } else if (role == RawSpeedRole) {
        return track.speed();
    } else if (role == RawHeadingRole) {
        return track.heading();
    } else if (role == TrackIdentityRole) {
        return static_cast<int>(track.identity());
    }

    // Display string formatting
    if (role == Qt::DisplayRole) {
        switch (col) {
        case ColumnId:
            return track.trackId();
        case ColumnCallsign:
            return track.callsign().isEmpty() ? QStringLiteral("TRK-%1").arg(track.trackId()) : track.callsign();
        case ColumnAffiliation:
            return track.identityString();
        case ColumnDomain:
            return track.domainString();
        case ColumnLatitude:
            return QString::asprintf("%.5f°", track.latitude());
        case ColumnLongitude:
            return QString::asprintf("%.5f°", track.longitude());
        case ColumnAltitude:
            return QString::asprintf("%.0f m", track.altitude());
        case ColumnSpeed:
            return QString::asprintf("%.0f km/h", track.speed());
        case ColumnHeading:
            return QString::asprintf("%.0f°", track.heading());
        case ColumnReportTime:
            return track.reportTime().isValid() ? track.reportTime().toString(QStringLiteral("hh:mm:ss")) : QStringLiteral("--:--:--");
        case ColumnRemarks:
            return track.remarks();
        default:
            return QVariant();
        }
    }

    // Text alignment per column type
    if (role == Qt::TextAlignmentRole) {
        switch (col) {
        case ColumnId:
        case ColumnLatitude:
        case ColumnLongitude:
        case ColumnAltitude:
        case ColumnSpeed:
        case ColumnHeading:
        case ColumnReportTime:
            return QVariant(Qt::AlignCenter);
        case ColumnCallsign:
        case ColumnAffiliation:
        case ColumnDomain:
        case ColumnRemarks:
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    // Tactical font emphasis
    if (role == Qt::FontRole) {
        if (col == ColumnCallsign) {
            QFont font;
            font.setBold(true);
            return font;
        }
    }

    // Tactical MIL-STD-2525 text foreground colors
    if (role == Qt::ForegroundRole) {
        if (col == ColumnAffiliation || col == ColumnCallsign) {
            switch (track.identity()) {
            case GISApp::Domain::Tracks::TrackIdentity::Hostile:
                return QColor(QStringLiteral("#ff4d4f")); // Tactical Red
            case GISApp::Domain::Tracks::TrackIdentity::Friendly:
                return QColor(QStringLiteral("#40a9ff")); // Tactical Blue / Cyan
            case GISApp::Domain::Tracks::TrackIdentity::Neutral:
                return QColor(QStringLiteral("#73d13d")); // Tactical Emerald Green
            case GISApp::Domain::Tracks::TrackIdentity::Unknown:
            default:
                return QColor(QStringLiteral("#d9d9d9")); // Muted silver
            }
        }
    }

    return QVariant();
}

} // namespace GISApp::UIModels::Tracks
