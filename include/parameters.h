#ifndef PARAMETERS_H
#define PARAMETERS_H

typedef enum
{
	POLARIZATION_HORIZONTAL = 0,
	POLARIZATION_VERTICAL = 1,
} c1812_polarization_t;

typedef enum
{
	RC_ZONE_COASTAL_LAND = 1,
	RC_ZONE_INLAND = 3,
	RC_ZONE_SEA = 4,
} c1812_radioclimactic_zone_t;

typedef struct
{
	// Scalar data
	double f;	// frequency [GHz]
	double p;	// percentage of average year for which the calculated signal level is exceeded [%]
	double htg; // transmitter height above ground level [m]
	double hrg; // receiver height above ground level [m]
	c1812_polarization_t pol;
	c1812_radioclimactic_zone_t zone;
	double ws;	// street width [m]
	double lon; // path center longitude [degrees]
	double lat; // path center latitude [degrees]
	double N0;	// sea-level surface refractivity [N-units]
	double DN;	// average radio-refractivity lapse-rate through the lowest 1 km of the atmosphere [N-units/km]

	// Vector data
	int n;		// number of points
	double *d;	// distance from transmitter [km]
	double *h;	// terrain height above sea level [m]
	double *Ct; // representative clutter height [m]

} c1812_parameters_t;

#endif