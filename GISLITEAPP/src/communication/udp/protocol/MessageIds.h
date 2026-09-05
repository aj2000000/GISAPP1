/**
 * @file MessageIds.h
 * @brief Canonical numeric identifiers for UDP wire datagram payloads.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 *
 * Each incoming binary UDP packet contains a Message ID in its header that maps
 * directly to a registered IUdpMessageHandler strategy in the dispatcher.
 */

#ifndef MESSAGEIDS_H
#define MESSAGEIDS_H

#include "IrsTypes.h"

/**
 * @name Standard Protocol Message Identifiers
 * @{
 */
#define MAIN_LITE_TRACK_MSG_ID 613  ///< Dynamic tactical tracks payload (MAIN_LITE_TRACK_MSG)
#define SAMPLE_ENTITY_MSG_ID   901  ///< Sample entity telemetry payload
#define SENSOR_MSG_ID          902  ///< Sensor telemetry payload
#define EXP_MESSAGE_ID         903  ///< Experimental telemetry payload
/** @} */

#endif // MESSAGEIDS_H
