/**
 * @file TrackTableModel.cpp
 * @brief Implementation of TrackTableModel displaying canonical STRUCT_TRACK fields.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "TrackTableModel.h"
#include "fieldkeyvaluemapper.h"

namespace GISApp::UIModels::Tracks {

TrackTableModel::TrackTableModel(GISApp::Repositories::ITrackRepository *repo, QObject *parent)
    : QAbstractTableModel(parent)
    , m_repo(repo)
{
    setupRepositoryConnections();
    reloadTracks();
}

void TrackTableModel::setRepository(GISApp::Repositories::ITrackRepository *repo)
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
        case ColumnId:              return tr("Track ID");
        case ColumnName:            return tr("Track Name");
        case ColumnIdentity:        return tr("Identity");
        case ColumnType:            return tr("Domain Type");
        case ColumnSubType:         return tr("Subtype");
        case ColumnClassification:  return tr("Classification");
        case ColumnStrength:        return tr("Strength");
        case ColumnActivity:        return tr("Activity");
        case ColumnLatitude:        return tr("Latitude");
        case ColumnLongitude:       return tr("Longitude");
        case ColumnHeight:          return tr("Height (m)");
        case ColumnDirection:       return tr("Direction (°)");
        case ColumnSystemType:      return tr("System Type");
        case ColumnReportTime:      return tr("Time (UTC)");
        case ColumnRemarks:         return tr("Remarks");
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
    auto &mapper = FieldKeyValueMapper::instance();

    // Raw value extraction via custom user roles
    if (role == TrackIdRole) {
        return track.trackId();
    } else if (role == RawLatitudeRole) {
        return track.latatitude();
    } else if (role == RawLongitudeRole) {
        return track.longitude();
    } else if (role == RawHeightRole) {
        return track.height();
    } else if (role == RawDirectionRole) {
        return track.dir();
    } else if (role == TrackIdentityRole) {
        return static_cast<int>(track.identity());
    }

    // Display string formatting using FieldKeyValueMapper as source of truth
    if (role == Qt::DisplayRole) {
        switch (col) {
        case ColumnId:
            return track.trackId();
        case ColumnName:
            return track.trackName();
        case ColumnIdentity:
            return mapper.trackIdentityMapping(track.identity());
        case ColumnType:
            return mapper.trackTypeMapping(track.type());
        case ColumnSubType:
            return mapper.trackSubTypeMapping(track.type(), track.subType());
        case ColumnClassification:
            return mapper.trackClassificationMapping(track.type(), track.classification());
        case ColumnStrength:
            return mapper.trackStrengthMapping(track.strength());
        case ColumnActivity:
            return mapper.trackActivityTypeMapping(track.type(), track.actType());
        case ColumnLatitude:
            return QString::asprintf("%.5f°", track.latatitude());
        case ColumnLongitude:
            return QString::asprintf("%.5f°", track.longitude());
        case ColumnHeight:
            return QString::asprintf("%.0f m", track.height());
        case ColumnDirection:
            return QString::asprintf("%.0f°", track.dir());
        case ColumnSystemType:
            return mapper.systemTrackTypeMapping(track.systemTrackType());
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
        case ColumnHeight:
        case ColumnDirection:
        case ColumnStrength:
        case ColumnReportTime:
            return QVariant(Qt::AlignCenter);
        case ColumnName:
        case ColumnIdentity:
        case ColumnType:
        case ColumnSubType:
        case ColumnClassification:
        case ColumnActivity:
        case ColumnSystemType:
        case ColumnRemarks:
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    // Tactical font emphasis
    if (role == Qt::FontRole) {
        if (col == ColumnName) {
            QFont font;
            font.setBold(true);
            return font;
        }
    }

    // Tactical MIL-STD-2525 text foreground colors
    if (role == Qt::ForegroundRole) {
        if (col == ColumnIdentity || col == ColumnName) {
            switch (track.identity()) {
            case HOSTILE:
                return QColor(QStringLiteral("#ff4d4f")); // Tactical Red
            case FRIENDLY:
                return QColor(QStringLiteral("#40a9ff")); // Tactical Blue / Cyan
            case 3: // Neutral
                return QColor(QStringLiteral("#73d13d")); // Tactical Emerald Green
            default:
                return QColor(QStringLiteral("#ffd600")); // Tactical Amber / Unknown
            }
        }
    }

    return QVariant();
}

} // namespace GISApp::UIModels::Tracks
