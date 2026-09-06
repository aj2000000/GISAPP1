/**
 * @file ComplexEntityController.cpp
 * @brief Implementation of ComplexEntityController orchestrating domain workflows and map interactions.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "ComplexEntityController.h"
#include "ComplexEntityService.h"
#include "ComplexEntityMapRenderer.h"
#include "ComplexEntityMapFeatureAdapter.h"
#include "ComplexEntityDetailDialog.h"
#include "ComplexEntityEditDialog.h"
#include "ComplexEntityLocationEditOverlay.h"
#include "IMapFeature.h"
#include "MapController.h"
#include "MapWidget.h"
#include "MapViewContainer.h"

#include <QMapLibre/Map>
#include <QMapLibre/Types>
#include <QMenu>
#include <QAction>
#include <QGuiApplication>
#include <QClipboard>
#include <QDebug>
#include <cmath>
#include <algorithm>

namespace GISApp::Controllers::ComplexEntities {

namespace {

/**
 * @brief Calculates Euclidean distance from a point to a finite line segment.
 * @param[in] p Test point.
 * @param[in] a Segment start point.
 * @param[in] b Segment end point.
 * @return Minimum distance in pixels.
 */
double distanceToSegment(const QPointF &p, const QPointF &a, const QPointF &b)
{
    double l2 = (b.x() - a.x()) * (b.x() - a.x()) + (b.y() - a.y()) * (b.y() - a.y());
    if (l2 <= 1e-6) {
        double dx = p.x() - a.x();
        double dy = p.y() - a.y();
        return std::sqrt(dx * dx + dy * dy);
    }
    double t = ((p.x() - a.x()) * (b.x() - a.x()) + (p.y() - a.y()) * (b.y() - a.y())) / l2;
    t = std::max(0.0, std::min(1.0, t));
    double projX = a.x() + t * (b.x() - a.x());
    double projY = a.y() + t * (b.y() - a.y());
    double dx = p.x() - projX;
    double dy = p.y() - projY;
    return std::sqrt(dx * dx + dy * dy);
}

/**
 * @brief Determines whether a 2D point lies within a polygon using the Jordan ray-casting algorithm.
 * @param[in] pt Test point.
 * @param[in] poly Ordered polygon vertices.
 * @return True if point is inside, false otherwise.
 */
bool isPointInPolygon(const QPointF &pt, const QVector<QPointF> &poly)
{
    if (poly.size() < 3) return false;
    bool inside = false;
    for (int i = 0, j = poly.size() - 1; i < poly.size(); j = i++) {
        if (((poly[i].y() > pt.y()) != (poly[j].y() > pt.y())) &&
            (pt.x() < (poly[j].x() - poly[i].x()) * (pt.y() - poly[i].y()) / (poly[j].y() - poly[i].y()) + poly[i].x())) {
            inside = !inside;
        }
    }
    return inside;
}

} // anonymous namespace

ComplexEntityController::ComplexEntityController(
    GISApp::Services::ComplexEntities::ComplexEntityService *entityService,
    GISApp::UI::Renderers::ComplexEntityMapRenderer *renderer,
    GISApp::Controllers::MapController *mapController,
    QObject *parent)
    : QObject(parent)
    , m_service(entityService)
    , m_renderer(renderer)
    , m_mapController(mapController)
{
    setupConnections();
    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->registerContextMenuContributor(this);
    }
}

ComplexEntityController::~ComplexEntityController()
{
    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->unregisterContextMenuContributor(this);
    }

    if (m_mapViewContainer && m_mapViewContainer->mapWidget()) {
        m_mapViewContainer->mapWidget()->removeInteractionListener(this);
    }

    for (auto &dialog : m_detailDialogs) {
        if (dialog) {
            dialog->close();
            dialog->deleteLater();
        }
    }
    m_detailDialogs.clear();
}

void ComplexEntityController::initialize()
{
    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->registerContextMenuContributor(this);
        connect(m_mapController->attachedMap(), &GISApp::UI::MapWidget::coordinateClicked,
                this, [this](const GISApp::Core::Models::GeoCoordinate &coord) {
                    for (auto &dlg : m_editDialogs) {
                        if (dlg && dlg->isPickModeActive()) {
                            dlg->onMapCoordinateClicked(coord.latitude(), coord.longitude());
                        }
                    }
                });
    }

    if (m_service && m_renderer) {
        onEntitiesUpdated(m_service->getAllEntities());
    }
}

void ComplexEntityController::setupConnections()
{
    if (!m_service) return;

    connect(m_service, &GISApp::Services::ComplexEntities::ComplexEntityService::entitiesUpdated,
            this, &ComplexEntityController::onEntitiesUpdated);

    connect(m_service, &GISApp::Services::ComplexEntities::ComplexEntityService::entityUpserted,
            this, [this](int entityId) {
                if (m_service) {
                    auto opt = m_service->getEntityById(entityId);
                    if (opt.has_value()) {
                        onEntityUpdated(opt.value());
                    }
                }
            });

    connect(m_service, &GISApp::Services::ComplexEntities::ComplexEntityService::entityRemoved,
            this, [this](int entityId) {
                onEntityRemoved(static_cast<quint32>(entityId));
            });
}

void ComplexEntityController::setEntitiesVisible(bool visible)
{
    if (m_renderer) {
        m_renderer->setEntitiesVisible(visible);
        emit entityVisibilityChanged(visible);
    }
}

bool ComplexEntityController::isEntitiesVisible() const
{
    return m_renderer ? m_renderer->isEntitiesVisible() : false;
}

void ComplexEntityController::setComplexEntityMapRenderer(GISApp::UI::Renderers::ComplexEntityMapRenderer *renderer)
{
    m_renderer = renderer;
    if (m_renderer && m_service) {
        onEntitiesUpdated(m_service->getAllEntities());
    }
}

void ComplexEntityController::setMapController(GISApp::Controllers::MapController *mapController)
{
    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->unregisterContextMenuContributor(this);
    }

    m_mapController = mapController;

    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->registerContextMenuContributor(this);
        connect(m_mapController->attachedMap(), &GISApp::UI::MapWidget::coordinateClicked,
                this, [this](const GISApp::Core::Models::GeoCoordinate &coord) {
                    for (auto &dlg : m_editDialogs) {
                        if (dlg && dlg->isPickModeActive()) {
                            dlg->onMapCoordinateClicked(coord.latitude(), coord.longitude());
                        }
                    }
                });
    }
}

bool ComplexEntityController::calculateEntitiesCenter(double &outLat, double &outLon, double &outZoom) const
{
    if (!m_service) return false;
    return m_service->calculateEntitiesCenter(outLat, outLon, outZoom);
}

void ComplexEntityController::onEntitiesUpdated(const QVector<GISApp::Domain::ComplexEntities::ComplexEntity> &entities)
{
    if (!m_renderer) {
        return;
    }

    QVector<GISApp::UI::Renderers::ComplexEntityMapFeatureAdapter> adapters;
    adapters.reserve(entities.size() * 2 + 16);

    for (const auto &entity : entities) {
        adapters.emplace_back(entity, GISApp::UI::Renderers::ComplexEntityMapFeatureAdapter::GeometryRole::Primary);

        if (entity.entityType() == 2 || entity.entityType() == 3 || entity.entityType() == 7 || entity.entityType() == 8) {
            adapters.emplace_back(entity, GISApp::UI::Renderers::ComplexEntityMapFeatureAdapter::GeometryRole::LabelAnchor);
        }

        // When actively editing an entity on overlay, project yellow dot control points!
        if (m_editingEntityId != 0 && entity.Id() == m_editingEntityId) {
            const auto &pts = entity.locationPoints();
            for (int i = 0; i < pts.size(); ++i) {
                adapters.emplace_back(entity, i, (i == m_selectedControlPointIndex));
            }
        }
    }

    QVector<const GISApp::Core::Interfaces::IMapFeature*> features;
    features.reserve(adapters.size());
    for (const auto &adapter : adapters) {
        features.append(&adapter);
    }

    m_renderer->renderFeatures(features);
}

void ComplexEntityController::refreshEntityRendering()
{
    if (m_service) {
        onEntitiesUpdated(m_service->getAllEntities());
    }
}

void ComplexEntityController::onEntityUpdated(const GISApp::Domain::ComplexEntities::ComplexEntity &entity)
{
    quint32 entityId = entity.Id();
    if (m_detailDialogs.contains(entityId) && m_detailDialogs[entityId]) {
        m_detailDialogs[entityId]->updateEntityData(entity);
    }
    if (m_editDialogs.contains(entityId) && m_editDialogs[entityId]) {
        m_editDialogs[entityId]->updateEntityData(entity);
    }
}

void ComplexEntityController::onEntityRemoved(quint32 entityId)
{
    if (m_detailDialogs.contains(entityId)) {
        if (m_detailDialogs[entityId]) {
            m_detailDialogs[entityId]->close();
            m_detailDialogs[entityId]->deleteLater();
        }
        m_detailDialogs.remove(entityId);
    }
    if (m_editDialogs.contains(entityId)) {
        if (m_editDialogs[entityId]) {
            m_editDialogs[entityId]->close();
            m_editDialogs[entityId]->deleteLater();
        }
        m_editDialogs.remove(entityId);
    }
}

void ComplexEntityController::onEntitySelected(double latitude, double longitude)
{
    if (m_mapController) {
        qDebug() << "[ComplexEntityController] Centering map camera on complex entity at:" << latitude << longitude;
        m_mapController->setCenter(latitude, longitude);
    }
}

void ComplexEntityController::showEntityDetails(quint32 entityId)
{
    if (!m_service) return;

    auto optEntity = m_service->getEntityById(static_cast<int>(entityId));
    if (!optEntity.has_value()) {
        qWarning() << "[ComplexEntityController] Cannot show details; entity not found:" << entityId;
        return;
    }

    if (m_detailDialogs.contains(entityId) && m_detailDialogs[entityId]) {
        m_detailDialogs[entityId]->updateEntityData(optEntity.value());
        m_detailDialogs[entityId]->raise();
        m_detailDialogs[entityId]->activateWindow();
        return;
    }

    auto *dialog = new GISApp::UI::ComplexEntities::ComplexEntityDetailDialog(optEntity.value());
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(dialog, &GISApp::UI::ComplexEntities::ComplexEntityDetailDialog::centerRequested,
            this, &ComplexEntityController::onEntitySelected);

    connect(dialog, &GISApp::UI::ComplexEntities::ComplexEntityDetailDialog::editRequested,
            this, &ComplexEntityController::showEditEntityDialog);

    connect(dialog, &QObject::destroyed, this, [this, entityId]() {
        m_detailDialogs.remove(entityId);
    });

    m_detailDialogs[entityId] = dialog;
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void ComplexEntityController::showEditEntityDialog(quint32 entityId)
{
    if (!m_service) return;

    auto optEntity = m_service->getEntityById(static_cast<int>(entityId));
    if (!optEntity.has_value()) {
        qWarning() << "[ComplexEntityController] Cannot show editor; entity not found:" << entityId;
        return;
    }

    if (m_editDialogs.contains(entityId) && m_editDialogs[entityId]) {
        m_editDialogs[entityId]->updateEntityData(optEntity.value());
        m_editDialogs[entityId]->raise();
        m_editDialogs[entityId]->activateWindow();
        return;
    }

    auto *dialog = new GISApp::UI::ComplexEntities::ComplexEntityEditDialog(optEntity.value());
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(dialog, &GISApp::UI::ComplexEntities::ComplexEntityEditDialog::centerRequested,
            this, &ComplexEntityController::onEntitySelected);

    connect(dialog, &GISApp::UI::ComplexEntities::ComplexEntityEditDialog::entitySaved,
            this, [this](const GISApp::Domain::ComplexEntities::ComplexEntity &updatedEntity) {
                if (m_service) {
                    m_service->updateEntity(updatedEntity);
                    qDebug() << "[ComplexEntityController] ✅ Saved edited complex entity:" << updatedEntity.Id();
                }
            });

    connect(dialog, &GISApp::UI::ComplexEntities::ComplexEntityEditDialog::moveOnOverlayRequested,
            this, &ComplexEntityController::startLocationEditing);

    connect(dialog, &GISApp::UI::ComplexEntities::ComplexEntityEditDialog::editControlPointsRequested,
            this, &ComplexEntityController::startControlPointEditing);

    connect(dialog, &QObject::destroyed, this, [this, entityId]() {
        m_editDialogs.remove(entityId);
    });

    m_editDialogs[entityId] = dialog;
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void ComplexEntityController::setMapViewContainer(GISApp::UI::MapViewContainer *mapViewContainer)
{
    if (m_mapViewContainer && m_mapViewContainer->mapWidget()) {
        m_mapViewContainer->mapWidget()->removeInteractionListener(this);
    }

    m_mapViewContainer = mapViewContainer;
    if (!m_mapViewContainer) {
        return;
    }

    auto *overlay = m_mapViewContainer->locationEditOverlay();
    if (overlay) {
        connect(overlay, &GISApp::UI::ComplexEntities::ComplexEntityLocationEditOverlay::positionShifted,
                this, &ComplexEntityController::onLocationPositionShifted);
        connect(overlay, &GISApp::UI::ComplexEntities::ComplexEntityLocationEditOverlay::controlPointModified,
                this, &ComplexEntityController::onControlPointModified);
        connect(overlay, &GISApp::UI::ComplexEntities::ComplexEntityLocationEditOverlay::controlPointAdded,
                this, &ComplexEntityController::onControlPointAdded);
        connect(overlay, &GISApp::UI::ComplexEntities::ComplexEntityLocationEditOverlay::controlPointRemoved,
                this, &ComplexEntityController::onControlPointRemoved);
        connect(overlay, &GISApp::UI::ComplexEntities::ComplexEntityLocationEditOverlay::controlPointSelected,
                this, [this](int idx) {
                    m_selectedControlPointIndex = idx;
                    refreshEntityRendering();
                });
        connect(overlay, &GISApp::UI::ComplexEntities::ComplexEntityLocationEditOverlay::saveRequested,
                this, &ComplexEntityController::onLocationEditSaved);
        connect(overlay, &GISApp::UI::ComplexEntities::ComplexEntityLocationEditOverlay::resetRequested,
                this, &ComplexEntityController::onLocationEditReset);
        connect(overlay, &GISApp::UI::ComplexEntities::ComplexEntityLocationEditOverlay::cancelRequested,
                this, &ComplexEntityController::onLocationEditCancelled);
        connect(overlay, &GISApp::UI::ComplexEntities::ComplexEntityLocationEditOverlay::allPointsModified,
                this, &ComplexEntityController::onAllPointsModified);
    }

    auto *mapWidget = m_mapViewContainer->mapWidget();
    if (mapWidget) {
        mapWidget->addInteractionListener(this);
        connect(mapWidget, &GISApp::UI::MapWidget::coordinateClicked, this, [this](const GISApp::Core::Models::GeoCoordinate &coord) {
            if (m_mapViewContainer && m_mapViewContainer->locationEditOverlay() &&
                m_mapViewContainer->locationEditOverlay()->isMapClickModeActive() &&
                !m_isDraggingControlPoint) {
                m_mapViewContainer->locationEditOverlay()->handleMapCoordinateClicked(coord.latitude(), coord.longitude());
            }
        });
    }
}

void ComplexEntityController::startLocationEditing(quint32 entityId)
{
    if (!m_service || !m_mapViewContainer) return;

    auto optEntity = m_service->getEntityById(static_cast<int>(entityId));
    if (!optEntity.has_value()) {
        qWarning() << "[ComplexEntityController] Cannot edit location; entity not found:" << entityId;
        return;
    }

    // Backup original state
    m_initialEditEntities[entityId] = optEntity.value();
    m_editingEntityId = entityId;
    m_selectedControlPointIndex = 0;

    auto *overlay = m_mapViewContainer->locationEditOverlay();
    if (overlay) {
        overlay->startEditing(optEntity.value(), GISApp::UI::ComplexEntities::ComplexEntityLocationEditOverlay::EditMode::MoveEntireEntity);
        m_mapViewContainer->updateOverlayPositions();
    }
    refreshEntityRendering();
}

void ComplexEntityController::startControlPointEditing(quint32 entityId)
{
    if (!m_service || !m_mapViewContainer) return;

    auto optEntity = m_service->getEntityById(static_cast<int>(entityId));
    if (!optEntity.has_value()) {
        qWarning() << "[ComplexEntityController] Cannot edit control points; entity not found:" << entityId;
        return;
    }

    // Backup original state
    m_initialEditEntities[entityId] = optEntity.value();
    m_editingEntityId = entityId;
    m_selectedControlPointIndex = 0;

    auto *overlay = m_mapViewContainer->locationEditOverlay();
    if (overlay) {
        overlay->startEditing(optEntity.value(), GISApp::UI::ComplexEntities::ComplexEntityLocationEditOverlay::EditMode::EditControlPoints);
        m_mapViewContainer->updateOverlayPositions();
    }
    refreshEntityRendering();
}

void ComplexEntityController::onLocationPositionShifted(quint32 entityId, double deltaLat, double deltaLon)
{
    if (!m_service) return;

    auto optEntity = m_service->getEntityById(static_cast<int>(entityId));
    if (!optEntity.has_value()) return;

    // Rigid translation: shift all points by the exact same delta
    GISApp::Domain::ComplexEntities::ComplexEntity moved = optEntity.value();
    auto pts = moved.locationPoints();
    for (auto &p : pts) {
        p.latatitude += deltaLat;
        p.longitude += deltaLon;
    }
    moved.setLocationPoints(pts);

    // Update in-memory and trigger live GPU map re-rendering
    m_service->updateEntity(moved);

    // Also update any open edit dialogs
    if (m_editDialogs.contains(entityId) && m_editDialogs[entityId]) {
        m_editDialogs[entityId]->updateEntityData(moved);
    }
    if (m_detailDialogs.contains(entityId) && m_detailDialogs[entityId]) {
        m_detailDialogs[entityId]->updateEntityData(moved);
    }
}

void ComplexEntityController::onControlPointModified(quint32 entityId, int pointIndex, double newLat, double newLon)
{
    if (!m_service) return;

    auto optEntity = m_service->getEntityById(static_cast<int>(entityId));
    if (!optEntity.has_value()) return;

    GISApp::Domain::ComplexEntities::ComplexEntity modified = optEntity.value();
    auto pts = modified.locationPoints();
    if (pointIndex >= 0 && pointIndex < pts.size()) {
        pts[pointIndex].latatitude = newLat;
        pts[pointIndex].longitude = newLon;
        modified.setLocationPoints(pts);

        m_service->updateEntity(modified);
        refreshEntityRendering();

        if (m_editDialogs.contains(entityId) && m_editDialogs[entityId]) {
            m_editDialogs[entityId]->updateEntityData(modified);
        }
        if (m_detailDialogs.contains(entityId) && m_detailDialogs[entityId]) {
            m_detailDialogs[entityId]->updateEntityData(modified);
        }
        qDebug() << "[ComplexEntityController] Modified control point #" << (pointIndex + 1)
                 << "for entity:" << entityId << "->" << newLat << newLon;
    }
}

void ComplexEntityController::onControlPointAdded(quint32 entityId, int afterIndex, double lat, double lon)
{
    if (!m_service) return;

    auto optEntity = m_service->getEntityById(static_cast<int>(entityId));
    if (!optEntity.has_value()) return;

    GISApp::Domain::ComplexEntities::ComplexEntity modified = optEntity.value();
    auto pts = modified.locationPoints();
    STRUCT_LOCATION newLoc{};
    newLoc.latatitude = lat;
    newLoc.longitude = lon;
    newLoc.height = (afterIndex >= 0 && afterIndex < pts.size()) ? pts[afterIndex].height : 850.0;
    newLoc.dir = 0.0;

    int insertIdx = std::clamp(afterIndex + 1, 0, static_cast<int>(pts.size()));
    pts.insert(insertIdx, newLoc);
    modified.setLocationPoints(pts);

    m_service->updateEntity(modified);

    if (m_editDialogs.contains(entityId) && m_editDialogs[entityId]) {
        m_editDialogs[entityId]->updateEntityData(modified);
    }
    if (m_detailDialogs.contains(entityId) && m_detailDialogs[entityId]) {
        m_detailDialogs[entityId]->updateEntityData(modified);
    }
    qDebug() << "[ComplexEntityController] Inserted new control point at index:" << insertIdx
             << "for entity:" << entityId;
}

void ComplexEntityController::onControlPointRemoved(quint32 entityId, int pointIndex)
{
    if (!m_service) return;

    auto optEntity = m_service->getEntityById(static_cast<int>(entityId));
    if (!optEntity.has_value()) return;

    GISApp::Domain::ComplexEntities::ComplexEntity modified = optEntity.value();
    auto pts = modified.locationPoints();
    if (pointIndex >= 0 && pointIndex < pts.size()) {
        pts.removeAt(pointIndex);
        modified.setLocationPoints(pts);

        m_service->updateEntity(modified);

        if (m_editDialogs.contains(entityId) && m_editDialogs[entityId]) {
            m_editDialogs[entityId]->updateEntityData(modified);
        }
        if (m_detailDialogs.contains(entityId) && m_detailDialogs[entityId]) {
            m_detailDialogs[entityId]->updateEntityData(modified);
        }
        qDebug() << "[ComplexEntityController] Deleted control point at index:" << pointIndex
                 << "for entity:" << entityId;
    }
}

void ComplexEntityController::onAllPointsModified(quint32 entityId, const QVector<STRUCT_LOCATION> &newPoints)
{
    if (!m_service) return;

    auto optEntity = m_service->getEntityById(static_cast<int>(entityId));
    if (!optEntity.has_value()) return;

    GISApp::Domain::ComplexEntities::ComplexEntity modified = optEntity.value();
    modified.setLocationPoints(newPoints);
    m_service->updateEntity(modified);
    refreshEntityRendering();

    if (m_editDialogs.contains(entityId) && m_editDialogs[entityId]) {
        m_editDialogs[entityId]->updateEntityData(modified);
    }
    if (m_detailDialogs.contains(entityId) && m_detailDialogs[entityId]) {
        m_detailDialogs[entityId]->updateEntityData(modified);
    }
    qDebug() << "[ComplexEntityController] Bulk-updated all" << newPoints.size()
             << "control points for entity:" << entityId;
}

void ComplexEntityController::onLocationEditSaved(quint32 entityId)
{
    m_initialEditEntities.remove(entityId);
    m_editingEntityId = 0;
    m_isDraggingControlPoint = false;
    m_draggingPointIndex = -1;
    if (m_mapViewContainer && m_mapViewContainer->mapWidget()) {
        m_mapViewContainer->mapWidget()->unsetCursor();
    }
    m_isHoveringControlPoint = false;
    refreshEntityRendering();
    qDebug() << "[ComplexEntityController] ✅ Saved relocated complex entity:" << entityId;
}

void ComplexEntityController::onLocationEditReset(quint32 entityId)
{
    if (!m_service || !m_initialEditEntities.contains(entityId)) return;

    GISApp::Domain::ComplexEntities::ComplexEntity orig = m_initialEditEntities.value(entityId);
    m_service->updateEntity(orig);

    if (m_mapViewContainer && m_mapViewContainer->locationEditOverlay()) {
        m_mapViewContainer->locationEditOverlay()->updateEntityData(orig);
    }
    if (m_editDialogs.contains(entityId) && m_editDialogs[entityId]) {
        m_editDialogs[entityId]->updateEntityData(orig);
    }
    if (m_detailDialogs.contains(entityId) && m_detailDialogs[entityId]) {
        m_detailDialogs[entityId]->updateEntityData(orig);
    }
    refreshEntityRendering();
    qDebug() << "[ComplexEntityController] Reverted entity location to initial:" << entityId;
}

void ComplexEntityController::onLocationEditCancelled(quint32 entityId)
{
    if (!m_service || !m_initialEditEntities.contains(entityId)) return;

    m_editingEntityId = 0;
    m_isDraggingControlPoint = false;
    m_draggingPointIndex = -1;
    if (m_mapViewContainer && m_mapViewContainer->mapWidget()) {
        m_mapViewContainer->mapWidget()->unsetCursor();
    }
    m_isHoveringControlPoint = false;

    GISApp::Domain::ComplexEntities::ComplexEntity orig = m_initialEditEntities.take(entityId);
    m_service->updateEntity(orig);

    if (m_editDialogs.contains(entityId) && m_editDialogs[entityId]) {
        m_editDialogs[entityId]->updateEntityData(orig);
    }
    if (m_detailDialogs.contains(entityId) && m_detailDialogs[entityId]) {
        m_detailDialogs[entityId]->updateEntityData(orig);
    }
    refreshEntityRendering();
    qDebug() << "[ComplexEntityController] Cancelled entity relocation:" << entityId;
}

void ComplexEntityController::centerOnEntity(quint32 entityId)
{
    if (!m_service) return;

    auto optEntity = m_service->getEntityById(static_cast<int>(entityId));
    if (optEntity.has_value()) {
        const auto &pts = optEntity.value().locationPoints();
        if (!pts.isEmpty()) {
            onEntitySelected(pts.first().latatitude, pts.first().longitude);
        }
    }
}

void ComplexEntityController::deleteEntity(quint32 entityId)
{
    if (m_service) {
        m_service->deleteEntity(static_cast<int>(entityId));
    }
}

bool ComplexEntityController::contributeActions(QMenu *parentMenu, const QPoint &screenPos, const QPointF &geoCoord)
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
        Domain::ComplexEntities::ComplexEntity entity;
        double distancePx;
    };
    QVector<EntityHit> hits;

    constexpr double hitRadiusPx = 24.0;

    for (const auto &ent : entities) {
        const auto &pts = ent.locationPoints();
        if (pts.isEmpty()) continue;

        double minDist = 1e9;

        if (mapCore) {
            QVector<QPointF> screenPts;
            screenPts.reserve(pts.size());
            for (const auto &p : pts) {
                screenPts.append(mapCore->pixelForCoordinate(QMapLibre::Coordinate(p.latatitude, p.longitude)));
            }

            quint8 eType = ent.type();
            if (eType == 2 || eType == 7) {
                // Line & Formation Boundary: distance to line segments across group of points
                for (int i = 0; i < screenPts.size() - 1; ++i) {
                    double d = distanceToSegment(QPointF(screenPos), screenPts[i], screenPts[i + 1]);
                    if (d < minDist) minDist = d;
                }
            } else if (eType == 3 || eType == 8) {
                // Polygon & Deployment Area: point-in-polygon or distance to boundary
                if (isPointInPolygon(QPointF(screenPos), screenPts)) {
                    minDist = 0.0;
                } else {
                    for (int i = 0; i < screenPts.size(); ++i) {
                        int next = (i + 1) % screenPts.size();
                        double d = distanceToSegment(QPointF(screenPos), screenPts[i], screenPts[next]);
                        if (d < minDist) minDist = d;
                    }
                }
            } else {
                // Point, Text, Custom Image, Custom Painter: check distance to primary anchor
                double dx = screenPts.first().x() - screenPos.x();
                double dy = screenPts.first().y() - screenPos.y();
                minDist = std::sqrt(dx * dx + dy * dy);
            }
        } else {
            // Fallback geographic delta
            double eLat = pts.first().latatitude;
            double eLon = pts.first().longitude;
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
        quint32 entityId = ent.Id();
        const auto &pts = ent.locationPoints();
        double eLat = 0.0, eLon = 0.0;
        if (ent.type() == 7 || ent.type() == 2) {
            if (!ent.calculateMidpoint(eLat, eLon)) {
                eLat = pts.isEmpty() ? 0.0 : pts.first().latatitude;
                eLon = pts.isEmpty() ? 0.0 : pts.first().longitude;
            }
        } else if (!ent.calculateCentroid(eLat, eLon)) {
            eLat = pts.isEmpty() ? 0.0 : pts.first().latatitude;
            eLon = pts.isEmpty() ? 0.0 : pts.first().longitude;
        }

        QString typeIcon;
        QString typeDesc;
        switch (ent.type()) {
        case 1:
            typeIcon = QStringLiteral("🟢");
            typeDesc = QStringLiteral("Point");
            break;
        case 2:
            typeIcon = QStringLiteral("〰️");
            typeDesc = QStringLiteral("Line");
            break;
        case 3:
            typeIcon = QStringLiteral("⬡");
            typeDesc = QStringLiteral("Polygon");
            break;
        case 4:
            typeIcon = QStringLiteral("🔤");
            typeDesc = QStringLiteral("Text Only");
            break;
        case 5:
            typeIcon = QStringLiteral("🖼️");
            typeDesc = QStringLiteral("Custom Image");
            break;
        case 6:
            typeIcon = QStringLiteral("🎨");
            typeDesc = QStringLiteral("Custom Painter");
            break;
        case 7:
            typeIcon = QStringLiteral("⚔️");
            typeDesc = QStringLiteral("Formation Boundary");
            break;
        case 8:
            typeIcon = QStringLiteral("🛡️");
            typeDesc = QStringLiteral("Deployment Area");
            break;
        default:
            typeIcon = QStringLiteral("💠");
            typeDesc = QStringLiteral("Type %1").arg(ent.type());
            break;
        }

        QString displayName = ent.Name().isEmpty() ? QStringLiteral("CPLX-%1").arg(entityId) : ent.Name();

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

        // 1b. Edit Entity over Overlay
        QAction *editAct = entityMenu->addAction(QStringLiteral("✏️ Edit Entity"));
        connect(editAct, &QAction::triggered, this, [this, entityId]() {
            showEditEntityDialog(entityId);
        });

        // 1c. Move Entity (Preserve Shape)
        QAction *moveAct = entityMenu->addAction(QStringLiteral("📍 Move Entity (Preserve Shape)"));
        connect(moveAct, &QAction::triggered, this, [this, entityId]() {
            startLocationEditing(entityId);
        });

        // 1d. Edit Control Points (Geometry Modification)
        QAction *ctrlPtsAct = entityMenu->addAction(QStringLiteral("📐 Edit Control Points"));
        connect(ctrlPtsAct, &QAction::triggered, this, [this, entityId]() {
            startControlPointEditing(entityId);
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
            qDebug() << "[ComplexEntityController] Copied coordinates to clipboard:" << coordStr;
        });

        // 4. Request Complex Entities...
        QAction *reqAct = entityMenu->addAction(QStringLiteral("🔍 Request Complex Entities..."));
        connect(reqAct, &QAction::triggered, this, [this]() {
            emit requestComplexEntitiesTriggered();
        });

        entityMenu->addSeparator();

        // 5. Delete Entity (Placeholder)
        QAction *deleteAct = entityMenu->addAction(QStringLiteral("🗑️ Delete Entity (Placeholder)"));
        deleteAct->setEnabled(false);
        deleteAct->setToolTip(QStringLiteral("Entity deletion is currently disabled"));
    }

    return true;
}

bool ComplexEntityController::onMapMousePress(const QPointF &screenPos, const QPointF &geoCoord, Qt::MouseButton button)
{
    if (m_editingEntityId == 0 || button != Qt::LeftButton || !m_service) {
        return false;
    }

    auto optEntity = m_service->getEntityById(static_cast<int>(m_editingEntityId));
    if (!optEntity.has_value()) {
        return false;
    }

    const auto &entity = optEntity.value();
    const auto &pts = entity.locationPoints();
    if (pts.isEmpty()) {
        return false;
    }

    QMapLibre::Map *map = nullptr;
    if (m_mapController && m_mapController->attachedMap()) {
        map = m_mapController->attachedMap()->rawMap();
    } else if (m_mapViewContainer && m_mapViewContainer->mapWidget()) {
        map = m_mapViewContainer->mapWidget()->rawMap();
    }
    if (!map) {
        return false;
    }

    // Comfortable hit radius in pixels for yellow control point dots
    constexpr double HIT_TOLERANCE_PX = 16.0;

    int hitIndex = -1;
    double minDistance = 1e9;

    for (int i = 0; i < pts.size(); ++i) {
        QPointF ptScreen = map->pixelForCoordinate(QMapLibre::Coordinate(pts[i].latatitude, pts[i].longitude));
        double dist = std::hypot(screenPos.x() - ptScreen.x(), screenPos.y() - ptScreen.y());
        if (dist <= HIT_TOLERANCE_PX && dist < minDistance) {
            minDistance = dist;
            hitIndex = i;
        }
    }

    if (hitIndex >= 0) {
        m_isDraggingControlPoint = true;
        m_draggingPointIndex = hitIndex;
        m_selectedControlPointIndex = hitIndex;
        m_dragLastGeo = geoCoord;

        // Synchronize ribbon overlay point selector and save undo snapshot for the drag
        if (m_mapViewContainer && m_mapViewContainer->locationEditOverlay()) {
            m_mapViewContainer->locationEditOverlay()->saveUndoSnapshot();
            m_mapViewContainer->locationEditOverlay()->setSelectedPointIndex(hitIndex);
        }

        if (m_mapViewContainer && m_mapViewContainer->mapWidget()) {
            m_mapViewContainer->mapWidget()->setCursor(Qt::ClosedHandCursor);
        }

        refreshEntityRendering();
        qDebug() << "[ComplexEntityController] 🎯 Grabbed control point #" << (hitIndex + 1)
                 << "for dragging on entity:" << m_editingEntityId;
        return true; // Consumed: suppress MapLibre default canvas pan
    }

    return false;
}

bool ComplexEntityController::onMapMouseMove(const QPointF &screenPos, const QPointF &geoCoord, Qt::MouseButtons buttons)
{
    if (m_editingEntityId == 0 || !m_service) {
        return false;
    }

    // 1. Active Dragging
    if (m_isDraggingControlPoint && (buttons & Qt::LeftButton)) {
        auto *overlay = m_mapViewContainer ? m_mapViewContainer->locationEditOverlay() : nullptr;
        bool isMoveMode = overlay && (overlay->editMode() == GISApp::UI::ComplexEntities::ComplexEntityLocationEditOverlay::EditMode::MoveEntireEntity);

        if (isMoveMode) {
            // Rigid translation: move entire entity preserving 100% shape
            double deltaLat = geoCoord.x() - m_dragLastGeo.x();
            double deltaLon = geoCoord.y() - m_dragLastGeo.y();
            m_dragLastGeo = geoCoord;

            if (std::abs(deltaLat) > 1e-9 || std::abs(deltaLon) > 1e-9) {
                onLocationPositionShifted(m_editingEntityId, deltaLat, deltaLon);
                auto optEnt = m_service->getEntityById(static_cast<int>(m_editingEntityId));
                if (optEnt.has_value() && overlay) {
                    overlay->updateEntityData(optEnt.value());
                }
            }
        } else {
            // Control points mode: move only the dragged vertex
            if (m_draggingPointIndex >= 0) {
                onControlPointModified(m_editingEntityId, m_draggingPointIndex, geoCoord.x(), geoCoord.y());
                m_dragLastGeo = geoCoord;
                auto optEnt = m_service->getEntityById(static_cast<int>(m_editingEntityId));
                if (optEnt.has_value() && overlay) {
                    overlay->updateEntityData(optEnt.value());
                }
            }
        }
        return true; // Consumed drag event
    }

    // 2. Passive Hover (Change cursor to pointing hand when hovering over a yellow control point)
    if (buttons == Qt::NoButton) {
        auto optEntity = m_service->getEntityById(static_cast<int>(m_editingEntityId));
        if (!optEntity.has_value()) return false;

        QMapLibre::Map *map = nullptr;
        if (m_mapController && m_mapController->attachedMap()) {
            map = m_mapController->attachedMap()->rawMap();
        } else if (m_mapViewContainer && m_mapViewContainer->mapWidget()) {
            map = m_mapViewContainer->mapWidget()->rawMap();
        }
        if (!map) return false;

        constexpr double HOVER_TOLERANCE_PX = 16.0;
        bool nearControlPoint = false;
        const auto &pts = optEntity.value().locationPoints();

        for (int i = 0; i < pts.size(); ++i) {
            QPointF ptScreen = map->pixelForCoordinate(QMapLibre::Coordinate(pts[i].latatitude, pts[i].longitude));
            double dist = std::hypot(screenPos.x() - ptScreen.x(), screenPos.y() - ptScreen.y());
            if (dist <= HOVER_TOLERANCE_PX) {
                nearControlPoint = true;
                break;
            }
        }

        if (nearControlPoint && !m_isHoveringControlPoint) {
            m_isHoveringControlPoint = true;
            if (m_mapViewContainer && m_mapViewContainer->mapWidget()) {
                m_mapViewContainer->mapWidget()->setCursor(Qt::PointingHandCursor);
            }
        } else if (!nearControlPoint && m_isHoveringControlPoint) {
            m_isHoveringControlPoint = false;
            if (m_mapViewContainer && m_mapViewContainer->mapWidget()) {
                m_mapViewContainer->mapWidget()->unsetCursor();
            }
        }
    }

    return false;
}

bool ComplexEntityController::onMapMouseRelease(const QPointF &screenPos, const QPointF &geoCoord, Qt::MouseButton button)
{
    Q_UNUSED(screenPos);
    Q_UNUSED(geoCoord);
    Q_UNUSED(button);

    if (m_isDraggingControlPoint) {
        m_isDraggingControlPoint = false;
        m_draggingPointIndex = -1;
        if (m_mapViewContainer && m_mapViewContainer->mapWidget()) {
            m_mapViewContainer->mapWidget()->unsetCursor();
        }
        m_isHoveringControlPoint = false;
        refreshEntityRendering();
        qDebug() << "[ComplexEntityController] ✅ Finished control point drag on entity:" << m_editingEntityId;
        return true;
    }

    return false;
}

} // namespace GISApp::Controllers::ComplexEntities
