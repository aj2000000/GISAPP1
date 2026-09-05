/**
 * @file RightToolPanel.cpp
 * @brief Implementation of RightToolPanel layout and signaling.
 */

#include "RightToolPanel.h"

namespace GISApp::UI {

RightToolPanel::RightToolPanel(QWidget *parent)
    : QFrame(parent)
{
    setObjectName("RightToolPanel");
    setAttribute(Qt::WA_StyledBackground, true);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(4, 4, 4, 4);
    m_layout->setSpacing(4);

    m_layout->addWidget(createButton("Navigate", "➣"));
    m_layout->addWidget(createButton("Search", "🔍"));
    m_layout->addWidget(createButton("Filter", "Y"));
    m_layout->addWidget(createButton("Layers", "⬡"));
    m_layout->addWidget(createButton("Target", "🎯"));
    m_layout->addWidget(createButton("Bookmark", "🔖"));
}

QToolButton* RightToolPanel::createButton(const QString &name, const QString &text)
{
    QToolButton *btn = new QToolButton(this);
    btn->setText(text);
    btn->setToolTip(name);
    btn->setFixedSize(36, 36);

    connect(btn, &QToolButton::clicked, [this, name]() {
        emit toolTriggered(name);
    });

    return btn;
}

} // namespace GISApp::UI
