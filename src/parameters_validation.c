#include <stdlib.h>
#include "../include/parameters.h"
#include "../include/parameters_validation.h"

c1812_error_t c1812_validate_scalar_data(c1812_parameters_t *parameters) {
	if (parameters->f < C1812_MIN_FREQUENCY || parameters->f > C1812_MAX_FREQUENCY) {
		return C1812_INVALID_FREQUENCY;
	}
	if (parameters->p < C1812_MIN_TIME_PERCENTAGE || parameters->p > C1812_MAX_TIME_PERCENTAGE) {
		return C1812_INVALID_PERCENTAGE_OF_TIME;
	}
	if (parameters->htg < C1812_MIN_TRANSMITTER_HEIGHT || parameters->htg > C1812_MAX_TRANSMITTER_HEIGHT) {
		return C1812_INVALID_TRANSMITTER_HEIGHT;
	}
	if (parameters->hrg < C1812_MIN_RECEIVER_HEIGHT || parameters->hrg > C1812_MAX_RECEIVER_HEIGHT) {
		return C1812_INVALID_RECEIVER_HEIGHT;
	}
	if (parameters->pol != VERTICAL && parameters->pol != HORIZONTAL) {
		return C1812_INVALID_POLARIZATION;
	}
	if (parameters->zone != COASTAL_LAND && parameters->zone != INLAND && parameters->zone != SEA) {
		return C1812_INVALID_RADIOCLIMACTIC_ZONE;
	}
	if (parameters->ws < C1812_MIN_STREET_WIDTH || parameters->ws > C1812_MAX_STREET_WIDTH) {
		return C1812_INVALID_STREET_WIDTH;
	}
	if (parameters->lon < C1812_MIN_LONGITUDE || parameters->lon > C1812_MAX_LONGITUDE) {
		return C1812_INVALID_LONGITUDE;
	}
	if (parameters->lat < C1812_MIN_LATITUDE || parameters->lat > C1812_MAX_LATITUDE) {
		return C1812_INVALID_LATITUDE;
	}
	if (parameters->N0 < C1812_MIN_SURFACE_REFRACTIVITY || parameters->N0 > C1812_MAX_SURFACE_REFRACTIVITY) {
		return C1812_INVALID_SURFACE_REFRACTIVITY;
	}
	if (parameters->DN < C1812_MIN_REFRACTIVITY_LAPSE_RATE || parameters->DN > C1812_MAX_REFRACTIVITY_LAPSE_RATE) {
		return C1812_INVALID_REFRACTIVITY_LAPSE_RATE;
	}
	return C1812_OK;
}

c1812_error_t c1812_validate_vector_data(c1812_parameters_t *parameters) 
{
	if (parameters->n < C1812_MIN_NUMBER_OF_POINTS) {
		return C1812_INVALID_NUMBER_OF_POINTS;
	}
	if (parameters->d == NULL) {
		return C1812_INVALID_DISTANCE_FROM_TRANSMITTER;
	}
	// Validate distance from transmitter is non-negative and increasing
	for (int i = 0; i < parameters->n; i++) {
		if (parameters->d[i] < 0) {
			return C1812_INVALID_DISTANCE_FROM_TRANSMITTER;
		}
		if (i > 0 && parameters->d[i] <= parameters->d[i-1]) {
			return C1812_INVALID_DISTANCE_FROM_TRANSMITTER;
		}
	}
	if (parameters->h == NULL) {
		return C1812_INVALID_TERRAIN_HEIGHT;
	}
	// Validate terrain height is between min/max bounds
	for (int i = 0; i < parameters->n; i++) {
		if (parameters->h[i] < C1812_MIN_TERRAIN_HEIGHT || parameters->h[i] > C1812_MAX_TERRAIN_HEIGHT) {
			return C1812_INVALID_TERRAIN_HEIGHT;
		}
	}
	// If clutter height data is present, validate it
	if (parameters->Ct != NULL) {
		for (int i = 0; i < parameters->n; i++) {
			if (parameters->Ct[i] < C1812_MIN_CLUTTER_HEIGHT || parameters->Ct[i] > C1812_MAX_CLUTTER_HEIGHT) {
				return C1812_INVALID_CLUTTER_HEIGHT;
			}
		}
	}
	return C1812_OK;
}

c1812_error_t c1812_validate(c1812_parameters_t *parameters)
{
	if (parameters == NULL) {
		return C1812_NULL;
	}
	c1812_error_t error = c1812_validate_scalar_data(parameters);
	if (error != C1812_OK) {
		return error;
	}
	error = c1812_validate_vector_data(parameters);
	if (error != C1812_OK) {
		return error;
	}
	return C1812_OK;
}

