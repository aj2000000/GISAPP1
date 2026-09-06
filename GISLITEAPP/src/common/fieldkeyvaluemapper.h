/**
 * @file fieldkeyvaluemapper.h
 * @brief Singleton mapper converting wire protocol enum keys and track attribute codes into human-readable strings.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef FIELDKEYVALUEMAPPER_H
#define FIELDKEYVALUEMAPPER_H

#include <QString>
#include <QVariant>
#include <QMap>

#include "IrsTypes.h"
#include "WireStructures.h"

/**
 * @class FieldKeyValueMapper
 * @brief Central singleton providing canonical conversions from system protocol codes and STRUCT_TRACK_ATTRIBUTES
 *        into standardized human-readable strings for UI tables, detail views, and map popups.
 *
 * Architectural Role & Responsibility:
 * - Resides in `src/common/` as a domain/infrastructure-agnostic utility.
 * - Serves as the single source of truth for translating raw integer/enum fields from `STRUCT_TRACK`
 *   and `STRUCT_TRACK_ATTRIBUTES` into displayable operational terminology.
 * - Eliminates ad-hoc switch statements across Qt models (`TrackTableModel`), dialogs (`TrackTablePanelDialog`),
 *   and presentation layers.
 */
class FieldKeyValueMapper
{
public:
    /**
     * @brief Accesses the global singleton instance.
     * @return Reference to FieldKeyValueMapper.
     */
    static FieldKeyValueMapper& instance();

    // =========================================================================
    // 1. Core STRUCT_TRACK Properties Mapping
    // =========================================================================

    /**
     * @brief Maps track tactical identity code to human-readable string.
     * @param[in] identity Integer code (1=Hostile, 2=Friendly, 3=Neutral, 0=Unknown).
     * @return Formatted string ("HOSTILE", "FRIENDLY", "NEUTRAL", "UNKNOWN").
     */
    [[nodiscard]] QString trackIdentityMapping(int identity);
    [[nodiscard]] QString trackIdentityMapping(IDENTITY identity);

    /**
     * @brief Maps system track designation code to human-readable string.
     * @param[in] type Integer code (1=SYSTEM1, 2=SYSTEM2 / FUSED).
     * @return Formatted string ("SYSTEM 1", "SYSTEM 2 / FUSED", "UNKNOWN").
     */
    [[nodiscard]] QString systemTrackTypeMapping(int type);
    [[nodiscard]] QString systemTrackTypeMapping(SYSTEM_TRACK_TYPE type);

    /**
     * @brief Maps contributing sensor/radar source code to human-readable string.
     * @param[in] source Integer code (1=SOURCE1, 2=SOURCE2).
     * @return Formatted string ("SOURCE 1", "SOURCE 2", "UNKNOWN").
     */
    [[nodiscard]] QString trackSourceMapping(int source);
    [[nodiscard]] QString trackSourceMapping(TRACK_SOURCE source);

    // =========================================================================
    // 2. STRUCT_TRACK_ATTRIBUTES Detailed Properties Mapping
    // =========================================================================

    /**
     * @brief Maps track domain type code (type field).
     * @param[in] type Integer code (1=Air, 2=Surface, 3=Subsurface, 4=Land).
     * @return Human-readable domain string ("AIR", "SURFACE", "SUBSURFACE", "LAND", "UNKNOWN").
     */
    [[nodiscard]] QString trackTypeMapping(int type);

    /**
     * @brief Maps track specific subtype code (sub_type field) in relation to domain type.
     * @param[in] domainType Track domain type (1=Air, 2=Surface, etc.).
     * @param[in] subType Subtype integer code.
     * @return Domain-aware subtype string (e.g. "AIR 1", "SURFACE 2").
     */
    [[nodiscard]] QString trackSubTypeMapping(int domainType, int subType);
    [[nodiscard]] QString trackSubTypeMapping(int subType);

    /**
     * @brief Maps Security/IFF classification code (classification field).
     * @param[in] domainType Track domain type (1=Air, 2=Surface, etc.).
     * @param[in] classification Classification integer code.
     * @return Classification string (e.g. "AIR CLASS 1", "SURFACE CLASS 2").
     */
    [[nodiscard]] QString trackClassificationMapping(int domainType, int classification);
    [[nodiscard]] QString trackClassificationMapping(int classification);

    /**
     * @brief Maps target formation count or strength (strength field).
     * @param[in] strength Formation count integer code.
     * @return Formatted strength description (e.g. "SINGLE (1)", "2 UNITS", etc.).
     */
    [[nodiscard]] QString trackStrengthMapping(int strength);

    /**
     * @brief Maps operational activity type code (act_type field).
     * @param[in] domainType Track domain type (1=Air, 2=Surface, etc.).
     * @param[in] actType Activity type integer code.
     * @return Activity type string (e.g. "AIR ACT 1", "SURFACE ACT 2").
     */
    [[nodiscard]] QString trackActivityTypeMapping(int domainType, int actType);
    [[nodiscard]] QString trackActivityTypeMapping(int actType);

    /**
     * @brief Maps operational activity subtype code (act_sub_type field).
     * @param[in] domainType Track domain type.
     * @param[in] actSubType Activity subtype integer code.
     * @return Activity subtype string (e.g. "AIR SUB ACT 1", "SURFACE SUB ACT 2").
     */
    [[nodiscard]] QString trackActivitySubTypeMapping(int domainType, int actSubType);
    [[nodiscard]] QString trackActivitySubTypeMapping(int actSubType);

    /**
     * @brief Maps operational activity classification code (act_classification field).
     * @param[in] domainType Track domain type.
     * @param[in] actClassification Activity classification integer code.
     * @return Activity classification string (e.g. "AIR ACT CLASS 1", "SURFACE ACT CLASS 2").
     */
    [[nodiscard]] QString trackActivityClassificationMapping(int domainType, int actClassification);
    [[nodiscard]] QString trackActivityClassificationMapping(int actClassification);

    /**
     * @brief Deconstructs and formats an entire STRUCT_TRACK_ATTRIBUTES structure into a key-value dictionary.
     * @param[in] attr Packed attribute structure.
     * @return Map of human-readable attribute names to mapped string values.
     */
    [[nodiscard]] QMap<QString, QString> mapTrackAttributes(const STRUCT_TRACK_ATTRIBUTES &attr);

    // =========================================================================
    // 3. Generic Key-Value Resolver for UI Views
    // =========================================================================

    /**
     * @brief Resolves any arbitrary field key and raw QVariant into a human-readable display string.
     * @param[in] fieldKey Normalized field identifier (e.g. "identity", "type", "sub_type", "strength", "act_type").
     * @param[in] value Raw integer or string value.
     * @param[in] domainType Optional domain context (e.g. Air, Surface) for contextual subtype/activity mapping.
     * @return Human-readable display string.
     */
    [[nodiscard]] QString mapValue(const QString &fieldKey, const QVariant &value, int domainType = 0);

private:
    FieldKeyValueMapper();
    ~FieldKeyValueMapper() = default;

    FieldKeyValueMapper(const FieldKeyValueMapper&) = delete;
    FieldKeyValueMapper& operator=(const FieldKeyValueMapper&) = delete;
};

#endif // FIELDKEYVALUEMAPPER_H
