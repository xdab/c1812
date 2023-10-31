#include "parameters.h"
#include "parameters_validation.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
	c1812_parameters_t parameters;

	parameters.f = 0.145;
	parameters.p = 50;
	parameters.htg = 10;
	parameters.hrg = 10;
	parameters.pol = VERTICAL;
	parameters.zone = INLAND;
	parameters.ws = 20;
	parameters.lon = 21.0;
	parameters.lat = 52.0;
	parameters.N0 = 300;
	parameters.DN = 40;

	const int n = 10;
	double d[n], h[n];
	for (int i = 0; i < n; i++) {
		d[i] = i * 0.1;
		h[i] = 75;
	}
	parameters.n = n;
	parameters.d = d;
	parameters.h = h;
	parameters.Ct = NULL;

	c1812_error_t error = c1812_validate(&parameters);
	if (error != C1812_OK) {
		printf("Error: %d\n", error);
		return 1;
	}
	
	printf("Parameters are valid.\n");
	return 0;
}
