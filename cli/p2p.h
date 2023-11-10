#ifndef P2P_H
#define P2P_H

#include "jobfile.h"
#include "c1812/parameters.h"
#include "terrain_file.h"
#include "clutter_file.h"

/**
 * @brief Point-to-point calculation
 *
 * @param job_parameters Job parameters
 * @param parameters Calculation parameters
 * @param tfs Terrain data files
 * @param cfs Clutter data files
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
int p2p(job_parameters_t *job_parameters, c1812_parameters_t *parameters, terrain_file_t *tfs, clutter_file_t *cfs);

#endif
