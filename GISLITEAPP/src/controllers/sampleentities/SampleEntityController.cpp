/**
 * @file SampleEntityController.cpp
 * @brief Implementation of SampleEntityController orchestrating sample entity workflows and context menus.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "SampleEntityController.h"
#include "SampleEntityService.h"
#include "SampleEntityMapRenderer.h"
#include "SampleEntityMapFeatureAdapter.h"
#include "SampleEntityDetailDialog.h"
#include "IMapFeature.h"
#include "MapController.h"
#include "MapWidget.h"

#include <QMapLibre/Map>
#include <QMenu>
#include <QAction>
#include <QGuiApplication>
#include <QClipboard>
#include <QDebug>
#include <cmath>
#include <algorithm>

namespace GISApp::Controllers::SampleEntities {

SampleEntityController::SampleEntityController(
    GISApp::Services::SampleEntities::SampleEntityService *entityService,
    GISApp::UI::Renderers::SampleEntityMapRenderer *renderer,
    GISApp::Controllers::MapController *mapController,
    QObject *parent)
    : QObject(parent)
    , m_service(entityService)
    , m_renderer(renderer)
    , m_mapController(mapController)
{
    setupConnections();
}

SampleEntityController::~SampleEntityController()
{
    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->unregisterContextMenuContributor(this);
    }
}

void SampleEntityController::setupConnections()
{
    if (m_service) {
        connect(m_service, &GISApp::Services::SampleEntities::SampleEntityService::entitiesUpdated,
                this, &SampleEntityController::onEntitiesUpdated);
        connect(m_service, &GISApp::Services::SampleEntities::SampleEntityService::entityUpdated,
                this, &SampleEntityController::onEntityUpdated);
        connect(m_service, &GISApp::Services::SampleEntities::SampleEntityService::entityRemoved,
                this, &SampleEntityController::onEntityRemoved);
    }

    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->registerContextMenuContributor(this);
    }
}

void SampleEntityController::setSampleEntityMapRenderer(GISApp::UI::Renderers::SampleEntityMapRenderer *renderer)
{
    m_renderer = renderer;
    if (m_renderer && m_service) {
        onEntitiesUpdated(m_service->getAllEntities());
    }
}

void SampleEntityController::setMapController(GISApp::Controllers::MapController *mapController)
{
    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->unregisterContextMenuContributor(this);
    }

    m_mapController = mapController;

    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->registerContextMenuContributor(this);
    }
}

void SampleEntityController::initialize()
{
    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->registerContextMenuContributor(this);
    }

    if (m_renderer && m_service) {
        onEntitiesUpdated(m_service->getAllEntities());
    }
}

void SampleEntityController::setEntitiesVisible(bool visible)
{
    if (m_renderer) {
        m_renderer->setEntitiesVisible(visible);
    }
    emit entityVisibilityChanged(visible);
}

bool SampleEntityController::isEntitiesVisible() const
{
    return m_renderer ? m_renderer->isEntitiesVisible() : false;
}

void SampleEntityController::onEntitiesUpdated(const QVector<GISApp::Domain::SampleEntities::SampleEntity> &entities)
{
    if (!m_renderer) {
        return;
    }

    QVector<GISApp::UI::Renderers::SampleEntityMapFeatureAdapter> adapters;
    adapters.reserve(entities.size() * 2);

    for (const auto &entity : entities) {
        if (entity.type() == 2) {
            // Bezier curve: Primary LineString geometry
            adapters.emplace_back(entity, GISApp::UI::Renderers::SampleEntityMapFeatureAdapter::SubFeatureType::Primary);
            // Center Anchor: Point geometry for center labeling
            adapters.emplace_back(entity, GISApp::UI::Renderers::SampleEntityMapFeatureAdapter::SubFeatureType::CenterAnchor);
        } else {
            // Type 1 (Point) & Type 3 (Icon)
            adapters.emplace_back(entity, GISApp::UI::Renderers::SampleEntityMapFeatureAdapter::SubFeatureType::Primary);
        }
    }

    QVector<const GISApp::Core::Interfaces::IMapFeature*> features;
    features.reserve(adapters.size());
    for (const auto &adapter : adapters) {
        features.append(&adapter);
    }

    m_renderer->renderFeatures(features);
}

bool SampleEntityController::calculateEntitiesCenter(double &outLat, double &outLon, double &outZoom) const
{
    if (!m_service) {
        return false;
    }
    const auto entities = m_service->getAllEntities();
    if (entities.isEmpty()) {
        return false;
    }

    double sumLat = 0.0;
    double sumLon = 0.0;
    double minLat = 90.0, maxLat = -90.0;
    double minLon = 180.0, maxLon = -180.0;

    for (const auto &ent : entities) {
        double lat = ent.location().latatitude;
        double lon = ent.location().longitude;
        sumLat += lat;
        sumLon += lon;
        minLat = std::min(minLat, lat);
        maxLat = std::max(maxLat, lat);
        minLon = std::min(minLon, lon);
        maxLon = std::max(maxLon, lon);
    }

    outLat = sumLat / entities.size();
    outLon = sumLon / entities.size();

    double deltaLat = std::abs(maxLat - minLat);
    double deltaLon = std::abs(maxLon - minLon);
    double maxDelta = std::max(deltaLat, deltaLon);

    if (maxDelta < 0.05) {
        outZoom = 13.0;
    } else if (maxDelta < 0.2) {
        outZoom = 11.0;
    } else if (maxDelta < 1.0) {
        outZoom = 9.0;
    } else if (maxDelta < 3.0) {
        outZoom = 7.0;
    } else {
        outZoom = 5.0;
    }

    return true;
}

void SampleEntityController::onEntitySelected(double latitude, double longitude)
{
    if (m_mapController) {
        qDebug() << "[SampleEntityController] Centering map camera on sample entity at:" << latitude << longitude;
        m_mapController->setCenter(latitude, longitude);
    }
}

bool SampleEntityController::contributeActions(QMenu *parentMenu, const QPoint &screenPos, const QPointF &geoCoord)
{
    if (!parentMenu || !m_service || !isEntitiesVisible()) {
        return false;
    }

    const auto entities = m_service->getAllEntities();
    if (entities.isEmpty()) {
        return false;
    }

    QMapLibre::Map *mapCore = nullptr;
    if (m_mapController && m_mapController->attachedMap()) {
        mapCore = m_mapController->attachedMap()->rawMap();
    }

    struct EntityHit {
        Domain::SampleEntities::SampleEntity entity;
        double distancePx;
    };
    QVector<EntityHit> hits;

    constexpr double hitRadiusPx = 24.0;

    for (const auto &ent : entities) {
        double eLat = ent.location().latatitude;
        double eLon = ent.location().longitude;
        double minDist = 1e9;

        if (mapCore) {
            // 1. Distance to center anchor
            QPointF p = mapCore->pixelForCoordinate(QMapLibre::Coordinate(eLat, eLon));
            double dx = p.x() - screenPos.x();
            double dy = p.y() - screenPos.y();
            minDist = std::sqrt(dx * dx + dy * dy);

            // 2. If Bezier curve (type == 2), also hit-test along the curve coordinates
            if (ent.type() == 2) {
                const auto curvePts = GISApp::UI::Renderers::SampleEntityMapFeatureAdapter::generateBezierCurve(
                    eLon, eLat, ent.location().dir);
                for (const auto &cpt : curvePts) {
                    QPointF cp = mapCore->pixelForCoordinate(QMapLibre::Coordinate(cpt.y(), cpt.x()));
                    double cdx = cp.x() - screenPos.x();
                    double cdy = cp.y() - screenPos.y();
                    double cdist = std::sqrt(cdx * cdx + cdy * cdy);
                    if (cdist < minDist) {
                        minDist = cdist;
                    }
                }
            }
        } else {
            // Fallback geographic delta
            double dLat = eLat - geoCoord.x();
            double dLon = eLon - geoCoord.y();
            minDist = std::sqrt(dLat * dLat + dLon * dLon) * 500.0;
        }

        if (minDist <= hitRadiusPx) {
            hits.append({ent, minDist});
        }
    }

    if (hits.isEmpty()) {
        return false;
    }

    std::sort(hits.begin(), hits.end(), [](const EntityHit &a, const EntityHit &b) {
        return a.distancePx < b.distancePx;
    });

    for (const auto &hit : hits) {
        const auto &ent = hit.entity;
        int entityId = static_cast<int>(ent.Id());
        double eLat = ent.location().latatitude;
        double eLon = ent.location().longitude;

        QString typeIcon;
        QString typeDesc;
        switch (ent.type()) {
        case 1:
            typeIcon = QStringLiteral("🟢");
            typeDesc = QStringLiteral("Point");
            break;
        case 2:
            typeIcon = QStringLiteral("〰️");
            typeDesc = QStringLiteral("Bezier Curve");
            break;
        case 3:
            typeIcon = QStringLiteral("🔷");
            typeDesc = QStringLiteral("Tactical Icon");
            break;
        default:
            typeIcon = QStringLiteral("🔶");
            typeDesc = QStringLiteral("Type %1").arg(ent.type());
            break;
        }

        QString displayName = ent.Name().isEmpty() ? QStringLiteral("SMPL-%1").arg(entityId) : ent.Name();

        // Level 2 Sub-Menu on Right-Click Menu
        QMenu *entityMenu = parentMenu->addMenu(QStringLiteral("%1 %2 (ID: %3 • %4)")
                            .arg(typeIcon)
                            .arg(displayName)
                            .arg(entityId)
                            .arg(typeDesc));

        // Level 3 Actions:
        // 1. Show Details
        QAction *detailsAct = entityMenu->addAction(QStringLiteral("📄 Show Details"));
        connect(detailsAct, &QAction::triggered, this, [this, entityId]() {
            showEntityDetails(entityId);
        });

        // 2. Center on Map
        QAction *centerAct = entityMenu->addAction(QStringLiteral("🎯 Center on Map"));
        connect(centerAct, &QAction::triggered, this, [this, eLat, eLon]() {
            onEntitySelected(eLat, eLon);
        });

        // 3. Copy Coordinates
        QAction *copyAct = entityMenu->addAction(QStringLiteral("📋 Copy Coordinates"));
        connect(copyAct, &QAction::triggered, this, [eLat, eLon]() {
            QString coordStr = QStringLiteral("%1, %2")
                               .arg(eLat, 0, 'f', 6)
                               .arg(eLon, 0, 'f', 6);
            QGuiApplication::clipboard()->setText(coordStr);
            qDebug() << "[SampleEntityController] Copied coordinates to clipboard:" << coordStr;
        });

        entityMenu->addSeparator();

        // 4. Delete Entity (Placeholder)
        QAction *deleteAct = entityMenu->addAction(QStringLiteral("🗑️ Delete Entity (Placeholder)"));
        deleteAct->setEnabled(false);
        deleteAct->setToolTip(QStringLiteral("Entity deletion is currently disabled"));
    }

    return true;
}

void SampleEntityController::showEntityDetails(int entityId)
{
    if (!m_service) {
        return;
    }

    if (m_detailDialogs.contains(entityId) && m_detailDialogs[entityId]) {
        m_detailDialogs[entityId]->raise();
        m_detailDialogs[entityId]->activateWindow();
        return;
    }

    auto opt = m_service->getEntityById(entityId);
    if (!opt.has_value()) {
        return;
    }

    auto *dialog = new GISApp::UI::SampleEntities::SampleEntityDetailDialog(opt.value());
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(dialog, &QObject::destroyed, this, [this, entityId]() {
        m_detailDialogs.remove(entityId);
    });

    m_detailDialogs.insert(entityId, dialog);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void SampleEntityController::onEntityUpdated(const GISApp::Domain::SampleEntities::SampleEntity &entity)
{
    int entityId = static_cast<int>(entity.Id());
    if (m_detailDialogs.contains(entityId) && m_detailDialogs[entityId]) {
        m_detailDialogs[entityId]->updateEntityData(entity);
    }
}

void SampleEntityController::onEntityRemoved(int entityId)
{
    if (m_detailDialogs.contains(entityId) && m_detailDialogs[entityId]) {
        m_detailDialogs[entityId]->close();
        m_detailDialogs.remove(entityId);
    }
}

void SampleEntityController::centerOnEntity(int entityId)
{
    if (!m_service || !m_mapController) {
        return;
    }
    auto opt = m_service->getEntityById(entityId);
    if (opt.has_value()) {
        m_mapController->setCenter(opt->location().latatitude, opt->location().longitude);
    }
}

void SampleEntityController::deleteEntity(int entityId)
{
    if (m_service) {
        m_service->deleteEntity(entityId);
    }
}

} // namespace GISApp::Controllers::SampleEntities
