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
} seh_input_t;

typedef struct
{
    double *g;  // path heights with representative clutter [m]
    double hts; // transmitter height above mean sea level [m]
    double hrs; // receiver height above mean sea level [m]
    double htc;
    double hrc;
    double hst;
    double hsr;
    double hstp;
    double hsrp;
    double hstd;
    double hsrd;
    double hte;
    double hre;
    double theta;
    double dlt;
    double dlr;
} seh_output_t;

void smooth_earth_heights(seh_input_t *input, seh_output_t *output);

#endif