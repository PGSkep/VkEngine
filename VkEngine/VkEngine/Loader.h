#ifndef	LOADER_H
#define LOADER_H

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <minwinbase.h>
#include <winnt.h>

#include "Vk.h"

namespace Loader
{
	struct ModelData
	{
		VkD::VERTEX_DATATYPE	vertexDataType;
		uint32_t				vertexDataCount;
		uint64_t				vertexDataSize;
		void*					vertexData;

		uint8_t					indexSize;
		uint32_t				indexDataCount;
		uint64_t				indexDataSize;
		void*					indexData;

		ModelData()
		{
			vertexDataType = VkD::VDT_NONE;
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
			vertexDataType = VkD::VDT_NONE;
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
			vertexDataType = (VkD::VERTEX_DATATYPE)_vertexDataType;
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

			if ((_vertexDataType & VkD::VDT_NORMAL) == VkD::VDT_NORMAL)
				processSteps = processSteps | aiProcess_GenNormals;
			if ((_vertexDataType & VkD::VDT_TANGENT_BITANGENT) == VkD::VDT_TANGENT_BITANGENT)
				processSteps = processSteps | aiProcess_CalcTangentSpace;

			processSteps = processSteps | aiProcess_ValidateDataStructure;

			pScene = Importer.ReadFile(_filename, processSteps);

			if (pScene == nullptr)
				return false;

			/// Get animation data
			// get bone data
			bool bones = false;

			/// Get data info
			VkD::VERTEX_DATATYPE missingVertexTypes = VkD::VDT_NONE;

			// find unavailable data
			if (!pScene->mMeshes[0]->HasPositions())
			{
				if ((_vertexDataType & VkD::VDT_X) == VkD::VDT_X)
					missingVertexTypes = (VkD::VERTEX_DATATYPE)(missingVertexTypes | VkD::VDT_X);
				if ((_vertexDataType & VkD::VDT_Y) == VkD::VDT_Y)
					missingVertexTypes = (VkD::VERTEX_DATATYPE)(missingVertexTypes | VkD::VDT_Y);
				if ((_vertexDataType & VkD::VDT_Z) == VkD::VDT_Z)
					missingVertexTypes = (VkD::VERTEX_DATATYPE)(missingVertexTypes | VkD::VDT_Z);
			}
			if (!pScene->mMeshes[0]->HasTextureCoords(0))
			{
				if ((_vertexDataType & VkD::VDT_UV) == VkD::VDT_UV)
					missingVertexTypes = (VkD::VERTEX_DATATYPE)(missingVertexTypes | VkD::VDT_UV);
			}
			if (!pScene->mMeshes[0]->HasVertexColors(0))
			{
				if ((_vertexDataType & VkD::VDT_R) == VkD::VDT_R)
					missingVertexTypes = (VkD::VERTEX_DATATYPE)(missingVertexTypes | VkD::VDT_R);
				if ((_vertexDataType & VkD::VDT_G) == VkD::VDT_G)
					missingVertexTypes = (VkD::VERTEX_DATATYPE)(missingVertexTypes | VkD::VDT_G);
				if ((_vertexDataType & VkD::VDT_B) == VkD::VDT_B)
					missingVertexTypes = (VkD::VERTEX_DATATYPE)(missingVertexTypes | VkD::VDT_B);
				if ((_vertexDataType & VkD::VDT_A) == VkD::VDT_A)
					missingVertexTypes = (VkD::VERTEX_DATATYPE)(missingVertexTypes | VkD::VDT_A);
			}
			if (!bones)
			{
				if ((_vertexDataType & VkD::VDT_SKELETON_BONE_INDEX_SIZE_8) == VkD::VDT_SKELETON_BONE_INDEX_SIZE_8)
					missingVertexTypes = (VkD::VERTEX_DATATYPE)(missingVertexTypes | VkD::VDT_SKELETON_BONE_INDEX_SIZE_8);
				if ((_vertexDataType & VkD::VDT_SKELETON_BONE_INDEX_SIZE_16) == VkD::VDT_SKELETON_BONE_INDEX_SIZE_16)
					missingVertexTypes = (VkD::VERTEX_DATATYPE)(missingVertexTypes | VkD::VDT_SKELETON_BONE_INDEX_SIZE_16);

				if ((_vertexDataType & VkD::VDT_SKELETON_4_BONE_PER_VERTEX) == VkD::VDT_SKELETON_4_BONE_PER_VERTEX)
					missingVertexTypes = (VkD::VERTEX_DATATYPE)(missingVertexTypes | VkD::VDT_SKELETON_4_BONE_PER_VERTEX);
			}
			if (!pScene->mMeshes[0]->HasNormals())
			{
				if ((_vertexDataType & VkD::VDT_NORMAL) == VkD::VDT_NORMAL)
					missingVertexTypes = (VkD::VERTEX_DATATYPE)(missingVertexTypes | VkD::VDT_NORMAL);
			}
			if (!pScene->mMeshes[0]->HasTangentsAndBitangents())
			{
				if ((_vertexDataType & VkD::VDT_TANGENT_BITANGENT) == VkD::VDT_TANGENT_BITANGENT)
					missingVertexTypes = (VkD::VERTEX_DATATYPE)(missingVertexTypes | VkD::VDT_TANGENT_BITANGENT);
			}

			// return failure if there is missing data and it cannot be generated
			if (_generateMissingData == false && missingVertexTypes != VkD::VDT_NONE)
				return false;

			vertexDataType = (VkD::VERTEX_DATATYPE)_vertexDataType;

			// return failure if bone index size is set to both 8 and 16
			if (((_vertexDataType & VkD::VDT_SKELETON_BONE_INDEX_SIZE_8) == VkD::VDT_SKELETON_BONE_INDEX_SIZE_8) && ((_vertexDataType & VkD::VDT_SKELETON_BONE_INDEX_SIZE_16) == VkD::VDT_SKELETON_BONE_INDEX_SIZE_16))
				return false;

			// get bone size and count
			uint32_t boneIndexSize = (((vertexDataType & VkD::VDT_SKELETON_BONE_INDEX_SIZE_8) == VkD::VDT_SKELETON_BONE_INDEX_SIZE_8) * sizeof(uint8_t)) +
				(((vertexDataType & VkD::VDT_SKELETON_BONE_INDEX_SIZE_16) == VkD::VDT_SKELETON_BONE_INDEX_SIZE_16) * sizeof(uint16_t));
			uint32_t bonePerVertexCount = (((vertexDataType & VkD::VDT_SKELETON_4_BONE_PER_VERTEX) == VkD::VDT_SKELETON_4_BONE_PER_VERTEX) * 4);

			// get vertex size
			uint32_t vertexSize = (
				// position
				(sizeof(float) * 1 * ((vertexDataType & VkD::VDT_X) == VkD::VDT_X)) +
				(sizeof(float) * 1 * ((vertexDataType & VkD::VDT_Y) == VkD::VDT_Y)) +
				(sizeof(float) * 1 * ((vertexDataType & VkD::VDT_Z) == VkD::VDT_Z)) +
				// tex coord
				(sizeof(float) * 2 * ((vertexDataType & VkD::VDT_UV) == VkD::VDT_UV)) +

				// color
				(sizeof(float) * 1 * ((vertexDataType & VkD::VDT_R) == VkD::VDT_R)) +
				(sizeof(float) * 1 * ((vertexDataType & VkD::VDT_G) == VkD::VDT_G)) +
				(sizeof(float) * 1 * ((vertexDataType & VkD::VDT_B) == VkD::VDT_B)) +
				(sizeof(float) * 1 * ((vertexDataType & VkD::VDT_A) == VkD::VDT_A)) +

				// bone indices + weight
				(boneIndexSize * bonePerVertexCount) +
				(sizeof(float) * bonePerVertexCount) +

				// normal, tangent, bitangent
				(sizeof(float) * 3 * ((vertexDataType & VkD::VDT_NORMAL) == VkD::VDT_NORMAL)) +
				(sizeof(float) * 6 * ((vertexDataType & VkD::VDT_TANGENT_BITANGENT) == VkD::VDT_TANGENT_BITANGENT))
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
					else if(indexSize == sizeof(uint32_t))
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
				if ((vertexDataType & VkD::VDT_X) == VkD::VDT_X)
				{
					if ((missingVertexTypes & VkD::VDT_X) == VkD::VDT_X)
						memset((void*)dataPos, 0, sizeof(float));
					else
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mVertices[iVertex].x, sizeof(float));

					dataPos += sizeof(float);
				}
				if ((vertexDataType & VkD::VDT_Y) == VkD::VDT_Y)
				{
					if ((missingVertexTypes & VkD::VDT_Y) == VkD::VDT_Y)
						memset((void*)dataPos, 0, sizeof(float));
					else
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mVertices[iVertex].y, sizeof(float));

					dataPos += sizeof(float);
				}
				if ((vertexDataType & VkD::VDT_Z) == VkD::VDT_Z)
				{
					if ((missingVertexTypes & VkD::VDT_Z) == VkD::VDT_Z)
						memset((void*)dataPos, 0, sizeof(float));
					else
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mVertices[iVertex].z, sizeof(float));

					dataPos += sizeof(float);
				}
				// tex coord
				if ((vertexDataType & VkD::VDT_UV) == VkD::VDT_UV)
				{
					if ((missingVertexTypes & VkD::VDT_UV) == VkD::VDT_UV)
						memset((void*)dataPos, 0, sizeof(float) * 2);
					else
					{
						float uv[2] = { pScene->mMeshes[0]->mTextureCoords[0][iVertex].x, -pScene->mMeshes[0]->mTextureCoords[0][iVertex].y };
						memcpy((void*)dataPos, &uv, sizeof(float) * 2);
					}

					dataPos += sizeof(float) * 2;
				}

				// color
				if ((vertexDataType & VkD::VDT_R) == VkD::VDT_R)
				{
					if ((missingVertexTypes & VkD::VDT_R) == VkD::VDT_R)
						memset((void*)dataPos, 0, sizeof(float));
					else
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mColors[iVertex]->r, sizeof(float));

					dataPos += sizeof(float);
				}
				if ((vertexDataType & VkD::VDT_G) == VkD::VDT_G)
				{
					if ((missingVertexTypes & VkD::VDT_G) == VkD::VDT_G)
						memset((void*)dataPos, 0, sizeof(float));
					else
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mColors[iVertex]->g, sizeof(float));

					dataPos += sizeof(float);
				}
				if ((vertexDataType & VkD::VDT_B) == VkD::VDT_B)
				{
					if ((missingVertexTypes & VkD::VDT_B) == VkD::VDT_B)
						memset((void*)dataPos, 0, sizeof(float));
					else
						memcpy((void*)dataPos, &pScene->mMeshes[0]->mColors[iVertex]->b, sizeof(float));

					dataPos += sizeof(float);
				}
				if ((vertexDataType & VkD::VDT_A) == VkD::VDT_A)
				{
					if ((missingVertexTypes & VkD::VDT_A) == VkD::VDT_A)
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

				float b[] = { 1.0f, 2.0f, 3.0f };
				// normal, tangent, bitangent
				if ((vertexDataType & VkD::VDT_NORMAL) == VkD::VDT_NORMAL)
				{
					if ((missingVertexTypes & VkD::VDT_NORMAL) == VkD::VDT_NORMAL)
						memset((void*)dataPos, 0, sizeof(float) * 3);
					else
						memcpy((void*)dataPos, b, sizeof(float) * 3);

					dataPos += sizeof(float) * 3;
				}
				if ((vertexDataType & VkD::VDT_TANGENT_BITANGENT) == VkD::VDT_TANGENT_BITANGENT)
				{
					if ((missingVertexTypes & VkD::VDT_TANGENT_BITANGENT) == VkD::VDT_TANGENT_BITANGENT)
					{
						memset(&((uint8_t*)vertexData)[dataPos], 0, sizeof(float) * 6);
						dataPos += sizeof(float) * 6;
					}
					else
					{
						memcpy((void*)dataPos, b, sizeof(float) * 3);
						dataPos += sizeof(float) * 3;
						memcpy((void*)dataPos, b, sizeof(float) * 3);
						dataPos += sizeof(float) * 3;
					}
				}
			}

			return true;
		}
	};
}

#endif