#ifndef RESULTS_H
#define RESULTS_H

#include "parameters.h"
#include "parameters_validation.h"

typedef enum
{
	RESULTS_ERR_NONE = 0,
	RESULTS_ERR_PARAMETERS = 1,
	RESULTS_ERR_UNKNOWN = 2
} c1812_results_error_t;

typedef struct 
{
	c1812_results_error_t error;
	c1812_parameters_error_t parameters_error;
	double Lb; // Basic transmission loss
} c1812_results_t;

#endif