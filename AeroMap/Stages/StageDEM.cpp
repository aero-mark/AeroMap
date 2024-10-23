// StageDEM.cpp
// Generate DTM/DSM.
//

#include "DEM.h"
#include "Gsd.h"
#include "StageDEM.h"

int StageDEM::Run()
{
	// Inputs:
	// Outputs:
	//		+ odm_dem
	//			dtm.tif
	//			dsm.tif
	//

	int status = 0;

	GetApp()->LogWrite("DEM...");
	BenchmarkStart();

	XString dem_input = tree.odm_georef_model_laz;
	bool pc_model_found = AeroLib::FileExists(dem_input);
	bool ignore_resolution = false;
	bool pseudo_georeference = false;

	AeroLib::Georef georef = AeroLib::ReadGeoref();
	if (georef.is_valid == false)
	{
		Logger::Write(__FUNCTION__, "Not georeferenced, using ungeoreferenced point cloud.");
		ignore_resolution = true;
		pseudo_georeference = true;
	}

//TODO:
	bool reconstruction_has_gcp = false;
	double resolution = Gsd::cap_resolution(
		arg.dem_resolution,						// resolution
		tree.opensfm_reconstruction,			// reconstruction json
		0.1,									// error estimate
		1.0,									// gsd scaling
		arg.ignore_gsd,							// ignore gsd
		ignore_resolution && arg.ignore_gsd,	// ignore resolution
		reconstruction_has_gcp);				// has ground control points
		
	// define paths and create working directories
	AeroLib::CreateFolder(tree.odm_dem);

	//if arg.pc_classify and pc_model_found:
	{
		//pc_classify_marker = XString::CombinePath(odm_dem_root, 'pc_classify_done.txt')

		//if not io.file_exists(pc_classify_marker) or self.rerun():
		//    log.ODM_INFO("Classifying {} using Simple Morphological Filter (1/2)".format(dem_input))
		//    commands.classify(dem_input,
		//                        arg.smrf_scalar,
		//                        arg.smrf_slope,
		//                        arg.smrf_threshold,
		//                        arg.smrf_window
		//                    )

		//    log.ODM_INFO("Classifying {} using OpenPointClass (2/2)".format(dem_input))
		//    classify(dem_input, arg.max_concurrency)

		//    with open(pc_classify_marker, 'w') as f:
		//        f.write('Classify: smrf\n')
		//        f.write('Scalar: {}\n'.format(arg.smrf_scalar))
		//        f.write('Slope: {}\n'.format(arg.smrf_slope))
		//        f.write('Threshold: {}\n'.format(arg.smrf_threshold))
		//        f.write('Window: {}\n'.format(arg.smrf_window))
	}

	//if arg.pc_rectify:
	//    commands.rectify(dem_input)

	// Do we need to process anything here?
	if ((arg.dsm || arg.dtm) && pc_model_found)
	{
		XString dsm_output_filename = XString::CombinePath(tree.odm_dem, "dsm.tif");
		XString dtm_output_filename = XString::CombinePath(tree.odm_dem, "dtm.tif");

		if ((arg.dtm && AeroLib::FileExists(dtm_output_filename) == false) ||
			(arg.dsm && AeroLib::FileExists(dsm_output_filename) == false) ||
			Rerun())
		{
			std::vector<XString> products;

			if (arg.dsm || (arg.dtm && arg.dem_euclidean_map))
				products.push_back("dsm");
			if (arg.dtm)
				products.push_back("dtm");

			std::vector<double> radius_steps = DEM::get_dem_radius_steps(tree.filtered_point_cloud_stats, arg.dem_gapfill_steps, resolution);

			for (XString product : products)
			{
				XString output_type = "max";
				if (product == "dtm")
					output_type = "idw";

				int max_tiles = GetProject().GetImageCount() / 2;

				DEM::create_dem(
					dem_input,                      // input point cloud
					product,                        // dem type
					radius_steps,
					output_type,
					arg.dem_gapfill_steps > 0,      // gap fill
					tree.odm_dem,                   // output directory
					resolution / 100.0,             // TODO: why /100 here?
					arg.max_concurrency,
					4096,                           // max tile size
					arg.dem_decimation,
					arg.dem_euclidean_map,          // keep unfilled copy
					true,                           // apply smoothing
					max_tiles);

		            //dem_geotiff_path = XString::CombinePath(odm_dem_root, "{}.tif".format(product))
		            //bounds_file_path = XString::CombinePath(tree.odm_georeferencing, "odm_georeferenced_model.bounds.gpkg")

		            //if arg.crop > 0 or arg.boundary:
		            //    # Crop DEM
		            //    Cropper.crop(bounds_file_path, dem_geotiff_path, utils.get_dem_vars(args), keep_original=not arg.optimize_disk_space)

		            //if arg.dem_euclidean_map:
		            //    unfilled_dem_path = io.related_file_path(dem_geotiff_path, postfix=".unfilled")

		            //    if arg.crop > 0 or arg.boundary:
		            //        # Crop unfilled DEM
		            //        Cropper.crop(bounds_file_path, unfilled_dem_path, utils.get_dem_vars(args), keep_original=not arg.optimize_disk_space)

		            //    commands.compute_euclidean_map(unfilled_dem_path,
		            //                        io.related_file_path(dem_geotiff_path, postfix=".euclideand"),
		            //                        overwrite=True)

		            //if pseudo_georeference:
		            //    pseudogeo.add_pseudo_georeferencing(dem_geotiff_path)

		            //if arg.tiles:
		            //    generate_dem_tiles(dem_geotiff_path, tree.path("%s_tiles" % product), arg.max_concurrency)

		            //if arg.cog:
		            //    convert_to_cogeo(dem_geotiff_path, max_workers=arg.max_concurrency)
			}
		}
		else
		{
			Logger::Write(__FUNCTION__, "Found existing outputs in: %s", tree.odm_dem.c_str());
		}
	}
	else
	{
		Logger::Write(__FUNCTION__, "DEM will not be generated");
	}

	BenchmarkStop("DEM");

	return status;
}
