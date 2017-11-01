#ifndef	DATA_PACK_H
#define DATA_PACK_H

#include "Definitions.h"

#include <vector>
#include <string>

class DataPack
{
public:
	struct TextureInfo
	{
		std::string filename;
		//Definitions::TEXTURE_DATATYPE datatype;

		static inline TextureInfo GetTextureInfo(const char* _filename)
		{
			return { _filename };
		}
	};
	struct MeshInfo
	{
		std::string filename;
		Definitions::VERTEX_DATATYPE datatype;

		static inline MeshInfo GetMeshInfo(const char* _filename, Definitions::VERTEX_DATATYPE _datatype)
		{
			return { _filename, _datatype };
		}
	};

	std::vector<MeshInfo> meshes;
	std::vector<TextureInfo> textures;
};

#endif