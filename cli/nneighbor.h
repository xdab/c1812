#ifndef NNEIGHBOR_H
#define NNEIGHBOR_H

/**
 * @brief Find element in array closest to target.
 *
 * @param array Array to search.
 * @param size Size of array.
 * @param target Target value.
 * @param index Pointer to an integer to store the index of the closest element.
 *
 * @return Element in array closest to target.
 */
double nneighbor(const double *array, int size, double target, int *index);

#endif