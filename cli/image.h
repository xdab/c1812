#ifndef IMAGE_H
#define IMAGE_H

#define BYTES_PER_PIXEL 3

typedef struct
{
	int width;
	int height;
	unsigned int **rgb_data[BYTES_PER_PIXEL];
} image_t;

/**
 * @brief Create an image
 *
 * @param image Pointer to image_t struct
 * @param width Width of image
 * @param height Height of image
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
int image_create(image_t *image, int width, int height);

/**
 * @brief Set pixel in image
 *
 * @param image Pointer to image_t struct
 * @param x X coordinate
 * @param y Y coordinate
 * @param r Red value
 * @param g Green value
 * @param b Blue value
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
int image_set(image_t *image, int x, int y, unsigned int r, unsigned int g, unsigned int b);

/**
 * @brief Write image to file
 *
 * @param image Pointer to image_t struct
 * @param filename Filename
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
int image_write(image_t *image, const char *filename);

/**
 * @brief Free image
 *
 * @param image Pointer to image_t struct
 */
void image_free(image_t *image);

#endif