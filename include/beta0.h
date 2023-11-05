#ifndef BETA0_H
#define BETA0_H

/*
 * This function computes the time percentage for which refractive index lapse-rates exceeding 
 * 100 N-units/km can be expected in the first 100m of the lower atmosphere
 *
 * @param phi path centre latitude (deg)
 * @param dtm the longest continuous land (inland + coastal) section of the great-circle path (km)
 * @param dlm the longest continuous inland section of the great-circle path (km)
 *
 * @return the time percentage that the refractivity gradient (DELTA-N) exceeds 100 N-units/km in the first 100m of the lower atmosphere
 */
double beta0(double phi, double dtm, double dlm);

#endif