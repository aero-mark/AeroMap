// StageMVS.cpp
// Multi view stereo.
//

#include "StageMVS.h"

int StageMVS::Run()
{
	// Inputs:
	// Outputs:
	//

	int status = 0;

	GetApp()->LogWrite("OpenMVS...");
	BenchmarkStart();

	// check if reconstruction was done before
	if ((QFile::exists(tree.openmvs_model.c_str()) == false) || Rerun())
	{
		//	if self.rerun() :
		//		if io.dir_exists(tree.openmvs) :
		//			shutil.rmtree(tree.openmvs)

		GetApp()->LogWrite("Exporting from OpenSFM...");
		QStringList args;
		args.push_back("export_openmvs");
		args.push_back(tree.opensfm.c_str());
		AeroLib::RunProgramEnv(tree.prog_opensfm, args);
		// cmd: opensfm export_openmvs "d:\test_odm\opensfm"

		XString depthmaps_dir = XString::CombinePath(tree.openmvs, "depthmaps");
		//if io.dir_exists(depthmaps_dir) and self.rerun():
			//shutil.rmtree(depthmaps_dir)
		AeroLib::CreateFolder(depthmaps_dir);

		//int depthmap_resolution = get_depthmap_resolution(args, photos)
		//log.ODM_INFO("Depthmap resolution set to: %spx" % depthmap_resolution)

		int max_resolution = 4000;		// int(outputs['undist_image_max_size'])

		int resolution_level = 3;
		//if outputs["undist_image_max_size"] <= depthmap_resolution:
			//resolution_level = 0
		//else:
			//resolution_level = int(round(math.log(outputs['undist_image_max_size'] / float(depthmap_resolution)) / math.log(2)))

		GetApp()->LogWrite("Densify pass 1/2...");
		args.clear();
		args.push_back(XString::CombinePath(tree.openmvs, "scene.mvs").c_str());
		args.push_back("-w");
		args.push_back(depthmaps_dir.c_str());
		args.push_back("--dense-config-file");
		args.push_back(XString::CombinePath(tree.openmvs, "Densify.ini").c_str());
		args.push_back("--resolution-level");		// how many times to scale down the images before point cloud computation
		args.push_back(XString::Format("%d", resolution_level).c_str());
		args.push_back("--max-resolution");			// do not scale images higher than this resolution
		args.push_back(XString::Format("%d", max_resolution).c_str());
		args.push_back("--max-threads");
		args.push_back(XString::Format("%d", arg.max_concurrency).c_str());
		args.push_back("--number-views-fuse");		// number of views used for depth-map estimation (0-all neighbor views available)
		args.push_back("2");
		args.push_back("--sub-resolution-levels");	// number of patch-match sub-resolution iterations
		args.push_back("2");
		args.push_back("--archive-type");
		args.push_back("3");
		args.push_back("-v");
		args.push_back("0");
		args.push_back("--cuda-device");
		args.push_back("-1");
		AeroLib::RunProgramEnv(tree.prog_densify, args);
		// cmd: DensifyPointCloud 
		//			"d:/test_odm/opensfm/undistorted/openmvs/scene.mvs"
		//			-w "d:/test_odm/opensfm/undistorted/openmvs/depthmaps"
		//			--dense-config-file "d:/test_odm/opensfm/undistorted/openmvs/Densify.ini"
		//			--resolution-level 3
		//			--max-resolution 4000 
		//			--max-threads 16 --number-views-fuse 2 --sub-resolution-levels 2 --archive-type 3
		//			-v 0 --cuda-device -1

		GetApp()->LogWrite("Densify pass 2/2...");
		args.clear();
		args.push_back("--filter-point-cloud");
		args.push_back("-20");						// literal in odm
		args.push_back("-i");
		args.push_back(XString::CombinePath(tree.openmvs, "scene_dense.mvs").c_str());
		args.push_back("-v");
		args.push_back("0");
		args.push_back("--cuda-device");
		args.push_back("-1");
		AeroLib::RunProgramEnv(tree.prog_densify, args);
		// cmd: DensifyPointCloud 
		//			--filter-point-cloud -20
		//			-i "d:\test_odm\opensfm\undistorted\openmvs\scene_dense.mvs"
		//			-v 0 --cuda-device -1
	}

	BenchmarkStop("OpenMVS");

	return status;
}
