#ifndef SMOOTH_EARTH_HEIGHTS_H
#define SMOOTH_EARTH_HEIGHTS_H

typedef struct
{
    int n;
    double *d;
    double *h;
    double *Ct;
    double dtot;
    double lambda;
    double htg;
    double hrg;
    double ae;

    // Optional caches
    double *v1_cache;
    double *v2_cache;
	double *theta_max_cache;
} seh_input_t;

typedef struct
{
    double hts; // transmitter height above mean sea level [m]
    double hrs; // receiver height above mean sea level [m]
    double htc;
    double hrc;
    double hst;
    double hsr;
    double hst_n;
    double hsr_n;
    double hstp;
    double hsrp;
    double hstd;
    double hsrd;
    double hte;
    double hre;
    double theta;
    double theta_t;
    double theta_r;
    double dlt;
    double dlr;
    double hm;
} seh_output_t;

void smooth_earth_heights(seh_input_t *input, seh_output_t *output);

#endif