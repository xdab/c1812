#ifndef TERRAIN_FILE_H
#define TERRAIN_FILE_H

typedef struct
{
    int y_size; // rows
    int x_size; // columns
    double *y;  // y axis ticks
    double *x;  // x axis ticks
    double **h; // grid of height values, h[y][x]
} terrain_file_t;

/**
 * @brief Clear terrain file structure.
 *
 * @param tf Pointer to terrain_file_t structure.
 */
void tf_zero(terrain_file_t *tf);

/**
 * @brief Parse terrain file from disk.
 *
 * @param tf Pointer to terrain_file_t structure.
 * @param path Path to terrain file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int tf_parse(terrain_file_t *tf, const char *path);

/**
 * @brief Store terrain file to disk.
 *
 * @param tf Pointer to terrain_file_t structure.
 * @param path Path to terrain file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int tf_store(terrain_file_t *tf, const char *path);

/**
 * @brief Open terrain file from disk.
 *
 * @param tf Pointer to terrain_file_t structure.
 * @param path Path to terrain file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int tf_open(terrain_file_t *tf, const char *path);

/**
 * @brief Deallocate and clear terrain_file_t structure.
 *
 * @param tf Pointer to terrain_file_t structure.
 */
void tf_free(terrain_file_t *tf);

/**
 * @brief Get nearest known height to the given x, y coordinates.
 *
 * @param tf The terrain_file_t structure to search.
 * @param x The x coordinate.
 * @param y The y coordinate.
 *
 * @return h - The nearest known height value.
 */
double tf_get_nn(terrain_file_t *tf, const double x, const double y);

/**
 * @brief Returns the bilinearly interpolated height to the given x, y coordinates.
 *
 * @param tf The terrain_file_t structure to search.
 * @param x The x coordinate.
 * @param y The y coordinate.
 *
 * @return h - The bilinearly interpolated height value.
 */
double tf_get_bilinear(terrain_file_t *tf, const double x, const double y);

#endif