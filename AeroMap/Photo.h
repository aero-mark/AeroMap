#ifndef PHOTO_H
#define PHOTO_H

#include "AeroMap.h"

class Photo
{
public:
	
	Photo();
	Photo(XString path_file);
	~Photo();

    void parse_exif_values(XString _path_file);

	static SizeType find_largest_photo_dims(const std::vector<Photo>& photos);
	static int      find_largest_photo_dim(const std::vector<Photo>& photos);
	static Photo    find_largest_photo(const std::vector<Photo>& photos);

	static double get_mm_per_unit(int resolution_unit);

private:

	// Standard tags (virtually all photos have these)
	int m_width;
	int m_height;
	XString m_camera_make;
	XString m_camera_model;
	int m_orientation;

	// Geo tags
	double m_latitude;
	double m_longitude;
	double m_altitude;

	// Multi-band fields
	XString m_band_name;
	int m_band_index;
	//m_capture_uuid;

	// Multi-spectral fields
	double m_fnumber;
	//m_radiometric_calibration = None
	//m_black_level = None
	//m_gain = None
	//m_gain_adjustment = None

    // Capture info
    //m_exposure_time = None
    int m_iso_speed;
    //m_bits_per_sample = None
    //m_vignetting_center = None
    //m_vignetting_polynomial = None
    //m_spectral_irradiance = None
    //m_horizontal_irradiance = None
    //m_irradiance_scale_to_si = None
    //m_utc_time = None

    // OPK angles
    double m_yaw;
    double m_pitch;
    double m_roll;
    double m_omega;
    double m_phi;
    double m_kappa;

    // DLS
    //m_sun_sensor = None
    //m_dls_yaw = None
    //m_dls_pitch = None
    //m_dls_roll = None

    // Aircraft speed
    double m_speed_x;
    double m_speed_y;
    double m_speed_z;

    // Original image width/height at capture time (before possible resizes)
    int m_exif_width;
    int m_exif_height;

    // m_center_wavelength = None
    // m_bandwidth = None

    // RTK
    double m_gps_xy_stddev;     // Dilution of Precision X/Y
    double m_gps_z_stddev;      // Dilution of Precision Z

    // Misc SFM
    XString m_camera_projection;
    double m_focal_ratio;
};

#endif // #ifndef PHOTO_H
