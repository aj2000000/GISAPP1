/**
 * @file SampleEntityDetailDialog.cpp
 * @brief Implementation of SampleEntityDetailDialog tactical inspector.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "SampleEntityDetailDialog.h"
#include <QStyle>

namespace GISApp::UI::SampleEntities {

SampleEntityDetailDialog::SampleEntityDetailDialog(
    const GISApp::Domain::SampleEntities::SampleEntity &entity,
    QWidget *parent)
    : QDialog(parent)
    , m_entityId(static_cast<int>(entity.Id()))
    , m_entity(entity)
{
    setObjectName(QStringLiteral("SampleEntityDetailDialog"));
    setWindowTitle(QStringLiteral("Sample Entity Details - %1 [ID: %2]")
                   .arg(m_entity.Name().isEmpty() ? QStringLiteral("ENT-%1").arg(m_entityId) : m_entity.Name())
                   .arg(m_entityId));
    setMinimumSize(460, 520);
    resize(500, 560);

    setupUi();
    updateEntityData(m_entity);
}

void SampleEntityDetailDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(12);

    // 1. Header Banner
    auto *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("SampleEntityDetailTitle"));
    headerLayout->addWidget(m_titleLabel, 1);

    m_typeBadge = new QLabel(this);
    m_typeBadge->setObjectName(QStringLiteral("SampleEntityDetailTypeBadge"));
    m_typeBadge->setAlignment(Qt::AlignCenter);
    m_typeBadge->setFixedHeight(26);
    headerLayout->addWidget(m_typeBadge);

    mainLayout->addLayout(headerLayout);

    // Scrollable area
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *scrollContent = new QWidget(scrollArea);
    scrollContent->setObjectName(QStringLiteral("scrollContent"));
    auto *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(0, 0, 4, 0);
    contentLayout->setSpacing(12);

    // 2. Identification Group
    auto *identGroup = new QGroupBox(QStringLiteral("IDENTIFICATION & TYPE"), scrollContent);
    identGroup->setObjectName(QStringLiteral("SampleEntityDetailGroup"));
    auto *identLayout = new QGridLayout(identGroup);
    identLayout->setContentsMargins(12, 12, 12, 12);
    identLayout->setVerticalSpacing(8);
    identLayout->setHorizontalSpacing(10);

    identLayout->addWidget(createPropertyRow(QStringLiteral("Entity ID:"), &m_idLabel), 0, 0);
    identLayout->addWidget(createPropertyRow(QStringLiteral("Designated Name:"), &m_nameLabel), 0, 1);
    identLayout->addWidget(createPropertyRow(QStringLiteral("Visual Type:"), &m_typeDescLabel), 1, 0, 1, 2);
    contentLayout->addWidget(identGroup);

    // 3. Geodetic Location Group
    auto *locGroup = new QGroupBox(QStringLiteral("GEODETIC LOCATION & KINEMATICS"), scrollContent);
    locGroup->setObjectName(QStringLiteral("SampleEntityDetailGroup"));
    auto *locLayout = new QGridLayout(locGroup);
    locLayout->setContentsMargins(12, 12, 12, 12);
    locLayout->setVerticalSpacing(8);
    locLayout->setHorizontalSpacing(10);

    locLayout->addWidget(createPropertyRow(QStringLiteral("Latitude:"), &m_latLabel), 0, 0);
    locLayout->addWidget(createPropertyRow(QStringLiteral("Longitude:"), &m_lonLabel), 0, 1);
    locLayout->addWidget(createPropertyRow(QStringLiteral("Height (MSL):"), &m_heightLabel), 1, 0);
    locLayout->addWidget(createPropertyRow(QStringLiteral("Direction / Bearing:"), &m_dirLabel), 1, 1);
    contentLayout->addWidget(locGroup);

    // 4. Telemetry & Remarks Group
    auto *metaGroup = new QGroupBox(QStringLiteral("TELEMETRY & REMARKS"), scrollContent);
    metaGroup->setObjectName(QStringLiteral("SampleEntityDetailGroup"));
    auto *metaLayout = new QGridLayout(metaGroup);
    metaLayout->setContentsMargins(12, 12, 12, 12);
    metaLayout->setVerticalSpacing(8);
    metaLayout->setHorizontalSpacing(10);

    metaLayout->addWidget(createPropertyRow(QStringLiteral("Report Time (UTC):"), &m_reportTimeLabel), 0, 0, 1, 2);
    metaLayout->addWidget(createPropertyRow(QStringLiteral("Remarks / Notes:"), &m_remarksLabel), 1, 0, 1, 2);
    contentLayout->addWidget(metaGroup);

    contentLayout->addStretch(1);
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);

    // 5. Close Button
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch(1);
    auto *closeBtn = new QPushButton(tr("Close"), this);
    closeBtn->setObjectName(QStringLiteral("SampleEntityDetailCloseBtn"));
    closeBtn->setFixedWidth(100);
    closeBtn->setDefault(true);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(closeBtn);
    mainLayout->addLayout(btnLayout);
}

QWidget* SampleEntityDetailDialog::createPropertyRow(const QString &labelText, QLabel **valueLabelPtr)
{
    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    auto *caption = new QLabel(labelText, container);
    caption->setObjectName(QStringLiteral("SampleEntityDetailCaption"));

    auto *valLabel = new QLabel(QStringLiteral("—"), container);
    valLabel->setObjectName(QStringLiteral("SampleEntityDetailValue"));
    valLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    layout->addWidget(caption);
    layout->addWidget(valLabel);

    *valueLabelPtr = valLabel;
    return container;
}

void SampleEntityDetailDialog::updateEntityData(const GISApp::Domain::SampleEntities::SampleEntity &entity)
{
    m_entity = entity;

    QString nameStr = entity.Name().isEmpty() ? QStringLiteral("ENT-%1").arg(entity.Id()) : entity.Name();
    m_titleLabel->setText(QStringLiteral("🔷 %1").arg(nameStr));

    QString typeStr;
    switch (entity.type()) {
    case 1:
        typeStr = QStringLiteral("Point Feature");
        break;
    case 2:
        typeStr = QStringLiteral("Bezier Curve");
        break;
    case 3:
        typeStr = QStringLiteral("SVG/PNG Icon");
        break;
    default:
        typeStr = QStringLiteral("Type %1").arg(entity.type());
        break;
    }

    m_typeBadge->setText(typeStr);
    m_typeBadge->setProperty("entityType", QString::number(entity.type()));
    if (m_typeBadge->style()) {
        m_typeBadge->style()->unpolish(m_typeBadge);
        m_typeBadge->style()->polish(m_typeBadge);
    }

    m_idLabel->setText(QString::number(entity.Id()));
    m_nameLabel->setText(nameStr);
    m_typeDescLabel->setText(QStringLiteral("%1 (Code: %2)").arg(typeStr).arg(entity.type()));

    m_latLabel->setText(QString::asprintf("%.6f°", entity.location().latatitude));
    m_lonLabel->setText(QString::asprintf("%.6f°", entity.location().longitude));
    m_heightLabel->setText(QString::asprintf("%.1f m", entity.location().height));
    m_dirLabel->setText(QString::asprintf("%.1f°", entity.location().dir));

    m_reportTimeLabel->setText(entity.reportTime().isValid()
                               ? entity.reportTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"))
                               : QStringLiteral("—"));
    m_remarksLabel->setText(entity.remarks().isEmpty() ? QStringLiteral("—") : entity.remarks());
}

} // namespace GISApp::UI::SampleEntities
