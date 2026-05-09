/*
 * app_protocol.h
 *
 * Shared wireless application protocol between input and display units.
 */

#ifndef APP_PROTOCOL_H
#define APP_PROTOCOL_H

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include <stdint.h>

/* ================================================================ */
/* =========================== Macros ============================= */
/* ================================================================ */
#define APP_PROTOCOL_PAYLOAD_LEN (32U)

/* ================================================================ */
/* ======================= Data Structures ======================== */
/* ================================================================ */
typedef enum
{
	INPUT_KEY = 1U,
	NAV_UP,
	NAV_DOWN,
	NAV_SELECT,
	NAV_BACK
} app_packet_type_t;

typedef struct
{
	uint8_t type;
	uint8_t value;
} Packet;

#endif
