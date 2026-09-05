/**
 * @file IrsTypes.h
 * @brief Interface Requirements Specification (IRS) fundamental types and system constants.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 *
 * This header defines cross-platform fundamental data types, bit-width specific typedefs,
 * CSCI (Computer Software Configuration Item) subsystem identifiers, and wire-level protocol
 * enumeration constants used across the UDP communication subsystem.
 */

#ifndef IRSTYPES_H
#define IRSTYPES_H

#include <cstring>

#define AES_BLOCK_SIZE 16
#define SMEMCPY std::memcpy

// ==============================================================================
// 1. Fixed-Width Primitive Types
// ==============================================================================

typedef char                    INT_8;
typedef unsigned char           UINT_8;
typedef short                   INT_16;
typedef unsigned short          UINT_16;
typedef int                     INT_32;
typedef unsigned int            UINT_32;
typedef float                   REAL_32;
typedef double                  REAL_64;
typedef long long               INT_64;
typedef unsigned long long      UINT_64;

// Fixed-length string buffers for binary wire deserialization
typedef char STRING_100[100];
typedef char STRING_200[200];
typedef char STRING_50[50];

#define UNKNOWN_VALUE 0

// ==============================================================================
// 2. CSCI (Computer Software Configuration Item) Subsystem Identifiers
// ==============================================================================

typedef UINT_16 CSCI_ID;

#define CSCI_ID_WCM  1  ///< Weapon Control Module
#define CSCI_ID_DB   2  ///< Database Subsystem
#define CSCI_ID_DFE  3  ///< Data Fusion Engine
#define CSCI_ID_DSS  4  ///< Decision Support System
#define CSCI_ID_WVM  5  ///< Weapon Vehicle Management
#define CSCI_ID_DCCC 6  ///< Data Communication Control Center
#define CSCI_ID_VMS  10 ///< Vehicle Management System

// ==============================================================================
// 3. Message Routing Types
// ==============================================================================

/**
 * @brief Numeric unique message identifier distinguishing payload types.
 */
typedef UINT_16 MESSAGE_ID;

/**
 * @brief Wire payload length excluding the standard message header.
 */
typedef UINT_16 MESSAGE_LENGTH;

/**
 * @brief Transmission precedence level indicating message urgency.
 */
typedef UINT_8 PRECEDENCE;

#define PRECEDENCE_NONE                  0
#define PRECEDENCE_FLASH                 1
#define PRECEDENCE_EMERGENCY             2
#define PRECEDENCE_OPERATIONAL_IMMEDIATE 3
#define PRECEDENCE_PRIORITY              4
#define PRECEDENCE_ROUTINE               5
#define PRECEDENCE_DEFFERED              6

typedef UINT_8 WORKSTATION_INDEX;
typedef UINT_8 SUCOMT_INDEX;
typedef UINT_16 PACKET_SEQ_NO;
typedef UINT_16 NO_OF_PACKETS;

// ==============================================================================
// 4. Tactical Status & Track Identification Types
// ==============================================================================

typedef UINT_8 HEALTH_STATUS;
#define HEALTH_STATUS_RED   1
#define HEALTH_STATUS_GREEN 2

typedef UINT_8 IDENTITY;
#define HOSTILE   1
#define FRIENDLY  2

typedef UINT_8 SYSTEM_TRACK_TYPE;
#define SYSTEM1 1
#define SYSTEM2 2
#define FUSED   2

typedef UINT_8 TRACK_SOURCE;
#define SOURCE1 1
#define SOURCE2 2

#endif // IRSTYPES_H
