#ifndef JOBFILE_H
#define JOBFILE_H

#include "c1812/parameters.h"

#define MAX_LINE_LENGTH 256
#define MAX_FIELD_LENGTH 32
#define MAX_VALUE_LENGTH (MAX_LINE_LENGTH - MAX_FIELD_LENGTH - 1)
#define MAX_TERRAIN_FILES 1
#define MAX_CLUTTER_FILES 1

typedef struct
{
    double txx;    // Transmitter X coordinate [m]
    double txy;    // Transmitter Y coordinate [m]
    double txh;    // Transmitter height above ground [m]
    double txpwr;  // Transmitter power [W]
    double txgain; // Transmitter antenna gain [dBi]

    double rxx;    // Receiver X coordinate [m]
    double rxy;    // Receiver Y coordinate [m]
    double rxh;    // Receiver height above ground [m]
    double rxgain; // Receiver antenna gain [dBi]

    double radius; // Point-to-area calculation radius [m]
    double xres;   // Calculation spatial resolution [m]
    double ares;   // Calculation angular resolution [deg]
    int threads;   // Number of threads to use, default 1

    char out[MAX_VALUE_LENGTH];                        // Output file path
    char img[MAX_VALUE_LENGTH];                        // Output image file path
    char terrain[MAX_TERRAIN_FILES][MAX_VALUE_LENGTH]; // Terrain file paths
    char clutter[MAX_CLUTTER_FILES][MAX_VALUE_LENGTH]; // Clutter file paths

} job_parameters_t;

/**
 * @brief Clear job parameters.
 *
 * @param job_parameters Pointer to job parameters.
 */
void jobfile_zero(job_parameters_t *job_parameters);

/**
 * @brief Read job and calculation parameters from file.
 *
 * @param job_parameters Pointer to job parameters.
 * @param parameters Pointer to calculation parameters.
 * @param path Path to job file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int jobfile_read(job_parameters_t *job_parameters, c1812_parameters_t *parameters, const char *path);

#endif