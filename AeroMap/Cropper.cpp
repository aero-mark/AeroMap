// Cropper.cpp
// Port of odm cropper.py
//

#include "AeroLib.h"
#include "Cropper.h"

Cropper::Cropper(XString storage_dir, XString files_prefix)
{
	m_storage_dir = storage_dir;
	m_files_prefix = files_prefix;
}

XString Cropper::crop(XString gpkg_path, XString geotiff_path, bool keep_original)
{
	if (AeroLib::FileExists(gpkg_path) == false)
	{
		Logger::Write(__FUNCTION__, "'%s' does not exist, will skip cropping.", gpkg_path.c_str());
		return geotiff_path;
	}
	if (AeroLib::FileExists(geotiff_path) == false)
	{
		Logger::Write(__FUNCTION__, "'%s' does not exist, will skip cropping.", geotiff_path.c_str());
		return geotiff_path;
	}

	//    Logger::Write(__FUNCTION__, "Cropping %s" % geotiff_path)

	// Rename original file
	// path/to/odm_orthophoto.tif --> path/to/odm_orthophoto.original.tif

	XString path = geotiff_path.GetPathName();
	XString filename = geotiff_path.GetFileName();
	// path = path/to
	// filename = odm_orthophoto.tif

	XString basename;
	XString ext;
	int pos = filename.ReverseFind('.');
	if (pos > -1)
	{
		basename = filename.Left(pos);
		ext = filename.Mid(pos);
		// basename = odm_orthophoto
		// ext = .tif
	}

	XString original_geotiff = XString::CombinePath(path, XString::Format("%s.original%s", basename.c_str(), ext.c_str()));
	//AeroLib::Replace(geotiff_path, original_geotiff);

	//    try:
	//        kwargs = {
	//            'gpkg_path': double_quote(gpkg_path),
	//            'geotiffInput': double_quote(original_geotiff),
	//            'geotiffOutput': double_quote(geotiff_path),
	//            'options': ' '.join(map(lambda k: '-co {}={}'.format(k, gdal_options[k]), gdal_options)),
	//            'warpOptions': ' '.join(warp_options),
	//            'max_memory': get_max_memory()
	//        }
	//
	//        run('gdalwarp -cutline {gpkg_path} '
	//            '-crop_to_cutline '
	//            '{options} '
	//            '{warpOptions} '
	//            '{geotiffInput} '
	//            '{geotiffOutput} '
	//            '--config GDAL_CACHEMAX {max_memory}%'.format(**kwargs))
	//
	//        if not keep_original:
	//            os.remove(original_geotiff)
	//
	//    except Exception as e:
	//        Logger::Write(__FUNCTION__('Something went wrong while cropping: {}'.format(e))
	//
	//        # Revert rename
	//        os.replace(original_geotiff, geotiff_path)

	return geotiff_path;
}

XString Cropper::path(XString suffix)
{
	// return a path relative to storage_dir and prefixed with files_prefix
	//

	return XString::CombinePath(m_storage_dir, XString::Format("%s.%s", m_files_prefix.c_str(), suffix.c_str()));
}

void Cropper::merge_bounds(std::vector<XString> input_bound_files, XString output_bounds, int buffer_distance)
{
	// Merge multiple bound files into a single bound computed from the convex hull
	// of all bounds (minus a buffer distance in meters)
	//

    //geomcol = ogr.Geometry(ogr.wkbGeometryCollection)

    //driver = ogr.GetDriverByName('GPKG')
    //srs = None

    //for input_bound_file in input_bound_files:
    //    ds = driver.Open(input_bound_file, 0) # ready-only

    //    layer = ds.GetLayer()
    //    srs = layer.GetSpatialRef()

    //    # Collect all Geometry
    //    for feature in layer:
    //        geomcol.AddGeometry(feature.GetGeometryRef())

    //    ds = None

    // Calculate convex hull
    //convexhull = geomcol.ConvexHull()

    // If buffer distance is specified
    // Create two buffers, one shrunk by
    // N + 3 and then that buffer expanded by 3
    // so that we get smooth corners. \m/
	const int BUFFER_SMOOTH_DISTANCE = 3;

    //if buffer_distance > 0:
    //    convexhull = convexhull.Buffer(-(buffer_distance + BUFFER_SMOOTH_DISTANCE))
    //    convexhull = convexhull.Buffer(BUFFER_SMOOTH_DISTANCE)

    // Save to a new file
    //if AeroLib::FileExists(output_bounds):
    //    driver.DeleteDataSource(output_bounds)

    //out_ds = driver.CreateDataSource(output_bounds)
    //layer = out_ds.CreateLayer("convexhull", srs=srs, geom_type=ogr.wkbPolygon)

    //feature_def = layer.GetLayerDefn()
    //feature = ogr.Feature(feature_def)
    //feature.SetGeometry(convexhull)
    //layer.CreateFeature(feature)
    //feature = None

    // Save and close output data source
    //out_ds = None
}

XString Cropper::create_bounds_geojson(XString pointcloud_path, int buffer_distance, int decimation_step)
{
	// Compute a buffered polygon around the data extents (not just a bounding box)
	// of the given point cloud.
	//
	// return filename to GeoJSON containing the polygon
	//
	
	if (AeroLib::FileExists(pointcloud_path) == false)
	{
		Logger::Write(__FUNCTION__, "Point cloud does not exist, cannot generate bounds: '%s'", pointcloud_path.c_str());
		return "";
	}

	// Do decimation prior to extracting boundary information
	XString decimated_pointcloud_path = path("decimated.las");
	
	//    run("pdal translate -i \"{}\" "
	//        "-o \"{}\" "
	//        "decimation "
	//        "--filters.decimation.step={} ".format(pointcloud_path, decimated_pointcloud_path, decimation_step))
	
	if (AeroLib::FileExists(decimated_pointcloud_path) == false)
	{
		Logger::Write(__FUNCTION__, "Could not decimate point cloud, thus cannot generate GPKG bounds: '%s'", decimated_pointcloud_path.c_str());
		return "";
	}

	// Use PDAL to dump boundary information
	// then read the information back
	
	XString boundary_file_path = path("boundary.json");
	
	//run('pdal info --boundary --filters.hexbin.edge_size=1 --filters.hexbin.threshold=0 "{0}" > "{1}"'.format(decimated_pointcloud_path,  boundary_file_path))
	
	//    pc_geojson_boundary_feature = None
	
	//    with open(boundary_file_path, 'r') as f:
	//        json_f = json.loads(f.read())
	//        pc_geojson_boundary_feature = json_f['boundary']['boundary_json']
	
	//    if pc_geojson_boundary_feature is None: 
	//			raise RuntimeError("Could not determine point cloud boundaries")
	
	// Write bounds to GeoJSON
	//    tmp_bounds_geojson_path = self.path('tmp-bounds.geojson')
	//    with open(tmp_bounds_geojson_path, "w") as f:
	//        f.write(json.dumps({
	//            "type": "FeatureCollection",
	//            "features": [{
	//                "type": "Feature",
	//                "geometry": pc_geojson_boundary_feature
	//            }]
	//        }))
	
	// Create a convex hull around the boundary
	// as to encompass the entire area (no holes)
	//    driver = ogr.GetDriverByName('GeoJSON')
	//    ds = driver.Open(tmp_bounds_geojson_path, 0) # ready-only
	//    layer = ds.GetLayer()
	
	// Collect all Geometry
	//    geomcol = ogr.Geometry(ogr.wkbGeometryCollection)
	//    for feature in layer:
	//        geomcol.AddGeometry(feature.GetGeometryRef())
	
	// Calculate convex hull
	//    convexhull = geomcol.ConvexHull()

	// If buffer distance is specified
	// Create two buffers, one shrunk by
	// N + 3 and then that buffer expanded by 3
	// so that we get smooth corners. \m/
	const int BUFFER_SMOOTH_DISTANCE = 3;

	if (buffer_distance > 0)
	{
		// For small areas, check that buffering doesn't obliterate
		// our hull
		//tmp = convexhull.Buffer(-(buffer_distance + BUFFER_SMOOTH_DISTANCE))
		//tmp = tmp.Buffer(BUFFER_SMOOTH_DISTANCE)
		//if tmp.Area() > 0:
		//    convexhull = tmp
		//else:
		//    Logger::Write(__FUNCTION__("Very small crop area detected, we will not smooth it.")
	}

	// Save to a new file
	XString bounds_geojson_path = path("bounds.geojson");
	//    if AeroLib::FileExists(bounds_geojson_path):
	//        os.remove(bounds_geojson_path)
	//
	//    out_ds = driver.CreateDataSource(bounds_geojson_path)
	//    layer = out_ds.CreateLayer("convexhull", geom_type=ogr.wkbPolygon)
	//
	//    feature_def = layer.GetLayerDefn()
	//    feature = ogr.Feature(feature_def)
	//    feature.SetGeometry(convexhull)
	//    layer.CreateFeature(feature)
	//    feature = None
	
	// Save and close data sources
	//    out_ds = ds = None

	// Remove decimated point cloud
	//    if AeroLib::FileExists(decimated_pointcloud_path):
	//        os.remove(decimated_pointcloud_path)

	// Remove tmp bounds
	// if AeroLib::FileExists(tmp_bounds_geojson_path):
	//     os.remove(tmp_bounds_geojson_path)

	return bounds_geojson_path;
}

XString Cropper::create_bounds_gpkg(XString pointcloud_path, int buffer_distance, int decimation_step)
{
	// Compute a buffered polygon around the data extents (not just a bounding box)
	// of the given point cloud.
	//
	// return filename to Geopackage containing the polygon
	//

	if (AeroLib::FileExists(pointcloud_path) == false)
	{
		Logger::Write(__FUNCTION__, "Point cloud does not exist, cannot generate GPKG bounds: '%'", pointcloud_path.c_str());
		return "";
	}

	XString bounds_geojson_path = create_bounds_geojson(pointcloud_path, buffer_distance, decimation_step);

	XString summary_file_path = XString::CombinePath(m_storage_dir, XString::Format("%s.summary.json", m_files_prefix.c_str()));
	//export_summary_json(pointcloud_path, summary_file_path);

	//    pc_proj4 = None
	//    with open(summary_file_path, 'r') as f:
	//        json_f = json.loads(f.read())
	//        pc_proj4 = json_f['summary']['srs']['proj4']

	//    if pc_proj4 is None: raise RuntimeError("Could not determine point cloud proj4 declaration")

	XString bounds_gpkg_path = XString::CombinePath(m_storage_dir, XString::Format("%s.bounds.gpkg", m_files_prefix.c_str()));

	//    if os.path.isfile(bounds_gpkg_path):
	//        os.remove(bounds_gpkg_path)

	//    # Convert bounds to GPKG
	//    kwargs = {
	//        'input': double_quote(bounds_geojson_path),
	//        'output': double_quote(bounds_gpkg_path),
	//        'proj4': pc_proj4
	//    }
	//
	//    run('ogr2ogr -overwrite -f GPKG -a_srs "{proj4}" {output} {input}'.format(**kwargs))

	return bounds_gpkg_path;
}
