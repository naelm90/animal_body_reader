#pragma once

#include "BBTypes.h"
#include <math.h>

class CBBCartVector2D;

// polar 2D vector
// direction in degrees between [-180..+180)
class CBBPolarVector2D
{
public:
	CBBPolarVector2D();
	CBBPolarVector2D(double magnitude, double direction);
	CBBPolarVector2D(const CBBPolarVector2D& v);
	CBBPolarVector2D(const CBBCartVector2D& v);
	CBBCartVector2D ToCartesian() const;
	CBBPolarVector2D& operator=(const CBBPolarVector2D& v);

	double GetNorth() const;
	double GetEast() const;
	double GetMagnitude() const; // always positive
	double GetDirection() const; // [-180..+180)
	double GetDirection360() const; // [0..+360)

	CBBPolarVector2D operator+(const CBBPolarVector2D& v);
	CBBPolarVector2D operator-(const CBBPolarVector2D& v);
	CBBPolarVector2D& operator+=(const CBBPolarVector2D& v);
	CBBPolarVector2D& operator-=(const CBBPolarVector2D& v);

	CBBPolarVector2D operator*(double s);	// s must be positive
	CBBPolarVector2D operator/(double s);	// s must be positive
	CBBPolarVector2D& operator*=(double s);	// s must be positive
	CBBPolarVector2D& operator/=(double s);	// s must be positive

	CBBPolarVector2D Rotate(double angleDeg) const;
	CBBPolarVector2D Normalize() const;

	double Dot(const CBBPolarVector2D& v) const;
	double Cross(const CBBPolarVector2D& v) const;

private:
	double _magnitude;
	double _direction; // in radians (while all API is in degrees)

	void PutInRange(); // make sure that: -pi <= _direction < pi
};


// cartesian 2D vector
class CBBCartVector2D
{
public:
	
	CBBCartVector2D();
	CBBCartVector2D(double north, double east);
	CBBCartVector2D(const CBBCartVector2D& v);
	CBBCartVector2D(const CBBPolarVector2D& v);
	CBBPolarVector2D ToPolar() const;
	CBBCartVector2D& operator=(const CBBCartVector2D& v);

	CBBCartVector2D operator+(const CBBCartVector2D& v);
	CBBCartVector2D operator-(const CBBCartVector2D& v);
	CBBCartVector2D& operator+=(const CBBCartVector2D& v);
	CBBCartVector2D& operator-=(const CBBCartVector2D& v);

	CBBCartVector2D operator*(double s);	// s must be positive
	CBBCartVector2D operator/(double s);	// s must be positive
	CBBCartVector2D& operator*=(double s);	// s must be positive
	CBBCartVector2D& operator/=(double s);	// s must be positive

	double GetNorth() const;
	double GetEast() const;
	double GetMagnitude() const; // always positive
	double GetDirection() const; // [-180..+180)
	double GetDirection360() const; // [0..+360)

	CBBCartVector2D Rotate(double angleDeg) const;
	CBBCartVector2D Normalize() const;
	
	double Dot(const CBBCartVector2D& v) const;
	double Cross(const CBBCartVector2D& v) const;

private:
	double _north;
	double _east;
};

 