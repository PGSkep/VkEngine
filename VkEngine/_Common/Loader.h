#ifndef	LOADER_H
#define LOADER_H

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include "Definitions.h"
#include "../_Common/Definitions.h"

namespace Loader
{
	struct TextureData
	{
		uint32_t width, height;
		uint64_t size;
		Definitions::TEXTURE_DATATYPE format;
		void* data;

		TextureData()
		{
			data = nullptr;
			width = 0;
			height = 0;
			format = Definitions::TDT_NONE;
		}
		~TextureData()
		{
			if (data != nullptr)
				delete[] data;
			data = nullptr;
			width = 0;
			height = 0;
			format = Definitions::TDT_NONE;
		}

		bool LoadTGA(const char* _filename)
		{
			if (data != nullptr)
				delete[] data;
			data = nullptr;
			width = 0;
			height = 0;
			format = Definitions::TDT_NONE;

			FILE* fTGA;
			fTGA = fopen(_filename, "rb");

			if (fTGA == NULL)
				return false;

			uint8_t header[12];
			if (fread(&header, sizeof(header), 1, fTGA) == 0)
				return false;

			//uint8_t compressedTGAcompare[12] = { 0,0,10,0,0,0,0,0,0,0,0,0 };
			uint8_t uncompressedTGAcompare[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };

			// Get data
			if (memcmp(uncompressedTGAcompare, &header, sizeof(header)) == 0)
			{
				uint8_t header[6];

				if (fread(header, sizeof(header), 1, fTGA) == 0)
					return false;

				width = header[1] * 256 + header[0];
				height = header[3] * 256 + header[2];
				uint8_t bpp = header[4];

				if (width == 0 || height == 0 || (bpp != 24 && bpp != 32))
					return false;

				size = width * height;
				if (bpp == 24)
				{
					format = Definitions::TDT_B8G8R8;
					size *= 3;
				}
				else if (bpp == 32)
				{
					format = Definitions::TDT_B8G8R8A8;
					size *= 4;
				}

				data = new uint8_t[size];
				if (fread(data, 1, size, fTGA) != size)
				{
					delete[] data;
					data = nullptr;
					return false;
				}
			}
			else return false;

			return true;
		}
	};
	struct ModelData
	{
		Definitions::VERTEX_DATATYPE	vertexDataType;
		uint32_t						vertexDataCount;
		uint64_t						vertexDataSize;
		void*							vertexData;

		uint8_t							indexSize;
		uint32_t						indexDataCount;
		uint64_t						indexDataSize;
		void*							indexData;

		ModelData()
		{
			vertexDataType = Definitions::VDT_NONE;
			vertexDataCount = 0;
			vertexDataSize = 0;
			vertexData = nullptr;

			indexSize = 0;
			indexDataCount = 0;
			indexDataSize = 0;
			indexData = nullptr;
		}
		~ModelData()
		{
			vertexDataType = Definitions::VDT_NONE;
			vertexDataCount = 0;
			vertexDataSize = 0;
			if (vertexData != nullptr)
				delete[] vertexData;

			indexSize = 0;
			indexDataCount = 0;
			indexDataSize = 0;
			if (indexData != nullptr)
				delete[] indexData;
		}

		bool LoadModel(const char* _filename, uint64_t _vertexDataType, bool _generateMissingData)
		{
			/// Clear
			vertexDataType = (Definitions::VERTEX_DATATYPE)_vertexDataType;
			vertexDataCount = 0;
			vertexDataSize = 0;
			if (vertexData != nullptr)
			{
				delete[] vertexData;
				vertexData = nullptr;
			}

			indexSize = 0;
			indexDataCount = 0;
			indexDataSize = 0;
			if (indexData != nullptr)
			{
				delete[] indexData;
				indexData = nullptr;
			}

			/// Init assimp
			Assimp::Importer Importer;
			const aiScene* pScene;

			int processSteps = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate;

			if ((_vertexDataType & Definitions::VDT_NORMAL) == Definitions::VDT_NORMAL)
				processSteps = processSteps | aiProcess_GenNormals;
			if ((_vertexDataType & Definitions::VDT_TANGENT_BITANGENT) == Definitions::VDT_TANGENT_BITANGENT)
				processSteps = processSteps | aiProcess_CalcTangentSpace;

			processSteps = processSteps | aiProcess_ValidateDataStructure;

			pScene = Importer.ReadFile(_filename, processSteps);

			if (pScene == nullptr)
				return false;

			/// Get animation data
			// get bone data
			bool bones = false;

			/// Get data info
			Definitions::VERTEX_DATATYPE missingVertexTypes = Definitions::VDT_NONE;

			// find unavailable data
			if (!pScene->mMeshes[0]->HasPositions())
			{
				if ((_vertexDataType & Definitions::VDT_X) == Definitions::VDT_X)
					missingVertexTypes = (Definitions::VERTEX_DATATYPE)(missingVertexTypes | Definitions::VDT_X);
				if ((_vertexDataType & Definitions::VDT_Y) == Definitions::VDT_Y)
					missingVertexTypes = (Definitions::VERTEX_DATATYPE)(missingVertexTypes | Definitions::VDT_Y);
				if ((_vertexDataType & Definitions::VDT_Z) == Definitions::VDT_Z)
					missingVertexTypes = (Definitions::VERTEX_DATATYPE)(missingVertexTypes | Definitions::VDT_Z);
			}
			if (!pScene->mMeshes[0]->HasTextureCoords(0))
			{
				if ((_vertexDataType & Definitions::VDT_UV) == Definitions::VDT_UV)
					missingVertexTypes = (Definitions::VERTEX_DATATYPE)(missingVertexTypes | Definitions::VDT_UV);
			}
			if (!pScene->mMeshes[0]->HasVertexColors(0))
			{
				if ((_vertexDataType & Definitions::VDT_R) == Definitions::VDT_R)
					missingVertexTypes = (Definitions::VERTEX_DATATYPE)(missingVertexTypes | Definitions::VDT_R);
				if ((_vertexDataType & Definitions::VDT_G) == Definitions::VDT_G)
					missingVertexTypes = (Definitions::VERTEX_DATATYPE)(missingVertexTypes | Definitions::VDT_G);
				if ((_vertexDataType & Definitions::VDT_B) == Definitions::VDT_B)
					missingVertexTypes = (Definitions::VERTEX_DATATYPE)(missingVertexTypes | Definitions::VDT_B);
				if ((_vertexDataType & Definitions::VDT_A) == Definitions::VDT_A)
					missingVertexTypes = (Definitions::VERTEX_DATATYPE)(missingVertexTypes | Definitions::VDT_A);
			}
			if (!bones)
			{
				if ((_vertexDataType & Definitions::VDT_SKELETON_BONE_INDEX_SIZE_8) == Definitions::VDT_SKELETON_BONE_INDEX_SIZE_8)
					missingVertexTypes = (Definitions::VERTEX_DATATYPE)(missingVertexTypes | Definitions::VDT_SKELETON_BONE_INDEX_SIZE_8);
				if ((_vertexDataType & Definitions::VDT_SKELETON_BONE_INDEX_SIZE_16) == Definitions::VDT_SKELETON_BONE_INDEX_SIZE_16)
					missingVertexTypes = (Definitions::VERTEX_DATATYPE)(missingVertexTypes | Definitions::VDT_SKELETON_BONE_INDEX_SIZE_16);

				if ((_vertexDataType & Definitions::VDT_SKELETON_4_BONE_PER_VERTEX) == Definitions::VDT_SKELETON_4_BONE_PER_VERTEX)
					missingVertexTypes = (Definitions::VERTEX_DATATYPE)(missingVertexTypes | Definitions::VDT_SKELETON_4_BONE_PER_VERTEX);
			}
			if (!pScene->mMeshes[0]->HasNormals())
			{
				if ((_vertexDataType & Definitions::VDT_NORMAL) == Definitions::VDT_NORMAL)
					missingVertexTypes = (Definitions::VERTEX_DATATYPE)(missingVertexTypes | Definitions::VDT_NORMAL);
			}
			if (!pScene->mMeshes[0]->HasTangentsAndBitangents())
			{
				if ((_vertexDataType & Definitions::VDT_TANGENT_BITANGENT) == Definitions::VDT_TANGENT_BITANGENT)
					missingVertexTypes = (Definitions::VERTEX_DATATYPE)(missingVertexTypes | Definitions::VDT_TANGENT_BITANGENT);
			}

			// return failure if there is missing data and it cannot be generated
			if (_generateMissingData == false && missingVertexTypes != Definitions::VDT_NONE)
				return false;

			vertexDataType = (Definitions::VERTEX_DATATYPE)_vertexDataType;

			// return failure if bone index size is set to both 8 and 16
			if (((_vertexDataType & Definitions::VDT_SKELETON_BONE_INDEX_SIZE_8) == Definitions::VDT_SKELETON_BONE_INDEX_SIZE_8) && ((_vertexDataType & Definitions::VDT_SKELETON_BONE_INDEX_SIZE_16) == Definitions::VDT_SKELETON_BONE_INDEX_SIZE_16))
				return false;

			// get bone size and count
			uint32_t boneIndexSize = (((vertexDataType & Definitions::VDT_SKELETON_BONE_INDEX_SIZE_8) == Definitions::VDT_SKELETON_BONE_INDEX_SIZE_8) * sizeof(uint8_t)) +
				(((vertexDataType & Definitions::VDT_SKELETON_BONE_INDEX_SIZE_16) == Definitions::VDT_SKELETON_BONE_INDEX_SIZE_16) * sizeof(uint16_t));
			uint32_t bonePerVertexCount = (((vertexDataType & Definitions::VDT_SKELETON_4_BONE_PER_VERTEX) == Definitions::VDT_SKELETON_4_BONE_PER_VERTEX) * 4);

			// get vertex size
			uint32_t vertexSize = (
				// position
				(sizeof(float) * 1 * ((vertexDataType & Definitions::VDT_X) == Definitions::VDT_X)) +
				(sizeof(float) * 1 * ((vertexDataType & Definitions::VDT_Y) == Definitions::VDT_Y)) +
				(sizeof(float) * 1 * ((vertexDataType & Definitions::VDT_Z) == Definitions::VDT_Z)) +
				// tex coord
				(sizeof(float) * 2 * ((vertexDataType & Definitions::VDT_UV) == Definitions::VDT_UV)) +

				// color
				(sizeof(float) * 1 * ((vertexDataType & Definitions::VDT_R) == Definitions::VDT_R)) +
				(sizeof(float) * 1 * ((vertexDataType & Definitions::VDT_G) == Definitions::VDT_G)) +
				(sizeof(float) * 1 * ((vertexDataType & Definitions::VDT_B) == Definitions::VDT_B)) +
				(sizeof(float) * 1 * ((vertexDataType & Definitions::VDT_A) == Definitions::VDT_A)) +

				// bone indices + weight
				(boneIndexSize * bonePerVertexCount) +
				(sizeof(float) * bonePerVertexCount) +

				// normal, tangent, bitangent
				(sizeof(float) * 3 * ((vertexDataType & Definitions::VDT_NORMAL) == Definitions::VDT_NORMAL)) +
				(sizeof(float) * 6 * ((vertexDataType & Definitions::VDT_TANGENT_BITANGENT) == Definitions::VDT_TANGENT_BITANGENT))
				);

			// get index and vertex size and count
			vertexDataCount = pScene->mMeshes[0]->mNumVertices;
			vertexDataSize = vertexDataCount * vertexSize;

			indexDataCount = pScene->mMeshes[0]->mNumFaces * 3;
			indexSize =
				(sizeof(uint16_t) * (indexDataCount <= (uint16_t)~0U)) +
				(sizeof(uint32_t) * (indexDataCount >  (uint16_t)~0U));
			indexDataSize = indexDataCount * indexSize;

			/// Allocate data
			vertexData = new uint8_t[vertexDataSize];
			memset(vertexData, 0, vertexDataSize);
			indexData = new uint8_t[indexDataSize];
			memset(indexData, 0, indexDataSize);

			/// Fill indices
			uint64_t unfitIndexDataSize = 0;

			uint64_t dataPos = 0;
			for (unsigned int iFace = 0; iFace != pScene->mMeshes[0]->mNumFaces; ++iFace)
			{
				const aiFace& face = pScene->mMeshes[0]->mFaces[iFace];

				if (face.mNumIndices == 3)
				{
					if (indexSize == sizeof(uint16_t))
					{
						uint16_t indices16[3] = { (uint16_t)face.mIndices[0], (uint16_t)face.mIndices[1], (uint16_t)face.mIndices[2] };
						memcpy(&((uint8_t*)indexData)[dataPos], indices16, indexSize * 3);
					}
					else if (indexSize == sizeof(uint32_t))
					{
						uint32_t indices32[3] = { face.mIndices[0], face.mIndices[1], face.mIndices[2] };
						memcpy(&((uint8_t*)indexData)[dataPos], indices32, indexSize * 3);
					}

					dataPos += indexSize * 3;
				}
				else
				{
					// IndexSize is based off face count, if the face is not a triangle (aka not valid) it's skipped and shouldn't count for indexSize
					unfitIndexDataSize += indexSize + face.mNumIndices;
				}
			}

			indexDataSize = indexDataSize - unfitIndexDataSize;

			/// Fill vertices
			dataPos = (uint64_t)vertexData;
			for (unsigned int iVertex = 0; iVertex != pScene->mMeshes[0]->mNumVertices; ++iVertex)
			{
				// position
				if ((vertexDataType & Definitions::VDT_X) == Definitions::VDT_X)
				{
					if ((missingVertexTypes & Definitions::VDT_X) == Definitions::VDT_X)
						memset((void*)dataPos, 0, sizeof(float));
					else
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mVertices[iVertex].x, sizeof(float));

					dataPos += sizeof(float);
				}
				if ((vertexDataType & Definitions::VDT_Y) == Definitions::VDT_Y)
				{
					if ((missingVertexTypes & Definitions::VDT_Y) == Definitions::VDT_Y)
						memset((void*)dataPos, 0, sizeof(float));
					else
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mVertices[iVertex].y, sizeof(float));

					dataPos += sizeof(float);
				}
				if ((vertexDataType & Definitions::VDT_Z) == Definitions::VDT_Z)
				{
					if ((missingVertexTypes & Definitions::VDT_Z) == Definitions::VDT_Z)
						memset((void*)dataPos, 0, sizeof(float));
					else
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mVertices[iVertex].z, sizeof(float));

					dataPos += sizeof(float);
				}
				// tex coord
				if ((vertexDataType & Definitions::VDT_UV) == Definitions::VDT_UV)
				{
					if ((missingVertexTypes & Definitions::VDT_UV) == Definitions::VDT_UV)
						memset((void*)dataPos, 0, sizeof(float) * 2);
					else
					{
						float uv[2] = { pScene->mMeshes[0]->mTextureCoords[0][iVertex].x, -pScene->mMeshes[0]->mTextureCoords[0][iVertex].y };
						memcpy((void*)dataPos, &uv, sizeof(float) * 2);
					}

					dataPos += sizeof(float) * 2;
				}

				// color
				if ((vertexDataType & Definitions::VDT_R) == Definitions::VDT_R)
				{
					if ((missingVertexTypes & Definitions::VDT_R) == Definitions::VDT_R)
						memset((void*)dataPos, 0, sizeof(float));
					else
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mColors[iVertex]->r, sizeof(float));

					dataPos += sizeof(float);
				}
				if ((vertexDataType & Definitions::VDT_G) == Definitions::VDT_G)
				{
					if ((missingVertexTypes & Definitions::VDT_G) == Definitions::VDT_G)
						memset((void*)dataPos, 0, sizeof(float));
					else
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mColors[iVertex]->g, sizeof(float));

					dataPos += sizeof(float);
				}
				if ((vertexDataType & Definitions::VDT_B) == Definitions::VDT_B)
				{
					if ((missingVertexTypes & Definitions::VDT_B) == Definitions::VDT_B)
						memset((void*)dataPos, 0, sizeof(float));
					else
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mColors[iVertex]->b, sizeof(float));

					dataPos += sizeof(float);
				}
				if ((vertexDataType & Definitions::VDT_A) == Definitions::VDT_A)
				{
					if ((missingVertexTypes & Definitions::VDT_A) == Definitions::VDT_A)
						memset((void*)dataPos, 24, sizeof(float));
					else
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mColors[iVertex]->a, sizeof(float));

					dataPos += sizeof(float);
				}

				// bone index and weight
				if (boneIndexSize * bonePerVertexCount > 0)
				{
					float dd = 0.654f;
					uint16_t ee = 13;

					if (bones)
					{
						memset((void*)dataPos, 0, boneIndexSize * bonePerVertexCount);
						dataPos += boneIndexSize * bonePerVertexCount;
						memset((void*)dataPos, 0, sizeof(float) * bonePerVertexCount);
						dataPos += sizeof(float) * bonePerVertexCount;
					}
					else
					{
						// TODO:
						memcpy((void*)dataPos, &ee, boneIndexSize * bonePerVertexCount);
						dataPos += boneIndexSize * bonePerVertexCount;
						memcpy((void*)dataPos, &dd, sizeof(float) * bonePerVertexCount);
						dataPos += sizeof(float) * bonePerVertexCount;
					}
				}

				// normal, tangent, bitangent
				if ((vertexDataType & Definitions::VDT_NORMAL) == Definitions::VDT_NORMAL)
				{
					if ((missingVertexTypes & Definitions::VDT_NORMAL) == Definitions::VDT_NORMAL)
						memset((void*)dataPos, 0, sizeof(float) * 3);
					else
						//&pScene->mMeshes[0]->mVertices[iVertex].x
						&pScene->mMeshes[0]->mNormals[iVertex].x;
					memcpy((void*)dataPos, &pScene->mMeshes[0]->mNormals[iVertex].x, sizeof(float) * 3);

					dataPos += sizeof(float) * 3;
				}
				if ((vertexDataType & Definitions::VDT_TANGENT_BITANGENT) == Definitions::VDT_TANGENT_BITANGENT)
				{
					if ((missingVertexTypes & Definitions::VDT_TANGENT_BITANGENT) == Definitions::VDT_TANGENT_BITANGENT)
					{
						memset(&((uint8_t*)vertexData)[dataPos], 0, sizeof(float) * 6);
						dataPos += sizeof(float) * 6;
					}
					else
					{
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mTangents[iVertex].x, sizeof(float) * 3);
						dataPos += sizeof(float) * 3;
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mBitangents[iVertex].x, sizeof(float) * 3);
						dataPos += sizeof(float) * 3;
					}
				}
			}

			return true;
		}
	};
}

#endif