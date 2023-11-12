#ifndef P2A_H
#define P2A_H

#include "p2pa_common.h"

/**
 * @brief Point-to-area calculation
 *
 * @param job_parameters Job parameters
 * @param parameters Calculation parameters
 * @param tfs Terrain data files
 * @param cfs Clutter data files
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
int p2a(job_parameters_t *job_parameters, c1812_parameters_t *parameters, terrain_file_t *tfs, clutter_file_t *cfs);

#endif