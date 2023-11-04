#ifndef DL_P_H
#define DL_P_H

//   This function computes the diffraction loss not exceeded for p// of time
//   as defined in ITU-R P.1812-5 (Section 4.3-5) and Attachment 4 to Annex 1
//
//     Input parameters:
//     d       -   vector of distances di of the i-th profile point (km)
//     g       -   vector gi of heights of the i-th profile point (meters
//                 above mean sea level) + Representative clutter height. 
//                 Both vectors g and d contain n+1 profile points
//     hts     -   transmitter antenna height in meters above sea level (i=0)
//     hrs     -   receiver antenna height in meters above sea level (i=n)
//     hstd    -   Effective height of interfering antenna (m amsl) c.f. 5.6.2
//     hsrd    -   Effective height of interfered-with antenna (m amsl) c.f. 5.6.2
//     f       -   frequency expressed in GHz
//     omega   -   the fraction of the path over sea
//     p       -   percentage of time
//     b0      -   the time percentage that the refractivity gradient (DELTA-N) exceeds 100 N-units/km in the first 100m of the lower atmosphere
//     DN      -   the average radio-refractive index lapse-rate through the
//                 lowest 1 km of the atmosphere. Note that DN is positive
//                 quantity in this procedure
//     flag4   -   Set to 1 if the alternative method is used to calculate Lbulls 
//                 without using terrain profile analysis (Attachment 4 to Annex 1)
//
//     Output parameters:
//     Ldp    -   diffraction loss for the general path not exceeded for p // of the time 
//                according to Section 4.3.4 of ITU-R P.1812-4. 
//                Ldp(1) is for the horizontal polarization 
//                Ldp(2) is for the vertical polarization
//     Ldb    -   diffraction loss for p = beta_0//
//     Ld50   -   diffraction loss for p = 50//
//     Lbulla50 -   Bullington diffraction (4.3.1) for actual terrain profile g and antenna heights
//     Lbulls50 -   Bullington diffraction (4.3.1) with all profile heights g set to zero and modified antenna heights
//     Ldshp50  -   Spherical diffraction (4.3.2) for the actual path d and modified antenna heights
//
//

typedef struct
{
    int n;
    double *d;
    double *g;
    double hts;
    double hrs;
    double hstd;
    double hsrd;
    double f;
    double omega;
    double p;
    double b0;
    double DN;
} dl_p_input_t;

typedef struct
{
    double Ldp[2];
    double Ldb[2];
    double Ld50[2];
    double Lbulla50[2];
    double Lbulls50[2];
    double Ldshp50[2];
} dl_p_output_t;

void dl_p(dl_p_input_t *input, dl_p_output_t *output);

#endif