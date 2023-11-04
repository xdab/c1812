#ifndef CALCULATION_H
#define CALCULATION_H

#include "parameters.h"
#include "results.h"

typedef struct
{
    // Copied from parameters
    double p; // time percentage [%]
    double f; // frequency [GHz]
    int n; // number of points
    double *d; // path distances [km]
    double *h; // path heights [m]
    double *Ct; // representative clutter heights [m]
    double htg; // transmitter height above ground [m]
    double hrg; // receiver height above ground [m]
    double DN; 

    // Calculated in c1812_calculate
    double lambda; // wavelength [m]
    double dtot; // total great-circle path distance [km]
    double dtm; // longest continuous land section of the great-circle path [km]
    double dlm; // longest continuous inland section of the great-circle path [km]
    double b0; // beta0
    double ae, ab; 
    double omega; // fraction of the path over sea

    // Calculated in smooth_earth_heights
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
    
} c1812_calculation_context_t;

void c1812_calculate(c1812_parameters_t *parameters, c1812_results_t *results);

#endif