#ifndef GSD_H
#define GSD_H

#include "AeroMap.h"		// application header

class Gsd
{
public:

    static double cap_resolution(double resolution, XString reconstruction_json, double gsd_error_estimate = 0.1, double gsd_scaling = 1.0,
        bool ignore_gsd = false, bool ignore_resolution = false, bool has_gcp = false);

    static double opensfm_reconstruction_average_gsd(XString reconstruction_json, bool use_all_shots = false);
    static double calculate_gsd_from_focal_ratio(double focal_ratio, double flight_height, int image_width);
};

#endif // #ifndef GSD_H
