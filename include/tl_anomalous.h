#ifndef TL_ANOMALOUS_H
#define TL_ANOMALOUS_H

typedef struct
{
    double dtot;
    double dlt;
    double dlr;
    double dct;
    double dcr;
    double dlm;
    double hts;
    double hrs;
    double hte;
    double hre;
    double hm;
    double theta_t;
    double theta_r;
    double f;
    double p;
    double omega;
    double ae;
    double b0;
} tl_anomalous_input_t;

typedef struct
{
    double Lba;
} tl_anomalous_output_t;

/**
 * Computes the basic transmission loss occurring during periods of anomalous propagation (ducting and layer reflection)
 * as defined in ITU-R P.1812-6 (Section 4.5).
 *
 * @param dtot Great-circle path distance (km)
 * @param dlt Interfering antenna horizon distance (km)
 * @param dlr Interfered-with antenna horizon distance (km)
 * @param dct Distance over land from the transmit antenna to the coast along the great-circle interference path (km).
 *            Set to zero for a terminal on a ship or sea platform.
 * @param dcr Distance over land from the receive antenna to the coast along the great-circle interference path (km).
 *            Set to zero for a terminal on a ship or sea platform.
 * @param dlm The longest continuous inland section of the great-circle path (km)
 * @param hts Tx antenna height above mean sea level amsl (m)
 * @param hrs Rx antenna height above mean sea level amsl (m)
 * @param hte Tx terminal effective height for the ducting/layer reflection model (m)
 * @param hre Rx terminal effective height for the ducting/layer reflection model (m)
 * @param hm The terrain roughness parameter (m)
 * @param theta_t Interfering antenna horizon elevation angle (mrad)
 * @param theta_r Interfered-with antenna horizon elevation angle (mrad)
 * @param f Frequency expressed in GHz
 * @param p Percentage of time
 * @param omega Fraction of the total path over water
 * @param ae The median effective Earth radius (km)
 * @param b0 The time percentage that the refractivity gradient (DELTA-N) exceeds 100 N-units/km in the first 100m of the lower atmosphere
 *
 * @return The basic transmission loss due to anomalous propagation (ducting and layer reflection)
 */
void tl_anomalous(tl_anomalous_input_t *input, tl_anomalous_output_t *output);

#endif