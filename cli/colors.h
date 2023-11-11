#ifndef COLORS_H
#define COLORS_H

typedef enum colormap
{
	COLORMAP_MAGMA,
	COLORMAP_INFERNO,
	COLORMAP_PLASMA,
	COLORMAP_VIRIDIS,
	COLORMAP_CIVIDIS,
	COLORMAP_TWILIGHT,
	COLORMAP_TWILIGHT_SHIFTED,
	COLORMAP_TURBO,
	COLORMAP_BLUES,
	COLORMAP_BRBG,
	COLORMAP_BUGN,
	COLORMAP_BUPU,
	COLORMAP_CMRMAP,
	COLORMAP_GNBU,
	COLORMAP_GREENS,
	COLORMAP_GREYS,
	COLORMAP_ORRD,
	COLORMAP_ORANGES,
	COLORMAP_PRGN,
	COLORMAP_PIYG,
	COLORMAP_PUBU,
	COLORMAP_PUBUGN,
	COLORMAP_PUOR,
	COLORMAP_PURD,
	COLORMAP_PURPLES,
	COLORMAP_RDBU,
	COLORMAP_RDGY,
	COLORMAP_RDPU,
	COLORMAP_RDYLBU,
	COLORMAP_RDYLGN,
	COLORMAP_REDS,
	COLORMAP_SPECTRAL,
	COLORMAP_WISTIA,
	COLORMAP_YLGN,
	COLORMAP_YLGNBU,
	COLORMAP_YLORBR,
	COLORMAP_YLORRD,
	COLORMAP_AFMHOT,
	COLORMAP_AUTUMN,
	COLORMAP_BINARY,
	COLORMAP_BONE,
	COLORMAP_BRG,
	COLORMAP_BWR,
	COLORMAP_COOL,
	COLORMAP_COOLWARM,
	COLORMAP_COPPER,
	COLORMAP_CUBEHELIX,
	COLORMAP_FLAG,
	COLORMAP_GIST_EARTH,
	COLORMAP_GIST_GRAY,
	COLORMAP_GIST_HEAT,
	COLORMAP_GIST_NCAR,
	COLORMAP_GIST_RAINBOW,
	COLORMAP_GIST_STERN,
	COLORMAP_GIST_YARG,
	COLORMAP_GNUPLOT,
	COLORMAP_GNUPLOT2,
	COLORMAP_GRAY,
	COLORMAP_HOT,
	COLORMAP_HSV,
	COLORMAP_JET,
	COLORMAP_NIPY_SPECTRAL,
	COLORMAP_OCEAN,
	COLORMAP_PINK,
	COLORMAP_PRISM,
	COLORMAP_RAINBOW,
	COLORMAP_SEISMIC,
	COLORMAP_SPRING,
	COLORMAP_SUMMER,
	COLORMAP_TERRAIN,
	COLORMAP_WINTER
} colormap_t;

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
 * @brief Get colormap_t from string
 *
 * @param s String
 *
 * @return Colormap enum value
 */
colormap_t cmap_parse(char *s);

/**
 * @brief Get RGB value from 'inferno' colormap from matplotlib
 *
 * @param cmap Colormap
 * @param v Value between 0.0 and 1.0
 *
 * @return RGB value
 */
int cmap_get(colormap_t cmap, double v);

#endif