#ifndef	DATA_PACK_H
#define DATA_PACK_H

#include "Definitions.h"

#include <vector>
#include <string>

class DataPack
{
public:
	struct Mesh
	{
		std::string filename;
		Definitions::VERTEX_DATATYPE datatype;
	};
	struct Texture
	{
		std::string filename;
		Definitions::TEXTURE_DATATYPE datatype;
	};

	std::vector<Mesh> meshes;
	std::vector<Texture> textures;
};

#endif