#include <stdlib.h>
#include "parameters.h"
#include "parameters_validation.h"

const double C1812_MIN_FREQUENCY = 0.03;
const double C1812_MAX_FREQUENCY = 6.0;

const double C1812_MIN_TIME_PERCENTAGE = 1.0;
const double C1812_MAX_TIME_PERCENTAGE = 50.0;

const double C1812_MIN_TRANSMITTER_HEIGHT = 1.0;
const double C1812_MAX_TRANSMITTER_HEIGHT = 3000.0;

const double C1812_MIN_RECEIVER_HEIGHT = 1.0;
const double C1812_MAX_RECEIVER_HEIGHT = 3000.0;

const double C1812_MIN_STREET_WIDTH = 1.0;
const double C1812_MAX_STREET_WIDTH = 100.0;

const int C1812_MIN_NUMBER_OF_POINTS = 2;

const double C1812_MIN_TERRAIN_HEIGHT = -500.0; // Less than the lowest point on Earth
const double C1812_MAX_TERRAIN_HEIGHT = 9000.0; // More than the highest point on Earth

const double C1812_MIN_CLUTTER_HEIGHT = 0.0;	// No clutter
const double C1812_MAX_CLUTTER_HEIGHT = 1000.0; // More than the highest building in the world

// +-80 degrees according to ITU-R P.1812-6 Table 1
const double C1812_MIN_LATITUDE = -80.0;
const double C1812_MAX_LATITUDE = 80.0;

const double C1812_MIN_LONGITUDE = -180.0;
const double C1812_MAX_LONGITUDE = 180.0;

const double C1812_MIN_SURFACE_REFRACTIVITY = 250.0;
const double C1812_MAX_SURFACE_REFRACTIVITY = 450.0;

const double C1812_MIN_REFRACTIVITY_LAPSE_RATE = 0.0;
const double C1812_MAX_REFRACTIVITY_LAPSE_RATE = 100.0;

c1812_parameters_error_t c1812_validate_scalar_data(c1812_parameters_t *parameters)
{
	if (parameters->f < C1812_MIN_FREQUENCY || parameters->f > C1812_MAX_FREQUENCY)
	{
		return PARAM_ERR_INVALID_FREQUENCY;
	}
	if (parameters->p < C1812_MIN_TIME_PERCENTAGE || parameters->p > C1812_MAX_TIME_PERCENTAGE)
	{
		return PARAM_ERR_INVALID_PERCENTAGE_OF_TIME;
	}
	if (parameters->htg < C1812_MIN_TRANSMITTER_HEIGHT || parameters->htg > C1812_MAX_TRANSMITTER_HEIGHT)
	{
		return PARAM_ERR_INVALID_TRANSMITTER_HEIGHT;
	}
	if (parameters->hrg < C1812_MIN_RECEIVER_HEIGHT || parameters->hrg > C1812_MAX_RECEIVER_HEIGHT)
	{
		return PARAM_ERR_INVALID_RECEIVER_HEIGHT;
	}
	if (parameters->pol != POLARIZATION_VERTICAL && parameters->pol != POLARIZATION_HORIZONTAL)
	{
		return PARAM_ERR_INVALID_POLARIZATION;
	}
	if (parameters->zone != RC_ZONE_COASTAL_LAND && parameters->zone != RC_ZONE_INLAND && parameters->zone != RC_ZONE_SEA)
	{
		return PARAM_ERR_INVALID_RADIOCLIMACTIC_ZONE;
	}
	if (parameters->ws < C1812_MIN_STREET_WIDTH || parameters->ws > C1812_MAX_STREET_WIDTH)
	{
		return PARAM_ERR_INVALID_STREET_WIDTH;
	}
	if (parameters->lon < C1812_MIN_LONGITUDE || parameters->lon > C1812_MAX_LONGITUDE)
	{
		return PARAM_ERR_INVALID_LONGITUDE;
	}
	if (parameters->lat < C1812_MIN_LATITUDE || parameters->lat > C1812_MAX_LATITUDE)
	{
		return PARAM_ERR_INVALID_LATITUDE;
	}
	if (parameters->N0 < C1812_MIN_SURFACE_REFRACTIVITY || parameters->N0 > C1812_MAX_SURFACE_REFRACTIVITY)
	{
		return PARAM_ERR_INVALID_SURFACE_REFRACTIVITY;
	}
	if (parameters->DN < C1812_MIN_REFRACTIVITY_LAPSE_RATE || parameters->DN > C1812_MAX_REFRACTIVITY_LAPSE_RATE)
	{
		return PARAM_ERR_INVALID_REFRACTIVITY_LAPSE_RATE;
	}
	return PARAM_ERR_NONE;
}

c1812_parameters_error_t c1812_validate_vector_data(c1812_parameters_t *parameters)
{
	if (parameters->n < C1812_MIN_NUMBER_OF_POINTS)
	{
		return PARAM_ERR_INVALID_NUMBER_OF_POINTS;
	}
	if (parameters->d == NULL)
	{
		return PARAM_ERR_INVALID_DISTANCE_FROM_TRANSMITTER;
	}
	// Validate distance from transmitter is non-negative and increasing
	for (int i = 0; i < parameters->n; i++)
	{
		if (parameters->d[i] < 0)
		{
			return PARAM_ERR_INVALID_DISTANCE_FROM_TRANSMITTER;
		}
		if (i > 0 && parameters->d[i] <= parameters->d[i - 1])
		{
			return PARAM_ERR_INVALID_DISTANCE_FROM_TRANSMITTER;
		}
	}
	if (parameters->h == NULL)
	{
		return PARAM_ERR_INVALID_TERRAIN_HEIGHT;
	}
	// Validate terrain height is between min/max bounds
	for (int i = 0; i < parameters->n; i++)
	{
		if (parameters->h[i] < C1812_MIN_TERRAIN_HEIGHT || parameters->h[i] > C1812_MAX_TERRAIN_HEIGHT)
		{
			return PARAM_ERR_INVALID_TERRAIN_HEIGHT;
		}
	}
	// If clutter height data is present, validate it
	if (parameters->Ct != NULL)
	{
		for (int i = 0; i < parameters->n; i++)
		{
			if (parameters->Ct[i] < C1812_MIN_CLUTTER_HEIGHT || parameters->Ct[i] > C1812_MAX_CLUTTER_HEIGHT)
			{
				return PARAM_ERR_INVALID_CLUTTER_HEIGHT;
			}
		}
	}
	return PARAM_ERR_NONE;
}

c1812_parameters_error_t c1812_validate_parameters(c1812_parameters_t *parameters)
{
	if (parameters == NULL)
	{
		return PARAM_ERR_NULL;
	}
	c1812_parameters_error_t error = c1812_validate_scalar_data(parameters);
	if (error != PARAM_ERR_NONE)
	{
		return error;
	}
	error = c1812_validate_vector_data(parameters);
	if (error != PARAM_ERR_NONE)
	{
		return error;
	}
	return PARAM_ERR_NONE;
}
