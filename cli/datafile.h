#ifndef DATAFILE_H
#define DATAFILE_H

typedef struct
{
    int y_size;
    int x_size;
    double *y;
    double *x;
    double **h; // y,x order
} datafile_t;

// I/O

void datafile_zero(datafile_t *datafile);
void datafile_parse(datafile_t *datafile, const char *path);
void datafile_store(datafile_t *datafile, const char *path);
void datafile_open(datafile_t *datafile, const char *path);
void datafile_free(datafile_t *datafile);

// Operations

/*
 * Returns the nearest known height to the given x, y coordinates.
 *
 * @param datafile The datafile to search.
 * @param x The x coordinate.
 * @param y The y coordinate.
 * 
 * @return h - The nearest known height value.
 */
double datafile_get_nn(datafile_t *datafile, const double x, const double y);

/*
 * Returns the bilinearly interpolated height to the given x, y coordinates.
 *
 * @param datafile The datafile to search.
 * @param x The x coordinate.
 * @param y The y coordinate.
 * 
 * @return h - The bilinearly interpolated height value.
 */
double datafile_get_bilinear(datafile_t *datafile, const double x, const double y);

#endif