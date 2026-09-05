/**
 * @file LeftSidebar.cpp
 * @brief Implementation of LeftSidebar navigation layout and button signaling.
 */

#include "LeftSidebar.h"
#include <QStyle>
#include <QFont>

namespace GISApp::UI {

LeftSidebar::LeftSidebar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("LeftSidebar");
    setFixedWidth(56);
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(6, 12, 6, 12);
    m_layout->setSpacing(10);
    m_topLayout = new QVBoxLayout();
    m_topLayout->setSpacing(8);
    m_bottomLayout = new QVBoxLayout();
    m_bottomLayout->setSpacing(8);
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);
    setupButtons();
    m_layout->addLayout(m_topLayout);
    m_layout->addStretch();
    m_layout->addLayout(m_bottomLayout);
}

void LeftSidebar::setupButtons()
{
    // --- Top Navigation Section ---
    QToolButton *homeBtn = createButton("Home", "⌂", true); // Home Icon
    homeBtn->setObjectName("HomeButton");

    QToolButton *settingsBtn = createButton("Settings", "⚙", true);
    QToolButton *layersBtn = createButton("Layers", "≡", true);
    QToolButton *analyticsBtn = createButton("Analytics", "📊", true);
    QToolButton *toolsBtn = createButton("Tools", "🔧", true);

    m_topLayout->addWidget(homeBtn);
    m_topLayout->addWidget(settingsBtn);
    m_topLayout->addWidget(layersBtn);
    m_topLayout->addWidget(analyticsBtn);
    m_topLayout->addWidget(toolsBtn);

    // Set Home as default active button
    homeBtn->setChecked(true);

    // --- Bottom Utility Section ---
    QToolButton *userBtn = createButton("User", "👤", false);
    QToolButton *searchBtn = createButton("Search", "🔍", false);
    QToolButton *helpBtn = createButton("Help", "❓", false);

    m_bottomLayout->addWidget(userBtn);
    m_bottomLayout->addWidget(searchBtn);
    m_bottomLayout->addWidget(helpBtn);
}

QToolButton* LeftSidebar::createButton(const QString &name, const QString &text, bool isCheckable)
{
    QToolButton *btn = new QToolButton(this);
    btn->setText(text);
    btn->setToolTip(name);
    btn->setFixedSize(44, 44);
    btn->setFont(QFont("Sans-Serif", 14, QFont::Bold));
    btn->setCheckable(isCheckable);
    if (isCheckable) {
        m_buttonGroup->addButton(btn);
    }
    connect(btn, &QToolButton::clicked, [this, name]() {
        emit actionTriggered(name);
    });
    return btn;
}

} // namespace GISApp::UI
