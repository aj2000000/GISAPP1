/**
 * @file fieldkeyvaluemapper.cpp
 * @brief Implementation of canonical wire protocol and track attribute key-value mapping.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "fieldkeyvaluemapper.h"

FieldKeyValueMapper& FieldKeyValueMapper::instance()
{
    static FieldKeyValueMapper instance;
    return instance;
}

FieldKeyValueMapper::FieldKeyValueMapper() = default;

// =============================================================================
// 1. Core STRUCT_TRACK Properties Mapping
// =============================================================================

QString FieldKeyValueMapper::trackIdentityMapping(IDENTITY identity)
{
    return trackIdentityMapping(static_cast<int>(identity));
}

QString FieldKeyValueMapper::trackIdentityMapping(int identity)
{
    switch (identity) {
    case HOSTILE: // 1
        return QStringLiteral("HOSTILE");
    case FRIENDLY: // 2
        return QStringLiteral("FRIENDLY");
    case 3:
        return QStringLiteral("NEUTRAL");
    default:
        return QStringLiteral("UNKNOWN");
    }
}

QString FieldKeyValueMapper::systemTrackTypeMapping(SYSTEM_TRACK_TYPE type)
{
    return systemTrackTypeMapping(static_cast<int>(type));
}

QString FieldKeyValueMapper::systemTrackTypeMapping(int type)
{
    switch (type) {
    case SYSTEM1: // 1
        return QStringLiteral("SYSTEM 1");
    case SYSTEM2: // 2 (or FUSED)
        return QStringLiteral("SYSTEM 2 / FUSED");
    default:
        return QStringLiteral("UNKNOWN");
    }
}

QString FieldKeyValueMapper::trackSourceMapping(TRACK_SOURCE source)
{
    return trackSourceMapping(static_cast<int>(source));
}

QString FieldKeyValueMapper::trackSourceMapping(int source)
{
    switch (source) {
    case SOURCE1: // 1
        return QStringLiteral("SOURCE 1");
    case SOURCE2: // 2
        return QStringLiteral("SOURCE 2");
    default:
        return QStringLiteral("UNKNOWN");
    }
}

// =============================================================================
// 2. STRUCT_TRACK_ATTRIBUTES Detailed Properties Mapping
// =============================================================================

QString FieldKeyValueMapper::trackTypeMapping(int type)
{
    switch (type) {
    case 1:  return QStringLiteral("AIR");
    case 2:  return QStringLiteral("SURFACE");
    case 3:  return QStringLiteral("SUBSURFACE");
    case 4:  return QStringLiteral("LAND");
    default: return QStringLiteral("UNKNOWN");
    }
}

QString FieldKeyValueMapper::trackSubTypeMapping(int domainType, int subType)
{
    if (subType <= 0) {
        return QStringLiteral("NONE");
    }

    switch (domainType) {
    case 1: // Air
        switch (subType) {
        case 1:  return QStringLiteral("AIR 1 (FIXED WING)");
        case 2:  return QStringLiteral("AIR 2 (ROTARY WING)");
        case 3:  return QStringLiteral("AIR 3 (UAV / DRONE)");
        default: return QStringLiteral("AIR %1").arg(subType);
        }
    case 2: // Surface
        switch (subType) {
        case 1:  return QStringLiteral("SURFACE 1 (COMBATANT)");
        case 2:  return QStringLiteral("SURFACE 2 (MERCHANT / CIVIL)");
        default: return QStringLiteral("SURFACE %1").arg(subType);
        }
    case 3: // Subsurface
        switch (subType) {
        case 1:  return QStringLiteral("SUBSURFACE 1 (SUBMARINE)");
        default: return QStringLiteral("SUBSURFACE %1").arg(subType);
        }
    case 4: // Land
        switch (subType) {
        case 1:  return QStringLiteral("LAND 1 (ARMORED)");
        case 2:  return QStringLiteral("LAND 2 (INFANTRY)");
        default: return QStringLiteral("LAND %1").arg(subType);
        }
    default:
        return QStringLiteral("SUBTYPE %1").arg(subType);
    }
}

QString FieldKeyValueMapper::trackSubTypeMapping(int subType)
{
    return trackSubTypeMapping(0, subType);
}

QString FieldKeyValueMapper::trackClassificationMapping(int domainType, int classification)
{
    if (classification <= 0) {
        return QStringLiteral("UNCLASSIFIED");
    }

    switch (domainType) {
    case 1: // Air
        return QStringLiteral("AIR CLASS %1").arg(classification);
    case 2: // Surface
        return QStringLiteral("SURFACE CLASS %1").arg(classification);
    case 3: // Subsurface
        return QStringLiteral("SUBSURFACE CLASS %1").arg(classification);
    case 4: // Land
        return QStringLiteral("LAND CLASS %1").arg(classification);
    default:
        return QStringLiteral("CLASS %1").arg(classification);
    }
}

QString FieldKeyValueMapper::trackClassificationMapping(int classification)
{
    return trackClassificationMapping(0, classification);
}

QString FieldKeyValueMapper::trackStrengthMapping(int strength)
{
    if (strength <= 1) {
        return QStringLiteral("SINGLE (1)");
    }
    return QStringLiteral("%1 UNITS").arg(strength);
}

QString FieldKeyValueMapper::trackActivityTypeMapping(int domainType, int actType)
{
    if (actType <= 0) {
        return QStringLiteral("NONE");
    }

    switch (domainType) {
    case 1: // Air
        switch (actType) {
        case 1:  return QStringLiteral("AIR ACT 1 (PATROL)");
        case 2:  return QStringLiteral("AIR ACT 2 (INTERCEPT)");
        case 3:  return QStringLiteral("AIR ACT 3 (TRANSIT)");
        default: return QStringLiteral("AIR ACT %1").arg(actType);
        }
    case 2: // Surface
        switch (actType) {
        case 1:  return QStringLiteral("SURFACE ACT 1 (CRUISING)");
        case 2:  return QStringLiteral("SURFACE ACT 2 (ANCHORED)");
        default: return QStringLiteral("SURFACE ACT %1").arg(actType);
        }
    default:
        return QStringLiteral("ACT %1").arg(actType);
    }
}

QString FieldKeyValueMapper::trackActivityTypeMapping(int actType)
{
    return trackActivityTypeMapping(0, actType);
}

QString FieldKeyValueMapper::trackActivitySubTypeMapping(int domainType, int actSubType)
{
    if (actSubType <= 0) {
        return QStringLiteral("NONE");
    }

    switch (domainType) {
    case 1: // Air
        return QStringLiteral("AIR SUB ACT %1").arg(actSubType);
    case 2: // Surface
        return QStringLiteral("SURFACE SUB ACT %1").arg(actSubType);
    default:
        return QStringLiteral("SUB ACT %1").arg(actSubType);
    }
}

QString FieldKeyValueMapper::trackActivitySubTypeMapping(int actSubType)
{
    return trackActivitySubTypeMapping(0, actSubType);
}

QString FieldKeyValueMapper::trackActivityClassificationMapping(int domainType, int actClassification)
{
    if (actClassification <= 0) {
        return QStringLiteral("NONE");
    }

    switch (domainType) {
    case 1: // Air
        return QStringLiteral("AIR ACT CLASS %1").arg(actClassification);
    case 2: // Surface
        return QStringLiteral("SURFACE ACT CLASS %1").arg(actClassification);
    default:
        return QStringLiteral("ACT CLASS %1").arg(actClassification);
    }
}

QString FieldKeyValueMapper::trackActivityClassificationMapping(int actClassification)
{
    return trackActivityClassificationMapping(0, actClassification);
}

QMap<QString, QString> FieldKeyValueMapper::mapTrackAttributes(const STRUCT_TRACK_ATTRIBUTES &attr)
{
    QMap<QString, QString> map;
    map.insert(QStringLiteral("Type"), trackTypeMapping(attr.type));
    map.insert(QStringLiteral("Subtype"), trackSubTypeMapping(attr.type, attr.sub_type));
    map.insert(QStringLiteral("Classification"), trackClassificationMapping(attr.type, attr.classification));
    map.insert(QStringLiteral("Strength"), trackStrengthMapping(attr.strength));
    map.insert(QStringLiteral("Activity Type"), trackActivityTypeMapping(attr.type, attr.act_type));
    map.insert(QStringLiteral("Activity Subtype"), trackActivitySubTypeMapping(attr.type, attr.act_sub_type));
    map.insert(QStringLiteral("Activity Classification"), trackActivityClassificationMapping(attr.type, attr.act_classification));
    return map;
}

// =============================================================================
// 3. Generic Key-Value Resolver for UI Views
// =============================================================================

QString FieldKeyValueMapper::mapValue(const QString &fieldKey, const QVariant &value, int domainType)
{
    const QString key = fieldKey.toLower().trimmed();
    bool ok = false;
    const int intVal = value.toInt(&ok);

    if (!ok) {
        return value.toString();
    }

    if (key == QStringLiteral("identity") || key == QStringLiteral("track_identity") || key == QStringLiteral("trackidentity")) {
        return trackIdentityMapping(intVal);
    }
    if (key == QStringLiteral("type") || key == QStringLiteral("domain") || key == QStringLiteral("track_type")) {
        return trackTypeMapping(intVal);
    }
    if (key == QStringLiteral("subtype") || key == QStringLiteral("sub_type") || key == QStringLiteral("track_sub_type")) {
        return trackSubTypeMapping(domainType, intVal);
    }
    if (key == QStringLiteral("classification") || key == QStringLiteral("track_classification")) {
        return trackClassificationMapping(domainType, intVal);
    }
    if (key == QStringLiteral("strength") || key == QStringLiteral("track_strength")) {
        return trackStrengthMapping(intVal);
    }
    if (key == QStringLiteral("acttype") || key == QStringLiteral("act_type") || key == QStringLiteral("track_act_type") || key == QStringLiteral("activity")) {
        return trackActivityTypeMapping(domainType, intVal);
    }
    if (key == QStringLiteral("actsubtype") || key == QStringLiteral("act_sub_type") || key == QStringLiteral("track_act_sub_type")) {
        return trackActivitySubTypeMapping(domainType, intVal);
    }
    if (key == QStringLiteral("actclassification") || key == QStringLiteral("act_classification") || key == QStringLiteral("track_act_classification")) {
        return trackActivityClassificationMapping(domainType, intVal);
    }
    if (key == QStringLiteral("systemtype") || key == QStringLiteral("system_track_type") || key == QStringLiteral("sys_track_type")) {
        return systemTrackTypeMapping(intVal);
    }
    if (key == QStringLiteral("source") || key == QStringLiteral("sources") || key == QStringLiteral("track_source") || key == QStringLiteral("track_sources")) {
        return trackSourceMapping(intVal);
    }

    return value.toString();
}
