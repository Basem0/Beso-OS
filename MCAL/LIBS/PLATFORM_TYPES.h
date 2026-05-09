/*
 * PLATFORM_TYPES.h
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

#ifndef LIBS_PLATFORM_TYPES_H_
#define LIBS_PLATFORM_TYPES_H_

/* ================================================================ */
/* ===================== Integer Data Types ======================= */
/* ================================================================ */
typedef unsigned char                		uint8;
typedef unsigned short               		uint16;
typedef unsigned int                 		uint32;
typedef unsigned long long int       		uint64;

typedef signed char                 		sint8;
typedef signed short                		sint16;
typedef signed int                  		sint32;
typedef signed long long int        		sint64;

typedef volatile unsigned char              vuint8;
typedef volatile unsigned short             vuint16;
typedef volatile unsigned int               vuint32;
typedef volatile unsigned long long int     vuint64;

typedef volatile signed char                vsint8;
typedef volatile signed short               vsint16;
typedef volatile signed int                 vsint32;
typedef volatile signed long long int       vsint64;

/* ================================================================ */
/* ====================== Floating Data Types ===================== */
/* ================================================================ */
typedef float       	f32;
typedef double      	f64;

/* ================================================================ */
/* ======================= Boolean Data Type ====================== */
/* ================================================================ */
typedef enum
{
	false,
	true,
}bool;

/* ================================================================ */
/* ========================== Error Types ========================= */
/* ================================================================ */
typedef enum
{
	InvalidArgument,    //in any configuration to Pins or Ports
	OverFlow            //in any configuration to LEDs For Example
}ErrorType;

/* ================================================================ */
/* ============================ Macros ============================ */
/* ================================================================ */
#define NULL ((void *)0)

#endif /* LIBS_PLATFORM_TYPES_H_ */