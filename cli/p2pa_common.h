#ifndef P2PA_COMMON_H
#define P2PA_COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "jobfile.h"
#include "terrain_file.h"
#include "clutter_file.h"

#include "c1812/parameters.h"
#include "c1812/calculate.h"
#include "c1812/sunit.h"
#include "c1812/rf.h"
#include "c1812/custom_math.h"

#define KM_M 1000.0
#define M_CM 100.0
#define M_DM 10.0

#define tf_interpolation_func tf_get_bicubic
#define cf_interpolation_func cf_get_bilinear

#endif