/**
 * @file ComplexEntityDetailDialog.cpp
 * @brief Implementation of ComplexEntityDetailDialog tactical inspector.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "ComplexEntityDetailDialog.h"

#include <QHeaderView>
#include <QStyle>
#include <QDateTime>
#include <QGuiApplication>
#include <QClipboard>

namespace GISApp::UI::ComplexEntities {

/**
 * @brief Constructs ComplexEntityDetailDialog for a specific complex entity.
 * @param[in] entity Initial entity domain object.
 * @param[in] parent Optional parent widget.
 */
ComplexEntityDetailDialog::ComplexEntityDetailDialog(
    const GISApp::Domain::ComplexEntities::ComplexEntity &entity,
    QWidget *parent)
    : QDialog(parent)
    , m_entityId(entity.id())
    , m_entity(entity)
{
    setObjectName(QStringLiteral("ComplexEntityDetailDialog"));
    setWindowTitle(QStringLiteral("Complex Entity Details - %1 [ID: %2]")
                   .arg(m_entity.name().isEmpty() ? QStringLiteral("CPLX-%1").arg(m_entityId) : m_entity.name())
                   .arg(m_entityId));
    setMinimumSize(700, 720);
    resize(760, 840);

    setupUi();
    updateEntityData(m_entity);
}

/**
 * @brief Builds layout, property grids, dynamic tables, headers, and action buttons.
 */
void ComplexEntityDetailDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(12);

    // 1. Header Banner
    auto *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(12);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("ComplexEntityDetailTitle"));
    headerLayout->addWidget(m_titleLabel, 1);

    m_typeBadge = new QLabel(this);
    m_typeBadge->setObjectName(QStringLiteral("ComplexEntityDetailTypeBadge"));
    m_typeBadge->setAlignment(Qt::AlignCenter);
    m_typeBadge->setFixedHeight(28);
    headerLayout->addWidget(m_typeBadge);

    mainLayout->addLayout(headerLayout);

    // 2. Scrollable Area
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *scrollContent = new QWidget(scrollArea);
    scrollContent->setObjectName(QStringLiteral("scrollContent"));
    auto *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(0, 0, 6, 0);
    contentLayout->setSpacing(14);

    // 3. Identification & Telemetry Group
    auto *identGroup = new QGroupBox(tr("IDENTIFICATION && TELEMETRY"), scrollContent);
    identGroup->setObjectName(QStringLiteral("ComplexEntityDetailGroup"));
    auto *identLayout = new QGridLayout(identGroup);
    identLayout->setContentsMargins(14, 14, 14, 14);
    identLayout->setVerticalSpacing(8);
    identLayout->setHorizontalSpacing(14);

    identLayout->addWidget(createPropertyRow(tr("Entity ID:"), &m_idLabel), 0, 0);
    identLayout->addWidget(createPropertyRow(tr("Designated Name:"), &m_nameLabel), 0, 1);
    identLayout->addWidget(createPropertyRow(tr("Visual Classification:"), &m_typeDescLabel), 0, 2);

    identLayout->addWidget(createPropertyRow(tr("Primary Coordinate:"), &m_primaryLocLabel), 1, 0);
    identLayout->addWidget(createPropertyRow(tr("Geometric Centroid:"), &m_centroidLabel), 1, 1);
    identLayout->addWidget(createPropertyRow(tr("Altitude & Bearing:"), &m_primaryAltLabel), 1, 2);

    identLayout->addWidget(createPropertyRow(tr("Telemetry Timestamp:"), &m_updatedLabel), 2, 0);
    identLayout->addWidget(createPropertyRow(tr("Operational Remarks:"), &m_remarksLabel), 2, 1, 1, 2);

    contentLayout->addWidget(identGroup);

    // 4. 4-Way Spatial Annotations Group
    m_annotGroup = new QGroupBox(tr("SPATIAL ANNOTATIONS (4-WAY)"), scrollContent);
    m_annotGroup->setObjectName(QStringLiteral("ComplexEntityDetailGroup"));
    auto *annotLayout = new QGridLayout(m_annotGroup);
    annotLayout->setContentsMargins(14, 14, 14, 14);
    annotLayout->setVerticalSpacing(8);
    annotLayout->setHorizontalSpacing(14);

    m_topAnnotContainer = createPropertyRow(tr("Top Annotation (North / Tactical Area):"), &m_topAnnotationLabel);
    m_bottomAnnotContainer = createPropertyRow(tr("Bottom Annotation (South / Phase Line):"), &m_bottomAnnotationLabel);
    m_leftAnnotContainer = createPropertyRow(tr("Left Annotation (Top Flank Unit / 11 Inf Div):"), &m_leftAnnotationLabel);
    m_rightAnnotContainer = createPropertyRow(tr("Right Annotation (Bottom Flank Unit / 14 Inf Div):"), &m_rightAnnotationLabel);

    annotLayout->addWidget(m_topAnnotContainer, 0, 0);
    annotLayout->addWidget(m_bottomAnnotContainer, 0, 1);
    annotLayout->addWidget(m_leftAnnotContainer, 1, 0);
    annotLayout->addWidget(m_rightAnnotContainer, 1, 1);
    contentLayout->addWidget(m_annotGroup);

    // 5. Special Parameters Group
    m_paramGroup = new QGroupBox(tr("SPECIAL OPERATIONAL PARAMETERS"), scrollContent);
    m_paramGroup->setObjectName(QStringLiteral("ComplexEntityDetailGroup"));
    auto *paramLayout = new QGridLayout(m_paramGroup);
    paramLayout->setContentsMargins(14, 14, 14, 14);
    paramLayout->setVerticalSpacing(8);
    paramLayout->setHorizontalSpacing(14);

    m_param1Container = createPropertyRow(tr("Special Param 1 (Echelon Symbology):"), &m_param1Label);
    m_param2Container = createPropertyRow(tr("Special Param 2 (Line Style):"), &m_param2Label);
    m_param3Container = createPropertyRow(tr("Special Param 3 (Allegiance):"), &m_param3Label);
    m_param4Container = createPropertyRow(tr("Special Param 4:"), &m_param4Label);

    paramLayout->addWidget(m_param1Container, 0, 0);
    paramLayout->addWidget(m_param2Container, 0, 1);
    paramLayout->addWidget(m_param3Container, 0, 2);
    paramLayout->addWidget(m_param4Container, 0, 3);
    contentLayout->addWidget(m_paramGroup);

    // 6. Geodetic Location Points Table Group
    m_pointsGroup = new QGroupBox(tr("GEODETIC LOCATION POINTS (STRUCT_LOCATION)"), scrollContent);
    m_pointsGroup->setObjectName(QStringLiteral("ComplexEntityDetailGroup"));
    auto *pointsLayout = new QVBoxLayout(m_pointsGroup);
    pointsLayout->setContentsMargins(14, 14, 14, 14);
    pointsLayout->setSpacing(8);

    m_pointsTable = new QTableWidget(m_pointsGroup);
    m_pointsTable->setObjectName(QStringLiteral("ComplexEntityPointsTable"));
    m_pointsTable->setColumnCount(5);
    m_pointsTable->setHorizontalHeaderLabels({
        tr("#"),
        tr("Latitude"),
        tr("Longitude"),
        tr("Altitude (MSL)"),
        tr("Bearing / Dir")
    });
    m_pointsTable->verticalHeader()->setVisible(false);
    m_pointsTable->horizontalHeader()->setStretchLastSection(false);
    m_pointsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_pointsTable->setColumnWidth(0, 48);
    m_pointsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_pointsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_pointsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_pointsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_pointsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pointsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pointsTable->setAlternatingRowColors(true);
    pointsLayout->addWidget(m_pointsTable);
    contentLayout->addWidget(m_pointsGroup);

    // 7. Dynamic Entity Details Table Group
    m_detailsGroup = new QGroupBox(tr("DYNAMIC ATTRIBUTES (STRUCT_DETAILS)"), scrollContent);
    m_detailsGroup->setObjectName(QStringLiteral("ComplexEntityDetailGroup"));
    auto *detailsLayout = new QVBoxLayout(m_detailsGroup);
    detailsLayout->setContentsMargins(14, 14, 14, 14);
    detailsLayout->setSpacing(8);

    m_detailsTable = new QTableWidget(m_detailsGroup);
    m_detailsTable->setObjectName(QStringLiteral("ComplexEntityDetailsTable"));
    m_detailsTable->setColumnCount(3);
    m_detailsTable->setHorizontalHeaderLabels({
        tr("Attribute Key (valkey)"),
        tr("Data Type"),
        tr("Attribute Value (valStr)")
    });
    m_detailsTable->verticalHeader()->setVisible(false);
    m_detailsTable->horizontalHeader()->setStretchLastSection(false);
    m_detailsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_detailsTable->setColumnWidth(0, 220);
    m_detailsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_detailsTable->setColumnWidth(1, 120);
    m_detailsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_detailsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_detailsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_detailsTable->setAlternatingRowColors(true);
    detailsLayout->addWidget(m_detailsTable);
    contentLayout->addWidget(m_detailsGroup);

    contentLayout->addStretch(1);
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);

    // 8. Action Buttons
    auto *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    m_centerBtn = new QPushButton(tr("🎯 Center on Map"), this);
    m_centerBtn->setObjectName(QStringLiteral("ComplexEntityDetailCenterBtn"));
    connect(m_centerBtn, &QPushButton::clicked, this, &ComplexEntityDetailDialog::onCenterClicked);
    btnLayout->addWidget(m_centerBtn);

    m_copyBtn = new QPushButton(tr("📋 Copy Summary"), this);
    m_copyBtn->setObjectName(QStringLiteral("ComplexEntityDetailCopyBtn"));
    connect(m_copyBtn, &QPushButton::clicked, this, &ComplexEntityDetailDialog::onCopySummaryClicked);
    btnLayout->addWidget(m_copyBtn);

    auto *editBtn = new QPushButton(tr("✏️ Edit Entity"), this);
    editBtn->setObjectName(QStringLiteral("ComplexEntityDetailEditBtn"));
    editBtn->setToolTip(tr("Open interactive tactical editing overlay for this entity"));
    connect(editBtn, &QPushButton::clicked, this, &ComplexEntityDetailDialog::onEditClicked);
    btnLayout->addWidget(editBtn);

    btnLayout->addStretch(1);

    m_closeBtn = new QPushButton(tr("Close"), this);
    m_closeBtn->setObjectName(QStringLiteral("ComplexEntityDetailCloseBtn"));
    m_closeBtn->setFixedWidth(110);
    m_closeBtn->setDefault(true);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(btnLayout);
}

/**
 * @brief Helper to construct styled key-value label rows.
 * @param[in] labelText Descriptive title.
 * @param[out] valueLabelPtr Output pointer to dynamic value label.
 * @return Container widget.
 */
QWidget* ComplexEntityDetailDialog::createPropertyRow(const QString &labelText, QLabel **valueLabelPtr)
{
    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(3);

    auto *caption = new QLabel(labelText, container);
    caption->setObjectName(QStringLiteral("ComplexEntityDetailCaption"));

    auto *valLabel = new QLabel(QStringLiteral("—"), container);
    valLabel->setObjectName(QStringLiteral("ComplexEntityDetailValue"));
    valLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    layout->addWidget(caption);
    layout->addWidget(valLabel);

    *valueLabelPtr = valLabel;
    return container;
}

/**
 * @brief Adjusts table height dynamically based on its row count to eliminate empty vertical space.
 * @param[in,out] table Target table widget.
 * @param[in] minHeight Minimum permissible height in pixels.
 * @param[in] maxHeight Maximum permissible height in pixels before scrolling.
 */
void ComplexEntityDetailDialog::adjustTableHeight(QTableWidget *table, int minHeight, int maxHeight)
{
    if (!table) return;

    int rows = table->rowCount();
    int headerH = table->horizontalHeader()->height();
    if (headerH <= 0) headerH = 30;

    int rowH = 28;
    int calculated = headerH + (rows * rowH) + 6;
    table->setFixedHeight(qBound(minHeight, calculated, maxHeight));
}

/**
 * @brief Handles click on the 'Center on Map' action button.
 */
void ComplexEntityDetailDialog::onCenterClicked()
{
    double cLat = 0.0, cLon = 0.0;
    if (m_entity.type() == 7 || m_entity.type() == 2) {
        if (m_entity.calculateMidpoint(cLat, cLon)) {
            emit centerRequested(cLat, cLon);
            return;
        }
    }
    if (m_entity.calculateCentroid(cLat, cLon)) {
        emit centerRequested(cLat, cLon);
    } else {
        const auto &prim = m_entity.primaryLocation();
        emit centerRequested(prim.latatitude, prim.longitude);
    }
}

/**
 * @brief Handles click on the 'Copy Summary' action button.
 */
void ComplexEntityDetailDialog::onCopySummaryClicked()
{
    double cLat = 0.0, cLon = 0.0;
    if (m_entity.type() == 7 || m_entity.type() == 2) {
        if (!m_entity.calculateMidpoint(cLat, cLon)) {
            m_entity.calculateCentroid(cLat, cLon);
        }
    } else {
        m_entity.calculateCentroid(cLat, cLon);
    }
    const auto &prim = m_entity.primaryLocation();

    QString summary;
    if (m_entity.type() == 7) {
        summary = QStringLiteral(
            "=== FORMATION BOUNDARY SUMMARY ===\n"
            "ID: %1\n"
            "Name: %2\n"
            "Type: Formation Boundary (Code: 7)\n"
            "Polyline Midpoint (Echelon Anchor): %3°, %4°\n"
            "Location Points Count: %5\n"
            "Top Flank Unit (Left Annotation): %6\n"
            "Bottom Flank Unit (Right Annotation): %7\n"
            "Echelon Symbology (SP1): %8\n"
            "Line Style (SP2): %9\n"
            "Allegiance (SP3): %10\n"
        )
        .arg(m_entity.id())
        .arg(m_entity.name())
        .arg(cLat, 0, 'f', 6)
        .arg(cLon, 0, 'f', 6)
        .arg(m_entity.noOfLocationPoints())
        .arg(m_entity.leftAnnotation().isEmpty() ? QStringLiteral("—") : m_entity.leftAnnotation())
        .arg(m_entity.rightAnnotation().isEmpty() ? QStringLiteral("—") : m_entity.rightAnnotation())
        .arg(m_entity.specialParam1())
        .arg(m_entity.specialParam2() == 1 ? QStringLiteral("Solid Line") : QStringLiteral("Dashed/Dotted Line"))
        .arg(m_entity.specialParam3() == 2 ? QStringLiteral("Hostile") : QStringLiteral("Friendly"));
    } else if (m_entity.type() == 8) {
        summary = QStringLiteral(
            "=== TACTICAL DEPLOYMENT AREA SUMMARY ===\n"
            "ID: %1\n"
            "Name: %2\n"
            "Type: Deployment Area (Code: 8)\n"
            "Centroid: %3°, %4°\n"
            "Location Points Count: %5\n"
            "Echelon Symbology (SP1): %6\n"
            "Line Style (SP2): %7\n"
        )
        .arg(m_entity.id())
        .arg(m_entity.name())
        .arg(cLat, 0, 'f', 6)
        .arg(cLon, 0, 'f', 6)
        .arg(m_entity.noOfLocationPoints())
        .arg(m_entity.specialParam1())
        .arg(m_entity.specialParam2() == 1 ? QStringLiteral("Solid Line") : (m_entity.specialParam2() == 2 ? QStringLiteral("Dashed Line") : QStringLiteral("Dotted Line")));
    } else {
        summary = QStringLiteral(
            "=== COMPLEX ENTITY SUMMARY ===\n"
            "ID: %1\n"
            "Name: %2\n"
            "Type: %3 (Code: %4)\n"
            "Primary Location: %5°, %6°\n"
            "Centroid: %7°, %8°\n"
            "Altitude: %9 m | Direction: %10°\n"
            "Location Points Count: %11\n"
            "Top Annotation: %12\n"
            "Bottom Annotation: %13\n"
            "Left Annotation: %14\n"
            "Right Annotation: %15\n"
            "Special Params: [SP1=%16, SP2=%17, SP3=%18, SP4=%19]\n"
            "Dynamic Attributes Count: %20\n"
            "Remarks: %21\n"
        )
        .arg(m_entity.id())
        .arg(m_entity.name())
        .arg(GISApp::Domain::ComplexEntities::ComplexEntity::entityTypeToString(m_entity.type()))
        .arg(m_entity.type())
        .arg(prim.latatitude, 0, 'f', 6)
        .arg(prim.longitude, 0, 'f', 6)
        .arg(cLat, 0, 'f', 6)
        .arg(cLon, 0, 'f', 6)
        .arg(prim.height, 0, 'f', 1)
        .arg(prim.dir, 0, 'f', 1)
        .arg(m_entity.noOfLocationPoints())
        .arg(m_entity.topAnnotation().isEmpty() ? QStringLiteral("—") : m_entity.topAnnotation())
        .arg(m_entity.bottomAnnotation().isEmpty() ? QStringLiteral("—") : m_entity.bottomAnnotation())
        .arg(m_entity.leftAnnotation().isEmpty() ? QStringLiteral("—") : m_entity.leftAnnotation())
        .arg(m_entity.rightAnnotation().isEmpty() ? QStringLiteral("—") : m_entity.rightAnnotation())
        .arg(m_entity.specialParam1())
        .arg(m_entity.specialParam2())
        .arg(m_entity.specialParam3())
        .arg(m_entity.specialParam4())
        .arg(m_entity.noOfDetails())
        .arg(m_entity.remarks().isEmpty() ? QStringLiteral("—") : m_entity.remarks());
    }

    QGuiApplication::clipboard()->setText(summary);
}

/**
 * @brief Handles click on the 'Edit Entity' action button.
 */
void ComplexEntityDetailDialog::onEditClicked()
{
    emit editRequested(m_entityId);
}

/**
 * @brief Updates all visual fields to reflect fresh entity state in real-time.
 * @param[in] entity Updated complex entity domain model.
 */
void ComplexEntityDetailDialog::updateEntityData(const GISApp::Domain::ComplexEntities::ComplexEntity &entity)
{
    m_entity = entity;

    QString nameStr = entity.name().isEmpty() ? QStringLiteral("CPLX-%1").arg(entity.id()) : entity.name();
    m_titleLabel->setText(QStringLiteral("💠 %1").arg(nameStr));

    QString typeStr = GISApp::Domain::ComplexEntities::ComplexEntity::entityTypeToString(entity.type());

    m_typeBadge->setText(typeStr);
    m_typeBadge->setProperty("entityType", QString::number(entity.type()));
    if (m_typeBadge->style()) {
        m_typeBadge->style()->unpolish(m_typeBadge);
        m_typeBadge->style()->polish(m_typeBadge);
    }

    // 1. Identification & Telemetry
    m_idLabel->setText(QString::number(entity.id()));
    m_nameLabel->setText(nameStr);

    if (entity.type() == 7) {
        QString echelon;
        switch (entity.specialParam1()) {
        case 1: echelon = QStringLiteral("Platoon (•••)"); break;
        case 2: echelon = QStringLiteral("Company (I)"); break;
        case 3: echelon = QStringLiteral("Battalion (II)"); break;
        case 4: echelon = QStringLiteral("Brigade (X)"); break;
        case 5: echelon = QStringLiteral("Division (XX)"); break;
        case 6: echelon = QStringLiteral("Corps (XXX)"); break;
        case 7: echelon = QStringLiteral("Army (XXXX)"); break;
        default: echelon = QStringLiteral("Division (XX)"); break;
        }
        m_typeDescLabel->setText(QStringLiteral("Formation Boundary • %1").arg(echelon));
    } else if (entity.type() == 8) {
        QString echelon;
        switch (entity.specialParam1()) {
        case 1: echelon = QStringLiteral("Platoon (•••)"); break;
        case 2: echelon = QStringLiteral("Company (I)"); break;
        case 3: echelon = QStringLiteral("Battalion (II)"); break;
        case 4: echelon = QStringLiteral("Brigade (X)"); break;
        case 5: echelon = QStringLiteral("Division (XX)"); break;
        case 6: echelon = QStringLiteral("Corps (XXX)"); break;
        case 7: echelon = QStringLiteral("Army (XXXX)"); break;
        default: echelon = QStringLiteral("Brigade (X)"); break;
        }
        m_typeDescLabel->setText(QStringLiteral("Deployment Area • %1").arg(echelon));
    } else {
        m_typeDescLabel->setText(QStringLiteral("%1 (Code: %2)").arg(typeStr).arg(entity.type()));
    }

    m_remarksLabel->setText(entity.remarks().isEmpty() ? QStringLiteral("Standard Tactical Tracking") : entity.remarks());

    const auto &pts = entity.locationPoints();
    const auto &details = entity.details();
    const auto &prim = entity.primaryLocation();
    m_primaryLocLabel->setText(QString::asprintf("%.6f° N, %.6f° E", prim.latatitude, prim.longitude));

    double cLat = 0.0, cLon = 0.0;
    if (entity.type() == 7 || entity.type() == 2) {
        if (entity.calculateMidpoint(cLat, cLon)) {
            m_centroidLabel->setText(QString::asprintf("%.6f° N, %.6f° E (Midpoint)", cLat, cLon));
        } else {
            m_centroidLabel->setText(m_primaryLocLabel->text());
        }
    } else {
        if (entity.calculateCentroid(cLat, cLon)) {
            m_centroidLabel->setText(QString::asprintf("%.6f° N, %.6f° E", cLat, cLon));
        } else {
            m_centroidLabel->setText(m_primaryLocLabel->text());
        }
    }

    m_primaryAltLabel->setText(QString::asprintf("%.1f m (%.0f ft) | %.1f°", prim.height, prim.height * 3.28084, prim.dir));

    if (m_pointsGroup) {
        m_pointsGroup->setTitle(tr("GEODETIC LOCATION POINTS (STRUCT_LOCATION) [%1 Points]").arg(pts.size()));
    }
    if (m_detailsGroup) {
        m_detailsGroup->setTitle(tr("DYNAMIC ATTRIBUTES (STRUCT_DETAILS) [%1 Attributes]").arg(details.size()));
    }

    m_updatedLabel->setText(entity.lastUpdated().isValid()
                            ? entity.lastUpdated().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"))
                            : QStringLiteral("Active Live Record"));

    // 2. Annotations and Special Parameters
    if (entity.type() == 7) {
        // Formation Boundary: strictly only left_annotation, right_annotation, SP1, SP2, SP3, and location points!
        if (m_topAnnotContainer) m_topAnnotContainer->setVisible(false);
        if (m_bottomAnnotContainer) m_bottomAnnotContainer->setVisible(false);
        if (m_param4Container) m_param4Container->setVisible(false);

        if (m_annotGroup) m_annotGroup->setTitle(tr("FORMATION BOUNDARY FLANK UNITS (11 Inf Div / 14 Inf Div)"));
        if (m_paramGroup) m_paramGroup->setTitle(tr("TACTICAL BOUNDARY PARAMETERS"));

        // Left & Right Flank units (Top / Bottom of boundary line)
        m_leftAnnotationLabel->setText(entity.leftAnnotation().isEmpty() ? QStringLiteral("—") : entity.leftAnnotation());
        m_rightAnnotationLabel->setText(entity.rightAnnotation().isEmpty() ? QStringLiteral("—") : entity.rightAnnotation());

        // SP1: Echelon Symbology
        QString echStr;
        switch (entity.specialParam1()) {
        case 1: echStr = QStringLiteral("Platoon (•••)"); break;
        case 2: echStr = QStringLiteral("Company (I)"); break;
        case 3: echStr = QStringLiteral("Battalion (II)"); break;
        case 4: echStr = QStringLiteral("Brigade (X)"); break;
        case 5: echStr = QStringLiteral("Division (XX)"); break;
        case 6: echStr = QStringLiteral("Corps (XXX)"); break;
        case 7: echStr = QStringLiteral("Army (XXXX)"); break;
        default: echStr = QStringLiteral("Division (XX)"); break;
        }
        m_param1Label->setText(QStringLiteral("%1 (%2)").arg(entity.specialParam1()).arg(echStr));

        // SP2: Line Style
        QString lineStyleStr;
        switch (entity.specialParam2()) {
        case 1: lineStyleStr = QStringLiteral("Solid Line"); break;
        case 2: lineStyleStr = QStringLiteral("Dashed Line"); break;
        case 3: lineStyleStr = QStringLiteral("Dotted Line"); break;
        default: lineStyleStr = QStringLiteral("Solid Line"); break;
        }
        m_param2Label->setText(QStringLiteral("%1 (%2)").arg(entity.specialParam2()).arg(lineStyleStr));

        // SP3: Allegiance
        QString forceStr = (entity.specialParam3() == 2) ? QStringLiteral("Hostile (Red)") : QStringLiteral("Friendly (Gold)");
        m_param3Label->setText(QStringLiteral("%1 (%2)").arg(entity.specialParam3()).arg(forceStr));

        // Hide dynamic attributes if empty
        if (m_detailsGroup) {
            m_detailsGroup->setVisible(!details.isEmpty());
        }
    } else if (entity.type() == 8) {
        // Tactical Deployment Area: strictly hide all annotations, hide SP3 and SP4
        if (m_annotGroup) m_annotGroup->setVisible(false);
        if (m_topAnnotContainer) m_topAnnotContainer->setVisible(false);
        if (m_bottomAnnotContainer) m_bottomAnnotContainer->setVisible(false);
        if (m_leftAnnotContainer) m_leftAnnotContainer->setVisible(false);
        if (m_rightAnnotContainer) m_rightAnnotContainer->setVisible(false);

        if (m_paramGroup) {
            m_paramGroup->setVisible(true);
            m_paramGroup->setTitle(tr("TACTICAL DEPLOYMENT PARAMETERS"));
        }
        if (m_param1Container) m_param1Container->setVisible(true);
        if (m_param2Container) m_param2Container->setVisible(true);
        if (m_param3Container) m_param3Container->setVisible(false);
        if (m_param4Container) m_param4Container->setVisible(false);

        // SP1: Echelon Symbology
        QString echStr;
        switch (entity.specialParam1()) {
        case 1: echStr = QStringLiteral("Platoon (•••)"); break;
        case 2: echStr = QStringLiteral("Company (I)"); break;
        case 3: echStr = QStringLiteral("Battalion (II)"); break;
        case 4: echStr = QStringLiteral("Brigade (X)"); break;
        case 5: echStr = QStringLiteral("Division (XX)"); break;
        case 6: echStr = QStringLiteral("Corps (XXX)"); break;
        case 7: echStr = QStringLiteral("Army (XXXX)"); break;
        default: echStr = QStringLiteral("Brigade (X)"); break;
        }
        m_param1Label->setText(QStringLiteral("%1 (%2)").arg(entity.specialParam1()).arg(echStr));

        // SP2: Line Style
        QString lineStyleStr;
        switch (entity.specialParam2()) {
        case 1: lineStyleStr = QStringLiteral("Solid Line"); break;
        case 2: lineStyleStr = QStringLiteral("Dashed Line"); break;
        case 3: lineStyleStr = QStringLiteral("Dotted Line"); break;
        default: lineStyleStr = QStringLiteral("Solid Line"); break;
        }
        m_param2Label->setText(QStringLiteral("%1 (%2)").arg(entity.specialParam2()).arg(lineStyleStr));

        // Hide dynamic attributes if empty
        if (m_detailsGroup) {
            m_detailsGroup->setVisible(!details.isEmpty());
        }
    } else {
        if (m_annotGroup) m_annotGroup->setVisible(true);
        if (m_topAnnotContainer) m_topAnnotContainer->setVisible(true);
        if (m_bottomAnnotContainer) m_bottomAnnotContainer->setVisible(true);
        if (m_leftAnnotContainer) m_leftAnnotContainer->setVisible(true);
        if (m_rightAnnotContainer) m_rightAnnotContainer->setVisible(true);

        if (m_paramGroup) {
            m_paramGroup->setVisible(true);
            m_paramGroup->setTitle(tr("SPECIAL OPERATIONAL PARAMETERS"));
        }
        if (m_param1Container) m_param1Container->setVisible(true);
        if (m_param2Container) m_param2Container->setVisible(true);
        if (m_param3Container) m_param3Container->setVisible(true);
        if (m_param4Container) m_param4Container->setVisible(true);

        if (m_annotGroup) m_annotGroup->setTitle(tr("SPATIAL ANNOTATIONS (4-WAY)"));

        m_topAnnotationLabel->setText(entity.topAnnotation().isEmpty() ? QStringLiteral("—") : entity.topAnnotation());
        m_bottomAnnotationLabel->setText(entity.bottomAnnotation().isEmpty() ? QStringLiteral("—") : entity.bottomAnnotation());
        m_leftAnnotationLabel->setText(entity.leftAnnotation().isEmpty() ? QStringLiteral("—") : entity.leftAnnotation());
        m_rightAnnotationLabel->setText(entity.rightAnnotation().isEmpty() ? QStringLiteral("—") : entity.rightAnnotation());

        m_param1Label->setText(QString::number(entity.specialParam1()));
        m_param2Label->setText(QString::number(entity.specialParam2()));
        m_param3Label->setText(QString::number(entity.specialParam3()));
        m_param4Label->setText(QString::number(entity.specialParam4()));

        if (m_detailsGroup) {
            m_detailsGroup->setVisible(true);
        }
    }

    // 4. Populate Location Points Table
    m_pointsTable->setRowCount(pts.size());
    for (int i = 0; i < pts.size(); ++i) {
        const auto &pt = pts.at(i);
        auto *idxItem = new QTableWidgetItem(QString::number(i + 1));
        idxItem->setTextAlignment(Qt::AlignCenter);

        auto *latItem = new QTableWidgetItem(QString::asprintf("%.6f°", pt.latatitude));
        latItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        auto *lonItem = new QTableWidgetItem(QString::asprintf("%.6f°", pt.longitude));
        lonItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        auto *hItem = new QTableWidgetItem(QString::asprintf("%.1f m", pt.height));
        hItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        auto *dirItem = new QTableWidgetItem(QString::asprintf("%.1f°", pt.dir));
        dirItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        m_pointsTable->setItem(i, 0, idxItem);
        m_pointsTable->setItem(i, 1, latItem);
        m_pointsTable->setItem(i, 2, lonItem);
        m_pointsTable->setItem(i, 3, hItem);
        m_pointsTable->setItem(i, 4, dirItem);
    }
    adjustTableHeight(m_pointsTable, 85, 240);

    // 5. Populate Dynamic Details Table
    m_detailsTable->setRowCount(details.size());
    for (int i = 0; i < details.size(); ++i) {
        const auto &det = details.at(i);
        QString keyStr = QString::fromUtf8(det.valkey, static_cast<int>(strnlen(det.valkey, sizeof(det.valkey))));
        QString valStr = QString::fromUtf8(det.valStr, static_cast<int>(strnlen(det.valStr, sizeof(det.valStr))));

        QString typeName;
        switch (det.valType) {
        case 1:
            typeName = QStringLiteral("Integer");
            break;
        case 2:
            typeName = QStringLiteral("Double");
            break;
        case 3:
            typeName = QStringLiteral("String");
            break;
        default:
            typeName = QStringLiteral("Type %1").arg(det.valType);
            break;
        }

        auto *keyItem = new QTableWidgetItem(keyStr);
        auto *typeItem = new QTableWidgetItem(typeName);
        typeItem->setTextAlignment(Qt::AlignCenter);
        auto *valItem = new QTableWidgetItem(valStr);

        m_detailsTable->setItem(i, 0, keyItem);
        m_detailsTable->setItem(i, 1, typeItem);
        m_detailsTable->setItem(i, 2, valItem);
    }
    adjustTableHeight(m_detailsTable, 85, 240);

    m_centerBtn->setEnabled(!pts.isEmpty());
}

} // namespace GISApp::UI::ComplexEntities
