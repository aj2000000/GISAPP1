/**
 * @file ZoomControlsWidget.cpp
 * @brief Implementation of ZoomControlsWidget layout and zoom signals.
 */

#include "ZoomControlsWidget.h"

namespace GISApp::UI {

ZoomControlsWidget::ZoomControlsWidget(QWidget *parent)
    : QFrame(parent)
{
    setObjectName("ZoomControlsWidget");
    setAttribute(Qt::WA_StyledBackground, true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    QToolButton *resetBtn = new QToolButton(this);
    resetBtn->setText("🧭");
    resetBtn->setToolTip(tr("Reset Bearing & Center"));
    resetBtn->setFixedSize(36, 36);

    QToolButton *zoomInBtn = new QToolButton(this);
    zoomInBtn->setText("+");
    zoomInBtn->setToolTip(tr("Zoom In"));
    zoomInBtn->setFixedSize(36, 36);

    QToolButton *zoomOutBtn = new QToolButton(this);
    zoomOutBtn->setText("−");
    zoomOutBtn->setToolTip(tr("Zoom Out"));
    zoomOutBtn->setFixedSize(36, 36);

    layout->addWidget(resetBtn);
    layout->addWidget(zoomInBtn);
    layout->addWidget(zoomOutBtn);

    connect(resetBtn, &QToolButton::clicked, this, &ZoomControlsWidget::resetCenterRequested);
    connect(zoomInBtn, &QToolButton::clicked, this, &ZoomControlsWidget::zoomInRequested);
    connect(zoomOutBtn, &QToolButton::clicked, this, &ZoomControlsWidget::zoomOutRequested);
}

} // namespace GISApp::UI
