// StageTexture.cpp
// Apply textures
//

#include "StageTexture.h"

int StageTexture::Run()
{
	// Inputs:
	// Outputs:
	//		+ odm_texturing
	//			odm_textured_model_geo.conf
	//			odm_textured_model_geo.mtl
	//			odm_textured_model_geo.obj
	//		+ odm_texturing_25d
	//			odm_textured_model_geo.conf
	//			odm_textured_model_geo.mtl
	//			odm_textured_model_geo.obj
	//

	int status = 0;

	GetApp()->LogWrite("Texture...");
	BenchmarkStart();

	//max_dim = find_largest_photo_dim(reconstruction.photos)
	m_max_texture_size = 8 * 1024;	// default

	//if max_dim > 8000:
	//	log.ODM_INFO("Large input images (%s pixels), increasing maximum texture size." % max_dim)
	//	max_texture_size *= 3

	TextureModel3D();
	TextureModel25D();

	BenchmarkStop("Texture");

	return status;
}

int StageTexture::TextureModel3D()
{
	int status = 0;
	
	AeroLib::CreateFolder(tree.odm_texturing);

	XString odm_textured_model_obj = XString::CombinePath(tree.odm_texturing, tree.odm_textured_model_obj);
	if ((AeroLib::FileExists(odm_textured_model_obj) == false) || Rerun())
	{
		GetApp()->LogWrite("Writing MVS textured file: '%s'", odm_textured_model_obj.c_str());

		XString out_prefix = XString::CombinePath(tree.odm_texturing, "odm_textured_model_geo");

		QStringList args;
		args.push_back(tree.opensfm_reconstruction_nvm.c_str());		// input scene
		args.push_back(tree.odm_mesh.c_str());							// input mesh
		args.push_back(out_prefix.c_str());								// output prefix
		args.push_back("-d");					// Data term: {area, gmi}
		args.push_back("gmi");
		args.push_back("-o");					// Photometric outlier (pedestrians etc.) removal method: {none, gauss_damping, gauss_clamping}
		args.push_back("gauss_clamping");		
		args.push_back("-t");
		args.push_back("none");					// Tone mapping method: {none, gamma}
		args.push_back("--no_intermediate_results");
		args.push_back(XString::Format("--max_texture_size=%d", m_max_texture_size).c_str());
		AeroLib::RunProgramEnv(tree.prog_recon_tex, args);
		// cmd: texrecon 
		//			"d:/test_odm/opensfm/undistorted/reconstruction.nvm" 
		//			"d:/test_odm/odm_meshing/odm_mesh.ply" 
		//			"d:/test_odm/odm_texturing/odm_textured_model_geo" 
		//			-d gmi -o gauss_clamping -t none --no_intermediate_results --max_texture_size=8192
	}
	else
	{
		Logger::Write(__FUNCTION__, "Found existing textured model: '%s'", odm_textured_model_obj.c_str());
	}
	
	return status;
}

int StageTexture::TextureModel25D()
{
	int status = 0;

	AeroLib::CreateFolder(tree.odm_25dtexturing);

	XString odm_textured_model_obj = XString::CombinePath(tree.odm_25dtexturing, tree.odm_textured_model_obj);
	if ((AeroLib::FileExists(odm_textured_model_obj) == false) || Rerun())
	{
		GetApp()->LogWrite("Writing MVS textured file: '%s'", odm_textured_model_obj.c_str());

		XString out_prefix = XString::CombinePath(tree.odm_25dtexturing, "odm_textured_model_geo");

		QStringList args;
		args.push_back(tree.opensfm_reconstruction_nvm.c_str());		// input scene
		args.push_back(tree.odm_25dmesh.c_str());						// input mesh
		args.push_back(out_prefix.c_str());								// output prefix
		args.push_back("-d");
		args.push_back("gmi");
		args.push_back("-o");
		args.push_back("gauss_clamping");
		args.push_back("-t");
		args.push_back("none");
		args.push_back("--nadir_mode");
		args.push_back("--no_intermediate_results");
		args.push_back(XString::Format("--max_texture_size=%d", m_max_texture_size).c_str());
		AeroLib::RunProgramEnv(tree.prog_recon_tex, args);
		// cmd: texrecon 
		//			"d:/test_odm/opensfm/undistorted/reconstruction.nvm" 
		//			"d:/test_odm/odm_meshing/odm_25dmesh.ply" 
		//			"d:/test_odm/odm_texturing_25d/odm_textured_model_geo" 
		//			-d gmi -o gauss_clamping -t none --no_intermediate_results
		//		    --nadir_mode
		//			--max_texture_size=8192
	}
	else
	{
		Logger::Write(__FUNCTION__, "Found existing textured model: '%s'", odm_textured_model_obj.c_str());
	}

	return status;
}
