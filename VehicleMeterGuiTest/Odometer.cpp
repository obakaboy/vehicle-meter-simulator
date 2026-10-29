#include "Odometer.h"

void Odometer::update(double speedKmh, double deltaTimeSec)
{
	const double distanceKm = speedKmh * deltaTimeSec / 3600.0;

	odoDistance_ += distanceKm;
	tripDistance_ += distanceKm;
}

void Odometer::resetTrip()
{
	tripDistance_ = 0.0;
}

void Odometer::startSession()
{
	sessionStartOdo_ = odoDistance_;
}

double Odometer::getTripDistance() const
{
	return tripDistance_;
}

double Odometer::getOdoDistance() const
{
	return odoDistance_;
}

double Odometer::getSinceStartDistance() const
{
	return odoDistance_ - sessionStartOdo_;
}
