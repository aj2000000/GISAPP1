/**
 * @file MapWidget.cpp
 * @brief Implementation of MapWidget wrapping MapLibre Native Qt QRhiWidget.
 */

#include "MapWidget.h"

#include <QMapLibreWidgets/MapWidget>
#include <QMapLibre/Map>
#include <QMapLibre/Settings>
#include <QMapLibre/Types>

#include <QVBoxLayout>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <QMouseEvent>
#include <QDebug>
#include <cmath>
#include <algorithm>

namespace GISApp::UI {

MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent)
    , m_nativeMapWidget(nullptr)
    , m_lastReportedZoom(4.0)
    , m_lastReportedBearing(0.0)
{
    setObjectName("GISAppMapWidget");
    setupMapLibre();
    loadInitialStyle();
}

MapWidget::~MapWidget()
{
}

void MapWidget::setupMapLibre()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Initialize MapLibre Native Qt Settings
    QMapLibre::Settings settings(QMapLibre::Settings::MapLibreProvider);
    settings.setDefaultCoordinate(QMapLibre::Coordinate(28.6139, 77.2090)); // Default: New Delhi / India
    settings.setDefaultZoom(4.0);

    // Construct native QRhiWidget-based MapWidget
    m_nativeMapWidget = new QMapLibre::MapWidget(settings);
    m_nativeMapWidget->setObjectName("NativeMapLibreWidget");

    // Enable continuous mouse tracking for live hover telemetry
    m_nativeMapWidget->setMouseTracking(true);
    setMouseTracking(true);

    // Install event filter on native MapLibre widget to intercept mouse hover events
    m_nativeMapWidget->installEventFilter(this);

    layout->addWidget(m_nativeMapWidget);

    // Forward native mouse hover coordinates
    connect(m_nativeMapWidget, &QMapLibre::MapWidget::onMouseMoveEvent, this,
            [this](QMapLibre::Coordinate coordinate) {
                if (std::isnan(coordinate.first) || std::isnan(coordinate.second)) return;
                if (coordinate.first < -90.0 || coordinate.first > 90.0) return;
                if (coordinate.second < -180.0 || coordinate.second > 180.0) return;

                emit coordinateHoveredRaw(coordinate.first, coordinate.second);
                emit coordinateHovered(GISApp::Core::Models::GeoCoordinate(
                    coordinate.first, coordinate.second, 0.0, true));
            });

    // Forward mouse click events
    connect(m_nativeMapWidget, &QMapLibre::MapWidget::onMousePressEvent, this,
            [this](QMapLibre::Coordinate coordinate) {
                emit coordinateClicked(GISApp::Core::Models::GeoCoordinate(
                    coordinate.first, coordinate.second, 0.0, true));
            });

    // Connect map state changes (zoom, bearing, loading)
    if (m_nativeMapWidget->map()) {
        connect(m_nativeMapWidget->map(), &QMapLibre::Map::mapChanged, this,
                [this](QMapLibre::Map::MapChange change) {
                    onMapLibreChange(static_cast<int>(change));
                });
    }
}

bool MapWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_nativeMapWidget && event->type() == QEvent::MouseMove) {
        auto *mouseEvent = static_cast<QMouseEvent*>(event);
        handleHoverPosition(mouseEvent->position());
    }
    return QWidget::eventFilter(watched, event);
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    handleHoverPosition(event->position());
    QWidget::mouseMoveEvent(event);
}

void MapWidget::handleHoverPosition(const QPointF &pos)
{
    if (!m_nativeMapWidget || !m_nativeMapWidget->map()) {
        return;
    }

    QMapLibre::Coordinate coord = m_nativeMapWidget->map()->coordinateForPixel(pos);
    if (std::isnan(coord.first) || std::isnan(coord.second)) {
        return;
    }

    if (coord.first < -90.0 || coord.first > 90.0) return;
    if (coord.second < -180.0 || coord.second > 180.0) return;

    emit coordinateHoveredRaw(coord.first, coord.second);
    emit coordinateHovered(GISApp::Core::Models::GeoCoordinate(coord.first, coord.second, 0.0, true));
}

void MapWidget::loadInitialStyle()
{
    if (!m_nativeMapWidget || !m_nativeMapWidget->map()) {
        return;
    }

    // Default style: MapLibre demo tiles or fallback local style
    QString defaultOnlineStyle = "https://demotiles.maplibre.org/style.json";

    // Check for local offline style file across standard relative development and deployment paths
    QString appDir = QCoreApplication::applicationDirPath();
    QString cwd = QDir::currentPath();
    QStringList searchPaths = {
        appDir + "/../../resources/map/styles/tactical_dark_fallback.json",
        appDir + "/../resources/map/styles/tactical_dark_fallback.json",
        appDir + "/resources/map/styles/tactical_dark_fallback.json",
        cwd + "/resources/map/styles/tactical_dark_fallback.json",
        cwd + "/GISLITEAPP/resources/map/styles/tactical_dark_fallback.json",
        ":/map/styles/tactical_dark_fallback.json"
    };

    QString localStylePath;
    for (const QString &path : searchPaths) {
        if (QFile::exists(path)) {
            localStylePath = QFileInfo(path).canonicalFilePath();
            break;
        }
    }

    // Prioritize local offline fallback style to ensure robust offline functionality
    if (!localStylePath.isEmpty()) {
        QFile file(localStylePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString jsonContent = QString::fromUtf8(file.readAll());
            file.close();
            m_nativeMapWidget->map()->setStyleJson(jsonContent);
            qInfo() << "[MapWidget] Successfully loaded offline fallback style from:" << localStylePath;
            return;
        }
    }

    // Fallback to online style if local file could not be read
    m_nativeMapWidget->map()->setStyleUrl(defaultOnlineStyle);
    qInfo() << "[MapWidget] Using default online style URL:" << defaultOnlineStyle;
}

void MapWidget::onMapLibreChange(int change)
{
    emit mapChanged(change);

    if (m_nativeMapWidget && m_nativeMapWidget->map()) {
        double currentZoom = m_nativeMapWidget->map()->zoom();
        double currentBearing = m_nativeMapWidget->map()->bearing();

        if (std::abs(currentZoom - m_lastReportedZoom) > 0.001) {
            m_lastReportedZoom = currentZoom;
            emit zoomChanged(currentZoom);
        }

        if (std::abs(currentBearing - m_lastReportedBearing) > 0.01) {
            m_lastReportedBearing = currentBearing;
            emit bearingChanged(currentBearing);
        }
    }
}

void MapWidget::zoomIn(double delta)
{
    if (m_nativeMapWidget && m_nativeMapWidget->map()) {
        double nextZoom = m_nativeMapWidget->map()->zoom() + delta;
        m_nativeMapWidget->map()->setZoom(nextZoom);
    }
}

void MapWidget::zoomOut(double delta)
{
    if (m_nativeMapWidget && m_nativeMapWidget->map()) {
        double nextZoom = std::max(0.0, m_nativeMapWidget->map()->zoom() - delta);
        m_nativeMapWidget->map()->setZoom(nextZoom);
    }
}

void MapWidget::setZoom(double zoom)
{
    if (m_nativeMapWidget && m_nativeMapWidget->map()) {
        m_nativeMapWidget->map()->setZoom(std::max(0.0, zoom));
    }
}

double MapWidget::zoom() const
{
    if (m_nativeMapWidget && m_nativeMapWidget->map()) {
        return m_nativeMapWidget->map()->zoom();
    }
    return m_lastReportedZoom;
}

void MapWidget::setBearing(double degrees)
{
    if (m_nativeMapWidget && m_nativeMapWidget->map()) {
        m_nativeMapWidget->map()->setBearing(degrees);
    }
}

double MapWidget::bearing() const
{
    if (m_nativeMapWidget && m_nativeMapWidget->map()) {
        return m_nativeMapWidget->map()->bearing();
    }
    return m_lastReportedBearing;
}

void MapWidget::resetBearing()
{
    setBearing(0.0);
}

void MapWidget::setCenter(double latitude, double longitude)
{
    if (m_nativeMapWidget && m_nativeMapWidget->map()) {
        m_nativeMapWidget->map()->setCoordinate(QMapLibre::Coordinate(latitude, longitude));
    }
}

void MapWidget::setCenter(const GISApp::Core::Models::GeoCoordinate &coord)
{
    if (coord.isValid()) {
        setCenter(coord.latitude(), coord.longitude());
    }
}

GISApp::Core::Models::GeoCoordinate MapWidget::center() const
{
    if (m_nativeMapWidget && m_nativeMapWidget->map()) {
        QMapLibre::Coordinate c = m_nativeMapWidget->map()->coordinate();
        return GISApp::Core::Models::GeoCoordinate(c.first, c.second, 0.0, true);
    }
    return GISApp::Core::Models::GeoCoordinate();
}

void MapWidget::setStyleUrl(const QString &url)
{
    if (m_nativeMapWidget && m_nativeMapWidget->map()) {
        m_nativeMapWidget->map()->setStyleUrl(url);
    }
}

void MapWidget::setStyleJson(const QString &json)
{
    if (m_nativeMapWidget && m_nativeMapWidget->map()) {
        m_nativeMapWidget->map()->setStyleJson(json);
    }
}

QString MapWidget::styleUrl() const
{
    if (m_nativeMapWidget && m_nativeMapWidget->map()) {
        return m_nativeMapWidget->map()->styleUrl();
    }
    return QString();
}

QString MapWidget::styleJson() const
{
    if (m_nativeMapWidget && m_nativeMapWidget->map()) {
        return m_nativeMapWidget->map()->styleJson();
    }
    return QString();
}

QMapLibre::Map* MapWidget::rawMap() const
{
    return m_nativeMapWidget ? m_nativeMapWidget->map() : nullptr;
}

QMapLibre::MapWidget* MapWidget::nativeWidget() const
{
    return m_nativeMapWidget;
}

} // namespace GISApp::UI
