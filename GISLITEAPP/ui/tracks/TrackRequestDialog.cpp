/**
 * @file TrackRequestDialog.cpp
 * @brief Implementation of TrackRequestDialog for querying tactical tracks over UDP.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "TrackRequestDialog.h"
#include "UdpDataStore.h"
#include "ThemeManager.h"

#include <QFrame>
#include <QIcon>
#include <QCalendarWidget>

namespace GISApp::UI::Tracks {

/**
 * @brief Constructs the TrackRequestDialog with date/time pickers and quick presets.
 * @param[in] parent Optional parent widget ownership.
 */
TrackRequestDialog::TrackRequestDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName("TrackRequestDialog");
    setWindowTitle(tr("Request Tactical Tracks (UDP 1501)"));
    setMinimumWidth(480);
    setAttribute(Qt::WA_DeleteOnClose, false);

    setupUi();
    setupConnections();

    // Default to past 24 hours
    setPresetPast24Hours();
}

/**
 * @brief Returns the operator-selected starting timestamp.
 * @return QDateTime representing beginning of query range.
 */
QDateTime TrackRequestDialog::fromDateTime() const
{
    return m_fromDateTimeEdit ? m_fromDateTimeEdit->dateTime() : QDateTime();
}

/**
 * @brief Returns the operator-selected ending timestamp.
 * @return QDateTime representing termination of query range.
 */
QDateTime TrackRequestDialog::toDateTime() const
{
    return m_toDateTimeEdit ? m_toDateTimeEdit->dateTime() : QDateTime();
}

/**
 * @brief Assembles visual hierarchy, controls, layouts, and preset action buttons.
 */
void TrackRequestDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(18, 18, 18, 18);

    // -------------------------------------------------------------------------
    // 1. Header Information Banner
    // -------------------------------------------------------------------------
    auto *headerFrame = new QFrame(this);
    headerFrame->setObjectName("CardFrame");
    auto *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setContentsMargins(12, 10, 12, 10);
    headerLayout->setSpacing(4);

    auto *titleLabel = new QLabel(tr("📡 Request Tracks from External Application"), headerFrame);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(12);
    titleLabel->setFont(titleFont);

    auto *descLabel = new QLabel(
        tr("Specify the temporal interval to request tactical tracks via UDP Message ID 1501 (REQ_ENTITY_MESSAGE)."),
        headerFrame);
    descLabel->setWordWrap(true);
    descLabel->setObjectName("CardValue");

    QString targetIp = GISApp::Communication::Udp::Config::UdpDataStore::instance().targetIp();
    quint16 targetPort = GISApp::Communication::Udp::Config::UdpDataStore::instance().targetPort();

    m_targetInfoLabel = new QLabel(
        tr("Target Endpoint: <b>%1:%2</b> (CSCI: DFE)").arg(targetIp).arg(targetPort),
        headerFrame);
    m_targetInfoLabel->setObjectName("CardKey");

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(descLabel);
    headerLayout->addWidget(m_targetInfoLabel);
    mainLayout->addWidget(headerFrame);

    // -------------------------------------------------------------------------
    // 2. Date & Time Selection Group
    // -------------------------------------------------------------------------
    auto *rangeGroupBox = new QGroupBox(tr("Query Time Range"), this);
    auto *gridLayout = new QGridLayout(rangeGroupBox);
    gridLayout->setHorizontalSpacing(12);
    gridLayout->setVerticalSpacing(10);
    gridLayout->setContentsMargins(14, 14, 14, 14);

    auto *fromLabel = new QLabel(tr("From Date & Time:"), rangeGroupBox);
    fromLabel->setObjectName("CardKey");
    m_fromDateTimeEdit = new QDateTimeEdit(rangeGroupBox);
    m_fromDateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    m_fromDateTimeEdit->setCalendarPopup(true);

    auto *toLabel = new QLabel(tr("To Date & Time:"), rangeGroupBox);
    toLabel->setObjectName("CardKey");
    m_toDateTimeEdit = new QDateTimeEdit(rangeGroupBox);
    m_toDateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    m_toDateTimeEdit->setCalendarPopup(true);

    gridLayout->addWidget(fromLabel, 0, 0);
    gridLayout->addWidget(m_fromDateTimeEdit, 0, 1);
    gridLayout->addWidget(toLabel, 1, 0);
    gridLayout->addWidget(m_toDateTimeEdit, 1, 1);

    mainLayout->addWidget(rangeGroupBox);

    // -------------------------------------------------------------------------
    // 3. Quick Range Presets
    // -------------------------------------------------------------------------
    auto *presetLayout = new QHBoxLayout();
    presetLayout->setSpacing(8);

    auto *presetLabel = new QLabel(tr("Presets:"), this);
    presetLabel->setObjectName("CardKey");
    presetLayout->addWidget(presetLabel);

    m_preset1HrBtn = new QPushButton(tr("Past 1h"), this);
    m_preset6HrBtn = new QPushButton(tr("Past 6h"), this);
    m_preset24HrBtn = new QPushButton(tr("Past 24h"), this);
    m_presetTodayBtn = new QPushButton(tr("Today"), this);

    presetLayout->addWidget(m_preset1HrBtn);
    presetLayout->addWidget(m_preset6HrBtn);
    presetLayout->addWidget(m_preset24HrBtn);
    presetLayout->addWidget(m_presetTodayBtn);
    presetLayout->addStretch();

    mainLayout->addLayout(presetLayout);

    // -------------------------------------------------------------------------
    // 4. Validation Feedback Label
    // -------------------------------------------------------------------------
    m_validationLabel = new QLabel(this);
    m_validationLabel->setObjectName(QStringLiteral("TrackRequestValidationLabel"));
    m_validationLabel->setVisible(false);
    mainLayout->addWidget(m_validationLabel);

    // -------------------------------------------------------------------------
    // 5. Action Dialog Buttons
    // -------------------------------------------------------------------------
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_sendButton = new QPushButton(tr("Send Request"), this);
    m_sendButton->setDefault(true);
    m_sendButton->setProperty("accent", true);

    btnLayout->addWidget(m_cancelButton);
    btnLayout->addWidget(m_sendButton);
    mainLayout->addLayout(btnLayout);
}

/**
 * @brief Binds widget signals to operational logic and validation.
 */
void TrackRequestDialog::setupConnections()
{
    connect(m_fromDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &TrackRequestDialog::validateRange);
    connect(m_toDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &TrackRequestDialog::validateRange);

    connect(m_preset1HrBtn, &QPushButton::clicked, this, &TrackRequestDialog::setPresetPast1Hour);
    connect(m_preset6HrBtn, &QPushButton::clicked, this, &TrackRequestDialog::setPresetPast6Hours);
    connect(m_preset24HrBtn, &QPushButton::clicked, this, &TrackRequestDialog::setPresetPast24Hours);
    connect(m_presetTodayBtn, &QPushButton::clicked, this, &TrackRequestDialog::setPresetToday);

    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_sendButton, &QPushButton::clicked, this, &QDialog::accept);
}

/**
 * @brief Ensures the starting timestamp does not exceed the termination timestamp.
 */
void TrackRequestDialog::validateRange()
{
    const QDateTime fromDt = m_fromDateTimeEdit->dateTime();
    const QDateTime toDt = m_toDateTimeEdit->dateTime();

    if (fromDt > toDt) {
        m_validationLabel->setText(tr("⚠️ 'From' date/time must be earlier than or equal to 'To' date/time."));
        m_validationLabel->setVisible(true);
        m_sendButton->setEnabled(false);
    } else {
        m_validationLabel->setVisible(false);
        m_sendButton->setEnabled(true);
    }
}

/**
 * @brief Configures a 1-hour temporal query ending at the current timestamp.
 */
void TrackRequestDialog::setPresetPast1Hour()
{
    const QDateTime now = QDateTime::currentDateTime();
    m_toDateTimeEdit->setDateTime(now);
    m_fromDateTimeEdit->setDateTime(now.addSecs(-3600));
}

/**
 * @brief Configures a 6-hour temporal query ending at the current timestamp.
 */
void TrackRequestDialog::setPresetPast6Hours()
{
    const QDateTime now = QDateTime::currentDateTime();
    m_toDateTimeEdit->setDateTime(now);
    m_fromDateTimeEdit->setDateTime(now.addSecs(-21600));
}

/**
 * @brief Configures a 24-hour temporal query ending at the current timestamp.
 */
void TrackRequestDialog::setPresetPast24Hours()
{
    const QDateTime now = QDateTime::currentDateTime();
    m_toDateTimeEdit->setDateTime(now);
    m_fromDateTimeEdit->setDateTime(now.addDays(-1));
}

/**
 * @brief Configures a temporal query spanning from 00:00:00 today to the current timestamp.
 */
void TrackRequestDialog::setPresetToday()
{
    const QDateTime now = QDateTime::currentDateTime();
    const QDateTime todayStart(now.date(), QTime(0, 0, 0));
    m_toDateTimeEdit->setDateTime(now);
    m_fromDateTimeEdit->setDateTime(todayStart);
}

} // namespace GISApp::UI::Tracks
