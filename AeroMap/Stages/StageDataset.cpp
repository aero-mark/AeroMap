// StageDataset.cpp
// Load dataset.
//

#include "StageDataset.h"

int StageDataset::Run()
{
	// Inputs:
	// Outputs:
	//		+ odm_georeferencing
	//			coords.txt
	//			odm_georeferencing_model_geo.txt	<- deprecated, likely not needed at all
	//			proj.txt
	//		benchmark.txt
	//		images.json
	//		img_list.txt
	//

	int status = 0;

	GetApp()->LogWrite("Load dataset...");
	BenchmarkStart();

	InitGeoref();

	WriteImageListText();
	WriteImageListJson();

	BenchmarkStop("Load dataset", true);

	return status;
}

int StageDataset::InitGeoref()
{
	// Create & initialze georeferencing folder.
	//

	int status = 0;

	AeroLib::CreateFolder(tree.odm_georef);

	// write 'coords.txt'

	// Create a coordinate file containing the GPS positions of all cameras
	// to be used later in the ODM toolchain for automatic georeferecing

	int utm_zone = 0;
	char hemi = ' ';
	std::vector<VEC3> coords;
	
	for (Project::ImageType image : GetProject().GetImageList())
	{
		//	if photo.latitude is None or photo.longitude is None:
		//		log.ODM_WARNING("GPS position not available for %s" % photo.filename)
		//		continue

		double lat = image.exif.GeoLocation.Latitude;
		double lon = image.exif.GeoLocation.Longitude;

		if (utm_zone == 0)
		{
			utm_zone = GIS::GetUTMZone(lon);
			hemi = (lat >= 0.0 ? 'N' : 'S');
		}

		double x, y;
		GIS::LatLonToXY_UTM(lat, lon, x, y, GIS::Ellipsoid::WGS_84);

		coords.push_back(VEC3(x, y, image.exif.GeoLocation.Altitude));
	}

	//if utm_zone is None:
	//	raise Exception("No images seem to have GPS information")

	// Calculate average
	double dx = 0.0;
	double dy = 0.0;
	double num = (double)coords.size();
	for (const VEC3& coord : coords)
	{
		dx += coord.x / num;
		dy += coord.y / num;
	}

	dx = (int)floor(dx);
	dy = (int)floor(dy);

	FILE* pFile = fopen(GetProject().GetCoordsFileName().c_str(), "wt");
	if (pFile)
	{
		fprintf(pFile, "WGS84 UTM %d%c\n", utm_zone, hemi);
		fprintf(pFile, "%d %d\n", (int)dx, (int)dy);
		for (const VEC3& coord : coords)
			fprintf(pFile, "%0.15f %0.15f %0.15f\n", coord.x - dx, coord.y - dy, coord.z);

		fclose(pFile);
	}

	// write 'odm_georeferencing_model_geo.txt'

	XString file_name = XString::CombinePath(tree.odm_georef, "odm_georeferencing_model_geo.txt");
	pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		fprintf(pFile, "WGS84 UTM %d%c\n", utm_zone, hemi);
		fprintf(pFile, "%d %d\n", (int)dx, (int)dy);

		fclose(pFile);
	}

	// write 'proj.txt'

	file_name = XString::CombinePath(tree.odm_georef, "proj.txt");
	pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		fprintf(pFile, "+proj=utm +zone=%d +datum=WGS84 +units=m +no_defs +type=crs\n", utm_zone);

		fclose(pFile);
	}

	return status;
}

int StageDataset::WriteImageListText()
{
	// Write 'img_list.txt'
	//

	int status = 0;

	XString file_name = XString::CombinePath(GetProject().GetDroneOutputPath(), "img_list.txt");
	FILE* pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		for (Project::ImageType image : GetProject().GetImageList())
		{
			// root name only
			fprintf(pFile, "%s\n", image.file_name.GetFileName().c_str());
		}

		fclose(pFile);
	}

	return status;
}

int StageDataset::WriteImageListJson()
{
	// Write 'images.json'
	//

	int status = 0;

	XString file_name = XString::CombinePath(GetProject().GetDroneOutputPath(), "images.json");
	FILE* pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		fprintf(pFile, "[\n");

		int ctr = 0;
		for (Project::ImageType image : GetProject().GetImageList())
		{
			fprintf(pFile, "	{\n");
			fprintf(pFile, "		\"filename\": \"%s\",\n", image.file_name.GetFileName().c_str());
			fprintf(pFile, "		\"mask\": null,\n");
			fprintf(pFile, "		\"width\": %d,\n", image.exif.ImageWidth);
			fprintf(pFile, "		\"height\": %d,\n", image.exif.ImageHeight);
			fprintf(pFile, "		\"camera_make\": \"%s\",\n", image.exif.Make.c_str());
			fprintf(pFile, "		\"camera_model\": \"%s\",\n", image.exif.Model.c_str());
			fprintf(pFile, "		\"orientation\": %d,\n", image.exif.Orientation);
			fprintf(pFile, "		\"latitude\": %0.15f,\n", image.exif.GeoLocation.Latitude);			//46.553156600003355
			fprintf(pFile, "		\"longitude\": %0.15f,\n", image.exif.GeoLocation.Longitude);
			fprintf(pFile, "		\"altitude\": %0.12f,\n", image.exif.GeoLocation.Altitude);			//980.296992481203
			fprintf(pFile, "		\"band_name\": \"RGB\",\n");		// in exif?
			fprintf(pFile, "		\"band_index\": 0,\n");
			fprintf(pFile, "		\"capture_uuid\": null,\n");
			fprintf(pFile, "		\"fnumber\": %0.1f,\n", image.exif.FNumber);
			fprintf(pFile, "		\"radiometric_calibration\": null,\n");
			fprintf(pFile, "		\"black_level\": null,\n");
			fprintf(pFile, "		\"gain\": null,\n");
			fprintf(pFile, "		\"gain_adjustment\": null,\n");
			fprintf(pFile, "		\"exposure_time\": %0.3f,\n", image.exif.ExposureTime);
			fprintf(pFile, "		\"iso_speed\": %d,\n", image.exif.ISOSpeedRatings);
			fprintf(pFile, "		\"bits_per_sample\": null,\n");
			fprintf(pFile, "		\"vignetting_center\": null,\n");
			fprintf(pFile, "		\"vignetting_polynomial\": null,\n");
			fprintf(pFile, "		\"spectral_irradiance\": null,\n");
			fprintf(pFile, "		\"horizontal_irradiance\": null,\n");
			fprintf(pFile, "		\"irradiance_scale_to_si\": null,\n");
			fprintf(pFile, "		\"utc_time\": %I64u000.0,\n", image.epoch);			//1299256936000.0
			fprintf(pFile, "		\"yaw\": null,\n");
			fprintf(pFile, "		\"pitch\": null,\n");
			fprintf(pFile, "		\"roll\": null,\n");
			fprintf(pFile, "		\"omega\": null,\n");
			fprintf(pFile, "		\"phi\": null,\n");
			fprintf(pFile, "		\"kappa\": null,\n");
			fprintf(pFile, "		\"sun_sensor\": null,\n");
			fprintf(pFile, "		\"dls_yaw\": null,\n");
			fprintf(pFile, "		\"dls_pitch\": null,\n");
			fprintf(pFile, "		\"dls_roll\": null,\n");
			fprintf(pFile, "		\"speed_x\": null,\n");
			fprintf(pFile, "		\"speed_y\": null,\n");
			fprintf(pFile, "		\"speed_z\": null,\n");
			fprintf(pFile, "		\"exif_width\": %d,\n", image.exif.ImageWidth);
			fprintf(pFile, "		\"exif_height\": %d,\n", image.exif.ImageHeight);
			fprintf(pFile, "		\"gps_xy_stddev\": null,\n");
			fprintf(pFile, "		\"gps_z_stddev\": null,\n");
			fprintf(pFile, "		\"camera_projection\": \"brown\",\n");
			fprintf(pFile, "		\"focal_ratio\": %0.16f\n", image.focal_ratio);		//0.8067639086097845
			fprintf(pFile, "	}%s\n", ++ctr < GetProject().GetImageCount() ? "," : "");
		}
		fprintf(pFile, "]\n");

		fclose(pFile);
	}

	return status;
}
