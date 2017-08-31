#pragma once

// angle
#define BB_RAD_2_DEG(rad)							((rad)		* 57.295779513082)
#define BB_DEG_2_RAD(deg)							((deg)		* 0.017453292519943)

// length
#define BB_KM_2_MILES(km)							((km)		* 0.62137119223733)
#define BB_MILES_2_KM(mi)							((mi)		* 1.609344)

#define BB_METERS_2_MILES(m)						((m)		* 0.00062137119223733)
#define BB_MILES_2_METERS(mi)						((mi)		* 1609.344)

#define BB_METERS_2_FEET(m)							((m)		* 3.2808398950131)
#define BB_FEET_2_METERS(ft)						((ft)		* 0.3048)

#define BB_CM_2_INCH(cm)							((cm)		* 0.39370078740157)
#define BB_INCH_2_CM(inch)							((inch)		* 2.54)

#define BB_KM_2_NAUTICAL_MILES(km)					((km)		* 0.53995680345572)
#define BB_NAUTICAL_MILES_2_KM(nmi)					((nmi)		* 1.852)

// speed
#define BB_KNOTS_2_METERS_PER_SEC(knots)			((knots)	*  0.51444444444)
#define BB_METERS_PER_SEC_2_KNOTS(mps)				((mps)		*  1.9438444924574)

#define BB_METERS_PER_SEC_2_KM_PER_HOUR(mps)		((mps)		* 3.6)
#define BB_KM_PER_HOUR_2_METERS_PER_SEC(kph)		((kph)		* 0.27777777777778)

#define BB_METERS_PER_SEC_2_MILES_PER_HOUR(mps)		((mps)		* 2.2369362920544)
#define BB_MILES_PER_HOUR_2_METERS_PER_SEC(mph)		(mph)		* 0.44704)

#define BB_KM_PER_HOUR_2_MILES_PER_HOUR(kph)		((kph)		* 0.62137119223733)
#define BB_MILES_PER_HOUR_2_KM_PER_HOUR(mph)		((mph)		* 1.609344)

// mass
#define BB_KG_2_POUNDS(kg)							((kg)		* 2.20462)
#define BB_POUNDS_2_KG(lbs)							((lbs)		* 0.45359237)

// temperature
#define BB_CELSIUS_2_FAHR(cel)						((cel)		* 1.8 + 32.0)
#define BB_FAHR_2_CELSIUS(fahr)						(((fahr)	- 32.0) / 1.8)

// pressure
#define BB_BAR_2_PASCAL(bar)						((bar)		* 100000)
#define BB_PASCAL_2_BAR(pa)							((pa)		* 0.00001)

// volume
#define BB_LITERS_2_GALLONS(l)						((l)		* 0.26417205235815)
#define BB_GALLONS_2_LITERS(g)						((g)		* 3.785411784)

