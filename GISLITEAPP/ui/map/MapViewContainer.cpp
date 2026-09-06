/**
 * @file MapViewContainer.cpp
 * @brief Implementation of MapViewContainer layout and overlay orchestration.
 */

#include "MapViewContainer.h"
#include "MapWidget.h"
#include "ZoomControlsWidget.h"
#include "RightToolPanel.h"
#include "LayerTreePanel.h"
#include "ComplexEntityLocationEditOverlay.h"

#include <QResizeEvent>
#include <QShowEvent>
#include <algorithm>

namespace GISApp::UI {

MapViewContainer::MapViewContainer(QWidget *parent)
    : QWidget(parent)
    , m_mapWidget(nullptr)
    , m_zoomControls(nullptr)
    , m_rightToolPanel(nullptr)
    , m_layerTreePanel(nullptr)
    , m_locationEditOverlay(nullptr)
    , m_overlayMargin(18)
{
    setObjectName("MapViewContainer");
    setupUi();
    setupConnections();
}

void MapViewContainer::setupUi()
{
    setMouseTracking(true);

    // 1. Base Layer: Full-bleed MapLibre canvas
    m_mapWidget = new MapWidget(this);
    m_mapWidget->setMouseTracking(true);

    // 2. Floating Overlays layered on top of map canvas
    m_rightToolPanel = new RightToolPanel(this);
    m_zoomControls = new ZoomControlsWidget(this);
    m_layerTreePanel = new GISApp::UI::Layers::LayerTreePanel(this);
    m_locationEditOverlay = new GISApp::UI::ComplexEntities::ComplexEntityLocationEditOverlay(this);

    m_rightToolPanel->raise();
    m_zoomControls->raise();
    m_layerTreePanel->raise();
    m_locationEditOverlay->raise();
}

void MapViewContainer::setupConnections()
{
    if (m_zoomControls && m_mapWidget) {
        connect(m_zoomControls, &ZoomControlsWidget::zoomInRequested, this, [this]() {
            m_mapWidget->zoomIn(1.0);
        });

        connect(m_zoomControls, &ZoomControlsWidget::zoomOutRequested, this, [this]() {
            m_mapWidget->zoomOut(1.0);
        });

        connect(m_zoomControls, &ZoomControlsWidget::resetCenterRequested, this, [this]() {
            m_mapWidget->resetBearing();
        });
    }

    if (m_mapWidget) {
        connect(m_mapWidget, &MapWidget::coordinateHoveredRaw, this, &MapViewContainer::coordinateHovered);
    }
}

void MapViewContainer::setOverlayMargins(int margin)
{
    m_overlayMargin = std::max(0, margin);
    updateOverlayPositions();
}

void MapViewContainer::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateOverlayPositions();
}

void MapViewContainer::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateOverlayPositions();
}

void MapViewContainer::updateOverlayPositions()
{
    if (width() <= 0 || height() <= 0) {
        return;
    }

    // Base map canvas expands to full extent of container
    if (m_mapWidget) {
        m_mapWidget->setGeometry(0, 0, width(), height());
    }

    // Position Right Tool Panel floating at top-right with margin offset
    if (m_rightToolPanel) {
        m_rightToolPanel->adjustSize();
        int panelW = m_rightToolPanel->width();
        int panelH = m_rightToolPanel->height();
        int x = std::max(m_overlayMargin, width() - panelW - m_overlayMargin);
        int y = m_overlayMargin;
        m_rightToolPanel->setGeometry(x, y, panelW, panelH);
        m_rightToolPanel->raise();
    }

    // Position Zoom Controls floating at bottom-left with margin offset
    if (m_zoomControls) {
        m_zoomControls->adjustSize();
        int zoomW = m_zoomControls->width();
        int zoomH = m_zoomControls->height();
        int x = m_overlayMargin;
        int y = std::max(m_overlayMargin, height() - zoomH - m_overlayMargin);
        m_zoomControls->setGeometry(x, y, zoomW, zoomH);
        m_zoomControls->raise();
    }

    // Position Floating Layer Tree Panel at top-left by default, preserving user drags
    if (m_layerTreePanel) {
        if (m_layerTreePanel->pos() == QPoint(0, 0)) {
            m_layerTreePanel->move(m_overlayMargin, m_overlayMargin);
        } else {
            int curX = std::clamp(m_layerTreePanel->x(), 0, std::max(0, width() - m_layerTreePanel->width()));
            int curY = std::clamp(m_layerTreePanel->y(), 0, std::max(0, height() - m_layerTreePanel->height()));
            m_layerTreePanel->move(curX, curY);
        }
        m_layerTreePanel->raise();
    }

    // Position Floating Location Edit Ribbon horizontally centered at top
    if (m_locationEditOverlay && m_locationEditOverlay->isVisible()) {
        m_locationEditOverlay->adjustSize();
        int overlayW = m_locationEditOverlay->width();
        int overlayH = m_locationEditOverlay->height();
        int x = std::max(0, (width() - overlayW) / 2);
        int y = m_overlayMargin;
        m_locationEditOverlay->setGeometry(x, y, overlayW, overlayH);
        m_locationEditOverlay->raise();
    }
}

} // namespace GISApp::UI
