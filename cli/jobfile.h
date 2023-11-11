#ifndef JOBFILE_H
#define JOBFILE_H

#include "c1812/parameters.h"
#include "colors.h"

#define MAX_LINE_LENGTH 256
#define MAX_FIELD_LENGTH 32
#define MAX_VALUE_LENGTH (MAX_LINE_LENGTH - MAX_FIELD_LENGTH - 1)
#define MAX_TERRAIN_FILES 1
#define MAX_CLUTTER_FILES 1

typedef enum
{
    IMG_DATA_TYPE_S_UNITS,
    IMG_DATA_TYPE_LOSS,
} job_parameters_img_data_t;

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

    char terrain[MAX_TERRAIN_FILES][MAX_VALUE_LENGTH]; // Terrain data file paths
    char clutter[MAX_CLUTTER_FILES][MAX_VALUE_LENGTH]; // Clutter data file paths

    char out[MAX_VALUE_LENGTH]; // Output RF file path

    char img[MAX_VALUE_LENGTH];              // Output image file path
    int img_size;                            // Output image size [px]
    colormap_t img_colormap;                 // Output image colormap
    double img_scale_min;                    // Output image scale minimum
    double img_scale_max;                    // Output image scale maximum
    job_parameters_img_data_t img_data_type; // Output image data type

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