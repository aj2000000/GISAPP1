/**
 * @file GeoCoordinate.h
 * @brief Header definition for GeoCoordinate domain value object.
 */

#ifndef GEOCOORDINATE_H
#define GEOCOORDINATE_H

namespace GISApp::Core::Models {

/**
 * @class GeoCoordinate
 * @brief Immutable-by-default domain value object representing a 3D WGS 84 geodetic coordinate.
 *
 * GeoCoordinate models geodetic coordinates consisting of latitude, longitude, and optional
 * altitude (elevation above MSL), paired with a validity flag.
 * It serves as the primary coordinate currency across domain models, UI controllers, and services,
 * decoupling application logic from vendor-specific coordinate types (such as QMapLibre::Coordinate).
 */
class GeoCoordinate
{
public:
    /**
     * @brief Default constructor creating an invalid coordinate located at (0.0, 0.0, 0.0).
     */
    GeoCoordinate()
        : m_latitude(0.0), m_longitude(0.0), m_altitude(0.0), m_valid(false) {}

    /**
     * @brief Parameterized constructor initializing a geodetic point.
     * @param[in] latitude Latitude in decimal degrees [-90.0, 90.0].
     * @param[in] longitude Longitude in decimal degrees [-180.0, 180.0].
     * @param[in] altitude Altitude in meters above mean sea level (default: 0.0).
     * @param[in] valid Validity flag (default: true).
     */
    GeoCoordinate(double latitude, double longitude, double altitude = 0.0, bool valid = true)
        : m_latitude(latitude), m_longitude(longitude), m_altitude(altitude), m_valid(valid) {}

    /**
     * @brief Checks if the coordinate holds valid geodetic numbers.
     * @return True if valid, false otherwise.
     */
    [[nodiscard]] bool isValid() const { return m_valid; }

    /**
     * @brief Retrieves geodetic latitude.
     * @return Latitude in decimal degrees.
     */
    [[nodiscard]] double latitude() const { return m_latitude; }

    /**
     * @brief Retrieves geodetic longitude.
     * @return Longitude in decimal degrees.
     */
    [[nodiscard]] double longitude() const { return m_longitude; }

    /**
     * @brief Retrieves altitude above mean sea level.
     * @return Altitude in meters.
     */
    [[nodiscard]] double altitude() const { return m_altitude; }

    /**
     * @brief Updates latitude and marks coordinate as valid.
     * @param[in] latitude Geodetic latitude [-90.0, 90.0].
     */
    void setLatitude(double latitude) { m_latitude = latitude; m_valid = true; }

    /**
     * @brief Updates longitude and marks coordinate as valid.
     * @param[in] longitude Geodetic longitude [-180.0, 180.0].
     */
    void setLongitude(double longitude) { m_longitude = longitude; m_valid = true; }

    /**
     * @brief Updates altitude.
     * @param[in] altitude Altitude in meters.
     */
    void setAltitude(double altitude) { m_altitude = altitude; }

    /**
     * @brief Sets coordinate validity status explicitly.
     * @param[in] valid Boolean flag.
     */
    void setValid(bool valid) { m_valid = valid; }

private:
    /// Geodetic latitude in decimal degrees
    double m_latitude;

    /// Geodetic longitude in decimal degrees
    double m_longitude;

    /// Altitude in meters above mean sea level
    double m_altitude;

    /// Indicates whether the coordinate contains valid numerical readings
    bool m_valid;
};

} // namespace GISApp::Core::Models

#endif // GEOCOORDINATE_H
