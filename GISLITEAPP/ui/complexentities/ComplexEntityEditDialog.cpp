/**
 * @file ComplexEntityEditDialog.cpp
 * @brief Implementation of the modeless tactical ComplexEntityEditDialog.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "ComplexEntityEditDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QScrollArea>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QIcon>
#include <cmath>

namespace GISApp::UI::ComplexEntities {

ComplexEntityEditDialog::ComplexEntityEditDialog(const GISApp::Domain::ComplexEntities::ComplexEntity &entity,
                                                 QWidget *parent)
    : QDialog(parent)
    , m_entityId(entity.id())
    , m_originalEntity(entity)
{
    setWindowTitle(tr("Edit Complex Entity — ID: %1").arg(m_entityId));
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_DeleteOnClose, false);
    resize(860, 780);

    setupUi();
    populateFields();
    updateAdaptiveSections(static_cast<int>(entity.entityType()));
}

ComplexEntityEditDialog::~ComplexEntityEditDialog()
{
    if (m_pickCoordBtn && m_pickCoordBtn->isChecked()) {
        emit pickCoordinateRequested(false);
    }
}

void ComplexEntityEditDialog::setupUi()
{
    // High-tech military tactical dark stylesheet
    setStyleSheet(QStringLiteral(
        "QDialog {"
        "  background-color: #0b111a;"
        "  color: #e2e8f0;"
        "  font-family: 'Segoe UI', 'SF Pro Text', Roboto, Arial, sans-serif;"
        "  font-size: 12px;"
        "}"
        "QGroupBox {"
        "  background-color: #101926;"
        "  border: 1px solid #1e293b;"
        "  border-radius: 8px;"
        "  margin-top: 14px;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "  color: #38bdf8;"
        "  padding-top: 14px;"
        "  padding-bottom: 10px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top left;"
        "  padding: 2px 10px;"
        "  background-color: #1e293b;"
        "  border-radius: 4px;"
        "  color: #38bdf8;"
        "}"
        "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {"
        "  background-color: #0f172a;"
        "  border: 1px solid #334155;"
        "  border-radius: 5px;"
        "  padding: 6px 10px;"
        "  color: #f8fafc;"
        "  font-size: 12px;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus {"
        "  border: 1px solid #38bdf8;"
        "  background-color: #1e293b;"
        "}"
        "QComboBox::drop-down { border: none; width: 22px; }"
        "QComboBox QAbstractItemView {"
        "  background-color: #0f172a;"
        "  color: #f8fafc;"
        "  border: 1px solid #38bdf8;"
        "  selection-background-color: #0284c7;"
        "}"
        "QTableWidget {"
        "  background-color: #090d14;"
        "  border: 1px solid #1e293b;"
        "  border-radius: 6px;"
        "  gridline-color: #1e293b;"
        "  color: #f8fafc;"
        "  font-size: 11.5px;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #0369a1;"
        "  color: #ffffff;"
        "}"
        "QHeaderView::section {"
        "  background-color: #162032;"
        "  color: #94a3b8;"
        "  border: none;"
        "  border-right: 1px solid #1e293b;"
        "  border-bottom: 2px solid #0284c7;"
        "  padding: 5px 8px;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "}"
        "QPushButton {"
        "  background-color: #1e293b;"
        "  color: #f8fafc;"
        "  border: 1px solid #334155;"
        "  border-radius: 5px;"
        "  padding: 6px 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #334155;"
        "  border-color: #38bdf8;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #0284c7;"
        "}"
        "QToolButton {"
        "  background-color: #1e293b;"
        "  color: #f8fafc;"
        "  border: 1px solid #334155;"
        "  border-radius: 5px;"
        "  padding: 6px 12px;"
        "  font-weight: bold;"
        "}"
        "QToolButton:checked {"
        "  background-color: #dc2626;"
        "  border-color: #ef4444;"
        "  color: #ffffff;"
        "}"
        "QToolButton:hover {"
        "  border-color: #38bdf8;"
        "}"
    ));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(14);

    // 1. Header Banner
    auto *headerLayout = new QHBoxLayout();
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: bold; color: #f8fafc;"));
    headerLayout->addWidget(m_titleLabel);

    headerLayout->addStretch();

    m_typeBadge = new QLabel(this);
    m_typeBadge->setStyleSheet(QStringLiteral(
        "padding: 4px 12px; border-radius: 4px; font-size: 11px; font-weight: bold;"
        "background-color: #0284c7; color: #ffffff;"
    ));
    headerLayout->addWidget(m_typeBadge);
    mainLayout->addLayout(headerLayout);

    // Scroll Area for Form Body
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(QStringLiteral("background: transparent;"));

    auto *contentWidget = new QWidget(scrollArea);
    contentWidget->setStyleSheet(QStringLiteral("background: transparent;"));
    auto *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 8, 0);
    contentLayout->setSpacing(14);

    // 2. Section: Identification & General Info
    auto *genGroup = new QGroupBox(tr("IDENTIFICATION & CLASSIFICATION"), contentWidget);
    auto *genForm = new QFormLayout(genGroup);
    genForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    genForm->setContentsMargins(16, 16, 16, 14);
    genForm->setSpacing(10);

    m_nameEdit = new QLineEdit(genGroup);
    m_nameEdit->setPlaceholderText(tr("Designated tactical callsign / name"));
    genForm->addRow(tr("Designated Name:"), m_nameEdit);

    m_typeCombo = new QComboBox(genGroup);
    m_typeCombo->addItem(tr("🟢 Point (Type 1)"), 1);
    m_typeCombo->addItem(tr("〰️ Line Polyline (Type 2)"), 2);
    m_typeCombo->addItem(tr("⬡ Polygon Region (Type 3)"), 3);
    m_typeCombo->addItem(tr("🔤 Text Only (Type 4)"), 4);
    m_typeCombo->addItem(tr("🖼️ Custom Image (Type 5)"), 5);
    m_typeCombo->addItem(tr("🎨 Custom Painter (Type 6)"), 6);
    m_typeCombo->addItem(tr("⚔️ Formation Boundary (Type 7)"), 7);
    m_typeCombo->addItem(tr("🛡️ Tactical Deployment Area (Type 8)"), 8);
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ComplexEntityEditDialog::onEntityTypeChanged);
    genForm->addRow(tr("Entity Type:"), m_typeCombo);

    m_remarksEdit = new QLineEdit(genGroup);
    m_remarksEdit->setPlaceholderText(tr("Operational remarks, mission notes, or clearance"));
    genForm->addRow(tr("Remarks:"), m_remarksEdit);

    contentLayout->addWidget(genGroup);

    // 3. Section: Spatial Annotations
    m_annotGroup = new QGroupBox(tr("SPATIAL ANNOTATIONS"), contentWidget);
    auto *annotLayout = new QGridLayout(m_annotGroup);
    annotLayout->setContentsMargins(16, 16, 16, 14);
    annotLayout->setSpacing(10);

    m_leftAnnotContainer = new QWidget(m_annotGroup);
    auto *lLayout = new QHBoxLayout(m_leftAnnotContainer);
    lLayout->setContentsMargins(0, 0, 0, 0);
    m_leftAnnotLabel = new QLabel(tr("Left Annotation:"), m_leftAnnotContainer);
    m_leftAnnotLabel->setFixedWidth(130);
    m_leftAnnotEdit = new QLineEdit(m_leftAnnotContainer);
    lLayout->addWidget(m_leftAnnotLabel);
    lLayout->addWidget(m_leftAnnotEdit);
    annotLayout->addWidget(m_leftAnnotContainer, 0, 0);

    m_rightAnnotContainer = new QWidget(m_annotGroup);
    auto *rLayout = new QHBoxLayout(m_rightAnnotContainer);
    rLayout->setContentsMargins(0, 0, 0, 0);
    m_rightAnnotLabel = new QLabel(tr("Right Annotation:"), m_rightAnnotContainer);
    m_rightAnnotLabel->setFixedWidth(130);
    m_rightAnnotEdit = new QLineEdit(m_rightAnnotContainer);
    rLayout->addWidget(m_rightAnnotLabel);
    rLayout->addWidget(m_rightAnnotEdit);
    annotLayout->addWidget(m_rightAnnotContainer, 0, 1);

    m_topAnnotContainer = new QWidget(m_annotGroup);
    auto *tLayout = new QHBoxLayout(m_topAnnotContainer);
    tLayout->setContentsMargins(0, 0, 0, 0);
    auto *tLabel = new QLabel(tr("Top Annotation:"), m_topAnnotContainer);
    tLabel->setFixedWidth(130);
    m_topAnnotEdit = new QLineEdit(m_topAnnotContainer);
    tLayout->addWidget(tLabel);
    tLayout->addWidget(m_topAnnotEdit);
    annotLayout->addWidget(m_topAnnotContainer, 1, 0);

    m_bottomAnnotContainer = new QWidget(m_annotGroup);
    auto *bLayout = new QHBoxLayout(m_bottomAnnotContainer);
    bLayout->setContentsMargins(0, 0, 0, 0);
    auto *bLabel = new QLabel(tr("Bottom Annotation:"), m_bottomAnnotContainer);
    bLabel->setFixedWidth(130);
    m_bottomAnnotEdit = new QLineEdit(m_bottomAnnotContainer);
    bLayout->addWidget(bLabel);
    bLayout->addWidget(m_bottomAnnotEdit);
    annotLayout->addWidget(m_bottomAnnotContainer, 1, 1);

    contentLayout->addWidget(m_annotGroup);

    // 4. Section: Parameters (Adaptive for Echelon, Line Style, Allegiance)
    m_paramGroup = new QGroupBox(tr("TACTICAL & OPERATIONAL PARAMETERS"), contentWidget);
    auto *paramLayout = new QGridLayout(m_paramGroup);
    paramLayout->setContentsMargins(16, 16, 16, 14);
    paramLayout->setSpacing(10);

    // SP1: Echelon or Generic
    m_sp1Container = new QWidget(m_paramGroup);
    auto *sp1L = new QHBoxLayout(m_sp1Container);
    sp1L->setContentsMargins(0, 0, 0, 0);
    m_sp1Label = new QLabel(tr("Echelon Symbology:"), m_sp1Container);
    m_sp1Label->setFixedWidth(130);
    m_echelonCombo = new QComboBox(m_sp1Container);
    m_echelonCombo->addItem(tr("Platoon (•••)"), 1);
    m_echelonCombo->addItem(tr("Company (I)"), 2);
    m_echelonCombo->addItem(tr("Battalion (II)"), 3);
    m_echelonCombo->addItem(tr("Brigade (X)"), 4);
    m_echelonCombo->addItem(tr("Division (XX)"), 5);
    m_echelonCombo->addItem(tr("Corps (XXX)"), 6);
    m_echelonCombo->addItem(tr("Army (XXXX)"), 7);
    m_sp1Spin = new QSpinBox(m_sp1Container);
    m_sp1Spin->setRange(0, 65535);
    sp1L->addWidget(m_sp1Label);
    sp1L->addWidget(m_echelonCombo);
    sp1L->addWidget(m_sp1Spin);
    paramLayout->addWidget(m_sp1Container, 0, 0);

    // SP2: Line Style or Generic
    m_sp2Container = new QWidget(m_paramGroup);
    auto *sp2L = new QHBoxLayout(m_sp2Container);
    sp2L->setContentsMargins(0, 0, 0, 0);
    m_sp2Label = new QLabel(tr("Line Style:"), m_sp2Container);
    m_sp2Label->setFixedWidth(130);
    m_lineStyleCombo = new QComboBox(m_sp2Container);
    m_lineStyleCombo->addItem(tr("Solid Line"), 1);
    m_lineStyleCombo->addItem(tr("Dashed Line"), 2);
    m_lineStyleCombo->addItem(tr("Dotted Line"), 3);
    m_sp2Spin = new QSpinBox(m_sp2Container);
    m_sp2Spin->setRange(0, 65535);
    sp2L->addWidget(m_sp2Label);
    sp2L->addWidget(m_lineStyleCombo);
    sp2L->addWidget(m_sp2Spin);
    paramLayout->addWidget(m_sp2Container, 0, 1);

    // SP3: Allegiance or Generic
    m_sp3Container = new QWidget(m_paramGroup);
    auto *sp3L = new QHBoxLayout(m_sp3Container);
    sp3L->setContentsMargins(0, 0, 0, 0);
    m_sp3Label = new QLabel(tr("Allegiance:"), m_sp3Container);
    m_sp3Label->setFixedWidth(130);
    m_allegianceCombo = new QComboBox(m_sp3Container);
    m_allegianceCombo->addItem(tr("Friendly Force (Gold)"), 1);
    m_allegianceCombo->addItem(tr("Hostile Force (Red)"), 2);
    m_sp3Spin = new QSpinBox(m_sp3Container);
    m_sp3Spin->setRange(0, 65535);
    sp3L->addWidget(m_sp3Label);
    sp3L->addWidget(m_allegianceCombo);
    sp3L->addWidget(m_sp3Spin);
    paramLayout->addWidget(m_sp3Container, 1, 0);

    // SP4: Generic
    m_sp4Container = new QWidget(m_paramGroup);
    auto *sp4L = new QHBoxLayout(m_sp4Container);
    sp4L->setContentsMargins(0, 0, 0, 0);
    m_sp4Label = new QLabel(tr("Special Param 4:"), m_sp4Container);
    m_sp4Label->setFixedWidth(130);
    m_sp4Spin = new QSpinBox(m_sp4Container);
    m_sp4Spin->setRange(0, 65535);
    sp4L->addWidget(m_sp4Label);
    sp4L->addWidget(m_sp4Spin);
    paramLayout->addWidget(m_sp4Container, 1, 1);

    contentLayout->addWidget(m_paramGroup);

    // 5. Section: Geodetic Location Points Table
    m_pointsGroup = new QGroupBox(tr("GEODETIC LOCATION POINTS (STRUCT_LOCATION)"), contentWidget);
    auto *ptsLayout = new QVBoxLayout(m_pointsGroup);
    ptsLayout->setContentsMargins(16, 16, 16, 14);
    ptsLayout->setSpacing(10);

    // Table
    m_pointsTable = new QTableWidget(m_pointsGroup);
    m_pointsTable->setColumnCount(5);
    m_pointsTable->setHorizontalHeaderLabels({
        tr("#"), tr("Latitude (°)"), tr("Longitude (°)"), tr("Altitude MSL (m)"), tr("Bearing (°)")
    });
    m_pointsTable->horizontalHeader()->setStretchLastSection(true);
    m_pointsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_pointsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_pointsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_pointsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_pointsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pointsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pointsTable->setMinimumHeight(200);
    ptsLayout->addWidget(m_pointsTable);

    // Table Toolbar
    auto *tblToolsLayout = new QHBoxLayout();
    m_addPointBtn = new QPushButton(tr("➕ Add Point"), m_pointsGroup);
    connect(m_addPointBtn, &QPushButton::clicked, this, &ComplexEntityEditDialog::onAddPointClicked);
    tblToolsLayout->addWidget(m_addPointBtn);

    m_removePointBtn = new QPushButton(tr("➖ Remove Point"), m_pointsGroup);
    connect(m_removePointBtn, &QPushButton::clicked, this, &ComplexEntityEditDialog::onRemovePointClicked);
    tblToolsLayout->addWidget(m_removePointBtn);

    m_moveUpBtn = new QPushButton(tr("🔼 Move Up"), m_pointsGroup);
    connect(m_moveUpBtn, &QPushButton::clicked, this, &ComplexEntityEditDialog::onMoveUpPointClicked);
    tblToolsLayout->addWidget(m_moveUpBtn);

    m_moveDownBtn = new QPushButton(tr("🔽 Move Down"), m_pointsGroup);
    connect(m_moveDownBtn, &QPushButton::clicked, this, &ComplexEntityEditDialog::onMoveDownPointClicked);
    tblToolsLayout->addWidget(m_moveDownBtn);

    tblToolsLayout->addStretch();

    m_pickCoordBtn = new QToolButton(m_pointsGroup);
    m_pickCoordBtn->setText(tr("📍 Pick from Map Canvas"));
    m_pickCoordBtn->setCheckable(true);
    connect(m_pickCoordBtn, &QToolButton::toggled, this, &ComplexEntityEditDialog::onPickModeToggled);
    tblToolsLayout->addWidget(m_pickCoordBtn);

    auto *moveRibbonBtn = new QPushButton(tr("🌐 Move on Ribbon"), m_pointsGroup);
    moveRibbonBtn->setToolTip(tr("Translate the entire entity on the map overlay preserving exact shape and geometry"));
    connect(moveRibbonBtn, &QPushButton::clicked, this, [this]() {
        emit moveOnOverlayRequested(m_entityId);
        accept();
    });
    tblToolsLayout->addWidget(moveRibbonBtn);

    auto *ctrlPtsRibbonBtn = new QPushButton(tr("📐 Control Points"), m_pointsGroup);
    ctrlPtsRibbonBtn->setToolTip(tr("Edit individual geometry control points and vertices on the map overlay"));
    connect(ctrlPtsRibbonBtn, &QPushButton::clicked, this, [this]() {
        emit editControlPointsRequested(m_entityId);
        accept();
    });
    tblToolsLayout->addWidget(ctrlPtsRibbonBtn);

    ptsLayout->addLayout(tblToolsLayout);
    contentLayout->addWidget(m_pointsGroup);

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea, 1);

    // 6. Action Footer
    auto *footerLayout = new QHBoxLayout();

    m_centerBtn = new QPushButton(tr("🎯 Center on Map"), this);
    connect(m_centerBtn, &QPushButton::clicked, this, &ComplexEntityEditDialog::onCenterClicked);
    footerLayout->addWidget(m_centerBtn);

    footerLayout->addStretch();

    m_cancelBtn = new QPushButton(tr("Cancel"), this);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    footerLayout->addWidget(m_cancelBtn);

    m_saveBtn = new QPushButton(tr("💾 Save Changes"), this);
    m_saveBtn->setStyleSheet(QStringLiteral(
        "background-color: #0284c7; color: #ffffff; border: 1px solid #38bdf8;"
        "padding: 8px 20px; font-size: 12.5px;"
    ));
    connect(m_saveBtn, &QPushButton::clicked, this, &ComplexEntityEditDialog::onSaveClicked);
    footerLayout->addWidget(m_saveBtn);

    mainLayout->addLayout(footerLayout);
}

void ComplexEntityEditDialog::populateFields()
{
    m_titleLabel->setText(tr("Editing: %1 (ID: %2)")
                          .arg(m_originalEntity.name().isEmpty() ? QStringLiteral("Complex Entity") : m_originalEntity.name())
                          .arg(m_entityId));

    m_nameEdit->setText(m_originalEntity.name());
    m_remarksEdit->setText(m_originalEntity.remarks());

    int typeIdx = m_typeCombo->findData(static_cast<int>(m_originalEntity.entityType()));
    if (typeIdx >= 0) {
        m_typeCombo->setCurrentIndex(typeIdx);
    }

    m_leftAnnotEdit->setText(m_originalEntity.leftAnnotation());
    m_rightAnnotEdit->setText(m_originalEntity.rightAnnotation());
    m_topAnnotEdit->setText(m_originalEntity.topAnnotation());
    m_bottomAnnotEdit->setText(m_originalEntity.bottomAnnotation());

    // Populate SP Spinboxes
    m_sp1Spin->setValue(m_originalEntity.specialParam1());
    m_sp2Spin->setValue(m_originalEntity.specialParam2());
    m_sp3Spin->setValue(m_originalEntity.specialParam3());
    m_sp4Spin->setValue(m_originalEntity.specialParam4());

    // Populate Combos for tactical echelons
    int echIdx = m_echelonCombo->findData(static_cast<int>(m_originalEntity.specialParam1()));
    m_echelonCombo->setCurrentIndex(echIdx >= 0 ? echIdx : 3); // default Brigade (X)

    int lineIdx = m_lineStyleCombo->findData(static_cast<int>(m_originalEntity.specialParam2()));
    m_lineStyleCombo->setCurrentIndex(lineIdx >= 0 ? lineIdx : 0); // default Solid

    int allegIdx = m_allegianceCombo->findData(static_cast<int>(m_originalEntity.specialParam3()));
    m_allegianceCombo->setCurrentIndex(allegIdx >= 0 ? allegIdx : 0); // default Friendly

    loadPointsTable(m_originalEntity.locationPoints());
}

void ComplexEntityEditDialog::updateAdaptiveSections(int entityType)
{
    m_typeBadge->setText(GISApp::Domain::ComplexEntities::ComplexEntity::entityTypeToString(static_cast<UINT_8>(entityType)));

    if (entityType == 7) {
        // Formation Boundary
        m_annotGroup->setVisible(true);
        m_annotGroup->setTitle(tr("FORMATION BOUNDARY FLANK UNITS"));
        m_leftAnnotContainer->setVisible(true);
        m_leftAnnotLabel->setText(tr("Top Flank Unit (Left):"));
        m_rightAnnotContainer->setVisible(true);
        m_rightAnnotLabel->setText(tr("Bottom Flank Unit (Right):"));
        m_topAnnotContainer->setVisible(false);
        m_bottomAnnotContainer->setVisible(false);

        m_paramGroup->setVisible(true);
        m_paramGroup->setTitle(tr("TACTICAL BOUNDARY PARAMETERS"));
        m_sp1Container->setVisible(true);
        m_sp1Label->setText(tr("Echelon Symbology:"));
        m_echelonCombo->setVisible(true);
        m_sp1Spin->setVisible(false);

        m_sp2Container->setVisible(true);
        m_sp2Label->setText(tr("Line Style:"));
        m_lineStyleCombo->setVisible(true);
        m_sp2Spin->setVisible(false);

        m_sp3Container->setVisible(true);
        m_sp3Label->setText(tr("Allegiance:"));
        m_allegianceCombo->setVisible(true);
        m_sp3Spin->setVisible(false);

        m_sp4Container->setVisible(false);
    } else if (entityType == 8) {
        // Tactical Deployment Area
        m_annotGroup->setVisible(false);

        m_paramGroup->setVisible(true);
        m_paramGroup->setTitle(tr("TACTICAL DEPLOYMENT PARAMETERS"));
        m_sp1Container->setVisible(true);
        m_sp1Label->setText(tr("Echelon Symbology:"));
        m_echelonCombo->setVisible(true);
        m_sp1Spin->setVisible(false);

        m_sp2Container->setVisible(true);
        m_sp2Label->setText(tr("Line Style:"));
        m_lineStyleCombo->setVisible(true);
        m_sp2Spin->setVisible(false);

        m_sp3Container->setVisible(false);
        m_sp4Container->setVisible(false);
    } else {
        // Generic Types 1-6
        m_annotGroup->setVisible(true);
        m_annotGroup->setTitle(tr("SPATIAL ANNOTATIONS (4-WAY)"));
        m_leftAnnotContainer->setVisible(true);
        m_leftAnnotLabel->setText(tr("Left Annotation:"));
        m_rightAnnotContainer->setVisible(true);
        m_rightAnnotLabel->setText(tr("Right Annotation:"));
        m_topAnnotContainer->setVisible(true);
        m_bottomAnnotContainer->setVisible(true);

        m_paramGroup->setVisible(true);
        m_paramGroup->setTitle(tr("SPECIAL OPERATIONAL PARAMETERS"));
        m_sp1Container->setVisible(true);
        m_sp1Label->setText(tr("Special Param 1:"));
        m_echelonCombo->setVisible(false);
        m_sp1Spin->setVisible(true);

        m_sp2Container->setVisible(true);
        m_sp2Label->setText(tr("Special Param 2:"));
        m_lineStyleCombo->setVisible(false);
        m_sp2Spin->setVisible(true);

        m_sp3Container->setVisible(true);
        m_sp3Label->setText(tr("Special Param 3:"));
        m_allegianceCombo->setVisible(false);
        m_sp3Spin->setVisible(true);

        m_sp4Container->setVisible(true);
        m_sp4Spin->setVisible(true);
    }
}

void ComplexEntityEditDialog::loadPointsTable(const QVector<STRUCT_LOCATION> &points)
{
    m_pointsTable->setRowCount(0);
    m_pointsTable->setRowCount(points.size());

    for (int i = 0; i < points.size(); ++i) {
        const auto &p = points[i];

        auto *idxItem = new QTableWidgetItem(QString::number(i + 1));
        idxItem->setFlags(idxItem->flags() & ~Qt::ItemIsEditable);
        idxItem->setTextAlignment(Qt::AlignCenter);

        auto *latItem = new QTableWidgetItem(QString::number(p.latatitude, 'f', 6));
        auto *lonItem = new QTableWidgetItem(QString::number(p.longitude, 'f', 6));
        auto *altItem = new QTableWidgetItem(QString::number(p.height, 'f', 1));
        auto *dirItem = new QTableWidgetItem(QString::number(p.dir, 'f', 1));

        latItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lonItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        altItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        dirItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        m_pointsTable->setItem(i, 0, idxItem);
        m_pointsTable->setItem(i, 1, latItem);
        m_pointsTable->setItem(i, 2, lonItem);
        m_pointsTable->setItem(i, 3, altItem);
        m_pointsTable->setItem(i, 4, dirItem);
    }
}

QVector<STRUCT_LOCATION> ComplexEntityEditDialog::collectPointsFromTable() const
{
    QVector<STRUCT_LOCATION> pts;
    pts.reserve(m_pointsTable->rowCount());

    for (int i = 0; i < m_pointsTable->rowCount(); ++i) {
        STRUCT_LOCATION loc{};
        auto *latItem = m_pointsTable->item(i, 1);
        auto *lonItem = m_pointsTable->item(i, 2);
        auto *altItem = m_pointsTable->item(i, 3);
        auto *dirItem = m_pointsTable->item(i, 4);

        loc.latatitude = latItem ? latItem->text().toDouble() : 0.0;
        loc.longitude = lonItem ? lonItem->text().toDouble() : 0.0;
        loc.height = altItem ? altItem->text().toDouble() : 0.0;
        loc.dir = dirItem ? dirItem->text().toDouble() : 0.0;

        pts.append(loc);
    }
    return pts;
}

void ComplexEntityEditDialog::updateEntityData(const GISApp::Domain::ComplexEntities::ComplexEntity &entity)
{
    m_originalEntity = entity;
    populateFields();
}

void ComplexEntityEditDialog::addPickedCoordinate(double latitude, double longitude)
{
    int row = m_pointsTable->rowCount();
    m_pointsTable->insertRow(row);

    auto *idxItem = new QTableWidgetItem(QString::number(row + 1));
    idxItem->setFlags(idxItem->flags() & ~Qt::ItemIsEditable);
    idxItem->setTextAlignment(Qt::AlignCenter);

    auto *latItem = new QTableWidgetItem(QString::number(latitude, 'f', 6));
    auto *lonItem = new QTableWidgetItem(QString::number(longitude, 'f', 6));
    auto *altItem = new QTableWidgetItem(QStringLiteral("850.0"));
    auto *dirItem = new QTableWidgetItem(QStringLiteral("0.0"));

    latItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lonItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    altItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    dirItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_pointsTable->setItem(row, 0, idxItem);
    m_pointsTable->setItem(row, 1, latItem);
    m_pointsTable->setItem(row, 2, lonItem);
    m_pointsTable->setItem(row, 3, altItem);
    m_pointsTable->setItem(row, 4, dirItem);

    m_pointsTable->selectRow(row);
}

bool ComplexEntityEditDialog::isPickModeActive() const
{
    return m_pickCoordBtn && m_pickCoordBtn->isChecked();
}

void ComplexEntityEditDialog::onMapCoordinateClicked(double lat, double lon)
{
    if (!isPickModeActive()) {
        return;
    }
    addPickedCoordinate(lat, lon);
    qDebug() << "[ComplexEntityEditDialog] Picked coordinate from map canvas:" << lat << lon;
}

void ComplexEntityEditDialog::onEntityTypeChanged(int index)
{
    int eType = m_typeCombo->itemData(index).toInt();
    updateAdaptiveSections(eType);
}

void ComplexEntityEditDialog::onAddPointClicked()
{
    double defLat = 26.50;
    double defLon = 74.20;

    int curRow = m_pointsTable->currentRow();
    if (curRow >= 0 && curRow < m_pointsTable->rowCount()) {
        auto *latIt = m_pointsTable->item(curRow, 1);
        auto *lonIt = m_pointsTable->item(curRow, 2);
        if (latIt && lonIt) {
            defLat = latIt->text().toDouble() + 0.01;
            defLon = lonIt->text().toDouble() + 0.01;
        }
    } else if (m_pointsTable->rowCount() > 0) {
        auto *latIt = m_pointsTable->item(m_pointsTable->rowCount() - 1, 1);
        auto *lonIt = m_pointsTable->item(m_pointsTable->rowCount() - 1, 2);
        if (latIt && lonIt) {
            defLat = latIt->text().toDouble() + 0.01;
            defLon = lonIt->text().toDouble() + 0.01;
        }
    }
    addPickedCoordinate(defLat, defLon);
}

void ComplexEntityEditDialog::onRemovePointClicked()
{
    int row = m_pointsTable->currentRow();
    if (row >= 0 && row < m_pointsTable->rowCount()) {
        m_pointsTable->removeRow(row);
        // Re-index remaining rows
        for (int i = 0; i < m_pointsTable->rowCount(); ++i) {
            auto *idxIt = m_pointsTable->item(i, 0);
            if (idxIt) idxIt->setText(QString::number(i + 1));
        }
    }
}

void ComplexEntityEditDialog::onMoveUpPointClicked()
{
    int row = m_pointsTable->currentRow();
    if (row <= 0) return;

    for (int col = 1; col < m_pointsTable->columnCount(); ++col) {
        auto *itemA = m_pointsTable->takeItem(row - 1, col);
        auto *itemB = m_pointsTable->takeItem(row, col);
        m_pointsTable->setItem(row - 1, col, itemB);
        m_pointsTable->setItem(row, col, itemA);
    }
    m_pointsTable->selectRow(row - 1);
}

void ComplexEntityEditDialog::onMoveDownPointClicked()
{
    int row = m_pointsTable->currentRow();
    if (row < 0 || row >= m_pointsTable->rowCount() - 1) return;

    for (int col = 1; col < m_pointsTable->columnCount(); ++col) {
        auto *itemA = m_pointsTable->takeItem(row, col);
        auto *itemB = m_pointsTable->takeItem(row + 1, col);
        m_pointsTable->setItem(row, col, itemB);
        m_pointsTable->setItem(row + 1, col, itemA);
    }
    m_pointsTable->selectRow(row + 1);
}

void ComplexEntityEditDialog::onPickModeToggled(bool checked)
{
    if (checked) {
        m_pickCoordBtn->setText(tr("📍 Click on Map... (Active)"));
    } else {
        m_pickCoordBtn->setText(tr("📍 Pick from Map Canvas"));
    }
    emit pickCoordinateRequested(checked);
}

void ComplexEntityEditDialog::onCenterClicked()
{
    auto pts = collectPointsFromTable();
    if (pts.isEmpty()) return;

    double cLat = 0.0, cLon = 0.0;
    for (const auto &p : pts) {
        cLat += p.latatitude;
        cLon += p.longitude;
    }
    cLat /= pts.size();
    cLon /= pts.size();

    emit centerRequested(cLat, cLon);
}

void ComplexEntityEditDialog::onSaveClicked()
{
    int entityType = m_typeCombo->currentData().toInt();
    auto pts = collectPointsFromTable();

    // Validation checks
    if (pts.isEmpty()) {
        QMessageBox::warning(this, tr("Missing Coordinates"),
                             tr("The entity must have at least one geodetic location point."));
        return;
    }

    if ((entityType == 2 || entityType == 7) && pts.size() < 2) {
        QMessageBox::warning(this, tr("Insufficient Points"),
                             tr("A Line or Formation Boundary requires at least 2 coordinate points."));
        return;
    }

    if ((entityType == 3 || entityType == 8) && pts.size() < 3) {
        QMessageBox::warning(this, tr("Insufficient Points"),
                             tr("A Polygon or Deployment Area requires at least 3 coordinate points to form an enclosed boundary."));
        return;
    }

    // Assemble modified entity
    GISApp::Domain::ComplexEntities::ComplexEntity updated(m_entityId, m_nameEdit->text().trimmed(), static_cast<UINT_8>(entityType));
    updated.setLocationPoints(pts);
    updated.setRemarks(m_remarksEdit->text().trimmed());
    updated.setReportTime(QDateTime::currentDateTimeUtc());

    if (entityType == 7) {
        // Formation Boundary
        updated.setLeftAnnotation(m_leftAnnotEdit->text().trimmed());
        updated.setRightAnnotation(m_rightAnnotEdit->text().trimmed());
        updated.setTopAnnotation(QString());
        updated.setBottomAnnotation(QString());
        updated.setSpecialParam1(static_cast<UINT_16>(m_echelonCombo->currentData().toInt()));
        updated.setSpecialParam2(static_cast<UINT_16>(m_lineStyleCombo->currentData().toInt()));
        updated.setSpecialParam3(static_cast<UINT_16>(m_allegianceCombo->currentData().toInt()));
        updated.setSpecialParam4(0);
        updated.setEntityDetails({});
    } else if (entityType == 8) {
        // Deployment Area
        updated.setLeftAnnotation(QString());
        updated.setRightAnnotation(QString());
        updated.setTopAnnotation(QString());
        updated.setBottomAnnotation(QString());
        updated.setSpecialParam1(static_cast<UINT_16>(m_echelonCombo->currentData().toInt()));
        updated.setSpecialParam2(static_cast<UINT_16>(m_lineStyleCombo->currentData().toInt()));
        updated.setSpecialParam3(0);
        updated.setSpecialParam4(0);
        updated.setEntityDetails({});
    } else {
        // Generic Types
        updated.setLeftAnnotation(m_leftAnnotEdit->text().trimmed());
        updated.setRightAnnotation(m_rightAnnotEdit->text().trimmed());
        updated.setTopAnnotation(m_topAnnotEdit->text().trimmed());
        updated.setBottomAnnotation(m_bottomAnnotEdit->text().trimmed());
        updated.setSpecialParam1(static_cast<UINT_16>(m_sp1Spin->value()));
        updated.setSpecialParam2(static_cast<UINT_16>(m_sp2Spin->value()));
        updated.setSpecialParam3(static_cast<UINT_16>(m_sp3Spin->value()));
        updated.setSpecialParam4(static_cast<UINT_16>(m_sp4Spin->value()));
        updated.setEntityDetails(m_originalEntity.details());
    }

    if (m_pickCoordBtn && m_pickCoordBtn->isChecked()) {
        emit pickCoordinateRequested(false);
    }

    emit entitySaved(updated);
    accept();
}

} // namespace GISApp::UI::ComplexEntities
