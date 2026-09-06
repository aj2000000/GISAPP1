/**
 * @file ComplexEntityLocationEditOverlay.cpp
 * @brief Implementation of ComplexEntityLocationEditOverlay floating tactical ribbon.
 *
 * Architectural Role & Implementation Details:
 * - Provides dual-mode in-place tactical editing directly over the map viewport:
 *   1. MoveEntireEntity (rigid translation, preserving 100% relative geometry).
 *   2. EditControlPoints (vertex-level selection, movement, addition, and deletion).
 * - Real-time telemetry, metric nudging, and responsive MapLibre GPU re-rendering.
 */

#include "ComplexEntityLocationEditOverlay.h"
#include <QGraphicsDropShadowEffect>
#include <QButtonGroup>
#include <QMessageBox>
#include <cmath>

namespace GISApp::UI::ComplexEntities {

static constexpr double METERS_PER_DEGREE_LAT = 111320.0;
static constexpr double PI = 3.14159265358979323846;

ComplexEntityLocationEditOverlay::ComplexEntityLocationEditOverlay(QWidget *parent)
    : QFrame(parent)
{
    setObjectName(QStringLiteral("ComplexEntityLocationEditOverlay"));
    setupUi();
    hide();
}

void ComplexEntityLocationEditOverlay::setupUi()
{
    setStyleSheet(QStringLiteral(
        "#ComplexEntityLocationEditOverlay {"
        "  background-color: rgba(11, 17, 26, 0.95);"
        "  border: 1.5px solid #0284c7;"
        "  border-radius: 10px;"
        "}"
        "QLabel {"
        "  color: #e2e8f0;"
        "  font-family: 'Segoe UI', 'Inter', sans-serif;"
        "  font-size: 11px;"
        "}"
        "QPushButton, QToolButton {"
        "  background-color: #1e293b;"
        "  color: #f1f5f9;"
        "  border: 1px solid #334155;"
        "  border-radius: 6px;"
        "  font-weight: 600;"
        "  font-size: 11px;"
        "  padding: 3px 8px;"
        "  min-height: 24px;"
        "}"
        "QPushButton:hover, QToolButton:hover {"
        "  background-color: #334155;"
        "  border-color: #38bdf8;"
        "}"
        "QPushButton:pressed, QToolButton:pressed {"
        "  background-color: #0f172a;"
        "}"
        "QToolButton:checked {"
        "  background-color: #0284c7;"
        "  color: #ffffff;"
        "  border-color: #38bdf8;"
        "}"
        "QComboBox {"
        "  background-color: #1e293b;"
        "  color: #f1f5f9;"
        "  border: 1px solid #334155;"
        "  border-radius: 6px;"
        "  padding: 2px 6px;"
        "  font-size: 11px;"
        "  min-height: 24px;"
        "}"
        "QComboBox::drop-down {"
        "  border: none;"
        "  width: 16px;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background-color: #0f172a;"
        "  color: #f1f5f9;"
        "  selection-background-color: #0284c7;"
        "  border: 1px solid #334155;"
        "}"
    ));

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 180));
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);

    // ═══════════════════════════════════════════════════════════════════
    // Root vertical layout: two rows stacked vertically
    // ═══════════════════════════════════════════════════════════════════
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 6, 12, 6);
    rootLayout->setSpacing(4);

    // ═══════════════════════════════════════════════════════════════════
    // ROW 1: Global Control Bar — Identity, Mode Selector, Action Buttons
    // ═══════════════════════════════════════════════════════════════════
    auto *row1Layout = new QHBoxLayout();
    row1Layout->setSpacing(8);
    row1Layout->setContentsMargins(0, 0, 0, 0);

    // 1a. Entity Title & Type Badge
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet(QStringLiteral("font-weight: 700; font-size: 12px; color: #38bdf8;"));
    row1Layout->addWidget(m_titleLabel);

    m_typeBadge = new QLabel(this);
    m_typeBadge->setStyleSheet(QStringLiteral(
        "background-color: rgba(2, 132, 199, 0.25);"
        "color: #38bdf8;"
        "border: 1px solid rgba(56, 189, 248, 0.4);"
        "border-radius: 9px;"
        "padding: 1px 7px;"
        "font-size: 10px;"
        "font-weight: 600;"
    ));
    row1Layout->addWidget(m_typeBadge);

    // Separator: identity | mode
    QFrame *sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::VLine);
    sep1->setStyleSheet(QStringLiteral("color: #334155;"));
    row1Layout->addWidget(sep1);

    // 1b. Mode Selector (Segmented Toggle)
    auto *modeGroup = new QButtonGroup(this);
    modeGroup->setExclusive(true);

    m_modeMoveBtn = new QToolButton(this);
    m_modeMoveBtn->setText(tr("🌐 Move Entity"));
    m_modeMoveBtn->setCheckable(true);
    m_modeMoveBtn->setChecked(true);
    m_modeMoveBtn->setToolTip(tr("Translate all points together preserving exact shape and geometry"));
    modeGroup->addButton(m_modeMoveBtn);
    connect(m_modeMoveBtn, &QToolButton::toggled, this, &ComplexEntityLocationEditOverlay::onModeMoveToggled);
    row1Layout->addWidget(m_modeMoveBtn);

    m_modePointsBtn = new QToolButton(this);
    m_modePointsBtn->setText(tr("📐 Control Points"));
    m_modePointsBtn->setCheckable(true);
    m_modePointsBtn->setToolTip(tr("Select and edit individual geometry control vertices"));
    modeGroup->addButton(m_modePointsBtn);
    connect(m_modePointsBtn, &QToolButton::toggled, this, &ComplexEntityLocationEditOverlay::onModePointsToggled);
    row1Layout->addWidget(m_modePointsBtn);

    // Elastic spacer pushes action buttons to the right edge
    row1Layout->addStretch(1);

    // 1c. Actions: Undo, Reset, Save, Cancel (always visible)
    m_undoBtn = new QPushButton(tr("↶ Undo"), this);
    m_undoBtn->setToolTip(tr("Undo the last editing operation"));
    m_undoBtn->setEnabled(false);
    connect(m_undoBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onUndoClicked);
    row1Layout->addWidget(m_undoBtn);

    m_resetBtn = new QPushButton(tr("↺ Reset"), this);
    m_resetBtn->setToolTip(tr("Revert to initial geometry"));
    connect(m_resetBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onResetClicked);
    row1Layout->addWidget(m_resetBtn);

    m_saveBtn = new QPushButton(tr("💾 Save"), this);
    m_saveBtn->setStyleSheet(QStringLiteral(
        "background-color: #0284c7; color: #ffffff; border: 1px solid #38bdf8;"
    ));
    m_saveBtn->setToolTip(tr("Commit edited locations/control points to database and map"));
    connect(m_saveBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onSaveClicked);
    row1Layout->addWidget(m_saveBtn);

    m_cancelBtn = new QPushButton(tr("✖ Cancel"), this);
    m_cancelBtn->setStyleSheet(QStringLiteral(
        "background-color: #334155; color: #cbd5e1; border: 1px solid #475569;"
    ));
    m_cancelBtn->setToolTip(tr("Discard changes and exit"));
    connect(m_cancelBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onCancelClicked);
    row1Layout->addWidget(m_cancelBtn);

    rootLayout->addLayout(row1Layout);

    // ─── Thin horizontal separator between Row 1 and Row 2 ───
    QFrame *rowSep = new QFrame(this);
    rowSep->setFrameShape(QFrame::HLine);
    rowSep->setStyleSheet(QStringLiteral("color: #1e293b;"));
    rowSep->setFixedHeight(1);
    rootLayout->addWidget(rowSep);

    // ═══════════════════════════════════════════════════════════════════
    // ROW 2: Mode-Specific Toolbar & Telemetry
    // ═══════════════════════════════════════════════════════════════════
    auto *row2Layout = new QHBoxLayout();
    row2Layout->setSpacing(6);
    row2Layout->setContentsMargins(0, 0, 0, 0);

    // 2a. Control Points Toolset (visible only in EditControlPoints mode)
    m_pointsWidgetContainer = new QWidget(this);
    auto *ptsToolsLayout = new QHBoxLayout(m_pointsWidgetContainer);
    ptsToolsLayout->setContentsMargins(0, 0, 0, 0);
    ptsToolsLayout->setSpacing(4);

    m_prevPtBtn = new QPushButton(QStringLiteral("◀"), m_pointsWidgetContainer);
    m_prevPtBtn->setToolTip(tr("Select previous control point"));
    m_prevPtBtn->setFixedWidth(24);
    connect(m_prevPtBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onPrevPointClicked);
    ptsToolsLayout->addWidget(m_prevPtBtn);

    m_pointCombo = new QComboBox(m_pointsWidgetContainer);
    m_pointCombo->setMinimumWidth(110);
    m_pointCombo->setToolTip(tr("Select active control vertex"));
    connect(m_pointCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ComplexEntityLocationEditOverlay::onPointComboChanged);
    ptsToolsLayout->addWidget(m_pointCombo);

    m_nextPtBtn = new QPushButton(QStringLiteral("▶"), m_pointsWidgetContainer);
    m_nextPtBtn->setToolTip(tr("Select next control point"));
    m_nextPtBtn->setFixedWidth(24);
    connect(m_nextPtBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onNextPointClicked);
    ptsToolsLayout->addWidget(m_nextPtBtn);

    m_addPtBtn = new QPushButton(QStringLiteral("➕"), m_pointsWidgetContainer);
    m_addPtBtn->setToolTip(tr("Insert new control point after active vertex"));
    m_addPtBtn->setFixedWidth(26);
    connect(m_addPtBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onAddPointClicked);
    ptsToolsLayout->addWidget(m_addPtBtn);

    m_removePtBtn = new QPushButton(QStringLiteral("➖"), m_pointsWidgetContainer);
    m_removePtBtn->setToolTip(tr("Delete active control point"));
    m_removePtBtn->setFixedWidth(26);
    connect(m_removePtBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onRemovePointClicked);
    ptsToolsLayout->addWidget(m_removePtBtn);

    row2Layout->addWidget(m_pointsWidgetContainer);
    m_pointsWidgetContainer->hide();

    // Separator: point tools | telemetry
    QFrame *sep2 = new QFrame(this);
    sep2->setFrameShape(QFrame::VLine);
    sep2->setStyleSheet(QStringLiteral("color: #334155;"));
    row2Layout->addWidget(sep2);

    // 2b. Telemetry / Coordinate Readout (shared, text changes per mode)
    auto *telemetryLayout = new QVBoxLayout();
    telemetryLayout->setSpacing(1);
    telemetryLayout->setContentsMargins(0, 0, 0, 0);

    m_anchorLabel = new QLabel(this);
    m_anchorLabel->setStyleSheet(QStringLiteral("font-size: 11px; color: #94a3b8; font-family: monospace;"));
    telemetryLayout->addWidget(m_anchorLabel);

    m_deltaLabel = new QLabel(this);
    m_deltaLabel->setStyleSheet(QStringLiteral("font-size: 11px; color: #10b981; font-weight: 600; font-family: monospace;"));
    telemetryLayout->addWidget(m_deltaLabel);

    row2Layout->addLayout(telemetryLayout);

    // Separator: telemetry | interaction tools
    QFrame *sep3 = new QFrame(this);
    sep3->setFrameShape(QFrame::VLine);
    sep3->setStyleSheet(QStringLiteral("color: #334155;"));
    row2Layout->addWidget(sep3);

    // 2c. Map Canvas Click Placement Toggle (disabled by default)
    m_mapClickBtn = new QToolButton(this);
    m_mapClickBtn->setText(tr("🎯 Click Map to Move"));
    m_mapClickBtn->setCheckable(true);
    m_mapClickBtn->setChecked(false);
    m_mapClickBtn->setToolTip(tr("Enable to click anywhere on map canvas to move target"));
    row2Layout->addWidget(m_mapClickBtn);

    // 2d. Metric Nudge Compass Controls
    m_stepCombo = new QComboBox(this);
    m_stepCombo->addItem(tr("10 m"), 10.0);
    m_stepCombo->addItem(tr("50 m"), 50.0);
    m_stepCombo->addItem(tr("100 m"), 100.0);
    m_stepCombo->addItem(tr("500 m"), 500.0);
    m_stepCombo->addItem(tr("1.0 km"), 1000.0);
    m_stepCombo->addItem(tr("5.0 km"), 5000.0);
    m_stepCombo->setCurrentIndex(2); // 100m default
    m_stepCombo->setToolTip(tr("Distance step applied per arrow click"));
    row2Layout->addWidget(m_stepCombo);

    m_northBtn = new QPushButton(tr("▲ N"), this);
    m_northBtn->setToolTip(tr("Shift North"));
    connect(m_northBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onNudgeNorth);
    row2Layout->addWidget(m_northBtn);

    m_southBtn = new QPushButton(tr("▼ S"), this);
    m_southBtn->setToolTip(tr("Shift South"));
    connect(m_southBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onNudgeSouth);
    row2Layout->addWidget(m_southBtn);

    m_westBtn = new QPushButton(tr("◄ W"), this);
    m_westBtn->setToolTip(tr("Shift West"));
    connect(m_westBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onNudgeWest);
    row2Layout->addWidget(m_westBtn);

    m_eastBtn = new QPushButton(tr("► E"), this);
    m_eastBtn->setToolTip(tr("Shift East"));
    connect(m_eastBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onNudgeEast);
    row2Layout->addWidget(m_eastBtn);

    // Separator: nudge compass | rotation
    QFrame *sep4 = new QFrame(this);
    sep4->setFrameShape(QFrame::VLine);
    sep4->setStyleSheet(QStringLiteral("color: #334155;"));
    row2Layout->addWidget(sep4);

    // 2e. Rotation Controls (CW / CCW around entity centroid)
    m_rotStepCombo = new QComboBox(this);
    m_rotStepCombo->addItem(tr("1°"), 1.0);
    m_rotStepCombo->addItem(tr("5°"), 5.0);
    m_rotStepCombo->addItem(tr("10°"), 10.0);
    m_rotStepCombo->addItem(tr("15°"), 15.0);
    m_rotStepCombo->addItem(tr("30°"), 30.0);
    m_rotStepCombo->addItem(tr("45°"), 45.0);
    m_rotStepCombo->addItem(tr("90°"), 90.0);
    m_rotStepCombo->setCurrentIndex(1); // 5° default
    m_rotStepCombo->setToolTip(tr("Rotation angle step in degrees"));
    row2Layout->addWidget(m_rotStepCombo);

    m_ccwBtn = new QPushButton(tr("↺ CCW"), this);
    m_ccwBtn->setToolTip(tr("Rotate counter-clockwise around entity centroid"));
    connect(m_ccwBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onRotateCCW);
    row2Layout->addWidget(m_ccwBtn);

    m_cwBtn = new QPushButton(tr("↻ CW"), this);
    m_cwBtn->setToolTip(tr("Rotate clockwise around entity centroid"));
    connect(m_cwBtn, &QPushButton::clicked, this, &ComplexEntityLocationEditOverlay::onRotateCW);
    row2Layout->addWidget(m_cwBtn);

    rootLayout->addLayout(row2Layout);
}

void ComplexEntityLocationEditOverlay::startEditing(const GISApp::Domain::ComplexEntities::ComplexEntity &entity,
                                                  EditMode mode)
{
    m_active = true;
    m_entityId = entity.Id();
    m_entityType = entity.entityType();
    m_entityName = entity.Name().isEmpty() ? QStringLiteral("CPLX-%1").arg(m_entityId) : entity.Name();
    m_currentPoints = entity.locationPoints();
    m_selectedPointIndex = 0;

    // Calculate anchor
    if (!m_currentPoints.isEmpty()) {
        double sLat = 0.0;
        double sLon = 0.0;
        for (const auto &p : m_currentPoints) {
            sLat += p.latatitude;
            sLon += p.longitude;
        }
        m_originalAnchorLat = sLat / m_currentPoints.size();
        m_originalAnchorLon = sLon / m_currentPoints.size();
    } else {
        m_originalAnchorLat = 0.0;
        m_originalAnchorLon = 0.0;
    }

    m_currentAnchorLat = m_originalAnchorLat;
    m_currentAnchorLon = m_originalAnchorLon;
    m_cumulativeDeltaLat = 0.0;
    m_cumulativeDeltaLon = 0.0;

    m_titleLabel->setText(QStringLiteral("📍 %1 (ID: %2)").arg(m_entityName).arg(m_entityId));
    m_typeBadge->setText(GISApp::Domain::ComplexEntities::ComplexEntity::entityTypeToString(m_entityType));
    m_mapClickBtn->setChecked(false);

    m_undoStack.clear();
    updateUndoBtnState();

    setEditMode(mode);
    rebuildPointCombo();
    updateTelemetryDisplay();

    show();
    raise();
    adjustSize();
}

void ComplexEntityLocationEditOverlay::stopEditing()
{
    m_active = false;
    m_entityId = 0;
    m_undoStack.clear();
    updateUndoBtnState();
    hide();
}

void ComplexEntityLocationEditOverlay::setEditMode(EditMode mode)
{
    m_editMode = mode;
    if (m_editMode == EditMode::MoveEntireEntity) {
        if (!m_modeMoveBtn->isChecked()) m_modeMoveBtn->setChecked(true);
        m_pointsWidgetContainer->hide();
        m_mapClickBtn->setText(tr("🎯 Click Map to Move"));
        m_mapClickBtn->setToolTip(tr("Click anywhere on map canvas to move the entire entity"));
    } else {
        if (!m_modePointsBtn->isChecked()) m_modePointsBtn->setChecked(true);
        m_pointsWidgetContainer->show();
        m_mapClickBtn->setText(tr("🎯 Click Map for Point"));
        m_mapClickBtn->setToolTip(tr("Click on map canvas to position the selected control point"));
    }

    updateTelemetryDisplay();
    adjustSize();
    emit editModeChanged(m_editMode);
}

void ComplexEntityLocationEditOverlay::onModeMoveToggled(bool checked)
{
    if (checked && m_editMode != EditMode::MoveEntireEntity) {
        setEditMode(EditMode::MoveEntireEntity);
    }
}

void ComplexEntityLocationEditOverlay::onModePointsToggled(bool checked)
{
    if (checked && m_editMode != EditMode::EditControlPoints) {
        setEditMode(EditMode::EditControlPoints);
    }
}

bool ComplexEntityLocationEditOverlay::isMapClickModeActive() const
{
    return m_active && m_mapClickBtn && m_mapClickBtn->isChecked();
}

void ComplexEntityLocationEditOverlay::setSelectedPointIndex(int index)
{
    if (index >= 0 && index < m_currentPoints.size()) {
        m_selectedPointIndex = index;
        if (m_pointCombo && m_pointCombo->currentIndex() != index) {
            m_pointCombo->blockSignals(true);
            m_pointCombo->setCurrentIndex(index);
            m_pointCombo->blockSignals(false);
        }
        updateTelemetryDisplay();
        emit controlPointSelected(m_selectedPointIndex);
    }
}

void ComplexEntityLocationEditOverlay::updateEntityData(const GISApp::Domain::ComplexEntities::ComplexEntity &entity)
{
    m_currentPoints = entity.locationPoints();
    if (m_selectedPointIndex >= m_currentPoints.size()) {
        m_selectedPointIndex = std::max(0, static_cast<int>(m_currentPoints.size() - 1));
    }
    rebuildPointCombo();
    updateTelemetryDisplay();
}

void ComplexEntityLocationEditOverlay::rebuildPointCombo()
{
    if (!m_pointCombo) return;

    m_pointCombo->blockSignals(true);
    m_pointCombo->clear();
    for (int i = 0; i < m_currentPoints.size(); ++i) {
        m_pointCombo->addItem(QStringLiteral("Pt %1: (%2°, %3°)")
            .arg(i + 1)
            .arg(m_currentPoints[i].latatitude, 0, 'f', 4)
            .arg(m_currentPoints[i].longitude, 0, 'f', 4),
            i);
    }
    if (m_selectedPointIndex >= 0 && m_selectedPointIndex < m_currentPoints.size()) {
        m_pointCombo->setCurrentIndex(m_selectedPointIndex);
    }
    m_pointCombo->blockSignals(false);
}

void ComplexEntityLocationEditOverlay::onPointComboChanged(int index)
{
    if (index >= 0 && index < m_currentPoints.size()) {
        m_selectedPointIndex = index;
        updateTelemetryDisplay();
        emit controlPointSelected(m_selectedPointIndex);
    }
}

void ComplexEntityLocationEditOverlay::onPrevPointClicked()
{
    if (m_currentPoints.isEmpty()) return;
    int nextIdx = (m_selectedPointIndex - 1 + m_currentPoints.size()) % m_currentPoints.size();
    setSelectedPointIndex(nextIdx);
}

void ComplexEntityLocationEditOverlay::onNextPointClicked()
{
    if (m_currentPoints.isEmpty()) return;
    int nextIdx = (m_selectedPointIndex + 1) % m_currentPoints.size();
    setSelectedPointIndex(nextIdx);
}

void ComplexEntityLocationEditOverlay::onAddPointClicked()
{
    if (!m_active || m_currentPoints.isEmpty()) return;

    saveUndoSnapshot();

    // Place new point slightly offset from current point
    double curLat = m_currentPoints[m_selectedPointIndex].latatitude;
    double curLon = m_currentPoints[m_selectedPointIndex].longitude;
    double newLat = curLat + 0.005;
    double newLon = curLon + 0.005;

    int afterIdx = m_selectedPointIndex;
    STRUCT_LOCATION newLoc{};
    newLoc.latatitude = newLat;
    newLoc.longitude = newLon;
    newLoc.height = m_currentPoints[m_selectedPointIndex].height;
    newLoc.dir = m_currentPoints[m_selectedPointIndex].dir;

    m_currentPoints.insert(afterIdx + 1, newLoc);
    m_selectedPointIndex = afterIdx + 1;
    rebuildPointCombo();
    updateTelemetryDisplay();

    emit controlPointAdded(m_entityId, afterIdx, newLat, newLon);
}

void ComplexEntityLocationEditOverlay::onRemovePointClicked()
{
    if (!m_active) return;

    int minRequired = (m_entityType == 3 || m_entityType == 8) ? 3 : 2;
    if (m_currentPoints.size() <= minRequired) {
        QMessageBox::warning(this, tr("Minimum Points Limit"),
                             tr("Cannot delete point. This entity type requires at least %1 control vertices.").arg(minRequired));
        return;
    }

    saveUndoSnapshot();
    int removedIdx = m_selectedPointIndex;
    m_currentPoints.removeAt(removedIdx);
    if (m_selectedPointIndex >= m_currentPoints.size()) {
        m_selectedPointIndex = m_currentPoints.size() - 1;
    }
    rebuildPointCombo();
    updateTelemetryDisplay();

    emit controlPointRemoved(m_entityId, removedIdx);
}

void ComplexEntityLocationEditOverlay::getCumulativeDelta(double &outDeltaLat, double &outDeltaLon) const
{
    outDeltaLat = m_cumulativeDeltaLat;
    outDeltaLon = m_cumulativeDeltaLon;
}

double ComplexEntityLocationEditOverlay::currentStepMeters() const
{
    return m_stepCombo ? m_stepCombo->currentData().toDouble() : 100.0;
}

void ComplexEntityLocationEditOverlay::handleMapCoordinateClicked(double clickedLat, double clickedLon)
{
    if (!m_active) return;

    if (m_editMode == EditMode::MoveEntireEntity) {
        // Move entire entity anchor to clicked coordinate
        double deltaLat = clickedLat - m_currentAnchorLat;
        double deltaLon = clickedLon - m_currentAnchorLon;
        applyIncrementalShift(deltaLat, deltaLon);
    } else {
        // Move selected control point to clicked coordinate
        if (m_selectedPointIndex >= 0 && m_selectedPointIndex < m_currentPoints.size()) {
            saveUndoSnapshot();
            m_currentPoints[m_selectedPointIndex].latatitude = clickedLat;
            m_currentPoints[m_selectedPointIndex].longitude = clickedLon;
            rebuildPointCombo();
            updateTelemetryDisplay();
            emit controlPointModified(m_entityId, m_selectedPointIndex, clickedLat, clickedLon);
        }
    }
}

void ComplexEntityLocationEditOverlay::applyIncrementalShift(double deltaLat, double deltaLon)
{
    saveUndoSnapshot();
    m_currentAnchorLat += deltaLat;
    m_currentAnchorLon += deltaLon;
    m_cumulativeDeltaLat += deltaLat;
    m_cumulativeDeltaLon += deltaLon;

    for (auto &p : m_currentPoints) {
        p.latatitude += deltaLat;
        p.longitude += deltaLon;
    }

    rebuildPointCombo();
    updateTelemetryDisplay();
    emit positionShifted(m_entityId, deltaLat, deltaLon);
}

void ComplexEntityLocationEditOverlay::onNudgeNorth()
{
    double step = currentStepMeters();
    double deltaLat = step / METERS_PER_DEGREE_LAT;

    if (m_editMode == EditMode::MoveEntireEntity) {
        applyIncrementalShift(deltaLat, 0.0);
    } else {
        if (m_selectedPointIndex >= 0 && m_selectedPointIndex < m_currentPoints.size()) {
            saveUndoSnapshot();
            m_currentPoints[m_selectedPointIndex].latatitude += deltaLat;
            rebuildPointCombo();
            updateTelemetryDisplay();
            emit controlPointModified(m_entityId, m_selectedPointIndex,
                                      m_currentPoints[m_selectedPointIndex].latatitude,
                                      m_currentPoints[m_selectedPointIndex].longitude);
        }
    }
}

void ComplexEntityLocationEditOverlay::onNudgeSouth()
{
    double step = currentStepMeters();
    double deltaLat = -step / METERS_PER_DEGREE_LAT;

    if (m_editMode == EditMode::MoveEntireEntity) {
        applyIncrementalShift(deltaLat, 0.0);
    } else {
        if (m_selectedPointIndex >= 0 && m_selectedPointIndex < m_currentPoints.size()) {
            saveUndoSnapshot();
            m_currentPoints[m_selectedPointIndex].latatitude += deltaLat;
            rebuildPointCombo();
            updateTelemetryDisplay();
            emit controlPointModified(m_entityId, m_selectedPointIndex,
                                      m_currentPoints[m_selectedPointIndex].latatitude,
                                      m_currentPoints[m_selectedPointIndex].longitude);
        }
    }
}

void ComplexEntityLocationEditOverlay::onNudgeEast()
{
    double step = currentStepMeters();
    double refLat = (m_editMode == EditMode::MoveEntireEntity)
        ? m_currentAnchorLat
        : (m_selectedPointIndex < m_currentPoints.size() ? m_currentPoints[m_selectedPointIndex].latatitude : 0.0);

    double latRad = refLat * (PI / 180.0);
    double cosLat = std::cos(latRad);
    if (std::abs(cosLat) < 0.001) cosLat = 0.001;

    double deltaLon = step / (METERS_PER_DEGREE_LAT * cosLat);

    if (m_editMode == EditMode::MoveEntireEntity) {
        applyIncrementalShift(0.0, deltaLon);
    } else {
        if (m_selectedPointIndex >= 0 && m_selectedPointIndex < m_currentPoints.size()) {
            saveUndoSnapshot();
            m_currentPoints[m_selectedPointIndex].longitude += deltaLon;
            rebuildPointCombo();
            updateTelemetryDisplay();
            emit controlPointModified(m_entityId, m_selectedPointIndex,
                                      m_currentPoints[m_selectedPointIndex].latatitude,
                                      m_currentPoints[m_selectedPointIndex].longitude);
        }
    }
}

void ComplexEntityLocationEditOverlay::onNudgeWest()
{
    double step = currentStepMeters();
    double refLat = (m_editMode == EditMode::MoveEntireEntity)
        ? m_currentAnchorLat
        : (m_selectedPointIndex < m_currentPoints.size() ? m_currentPoints[m_selectedPointIndex].latatitude : 0.0);

    double latRad = refLat * (PI / 180.0);
    double cosLat = std::cos(latRad);
    if (std::abs(cosLat) < 0.001) cosLat = 0.001;

    double deltaLon = -step / (METERS_PER_DEGREE_LAT * cosLat);

    if (m_editMode == EditMode::MoveEntireEntity) {
        applyIncrementalShift(0.0, deltaLon);
    } else {
        if (m_selectedPointIndex >= 0 && m_selectedPointIndex < m_currentPoints.size()) {
            saveUndoSnapshot();
            m_currentPoints[m_selectedPointIndex].longitude += deltaLon;
            rebuildPointCombo();
            updateTelemetryDisplay();
            emit controlPointModified(m_entityId, m_selectedPointIndex,
                                      m_currentPoints[m_selectedPointIndex].latatitude,
                                      m_currentPoints[m_selectedPointIndex].longitude);
        }
    }
}

void ComplexEntityLocationEditOverlay::onResetClicked()
{
    if (!m_active) return;

    if (m_editMode == EditMode::MoveEntireEntity) {
        if (std::abs(m_cumulativeDeltaLat) > 1e-9 || std::abs(m_cumulativeDeltaLon) > 1e-9) {
            emit positionShifted(m_entityId, -m_cumulativeDeltaLat, -m_cumulativeDeltaLon);
        }
        m_currentAnchorLat = m_originalAnchorLat;
        m_currentAnchorLon = m_originalAnchorLon;
        m_cumulativeDeltaLat = 0.0;
        m_cumulativeDeltaLon = 0.0;
    }

    m_undoStack.clear();
    updateUndoBtnState();
    emit resetRequested(m_entityId);
}

void ComplexEntityLocationEditOverlay::onSaveClicked()
{
    if (!m_active) return;
    quint32 id = m_entityId;
    stopEditing();
    emit saveRequested(id);
}

void ComplexEntityLocationEditOverlay::onCancelClicked()
{
    if (!m_active) return;
    quint32 id = m_entityId;
    stopEditing();
    emit cancelRequested(id);
}

void ComplexEntityLocationEditOverlay::updateTelemetryDisplay()
{
    if (m_editMode == EditMode::MoveEntireEntity) {
        char latHem = m_currentAnchorLat >= 0 ? 'N' : 'S';
        char lonHem = m_currentAnchorLon >= 0 ? 'E' : 'W';
        m_anchorLabel->setText(QStringLiteral("Anchor: %1° %2, %3° %4")
            .arg(std::abs(m_currentAnchorLat), 0, 'f', 6)
            .arg(latHem)
            .arg(std::abs(m_currentAnchorLon), 0, 'f', 6)
            .arg(lonHem));

        double latRad = m_currentAnchorLat * (PI / 180.0);
        double dLatMeters = m_cumulativeDeltaLat * METERS_PER_DEGREE_LAT;
        double dLonMeters = m_cumulativeDeltaLon * METERS_PER_DEGREE_LAT * std::cos(latRad);
        double distMeters = std::sqrt(dLatMeters * dLatMeters + dLonMeters * dLonMeters);

        QString distStr = (distMeters >= 1000.0)
            ? QStringLiteral("%1 km").arg(distMeters / 1000.0, 0, 'f', 2)
            : QStringLiteral("%1 m").arg(distMeters, 0, 'f', 0);

        QString latSign = m_cumulativeDeltaLat >= 0 ? QStringLiteral("+") : QString();
        QString lonSign = m_cumulativeDeltaLon >= 0 ? QStringLiteral("+") : QString();

        m_deltaLabel->setText(QStringLiteral("Δ: %1%2°, %3%4° | Shift: %5")
            .arg(latSign).arg(m_cumulativeDeltaLat, 0, 'f', 5)
            .arg(lonSign).arg(m_cumulativeDeltaLon, 0, 'f', 5)
            .arg(distStr));
    } else {
        // EditControlPoints mode readout
        if (m_selectedPointIndex >= 0 && m_selectedPointIndex < m_currentPoints.size()) {
            const auto &p = m_currentPoints[m_selectedPointIndex];
            char latHem = p.latatitude >= 0 ? 'N' : 'S';
            char lonHem = p.longitude >= 0 ? 'E' : 'W';
            m_anchorLabel->setText(QStringLiteral("Pt #%1: %2° %3, %4° %5")
                .arg(m_selectedPointIndex + 1)
                .arg(std::abs(p.latatitude), 0, 'f', 6)
                .arg(latHem)
                .arg(std::abs(p.longitude), 0, 'f', 6)
                .arg(lonHem));
        } else {
            m_anchorLabel->setText(tr("No vertex selected"));
        }

        m_deltaLabel->setText(QStringLiteral("Total Vertices: %1 | Geometry Mode")
            .arg(m_currentPoints.size()));
    }

    updateUndoBtnState();
}

void ComplexEntityLocationEditOverlay::saveUndoSnapshot()
{
    UndoSnapshot snap;
    snap.points = m_currentPoints;
    snap.selectedPointIndex = m_selectedPointIndex;
    snap.cumulativeDeltaLat = m_cumulativeDeltaLat;
    snap.cumulativeDeltaLon = m_cumulativeDeltaLon;
    snap.currentAnchorLat = m_currentAnchorLat;
    snap.currentAnchorLon = m_currentAnchorLon;

    if (m_undoStack.size() >= MAX_UNDO_DEPTH) {
        m_undoStack.removeFirst();
    }
    m_undoStack.append(snap);
    updateUndoBtnState();
}

void ComplexEntityLocationEditOverlay::onUndoClicked()
{
    if (!m_active || m_undoStack.isEmpty()) return;

    const UndoSnapshot snap = m_undoStack.takeLast();

    // Restore internal state from the snapshot
    m_currentPoints = snap.points;
    m_selectedPointIndex = snap.selectedPointIndex;
    m_cumulativeDeltaLat = snap.cumulativeDeltaLat;
    m_cumulativeDeltaLon = snap.cumulativeDeltaLon;
    m_currentAnchorLat = snap.currentAnchorLat;
    m_currentAnchorLon = snap.currentAnchorLon;

    rebuildPointCombo();
    updateTelemetryDisplay();

    // Emit full point set so the controller replaces all entity points atomically.
    // This correctly handles undo for translations, point edits, rotations, and add/remove.
    emit allPointsModified(m_entityId, m_currentPoints);
}

void ComplexEntityLocationEditOverlay::updateUndoBtnState()
{
    if (m_undoBtn) {
        m_undoBtn->setEnabled(!m_undoStack.isEmpty());
    }
}

double ComplexEntityLocationEditOverlay::currentRotationStepDegrees() const
{
    return m_rotStepCombo ? m_rotStepCombo->currentData().toDouble() : 5.0;
}

void ComplexEntityLocationEditOverlay::onRotateCW()
{
    applyRotation(currentRotationStepDegrees());
}

void ComplexEntityLocationEditOverlay::onRotateCCW()
{
    applyRotation(-currentRotationStepDegrees());
}

void ComplexEntityLocationEditOverlay::applyRotation(double angleDegrees)
{
    if (!m_active || m_currentPoints.size() < 2) return;

    saveUndoSnapshot();

    // Compute centroid of all control points
    double cLat = 0.0, cLon = 0.0;
    for (const auto &p : m_currentPoints) {
        cLat += p.latatitude;
        cLon += p.longitude;
    }
    cLat /= m_currentPoints.size();
    cLon /= m_currentPoints.size();

    double angleRad = angleDegrees * (PI / 180.0);
    double cosA = std::cos(angleRad);
    double sinA = std::sin(angleRad);

    // Longitude scaling factor at centroid latitude to ensure shape fidelity
    double centroidLatRad = cLat * (PI / 180.0);
    double cosLat = std::cos(centroidLatRad);
    if (std::abs(cosLat) < 0.001) cosLat = 0.001;

    for (auto &p : m_currentPoints) {
        // Convert geodetic offsets to local metric offsets (meters)
        double dY = (p.latatitude - cLat) * METERS_PER_DEGREE_LAT;
        double dX = (p.longitude - cLon) * METERS_PER_DEGREE_LAT * cosLat;

        // Apply 2D rotation matrix in meter-space
        double newDX = dX * cosA - dY * sinA;
        double newDY = dX * sinA + dY * cosA;

        // Convert back to geodetic degrees
        p.latatitude = cLat + newDY / METERS_PER_DEGREE_LAT;
        p.longitude  = cLon + newDX / (METERS_PER_DEGREE_LAT * cosLat);
    }

    rebuildPointCombo();
    updateTelemetryDisplay();
    emit allPointsModified(m_entityId, m_currentPoints);
}

} // namespace GISApp::UI::ComplexEntities
