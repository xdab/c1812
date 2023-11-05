#include "parameters.h"
#include "calculation.h"
#include "rf.h"
#include "sunit.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
	c1812_parameters_t parameters;

	parameters.f = 0.145;
	parameters.p = 50;
	parameters.htg = 10;
	parameters.hrg = 10;
	parameters.pol = POLARIZATION_VERTICAL;
	parameters.zone = RC_ZONE_INLAND;
	parameters.ws = 20;
	parameters.lon = 21.0;
	parameters.lat = 52.0;
	parameters.N0 = 300;
	parameters.DN = 40;

	const int n = 10;		// Number of points
	const double dt = 20.0; // Total distance [km]

	double d[n], h[n];
	for (int i = 0; i < n; i++)
	{
		d[i] = i * dt / (n - 1);
		h[i] = 75;
	}
	
	parameters.n = n;
	parameters.d = d;
	parameters.h = h;
	parameters.Ct = NULL;

	c1812_results_t results;

	c1812_calculate(&parameters, &results);

	putchar('\n');
	if (results.error == RESULTS_ERR_NONE)
	{
		double P = 25; // Power [W]
		double Gt = 9; // Transmitter antenna gain [dBi]
		double Gr = 1; // Receiver antenna gain [dBi]
		double Prx = link_budget(P, Gt, Gr, results.Lb);
		s_unit_t S;
		dBm_to_s_unit(Prx, &S);

		printf("Loss = %.1f dB\n", results.Lb);
		printf("Received power = %.1f dBm (S%d + %.1fdB)\n", Prx, S.full_units, S.dB_over);
	}
	else
	{
		printf("Error: %d\n", results.error);
	}

	return 0;
}
