#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include "Photo.h"

class Reconstruction
{
public:

	Reconstruction::Reconstruction(const std::vector<Photo> photos);
	Reconstruction::~Reconstruction();

	bool is_georef();
	bool has_gcp();

};

#endif // #ifndef RECONSTRUCTION_H

