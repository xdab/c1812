#ifndef OUTFILE_H
#define OUTFILE_H

#include <stdio.h>

typedef struct
{
    FILE *file;
    int n;
} outfile_t;

/**
 * @brief Open an output file.
 *
 * @param outfile Pointer to the output file structure.
 * @param filename Name of the output file.
 */
int outfile_open(outfile_t *outfile, const char *filename);

/**
 * @brief Write the header of the output file.
 *
 * @param outfile Pointer to the output file structure.
 * @param txx Center x coordinate.
 * @param txy Center y coordinate.
 * @param radius Radius of the circle.
 * @param ares Angular resolution.
 * @param n Number of points in a ray.
 */
int outfile_write_header(outfile_t *outfile, double txx, double txy, double radius, double ares, int n);

/**
 * @brief Write a ray to the output file.
 *
 * @param outfile Pointer to the output file structure.
 * @param Lb Pointer to the ray array.
 */
int outfile_write_ray(outfile_t *outfile, double *Lb);

/**
 * @brief Close the output file.
 *
 * @param outfile Pointer to the output file structure.
 */
int outfile_close(outfile_t *outfile);

#endif