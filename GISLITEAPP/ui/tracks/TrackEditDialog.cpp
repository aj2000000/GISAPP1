/**
 * @file TrackEditDialog.cpp
 * @brief Implementation of TrackEditDialog tactical track modifier.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "TrackEditDialog.h"
#include "ITrackRepository.h"
#include "fieldkeyvaluemapper.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>

namespace GISApp::UI::Tracks {

TrackEditDialog::TrackEditDialog(const GISApp::Domain::Tracks::TacticalTrack &track,
                                 GISApp::Repositories::ITrackRepository *repository,
                                 QWidget *parent)
    : QDialog(parent)
    , m_trackId(track.trackId())
    , m_track(track)
    , m_repo(repository)
{
    setWindowTitle(QStringLiteral("Edit Tactical Track - %1 [ID: %2]").arg(m_track.trackName()).arg(m_trackId));
    setMinimumSize(420, 480);
    resize(460, 520);

    setupUi();
}

TrackEditDialog::~TrackEditDialog() = default;

void TrackEditDialog::setupUi()
{
    setObjectName(QStringLiteral("TrackEditDialog"));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(12);

    // Header Label
    auto *titleLabel = new QLabel(QStringLiteral("Edit Parameters: %1 (ID: %2)")
                                      .arg(m_track.trackName().isEmpty() ? QStringLiteral("TRK-%1").arg(m_trackId) : m_track.trackName())
                                      .arg(m_trackId), this);
    titleLabel->setObjectName(QStringLiteral("TrackEditTitle"));
    mainLayout->addWidget(titleLabel);

    // 1. Identification & Classification Group
    auto *identGroup = new QGroupBox(QStringLiteral("IDENTIFICATION & CLASSIFICATION"), this);
    auto *identForm = new QFormLayout(identGroup);
    identForm->setContentsMargins(12, 12, 12, 12);
    identForm->setSpacing(8);

    m_nameEdit = new QLineEdit(m_track.trackName(), identGroup);
    identForm->addRow(QStringLiteral("Track Name:"), m_nameEdit);

    m_identityCombo = new QComboBox(identGroup);
    m_identityCombo->addItem(QStringLiteral("HOSTILE (MIL-STD Red)"), HOSTILE);
    m_identityCombo->addItem(QStringLiteral("FRIENDLY (MIL-STD Cyan)"), FRIENDLY);
    m_identityCombo->addItem(QStringLiteral("NEUTRAL (MIL-STD Green)"), 3);
    m_identityCombo->addItem(QStringLiteral("UNKNOWN (Amber)"), 0);
    int identIdx = m_identityCombo->findData(static_cast<int>(m_track.identity()));
    if (identIdx >= 0) m_identityCombo->setCurrentIndex(identIdx);
    identForm->addRow(QStringLiteral("Identity:"), m_identityCombo);

    m_domainTypeCombo = new QComboBox(identGroup);
    m_domainTypeCombo->addItem(QStringLiteral("AIR"), 1);
    m_domainTypeCombo->addItem(QStringLiteral("SURFACE"), 2);
    m_domainTypeCombo->addItem(QStringLiteral("SUBSURFACE"), 3);
    m_domainTypeCombo->addItem(QStringLiteral("LAND"), 4);
    int typeIdx = m_domainTypeCombo->findData(static_cast<int>(m_track.type()));
    if (typeIdx >= 0) m_domainTypeCombo->setCurrentIndex(typeIdx);
    identForm->addRow(QStringLiteral("Domain Type:"), m_domainTypeCombo);

    m_strengthSpin = new QSpinBox(identGroup);
    m_strengthSpin->setRange(1, 999);
    m_strengthSpin->setValue(m_track.strength());
    identForm->addRow(QStringLiteral("Target Strength:"), m_strengthSpin);

    mainLayout->addWidget(identGroup);

    // 2. Kinematics Group (Height & Direction)
    auto *kinGroup = new QGroupBox(QStringLiteral("KINEMATICS (STRUCT_LOCATION)"), this);
    auto *kinForm = new QFormLayout(kinGroup);
    kinForm->setContentsMargins(12, 12, 12, 12);
    kinForm->setSpacing(8);

    m_heightSpin = new QDoubleSpinBox(kinGroup);
    m_heightSpin->setRange(-1000.0, 60000.0);
    m_heightSpin->setDecimals(1);
    m_heightSpin->setSuffix(QStringLiteral(" m"));
    m_heightSpin->setValue(m_track.height());
    kinForm->addRow(QStringLiteral("Height (MSL):"), m_heightSpin);

    m_dirSpin = new QDoubleSpinBox(kinGroup);
    m_dirSpin->setRange(0.0, 359.9);
    m_dirSpin->setDecimals(1);
    m_dirSpin->setSuffix(QStringLiteral("°"));
    m_dirSpin->setValue(m_track.dir());
    kinForm->addRow(QStringLiteral("Direction / Bearing:"), m_dirSpin);

    mainLayout->addWidget(kinGroup);

    // 3. System & Remarks Group
    auto *sysGroup = new QGroupBox(QStringLiteral("SYSTEM DESIGNATION & REMARKS"), this);
    auto *sysForm = new QFormLayout(sysGroup);
    sysForm->setContentsMargins(12, 12, 12, 12);
    sysForm->setSpacing(8);

    m_sysTypeCombo = new QComboBox(sysGroup);
    m_sysTypeCombo->addItem(QStringLiteral("SYSTEM 1"), SYSTEM1);
    m_sysTypeCombo->addItem(QStringLiteral("SYSTEM 2 / FUSED"), SYSTEM2);
    int sysIdx = m_sysTypeCombo->findData(static_cast<int>(m_track.systemTrackType()));
    if (sysIdx >= 0) m_sysTypeCombo->setCurrentIndex(sysIdx);
    sysForm->addRow(QStringLiteral("System Type:"), m_sysTypeCombo);

    m_remarksEdit = new QLineEdit(m_track.remarks(), sysGroup);
    sysForm->addRow(QStringLiteral("Remarks:"), m_remarksEdit);

    mainLayout->addWidget(sysGroup);
    mainLayout->addStretch(1);

    // Action Buttons
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch(1);

    m_cancelButton = new QPushButton(QStringLiteral("Cancel"), this);
    m_cancelButton->setObjectName(QStringLiteral("TrackEditCancelBtn"));
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_cancelButton);

    m_saveButton = new QPushButton(QStringLiteral("Save Changes"), this);
    m_saveButton->setObjectName(QStringLiteral("TrackEditSaveBtn"));
    connect(m_saveButton, &QPushButton::clicked, this, &TrackEditDialog::onSaveClicked);
    btnLayout->addWidget(m_saveButton);

    mainLayout->addLayout(btnLayout);
}

void TrackEditDialog::onSaveClicked()
{
    if (!m_repo) {
        QMessageBox::warning(this, QStringLiteral("Error"), QStringLiteral("Database repository is not available."));
        return;
    }

    // Apply modifications to track domain model
    m_track.setTrackName(m_nameEdit->text().trimmed());
    m_track.setIdentity(static_cast<IDENTITY>(m_identityCombo->currentData().toUInt()));

    STRUCT_TRACK_ATTRIBUTES attr = m_track.attributes();
    attr.type = static_cast<UINT_8>(m_domainTypeCombo->currentData().toUInt());
    attr.strength = static_cast<UINT_8>(m_strengthSpin->value());
    m_track.setAttributes(attr);

    m_track.setHeight(m_heightSpin->value());
    m_track.setDir(m_dirSpin->value());
    m_track.setSystemTrackType(static_cast<SYSTEM_TRACK_TYPE>(m_sysTypeCombo->currentData().toUInt()));
    m_track.setRemarks(m_remarksEdit->text().trimmed());

    // Commit to persistent repository (updates SQLite, emits tracksUpdated, updates MapLibre GPU layer)
    m_repo->upsertTrack(m_track);

    accept();
}

} // namespace GISApp::UI::Tracks
