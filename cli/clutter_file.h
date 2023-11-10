#ifndef CLUTTER_FILE_H
#define CLUTTER_FILE_H

#include <stdint.h>

typedef struct
{
	int y_size;	   // rows
	int x_size;	   // columns
	double *y;	   // y axis ticks
	double *x;	   // x axis ticks
	uint16_t **Ct; // grid of height values, Ct[y][x], [centimeters]
} clutter_file_t;

/**
 * @brief Clear clutter data file.
 *
 * @param cf Pointer to ctfile_t structure
 */
void cf_zero(clutter_file_t *cf);

/**
 * @brief Read clutter data file from disk.
 *
 * @param cf Pointer to ctfile_t structure
 * @param path Path to clutter data file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int cf_open(clutter_file_t *cf, const char *path);

/**
 * @brief Deallocate and clear clutter data file.
 *
 * @param cf Pointer to ctfile_t structure
 */
void cf_free(clutter_file_t *cf);

/**
 * @brief Get nearest known clutter to the given x, y coordinates.
 *
 * @param cf The ctfile_t structure to search.
 * @param x The x coordinate.
 * @param y The y coordinate.
 *
 * @return Nearest known clutter value.
 */
uint16_t cf_get_nn(clutter_file_t *cf, const double x, const double y);

/**
 * @brief Get bilinearly interpolated clutter to the given x, y coordinates.
 *
 * @param cf The ctfile_t structure to search.
 * @param x The x coordinate.
 * @param y The y coordinate.
 *
 * @return Bilinearly interpolated clutter value.
 */
uint16_t cf_get_bilinear(clutter_file_t *cf, const double x, const double y);

#endif