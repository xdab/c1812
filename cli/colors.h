#ifndef COLORS_H
#define COLORS_H

/**
 * @brief Pack RGB values into a single integer
 *
 * @param r Red value
 * @param g Green value
 * @param b Blue value
 *
 * @return Packed RGB value
 */
int pack_rgb(unsigned char r, unsigned char g, unsigned char b);

/**
 * @brief Unpack RGB values from a single integer
 *
 * @param rgb Packed RGB value
 * @param r Pointer to red value
 * @param g Pointer to green value
 * @param b Pointer to blue value
 */
void unpack_rgb(int rgb, unsigned char *r, unsigned char *g, unsigned char *b);

/**
 * @brief Get RGB value from 'inferno' colormap from matplotlib
 *
 * @param v Value between 0.0 and 1.0
 *
 * @return RGB value
 */
int cmap_inferno_get(double v);

#endif