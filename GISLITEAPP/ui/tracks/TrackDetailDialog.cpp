/**
 * @file TrackDetailDialog.cpp
 * @brief Implementation of TrackDetailDialog tactical inspector styled via ThemeManager.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "TrackDetailDialog.h"
#include "fieldkeyvaluemapper.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QDateTime>
#include <QIcon>
#include <QStyle>
#include <cmath>

namespace GISApp::UI::Tracks {

TrackDetailDialog::TrackDetailDialog(const GISApp::Domain::Tracks::TacticalTrack &track,
                                     QWidget *parent)
    : QDialog(parent)
    , m_trackId(track.trackId())
    , m_track(track)
{
    setObjectName(QStringLiteral("TrackDetailDialog"));
    setWindowTitle(QStringLiteral("Track Details - %1 [ID: %2]").arg(m_track.trackName()).arg(m_trackId));
    setMinimumSize(480, 560);
    resize(520, 620);

    setupUi();
    updateTrackData(m_track);
}

void TrackDetailDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(12);

    // 1. Header Banner
    auto *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("TrackDetailTitle"));
    headerLayout->addWidget(m_titleLabel, 1);

    m_identityBadge = new QLabel(this);
    m_identityBadge->setObjectName(QStringLiteral("TrackDetailIdentityBadge"));
    m_identityBadge->setAlignment(Qt::AlignCenter);
    m_identityBadge->setFixedHeight(26);
    headerLayout->addWidget(m_identityBadge);

    mainLayout->addLayout(headerLayout);

    // Scrollable container for properties
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *scrollContent = new QWidget(scrollArea);
    scrollContent->setObjectName(QStringLiteral("scrollContent"));
    auto *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(0, 0, 4, 0);
    contentLayout->setSpacing(12);

    // 2. Section: Identification & Symbology
    auto *identGroup = new QGroupBox(QStringLiteral("IDENTIFICATION && WIRE METRICS"), scrollContent);
    auto *identLayout = new QGridLayout(identGroup);
    identLayout->setContentsMargins(12, 12, 12, 12);
    identLayout->setVerticalSpacing(8);
    identLayout->setHorizontalSpacing(10);

    identLayout->addWidget(createPropertyRow(QStringLiteral("Track ID:"), &m_trackIdLabel), 0, 0);
    identLayout->addWidget(createPropertyRow(QStringLiteral("Designated Name:"), &m_callsignLabel), 0, 1);
    identLayout->addWidget(createPropertyRow(QStringLiteral("Symbol Code:"), &m_symbolLabel), 1, 0, 1, 2);
    contentLayout->addWidget(identGroup);

    // 3. Section: Geodetic Location & Kinematics (STRUCT_LOCATION)
    auto *locGroup = new QGroupBox(QStringLiteral("GEODETIC LOCATION && KINEMATICS (STRUCT_LOCATION)"), scrollContent);
    auto *locLayout = new QGridLayout(locGroup);
    locLayout->setContentsMargins(12, 12, 12, 12);
    locLayout->setVerticalSpacing(8);
    locLayout->setHorizontalSpacing(10);

    locLayout->addWidget(createPropertyRow(QStringLiteral("Latitude:"), &m_latLabel), 0, 0);
    locLayout->addWidget(createPropertyRow(QStringLiteral("Longitude:"), &m_lonLabel), 0, 1);
    locLayout->addWidget(createPropertyRow(QStringLiteral("Height (MSL):"), &m_heightLabel), 1, 0);
    locLayout->addWidget(createPropertyRow(QStringLiteral("Direction / Bearing:"), &m_dirLabel), 1, 1);
    contentLayout->addWidget(locGroup);

    // 4. Section: Tactical Attributes (STRUCT_TRACK_ATTRIBUTES)
    auto *attrGroup = new QGroupBox(QStringLiteral("TACTICAL ATTRIBUTES (STRUCT_TRACK_ATTRIBUTES)"), scrollContent);
    auto *attrLayout = new QGridLayout(attrGroup);
    attrLayout->setContentsMargins(12, 12, 12, 12);
    attrLayout->setVerticalSpacing(8);
    attrLayout->setHorizontalSpacing(10);

    attrLayout->addWidget(createPropertyRow(QStringLiteral("Domain Type:"), &m_domainTypeLabel), 0, 0);
    attrLayout->addWidget(createPropertyRow(QStringLiteral("Specific Subtype:"), &m_subTypeLabel), 0, 1);
    attrLayout->addWidget(createPropertyRow(QStringLiteral("Classification:"), &m_classificationLabel), 1, 0);
    attrLayout->addWidget(createPropertyRow(QStringLiteral("Target Strength:"), &m_strengthLabel), 1, 1);
    attrLayout->addWidget(createPropertyRow(QStringLiteral("Activity Type:"), &m_actTypeLabel), 2, 0);
    attrLayout->addWidget(createPropertyRow(QStringLiteral("Activity Subtype:"), &m_actSubTypeLabel), 2, 1);
    attrLayout->addWidget(createPropertyRow(QStringLiteral("Activity Classification:"), &m_actClassificationLabel), 3, 0, 1, 2);
    contentLayout->addWidget(attrGroup);

    // 5. Section: System Track Type & Sensor Sources
    auto *sysGroup = new QGroupBox(QStringLiteral("SYSTEM && SENSOR FUSION"), scrollContent);
    auto *sysLayout = new QGridLayout(sysGroup);
    sysLayout->setContentsMargins(12, 12, 12, 12);
    sysLayout->setVerticalSpacing(8);
    sysLayout->setHorizontalSpacing(10);

    sysLayout->addWidget(createPropertyRow(QStringLiteral("System Track Type:"), &m_sysTypeLabel), 0, 0);
    sysLayout->addWidget(createPropertyRow(QStringLiteral("Contributing Sources:"), &m_sourcesLabel), 0, 1);
    contentLayout->addWidget(sysGroup);

    // 6. Section: Temporal & Operations Remarks
    auto *remarksGroup = new QGroupBox(QStringLiteral("OPERATIONAL TIMING && REMARKS"), scrollContent);
    auto *remarksLayout = new QGridLayout(remarksGroup);
    remarksLayout->setContentsMargins(12, 12, 12, 12);
    remarksLayout->setVerticalSpacing(8);
    remarksLayout->setHorizontalSpacing(10);

    remarksLayout->addWidget(createPropertyRow(QStringLiteral("Report Time (UTC):"), &m_reportTimeLabel), 0, 0, 1, 2);
    remarksLayout->addWidget(createPropertyRow(QStringLiteral("Operational Remarks:"), &m_remarksLabel), 1, 0, 1, 2);
    contentLayout->addWidget(remarksGroup);

    contentLayout->addStretch(1);
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);

    // Footer
    auto *footerLayout = new QHBoxLayout();
    footerLayout->addStretch(1);

    m_closeButton = new QPushButton(QStringLiteral("Close"), this);
    m_closeButton->setObjectName(QStringLiteral("TrackDetailCloseBtn"));
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    footerLayout->addWidget(m_closeButton);

    mainLayout->addLayout(footerLayout);
}

QWidget* TrackDetailDialog::createPropertyRow(const QString &labelText, QLabel **valueLabelPtr)
{
    auto *container = new QWidget();
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(3);

    auto *caption = new QLabel(labelText, container);
    caption->setObjectName(QStringLiteral("TrackDetailCaption"));
    layout->addWidget(caption);

    auto *valLabel = new QLabel(container);
    valLabel->setObjectName(QStringLiteral("TrackDetailValue"));
    valLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(valLabel);

    if (valueLabelPtr) {
        *valueLabelPtr = valLabel;
    }
    return container;
}

void TrackDetailDialog::updateTrackData(const GISApp::Domain::Tracks::TacticalTrack &track)
{
    m_track = track;

    // Header & Identity Badge
    m_titleLabel->setText(QStringLiteral("%1 (ID: %2)").arg(track.trackName()).arg(track.trackId()));

    QString identityStr = FieldKeyValueMapper::instance().trackIdentityMapping(track.identity());
    m_identityBadge->setText(identityStr);

    QString identCode;
    switch (track.identity()) {
    case HOSTILE:   identCode = QStringLiteral("HOSTILE"); break;
    case FRIENDLY:  identCode = QStringLiteral("FRIENDLY"); break;
    case 3:         identCode = QStringLiteral("NEUTRAL"); break;
    default:        identCode = QStringLiteral("UNKNOWN"); break;
    }
    m_identityBadge->setProperty("identity", identCode);
    if (m_identityBadge->style()) {
        m_identityBadge->style()->unpolish(m_identityBadge);
        m_identityBadge->style()->polish(m_identityBadge);
    }

    // Identification
    m_trackIdLabel->setText(QString::number(track.trackId()));
    m_callsignLabel->setText(track.trackName().isEmpty() ? QStringLiteral("UNNAMED") : track.trackName());
    m_symbolLabel->setText(track.symbolCode().isEmpty() ? QStringLiteral("N/A") : track.symbolCode());

    // Kinematics (STRUCT_LOCATION)
    m_latLabel->setText(QStringLiteral("%1°").arg(QString::number(track.latatitude(), 'f', 6)));
    m_lonLabel->setText(QStringLiteral("%1°").arg(QString::number(track.longitude(), 'f', 6)));
    m_heightLabel->setText(QStringLiteral("%1 m").arg(QString::number(track.height(), 'f', 1)));
    m_dirLabel->setText(QStringLiteral("%1°").arg(QString::number(track.dir(), 'f', 1)));

    // Attributes (FieldKeyValueMapper)
    auto &mapper = FieldKeyValueMapper::instance();
    m_domainTypeLabel->setText(mapper.trackTypeMapping(track.type()));
    m_subTypeLabel->setText(mapper.trackSubTypeMapping(track.type(), track.subType()));
    m_classificationLabel->setText(mapper.trackClassificationMapping(track.type(), track.classification()));
    m_strengthLabel->setText(mapper.trackStrengthMapping(track.strength()));
    m_actTypeLabel->setText(mapper.trackActivityTypeMapping(track.type(), track.actType()));
    m_actSubTypeLabel->setText(mapper.trackActivitySubTypeMapping(track.type(), track.actSubType()));
    m_actClassificationLabel->setText(mapper.trackActivityClassificationMapping(track.type(), track.actClassification()));

    // System & Sources
    m_sysTypeLabel->setText(mapper.systemTrackTypeMapping(track.systemTrackType()));
    if (track.trackSources().isEmpty()) {
        m_sourcesLabel->setText(QStringLiteral("0 (Direct / Local Feed)"));
    } else {
        QStringList srcNames;
        for (const auto &src : track.trackSources()) {
            srcNames << mapper.trackSourceMapping(src.source);
        }
        m_sourcesLabel->setText(QStringLiteral("%1 (%2)").arg(track.trackSources().size()).arg(srcNames.join(QStringLiteral(", "))));
    }

    // Temporal & Remarks
    m_reportTimeLabel->setText(track.reportTime().isValid()
                                   ? track.reportTime().toUTC().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss 'UTC'"))
                                   : QStringLiteral("N/A"));
    m_remarksLabel->setText(track.remarks().isEmpty() ? QStringLiteral("None") : track.remarks());
}

} // namespace GISApp::UI::Tracks
