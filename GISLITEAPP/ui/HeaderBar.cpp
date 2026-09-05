/**
 * @file HeaderBar.cpp
 * @brief Implementation of HeaderBar layout and action controls.
 */

#include "HeaderBar.h"

namespace GISApp::UI {

HeaderBar::HeaderBar(QWidget *parent)
    : QFrame(parent)
    , m_titleLabel(nullptr)
{
    setObjectName("HeaderBar");
    setFixedHeight(48);
    setupUi();
}

void HeaderBar::setupUi()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 4, 16, 4);

    // --- Left Title & Subtitle Info ---
    m_titleLabel = new QLabel(this);
    m_titleLabel->setText(
        "<span style='color:#10b981; font-weight:bold; font-size:14px;'>GIS-DSS</span> "
        "<span style='color:#4b5563;'>|</span> "
        "<span style='color:#e5e7eb; font-weight:500; font-size:14px;'>GISLITE Platform</span><br/>"
        "<span style='color:#6b7280; font-size:11px;'>v1.0.0 • System Operator</span>"
    );

    layout->addWidget(m_titleLabel);
    layout->addStretch();

    // --- Right Action Tools ---
    QPushButton *zonesBtn = createActionButton(tr("All Zones Visible ▾"), tr("Toggle Zones"));
    QPushButton *undoBtn  = createActionButton("↶", tr("Undo Action"));
    QPushButton *dlBtn    = createActionButton("📥", tr("Export Data"));
    QPushButton *playBtn  = createActionButton("▶", tr("Play Simulation"));
    QPushButton *notifBtn = createActionButton("🔔", tr("Notifications"));

    layout->addWidget(zonesBtn);
    layout->addWidget(undoBtn);
    layout->addWidget(dlBtn);
    layout->addWidget(playBtn);
    layout->addWidget(notifBtn);
}

QPushButton* HeaderBar::createActionButton(const QString &text, const QString &tooltip)
{
    QPushButton *btn = new QPushButton(text, this);
    btn->setToolTip(tooltip);
    btn->setObjectName("HeaderActionButton");
    btn->setMinimumHeight(28);

    connect(btn, &QPushButton::clicked, [this, tooltip]() {
        emit actionTriggered(tooltip);
    });

    return btn;
}

void HeaderBar::setOperatorInfo(const QString &title, const QString &company, const QString &versionAndRole)
{
    if (m_titleLabel) {
        m_titleLabel->setText(
            QString("<span style='color:#10b981; font-weight:bold; font-size:14px;'>%1</span> "
                    "<span style='color:#4b5563;'>|</span> "
                    "<span style='color:#e5e7eb; font-weight:500; font-size:14px;'>%2</span><br/>"
                    "<span style='color:#6b7280; font-size:11px;'>%3</span>")
                .arg(title, company, versionAndRole)
        );
    }
}

} // namespace GISApp::UI
