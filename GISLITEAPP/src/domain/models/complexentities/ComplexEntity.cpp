/**
 * @file ComplexEntity.cpp
 * @brief Implementation of ComplexEntity domain entity and JSON serialization.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "ComplexEntity.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <cstring>
#include <cmath>

namespace GISApp::Domain::ComplexEntities {

ComplexEntity::ComplexEntity(UINT_32 id, const QString &name, UINT_8 type)
    : m_entityId(id)
    , m_entityName(name)
    , m_entityType(type)
{
}

STRUCT_LOCATION ComplexEntity::primaryLocation() const
{
    if (!m_locationPoints.isEmpty()) {
        return m_locationPoints.first();
    }
    return STRUCT_LOCATION{0.0, 0.0, 0.0, 0.0};
}

bool ComplexEntity::calculateCentroid(double &outLat, double &outLon) const
{
    if (m_locationPoints.isEmpty()) {
        outLat = 0.0;
        outLon = 0.0;
        return false;
    }

    double sumLat = 0.0;
    double sumLon = 0.0;
    for (const auto &pt : m_locationPoints) {
        sumLat += pt.latatitude;
        sumLon += pt.longitude;
    }

    outLat = sumLat / static_cast<double>(m_locationPoints.size());
    outLon = sumLon / static_cast<double>(m_locationPoints.size());
    return true;
}

/**
 * @brief Calculates exact geodesic distance midpoint along the polyline path.
 * @param[out] outLat Calculated latitude in degrees.
 * @param[out] outLon Calculated longitude in degrees.
 * @return True if midpoint successfully computed, false if points vector empty.
 */
bool ComplexEntity::calculateMidpoint(double &outLat, double &outLon) const
{
    if (m_locationPoints.isEmpty()) {
        outLat = 0.0;
        outLon = 0.0;
        return false;
    }

    if (m_locationPoints.size() == 1) {
        outLat = m_locationPoints.first().latatitude;
        outLon = m_locationPoints.first().longitude;
        return true;
    }

    const int count = m_locationPoints.size();
    QVector<double> segLengths(count - 1, 0.0);
    double totalLength = 0.0;

    constexpr double degToRad = 3.14159265358979323846 / 180.0;
    constexpr double earthRadiusMeters = 6371008.8;

    for (int i = 0; i < count - 1; ++i) {
        double lat1 = m_locationPoints[i].latatitude * degToRad;
        double lon1 = m_locationPoints[i].longitude * degToRad;
        double lat2 = m_locationPoints[i + 1].latatitude * degToRad;
        double lon2 = m_locationPoints[i + 1].longitude * degToRad;

        double dLat = lat2 - lat1;
        double dLon = lon2 - lon1;
        double midLat = (lat1 + lat2) * 0.5;

        // Equirectangular distance approximation (accurate for operational tactical scales)
        double x = dLon * std::cos(midLat);
        double y = dLat;
        double dist = std::sqrt(x * x + y * y) * earthRadiusMeters;

        segLengths[i] = dist;
        totalLength += dist;
    }

    if (totalLength <= 0.001) {
        outLat = m_locationPoints.first().latatitude;
        outLon = m_locationPoints.first().longitude;
        return true;
    }

    const double halfLength = totalLength * 0.5;
    double accumulated = 0.0;

    for (int i = 0; i < count - 1; ++i) {
        const double segLen = segLengths[i];
        if (accumulated + segLen >= halfLength || i == count - 2) {
            double remaining = halfLength - accumulated;
            double t = (segLen > 0.001) ? (remaining / segLen) : 0.5;
            if (t < 0.0) t = 0.0;
            if (t > 1.0) t = 1.0;

            outLat = m_locationPoints[i].latatitude + t * (m_locationPoints[i + 1].latatitude - m_locationPoints[i].latatitude);
            outLon = m_locationPoints[i].longitude + t * (m_locationPoints[i + 1].longitude - m_locationPoints[i].longitude);
            return true;
        }
        accumulated += segLen;
    }

    outLat = m_locationPoints.last().latatitude;
    outLon = m_locationPoints.last().longitude;
    return true;
}

bool ComplexEntity::calculatePerimeterAnchor(double &outLat, double &outLon) const
{
    if (m_locationPoints.isEmpty()) {
        outLat = 0.0;
        outLon = 0.0;
        return false;
    }

    double minLat = m_locationPoints.first().latatitude;
    double minLon = m_locationPoints.first().longitude;

    for (const auto &pt : m_locationPoints) {
        if (pt.latatitude < minLat) {
            minLat = pt.latatitude;
            minLon = pt.longitude;
        }
    }

    outLat = minLat;
    outLon = minLon;
    return true;
}

QString ComplexEntity::locationPointsToJson() const
{
    QJsonArray arr;
    for (const auto &pt : m_locationPoints) {
        QJsonObject obj;
        obj[QStringLiteral("lat")] = pt.latatitude;
        obj[QStringLiteral("lon")] = pt.longitude;
        obj[QStringLiteral("height")] = pt.height;
        obj[QStringLiteral("dir")] = pt.dir;
        arr.append(obj);
    }
    return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

void ComplexEntity::loadLocationPointsFromJson(const QString &jsonStr)
{
    m_locationPoints.clear();
    if (jsonStr.trimmed().isEmpty()) {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (!doc.isArray()) {
        return;
    }

    const QJsonArray arr = doc.array();
    m_locationPoints.reserve(arr.size());
    for (const auto &val : arr) {
        if (val.isObject()) {
            QJsonObject obj = val.toObject();
            STRUCT_LOCATION loc;
            loc.latatitude = obj.value(QStringLiteral("lat")).toDouble();
            loc.longitude = obj.value(QStringLiteral("lon")).toDouble();
            loc.height = obj.value(QStringLiteral("height")).toDouble();
            loc.dir = obj.value(QStringLiteral("dir")).toDouble();
            m_locationPoints.append(loc);
        }
    }
}

QString ComplexEntity::detailsToJson() const
{
    QJsonArray arr;
    for (const auto &det : m_entityDetails) {
        QJsonObject obj;
        obj[QStringLiteral("val_type")] = det.valType;

        QByteArray keyBytes(det.valkey, sizeof(det.valkey));
        obj[QStringLiteral("key")] = QString::fromUtf8(keyBytes.constData()).trimmed();

        QByteArray strBytes(det.valStr, sizeof(det.valStr));
        obj[QStringLiteral("val")] = QString::fromUtf8(strBytes.constData()).trimmed();

        arr.append(obj);
    }
    return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

void ComplexEntity::loadDetailsFromJson(const QString &jsonStr)
{
    m_entityDetails.clear();
    if (jsonStr.trimmed().isEmpty()) {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (!doc.isArray()) {
        return;
    }

    const QJsonArray arr = doc.array();
    m_entityDetails.reserve(arr.size());
    for (const auto &val : arr) {
        if (val.isObject()) {
            QJsonObject obj = val.toObject();
            STRUCT_DETAILS det{};
            det.valType = obj.value(QStringLiteral("val_type")).toInt(1);

            const QString keyStr = obj.value(QStringLiteral("key")).toString();
            const QByteArray keyUtf8 = keyStr.toUtf8();
            std::memset(det.valkey, 0, sizeof(det.valkey));
            std::strncpy(det.valkey, keyUtf8.constData(), sizeof(det.valkey) - 1);

            const QString valString = obj.value(QStringLiteral("val")).toString();
            const QByteArray valUtf8 = valString.toUtf8();
            std::memset(det.valStr, 0, sizeof(det.valStr));
            std::strncpy(det.valStr, valUtf8.constData(), sizeof(det.valStr) - 1);

            m_entityDetails.append(det);
        }
    }
}

QString ComplexEntity::entityTypeToString(UINT_8 type)
{
    switch (type) {
    case 1:
        return QStringLiteral("Point");
    case 2:
        return QStringLiteral("Line");
    case 3:
        return QStringLiteral("Polygon");
    case 4:
        return QStringLiteral("Text Only");
    case 5:
        return QStringLiteral("Custom Image");
    case 6:
        return QStringLiteral("Custom Painter");
    case 7:
        return QStringLiteral("Formation Boundary");
    case 8:
        return QStringLiteral("Deployment Area");
    default:
        return QStringLiteral("Unknown (%1)").arg(type);
    }
}

} // namespace GISApp::Domain::ComplexEntities
