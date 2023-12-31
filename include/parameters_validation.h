#ifndef PARAMETERS_VALIDATION_H
#define PARAMETERS_VALIDATION_H

typedef enum
{
	PARAM_ERR_NONE,
	PARAM_ERR_NULL,
	PARAM_ERR_INVALID_FREQUENCY,
	PARAM_ERR_INVALID_PERCENTAGE_OF_TIME,
	PARAM_ERR_INVALID_TRANSMITTER_HEIGHT,
	PARAM_ERR_INVALID_RECEIVER_HEIGHT,
	PARAM_ERR_INVALID_POLARIZATION,
	PARAM_ERR_INVALID_RADIOCLIMACTIC_ZONE,
	PARAM_ERR_INVALID_STREET_WIDTH,
	PARAM_ERR_INVALID_NUMBER_OF_POINTS,
	PARAM_ERR_INVALID_DISTANCE_FROM_TRANSMITTER,
	PARAM_ERR_INVALID_TERRAIN_HEIGHT,
	PARAM_ERR_INVALID_CLUTTER_HEIGHT,
	PARAM_ERR_INVALID_SURFACE_REFRACTIVITY,
	PARAM_ERR_INVALID_REFRACTIVITY_LAPSE_RATE,
	PARAM_ERR_INVALID_LATITUDE,
	PARAM_ERR_INVALID_LONGITUDE,
} c1812_parameters_error_t;

c1812_parameters_error_t c1812_validate_parameters(c1812_parameters_t *parameters);

#endif
