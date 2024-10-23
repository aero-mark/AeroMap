#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include "AeroMap.h"		// application header

class PointCloud
{
public:

	static void filter(XString input_point_cloud, XString output_point_cloud, XString output_stats, double stddev = 2.5, double sample_radius = 0, int max_concurrency = 1 /* boundary=None */);
	static RectD get_extent(XString input_point_cloud);
	static double get_spacing(XString stats_file, double resolution_fallback = 5.0);
};

#endif // #ifndef POINTCLOUD_H
