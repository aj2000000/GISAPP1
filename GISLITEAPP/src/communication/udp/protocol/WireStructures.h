/**
 * @file WireStructures.h
 * @brief Packed binary wire layout structures for network serialization and deserialization.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 *
 * Defines binary wire protocol structures with byte-aligned packing (#pragma pack(push, 1)).
 * These structures map directly to the byte layout sent over UDP sockets by external tactical nodes.
 */

#ifndef WIRESTRUCTURES_H
#define WIRESTRUCTURES_H

#include <QtGlobal>
#include <QVector>
#include "IrsTypes.h"
#include "MessageIds.h"

// ==============================================================================
// Buffer Size Constraints
// ==============================================================================

const int NO_OF_MSG_IN_QUEUE     = 3000; ///< Maximum number of datagrams in ring buffer
const int MAX_MSG_SIZE           = 8000; ///< Maximum reassembled datagram message size in bytes
const int MAX_UDP_PACKET_SIZE    = 1500; ///< Maximum single UDP packet size on the wire (Ethernet MTU limit)
const int MAX_SENT_MSG_DATA_SIZE = MAX_UDP_PACKET_SIZE; ///< Backward-compatibility alias

#pragma pack(push, 1)

/**
 * @struct STRUCT_MESSAGE_HEADER
 * @brief Standard 16-byte binary wire header present in every UDP packet.
 */
typedef struct __attribute__ ((packed))
{
    CSCI_ID           source_id       = 0; ///< Sender subsystem identifier
    CSCI_ID           destination_id  = 0; ///< Target subsystem identifier
    MESSAGE_ID        message_id      = 0; ///< Payload identifier mapped to IUdpMessageHandler
    MESSAGE_LENGTH    message_len     = 0; ///< Length of payload excluding this header
    PRECEDENCE        precedence;          ///< Urgency/precedence of packet
    WORKSTATION_INDEX ws_index;            ///< Originating workstation index
    SUCOMT_INDEX      sucomt_index;        ///< Subsystem communication terminal index
    PACKET_SEQ_NO     packet_seq_no   = 0; ///< Sequence index (1-based) in multi-packet transmissions
    NO_OF_PACKETS     no_of_packets   = 0; ///< Total packet count for fragmented messages
} STRUCT_MESSAGE_HEADER;

/**
 * @struct STRUCT_MQBUF
 * @brief Ring buffer queue element containing header and raw message payload.
 */
typedef struct __attribute__ ((packed))
{
    STRUCT_MESSAGE_HEADER msg_header;          ///< Standard wire header
    char                  my_buf[MAX_MSG_SIZE]; ///< Payload buffer
} STRUCT_MQBUF;

/**
 * @struct STRUCT_LOCATION
 * @brief Spatial coordinates and movement kinematics for tactical entities.
 */
typedef struct __attribute__ ((packed))
{
    double latatitude; ///< WGS-84 latitude in degrees
    double longitude;  ///< WGS-84 longitude in degrees
    double height;     ///< Altitude / elevation above MSL in meters
    double dir;        ///< Heading / bearing direction in degrees [0..360)
} STRUCT_LOCATION;

/**
 * @struct STRUCT_TRACK_ATTRIBUTES
 * @brief Classification and tactical attributes of a track entity.
 */
typedef struct __attribute__ ((packed))
{
    UINT_8 type;               ///< Track domain type (Air, Surface, Subsurface, Land)
    UINT_8 sub_type;           ///< Track specific subtype
    UINT_8 classification;     ///< Security/IFF classification
    UINT_8 strength;           ///< Target formation count / strength
    UINT_8 act_type;           ///< Activity type
    UINT_8 act_sub_type;       ///< Activity subtype
    UINT_8 act_classification; ///< Activity classification
} STRUCT_TRACK_ATTRIBUTES;

/**
 * @struct STRUCT_TRACK_SOURCE
 * @brief Sensor or radar source identification reporting the track.
 */
typedef struct __attribute__ ((packed))
{
    TRACK_SOURCE source;    ///< Source classification (SOURCE1, SOURCE2)
    STRING_50    source_id; ///< Textual source sensor identifier
} STRUCT_TRACK_SOURCE;

/**
 * @struct STRUCT_TRACK_SYMBOL
 * @brief Symbology designation for map rendering (e.g. MIL-STD-2525 symbol).
 */
typedef struct __attribute__ ((packed))
{
    STRING_50 symbol_name; ///< Symbol identifier string
} STRUCT_TRACK_SYMBOL;

/**
 * @struct STRUCT_DATE
 * @brief Calendar date wire format.
 */
typedef struct __attribute__ ((packed))
{
    UINT_8  day;   ///< Day of month [1..31]
    UINT_8  month; ///< Month of year [1..12]
    UINT_16 year;  ///< Year (e.g. 2026)
} STRUCT_DATE;

/**
 * @struct STRUCT_TIME
 * @brief Clock time wire format.
 */
typedef struct __attribute__ ((packed))
{
    UINT_8  hour;   ///< Hour [0..23]
    UINT_8  minute; ///< Minute [0..59]
    UINT_16 second; ///< Second [0..59]
} STRUCT_TIME;

/**
 * @struct STRUCT_DATE_TIME
 * @brief Combined timestamp wire format.
 */
typedef struct __attribute__ ((packed))
{
    STRUCT_DATE date; ///< Date element
    STRUCT_TIME time; ///< Time element
} STRUCT_DATE_TIME;

/**
 * @struct STRUCT_TRACK_PAYLOAD
 * @brief Packed binary wire record for individual tactical track.
 */
typedef struct __attribute__ ((packed))
{
    UINT_32                 track_id;           ///< Unique numerical track ID
    STRING_100              track_name;         ///< Designated track callsign/name
    STRUCT_LOCATION         track_loc;          ///< Geographic position & kinematics (lat, lon, height, dir)
    IDENTITY                track_identity;     ///< Friendly (2), Hostile (1), Neutral (3), Unknown (0)
    STRUCT_TRACK_ATTRIBUTES track_attributes;   ///< Domain and operational attributes
    SYSTEM_TRACK_TYPE       sys_track_type;     ///< System track designation
    UINT_8                  no_of_sources;      ///< Number of contributing sensor sources
    STRUCT_TRACK_SYMBOL     track_symbol;       ///< Tactical map symbol representation
    STRUCT_DATE_TIME        track_report_time;  ///< Timestamp of detection/report
    STRING_100              track_remarks;      ///< Freeform operational remarks
} STRUCT_TRACK_PAYLOAD;

#pragma pack(pop)

/**
 * @struct STRUCT_TRACK
 * @brief Dynamic tactical track record representation with Qt containers.
 */
typedef struct
{
    STRUCT_MESSAGE_HEADER   msg_header;         ///< Embedded message header
    UINT_8                  track_id;           ///< Unique numerical track ID
    STRING_100              track_name;         ///< Designated track callsign/name
    STRUCT_LOCATION         track_loc;          ///< Geographic position & kinematics
    IDENTITY                track_identity;     ///< Friendly, Hostile, Neutral, Unknown
    STRUCT_TRACK_ATTRIBUTES track_attributes;   ///< Domain and operational attributes
    SYSTEM_TRACK_TYPE       sys_track_type;     ///< System track designation
    UINT_8                  no_of_sources;      ///< Number of contributing sensor sources
    QVector<STRUCT_TRACK_SOURCE> track_sources; ///< Contributing sensor list
    STRUCT_TRACK_SYMBOL     track_symbol;       ///< Tactical map symbol representation
    STRUCT_DATE_TIME        track_report_time;  ///< Timestamp of detection/report
    STRING_100              track_remarks;      ///< Freeform operational remarks
} STRUCT_TRACK;

/**
 * @struct MAIN_LITE_TRACK_MSG
 * @brief Message ID 613 datagram containing batch track records.
 */
typedef struct
{
    STRUCT_MESSAGE_HEADER msg_header;   ///< Standard header
    UINT_16               no_of_tracks; ///< Number of track elements in message
    QVector<STRUCT_TRACK> tracks;       ///< Dynamic list of tracks
} MAIN_LITE_TRACK_MSG;

#endif // WIRESTRUCTURES_H
