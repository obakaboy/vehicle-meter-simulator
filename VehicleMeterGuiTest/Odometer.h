#pragma once

class Odometer {
public:
	void update(double speed, double deltaTime);
	void resetTrip();
	void startSession();

	double getOdoDistance() const;
	double getTripDistance() const;
	double getSinceStartDistance() const;

private:
	double odoDistance_ = 0.0;
	double tripDistance_ = 0.0;
	double sessionStartOdo_ = 0.0;
};
