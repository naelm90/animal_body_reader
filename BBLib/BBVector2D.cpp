#include "BBVector2D.h"

//////////////// polar 2D vector methods ////////////////

CBBPolarVector2D::CBBPolarVector2D() : _magnitude(0.0), _direction(0.0) 
{
}
	
CBBPolarVector2D::CBBPolarVector2D(double magnitude, double directionDeg) : _magnitude(magnitude), _direction(directionDeg * BBDEG2RAD) 
{
	PutInRange();
}

CBBPolarVector2D::CBBPolarVector2D(const CBBPolarVector2D& v) : _magnitude(v._magnitude), _direction(v._direction) 
{
	PutInRange();
}
	
CBBPolarVector2D::CBBPolarVector2D(const CBBCartVector2D& v) 
{
	_magnitude = v.GetMagnitude();
	_direction = v.GetDirection() * BBDEG2RAD;
}
	
CBBCartVector2D CBBPolarVector2D::ToCartesian() const
{
	return CBBCartVector2D(*this);
}

CBBPolarVector2D& CBBPolarVector2D::operator=(const CBBPolarVector2D& v) 
{
	_magnitude = v._magnitude;
	_direction = v._direction;
	return *this;
}

double CBBPolarVector2D::GetNorth() const 
{
	return _magnitude * cos(_direction);
}
	
double CBBPolarVector2D::GetEast() const 
{
	return _magnitude * sin(_direction);
}

double CBBPolarVector2D::GetMagnitude() const 
{
	return _magnitude;
}

double CBBPolarVector2D::GetDirection() const 
{
	return _direction * BBRAD2DEG;
}

double CBBPolarVector2D::GetDirection360() const 
{
	return BBRAD2DEG * (_direction >= 0.0 ? _direction : _direction + 2.0 * BBPI);
}

CBBPolarVector2D CBBPolarVector2D::operator+(const CBBPolarVector2D& v) 
{
	return CBBPolarVector2D(CBBCartVector2D(*this) + CBBCartVector2D(v));
}

CBBPolarVector2D CBBPolarVector2D::operator-(const CBBPolarVector2D& v) 
{
	return CBBPolarVector2D(CBBCartVector2D(*this) - CBBCartVector2D(v));
}

CBBPolarVector2D& CBBPolarVector2D::operator+=(const CBBPolarVector2D& v) 
{
	return (*this = *this + v);
}

CBBPolarVector2D& CBBPolarVector2D::operator-=(const CBBPolarVector2D& v) 
{
	return (*this = *this - v);
}

CBBPolarVector2D CBBPolarVector2D::operator*(double s) 
{
	return CBBPolarVector2D(_magnitude * s, _direction * BBRAD2DEG);
}

CBBPolarVector2D CBBPolarVector2D::operator/(double s) 
{
	return CBBPolarVector2D(_magnitude / s, _direction  * BBRAD2DEG);
}

CBBPolarVector2D& CBBPolarVector2D::operator*=(double s) 
{
	_magnitude *= s;
	return *this;
}

CBBPolarVector2D& CBBPolarVector2D::operator/=(double s) 
{
	_magnitude /= s;
	return *this;
}

CBBPolarVector2D CBBPolarVector2D::Rotate(double angleDeg) const
{
	return CBBPolarVector2D(_magnitude, _direction * BBRAD2DEG + angleDeg);
}

CBBPolarVector2D CBBPolarVector2D::Normalize() const
{
	return CBBPolarVector2D(1.0, _direction * BBRAD2DEG);
}
	
double CBBPolarVector2D::Dot(const CBBPolarVector2D& v) const
{
	return ToCartesian().Dot(CBBCartVector2D(v));
}

double CBBPolarVector2D::Cross(const CBBPolarVector2D& v) const
{
	return ToCartesian().Cross(CBBCartVector2D(v));
}

void CBBPolarVector2D::PutInRange()
{
	if (_magnitude < 0.0) {
		_magnitude = -_magnitude;
		_direction += BBPI;
	}
	while (_direction < -BBPI) _direction += 2.0 * BBPI;
	while (_direction >= BBPI) _direction -= 2.0 * BBPI;
}


//////////////// cartesian 2D vector methods ////////////////


CBBCartVector2D::CBBCartVector2D() : _north(0.0), _east(0.0) 
{
}

CBBCartVector2D::CBBCartVector2D(double north, double east) : _north(north), _east(east) 
{
}

CBBCartVector2D::CBBCartVector2D(const CBBCartVector2D& v) : _north(v._north), _east(v._east) 
{
}

CBBCartVector2D::CBBCartVector2D(const CBBPolarVector2D& v) 
{
	_north = v.GetNorth();
	_east = v.GetEast();
}

CBBPolarVector2D CBBCartVector2D::ToPolar() const
{
	return CBBPolarVector2D(*this);
}

CBBCartVector2D& CBBCartVector2D::operator=(const CBBCartVector2D& v) 
{
	_north = v._north;
	_east = v._east;
	return *this;
}

CBBCartVector2D CBBCartVector2D::operator+(const CBBCartVector2D& v) 
{
	return CBBCartVector2D(_north + v._north, _east + v._east);
}

CBBCartVector2D CBBCartVector2D::operator-(const CBBCartVector2D& v) 
{
	return CBBCartVector2D(_north - v._north, _east - v._east);
}
	
CBBCartVector2D& CBBCartVector2D::operator+=(const CBBCartVector2D& v) 
{
	_north += v._north;
	_east += v._east;
	return *this;
}

CBBCartVector2D& CBBCartVector2D::operator-=(const CBBCartVector2D& v) 
{
	_north -= v._north;
	_east -= v._east;
	return *this;
}

CBBCartVector2D CBBCartVector2D::operator*(double s) 
{
	return CBBCartVector2D(_north * s, _east * s);
}

CBBCartVector2D CBBCartVector2D::operator/(double s) 
{
	return CBBCartVector2D(_north / s, _east / s);
}

CBBCartVector2D& CBBCartVector2D::operator*=(double s) 
{
	_north *= s;
	_east *= s;
	return *this;
}

CBBCartVector2D& CBBCartVector2D::operator/=(double s) 
{
	_north /= s;
	_east /= s;
	return *this;
}

double CBBCartVector2D::GetNorth() const 
{
	return _north;
}
	
double CBBCartVector2D::GetEast() const 
{
	return _east;
}

double CBBCartVector2D::GetMagnitude() const 
{
	return sqrt(_north * _north + _east * _east);
}

double CBBCartVector2D::GetDirection() const 
{
	return atan2(_east, _north) * BBRAD2DEG;
}

double CBBCartVector2D::GetDirection360() const 
{
	return ToPolar().GetDirection360();
}


CBBCartVector2D CBBCartVector2D::Rotate(double angleDeg) const
{
	return CBBCartVector2D(ToPolar().Rotate(angleDeg));
}
	
CBBCartVector2D CBBCartVector2D::Normalize() const
{
	return CBBCartVector2D(ToPolar().Normalize());
}
	
double CBBCartVector2D::Dot(const CBBCartVector2D& v) const
{
	return _north * v._north + _east * v._east;
}

double CBBCartVector2D::Cross(const CBBCartVector2D& v) const
{
	return (_north * v._east) - (_east * v._north);
}
