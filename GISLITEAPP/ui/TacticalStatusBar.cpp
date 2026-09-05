/**
 * @file TacticalStatusBar.cpp
 * @brief Implementation of TacticalStatusBar 3D telemetry display.
 */

#include "TacticalStatusBar.h"
#include <QLocale>
#include <cmath>

namespace GISApp::UI {

TacticalStatusBar::TacticalStatusBar(QWidget *parent)
    : QStatusBar(parent)
{
    setSizeGripEnabled(false);

    m_infoLabel = new QLabel("Coordinate System: WGS 84 / EPSG:4326 | Zoom: 10.00 | Scale: 1:507,210", this);
    m_infoLabel->setObjectName("InfoLabel");

    m_coordLabel = new QLabel("Lat: --.----° N  Lon: --.----° E  Alt: 0.0 m", this);
    m_coordLabel->setObjectName("CoordLabel");
    m_coordLabel->setMinimumWidth(380);
    m_coordLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    addWidget(m_infoLabel, 1);
    addPermanentWidget(m_coordLabel);
}

void TacticalStatusBar::updateCoordinates(const GISApp::Core::Models::GeoCoordinate &coord)
{
    if (!coord.isValid()) return;

    QString latDir = coord.latitude() >= 0 ? "N" : "S";
    QString lonDir = coord.longitude() >= 0 ? "E" : "W";

    QString coordText = QString("Lat: %1° %2  Lon: %3° %4  Alt: %5 m")
                            .arg(std::abs(coord.latitude()), 0, 'f', 4)
                            .arg(latDir)
                            .arg(std::abs(coord.longitude()), 0, 'f', 4)
                            .arg(lonDir)
                            .arg(coord.altitude(), 0, 'f', 1);

    m_coordLabel->setText(coordText);
}

void TacticalStatusBar::updateCoordinates(double latitude, double longitude, double altitude)
{
    updateCoordinates(GISApp::Core::Models::GeoCoordinate(latitude, longitude, altitude, true));
}

void TacticalStatusBar::updateScale(double scaleDenominator)
{
    updateZoomAndScale(0.0, scaleDenominator);
}

void TacticalStatusBar::updateZoomAndScale(double zoomLevel, double scaleDenominator)
{
    QString formattedScale = QLocale::system().toString(static_cast<long long>(scaleDenominator));
    m_infoLabel->setText(QString("Coordinate System: WGS 84 / EPSG:4326 | Zoom: %1 | Scale: 1:%2")
                             .arg(zoomLevel, 0, 'f', 2)
                             .arg(formattedScale));
}

} // namespace GISApp::UI
