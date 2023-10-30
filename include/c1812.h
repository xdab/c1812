#ifndef C1812_H
#define C1812_H

typedef enum {
	VERTICAL = 1,
	HORIZONTAL = 2,
} c1812_polarization_t;

typedef enum {
	COASTAL_LAND = 1,
	INLAND = 3,
	SEA = 4,
} c1812_radioclimactic_zone_t;

typedef struct {
	// Scalar data
	double f; // frequency [GHz]
	double p; // percentage of average year for which the calculated signal level is exceeded [%]
	double htg; // transmitter height above ground level [m]
	double hrg; // receiver height above ground level [m]
	c1812_polarization_t pol;
	c1812_radioclimactic_zone_t zone; 
	double ws; // street width [m]

	// Vector data
	int n; // number of points
	double* d; // distance from transmitter [km]
	double* h; // terrain height above sea level [m]
	double* Ct; // representative clutter height [m] 

} c1812_parameters_t;

typedef enum {
	C1812_NULL = -1,
	C1812_OK = 0,
	C1812_INVALID_FREQUENCY = 1,
	C1812_INVALID_PERCENTAGE_OF_TIME = 2,
	C1812_INVALID_TRANSMITTER_HEIGHT = 3,
	C1812_INVALID_RECEIVER_HEIGHT = 4,
	C1812_INVALID_POLARIZATION = 5,
	C1812_INVALID_RADIOCLIMACTIC_ZONE = 6,
	C1812_INVALID_STREET_WIDTH = 7,
	C1812_INVALID_NUMBER_OF_POINTS = 8,
	C1812_INVALID_DISTANCE_FROM_TRANSMITTER = 9,
	C1812_INVALID_TERRAIN_HEIGHT = 10,
	C1812_INVALID_CLUTTER_HEIGHT = 11,
} c1812_error_t;

const double C1812_MIN_FREQUENCY = 0.03;
const double C1812_MAX_FREQUENCY = 6.0;

const double C1812_MIN_TIME_PERCENTAGE = 1.0;
const double C1812_MAX_TIME_PERCENTAGE = 50.0;

const double C1812_MIN_TRANSMITTER_HEIGHT = 1.0;
const double C1812_MAX_TRANSMITTER_HEIGHT = 3000.0;

const double C1812_MIN_RECEIVER_HEIGHT = 1.0;
const double C1812_MAX_RECEIVER_HEIGHT = 3000.0;

const double C1812_MIN_STREET_WIDTH = 1.0;
const double C1812_DEFAULT_STREET_WIDTH = 27.0;
const double C1812_MAX_STREET_WIDTH = 100.0;

const int C1812_MIN_NUMBER_OF_POINTS = 2;

const double C1812_MIN_TERRAIN_HEIGHT = -500.0; // Less than the lowest point on Earth
const double C1812_MAX_TERRAIN_HEIGHT = 9000.0; // More than the highest point on Earth

const double C1812_MIN_CLUTTER_HEIGHT = 0.0; // No clutter
const double C1812_MAX_CLUTTER_HEIGHT = 1000.0; // More than the highest building in the world

c1812_error_t c1812_validate(c1812_parameters_t *parameters);

#endif 
