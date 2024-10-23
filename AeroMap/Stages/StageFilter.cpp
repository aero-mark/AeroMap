// StageFilter.cpp
// Statistical point filtering.
//

#include "PointCloud.h"
#include "StageFilter.h"

int StageFilter::Run()
{
	// Inputs:
	// Outputs:
	//

	int status = 0;

	GetApp()->LogWrite("Filter points...");
	BenchmarkStart();

    AeroLib::CreateFolder(tree.odm_filterpoints);

    XString inputPointCloud = "";

    // check if reconstruction was done before
    if (QFile::exists(tree.filtered_point_cloud.c_str()) || Rerun())
    {
        if (arg.fast_orthophoto)
            inputPointCloud = XString::CombinePath(tree.opensfm, "reconstruction.ply");
        else
            inputPointCloud = tree.openmvs_model;

        // Check if we need to compute boundary
        //if args.auto_boundary:
        {
            //if reconstruction.is_georeferenced():
            //    if not 'boundary' in outputs:
            //        boundary_distance = None

            //        if args.auto_boundary_distance > 0:
            //            boundary_distance = args.auto_boundary_distance
            //        else:
            //            avg_gsd = gsd.opensfm_reconstruction_average_gsd(tree.opensfm_reconstruction)
            //            if avg_gsd is not None:
            //                boundary_distance = avg_gsd * 100 # 100 is arbitrary

            //        if boundary_distance is not None:
            //            outputs['boundary'] = compute_boundary_from_shots(tree.opensfm_reconstruction, boundary_distance, reconstruction.get_proj_offset())
            //            if outputs['boundary'] is None:
            //                log.ODM_WARNING("Cannot compute boundary from camera shots")
            //        else:
            //            log.ODM_WARNING("Cannot compute boundary (GSD cannot be estimated)")
            //    else:
            //        log.ODM_WARNING("--auto-boundary set but so is --boundary, will use --boundary")
            //else:
            //    log.ODM_WARNING("Not a georeferenced reconstruction, will ignore --auto-boundary")
        }

        PointCloud::filter(inputPointCloud, tree.filtered_point_cloud, tree.filtered_point_cloud_stats,
            arg.pc_filter, arg.pc_sample, arg.max_concurrency /*boundary_offset(outputs.get('boundary'), reconstruction.get_proj_offset())*/);

        //# Quick check
        //info = point_cloud.ply_info(tree.filtered_point_cloud)
        //if info["vertex_count"] == 0:
        //    extra_msg = ''
        //    if 'boundary' in outputs:
        //        extra_msg = '. Also, since you used a boundary setting, make sure that the boundary polygon you specified covers the reconstruction area correctly.'
        //    raise system.ExitException("Uh oh! We ended up with an empty point cloud. This means that the reconstruction did not succeed. Have you followed best practices for data acquisition? See https://docs.opendronemap.org/flying/%s" % extra_msg)
    }
    else
    {
        Logger::Write(__FUNCTION__, "Found existing point cloud file: '%s'", tree.filtered_point_cloud.c_str());
    }

    //if args.optimize_disk_space and inputPointCloud:
    //    if os.path.isfile(inputPointCloud):
    //        os.remove(inputPointCloud)

	BenchmarkStop("Filter points");

	return status;
}
