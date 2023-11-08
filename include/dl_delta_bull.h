#ifndef DL_DELTA_BULL_H
#define DL_DELTA_BULL_H

typedef struct
{
    int n;
    double *d;
    double *h;
    double *Ct;
    double hts;
    double hrs;
    double hstd;
    double hsrd;
    double ap;
    double f;
    double lambda;
    double dtot;
    double omega;
} dl_delta_bull_input_t;

typedef struct
{
    double Ld[2];
    double Lbulla;
    double Lbulls;
    double Ldsph[2];
} dl_delta_bull_output_t;

/**
 * This function computes the complete 'delta-Bullington' diffraction loss
 * as defined in ITU-R P.1812-4 (Section 4.3.4)
 *
 * @param input->n number of terrain profile points
 * @param input->d distance of terrain profile points in km
 * @param input->h terrain profile in meters above sea level
 * @param input->Ct clutter height above terrain
 * @param input->hts transmitter antenna height in meters above sea level (i=0)
 * @param input->hrs receiver antenna height in meters above sea level (i=n)
 * @param input->hstd Effective height of interfering antenna (m amsl) c.f.
 * @param input->hsrd Effective height of interfered-with antenna (m amsl) c.f.
 * @param input->ap the effective Earth radius in kilometers
 * @param input->f frequency expressed in GHz
 * @param input->omega the fraction of the path over sea
 *
 * @return
 * Ld = diffraction loss for the general path according to Section 4.3.3 of ITU-R P.1812-4,
 * Lbulla = Bullington diffraction (4.3.1) for actual terrain profile g and antenna heights,
 * Lbulls = Bullington diffraction (4.3.1) with all profile heights g set to zero and modified antenna heights,
 * Ldshp = Spherical diffraction (4.3.2) for the actual path d and modified antenna heights
 */
void dl_delta_bull(dl_delta_bull_input_t *input, dl_delta_bull_output_t *output);

#endif