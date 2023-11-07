#ifndef DATAFILE_H
#define DATAFILE_H

typedef struct
{
    int y_size; // rows
    int x_size; // columns
    double *y;  // y axis ticks
    double *x;  // x axis ticks
    double **h; // grid of height values, h[y][x]
} datafile_t;

/**
 * @brief Clear datafile.
 *
 * @param datafile Pointer to datafile.
 */
void datafile_zero(datafile_t *datafile);

/**
 * @brief Parse datafile from disk.
 *
 * @param datafile Pointer to datafile.
 * @param path Path to datafile.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int datafile_parse(datafile_t *datafile, const char *path);

/**
 * @brief Store datafile to disk.
 *
 * @param datafile Pointer to datafile.
 * @param path Path to datafile.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int datafile_store(datafile_t *datafile, const char *path);

/**
 * @brief Open datafile from disk.
 *
 * @param datafile Pointer to datafile.
 * @param path Path to datafile.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int datafile_open(datafile_t *datafile, const char *path);

/**
 * @brief Deallocate and clear datafile.
 *
 * @param datafile Pointer to datafile.
 */
void datafile_free(datafile_t *datafile);

/**
 * @brief Get nearest known height to the given x, y coordinates.
 *
 * @param datafile The datafile to search.
 * @param x The x coordinate.
 * @param y The y coordinate.
 *
 * @return h - The nearest known height value.
 */
double datafile_get_nn(datafile_t *datafile, const double x, const double y);

/**
 * @brief Returns the bilinearly interpolated height to the given x, y coordinates.
 *
 * @param datafile The datafile to search.
 * @param x The x coordinate.
 * @param y The y coordinate.
 *
 * @return h - The bilinearly interpolated height value.
 */
double datafile_get_bilinear(datafile_t *datafile, const double x, const double y);

#endif