#ifndef	SCENE_H
#define SCENE_H

#include "Math3D.h"

#include <string>
#include <vector>

#include <vulkan\vulkan.h>

class Scene
{
public:
	bool done;

	static Math3D::Mat4 viewProjectionData[2];
	std::vector<Math3D::Mat4> modelMatrices;

//*
	// data
	struct Text2D
	{
		Math3D::Vec2 position;
		Math3D::Vec2 scale;
		Math3D::Vec2 spacing;
		Math3D::Vec4 color;

		std::string text;

		void Render(VkCommandBuffer _commandBuffer, VkPipeline _pipeline, VkPipelineLayout _pipelineLayout, std::vector<VkDescriptorSet> _descriptorSets)
		{
			vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

			vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, (uint32_t)_descriptorSets.size(), _descriptorSets.data(), 0, nullptr);

			struct TextVertexPushConstantData
			{
				Math3D::Vec2 position;
				Math3D::Vec2 size;
			} textVertexPushConstantData;
			Math3D::Vec4 uv;

			float xPos = 0.0f;
			float yPos = 0.0f;
			for (size_t iText = 0; text[iText] != '\0'; ++iText)
			{
				if (text[iText] == '\n')
				{
					xPos = 0.0f;
					yPos += spacing.y;
					continue;
				}
				textVertexPushConstantData.position = {
					position.x + xPos,
					position.y + yPos };
				textVertexPushConstantData.size = scale;
				uv = {
					(float)(text[iText] % 16) / 16,
					(float)(text[iText] / 16) / 16,
					(float)(text[iText] % 16) / 16 + 1.0f / 16.0f,
					(float)(text[iText] / 16) / 16 + 1.0f / 16.0f };

				vkCmdPushConstants(_commandBuffer, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(TextVertexPushConstantData), &textVertexPushConstantData);
				vkCmdPushConstants(_commandBuffer, _pipelineLayout, VK_SHADER_STAGE_GEOMETRY_BIT, 16, sizeof(Math3D::Vec4), &uv);
				vkCmdPushConstants(_commandBuffer, _pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 32, sizeof(Math3D::Vec4), &color);

				vkCmdDraw(_commandBuffer, 1, 1, 0, 0);

				xPos += spacing.x;
			}
		}
	};
	struct Camera
	{
		Math3D::Vec3 up;
		Math3D::Vec3 position;

		Math3D::Mat4 view;
		Math3D::Mat4 projection;

		Camera()
		{
			up = { 0.0f, 1.0f, 0.0f };
			position = { 0.0f, 0.0f, 0.0f };
			view = { 0.0f, 0.0f, 0.0f, 0.0f };
			projection = { 0.0f, 0.0f, 0.0f, 0.0f };
		}
	};
	struct Model
	{
		bool visible;
		Math3D::Mat4 transform;
		VkPipeline pipeline;
	};

	// object
	struct GameObject
	{
		std::string name;

		struct Data
		{
			void* data;
			void(*destructor)(void* _data);

			static inline Data GetData(void* _data, void(*_destructor)(void*))
			{
				return { _data, _destructor };
			};
		};
		std::vector<Data> datas;

		struct Module
		{
			std::string name;
			void* data;
			std::vector<void(*)(void* _data)> functions;

			static inline Module GetModule(std::string _name, void* _data, std::vector<void(*)(void*)> _functions)
			{
				return { _name, _data, _functions };
			}
		};
		std::vector<Module> modules;

		static inline GameObject GetGameObject(std::string _name, std::vector<Data> _datas, std::vector<Module> _modules)
		{
			return { _name, _datas, _modules };
		};
	};

	Camera camera;
	static std::vector<GameObject> gameObjects;
	std::vector<Text2D> texts2D;

	void Load();
	void Update();
	void ShutDown();
};
//*/

#endif