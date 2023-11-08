#ifndef P2A_H
#define P2A_H

#include "jobfile.h"
#include "c1812/parameters.h"
#include "datafile.h"

/**
 * @brief Point-to-area calculation
 *
 * @param job_parameters Job parameters
 * @param parameters Calculation parameters
 * @param datafiles Datafiles
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
int p2a(job_parameters_t *job_parameters, c1812_parameters_t *parameters, datafile_t *datafiles);

#endif