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

	Math3D::Mat4 viewProjectionData[2];
	std::vector<Math3D::Mat4> modelMatrices;

	// data
	struct Camera
	{
		Math3D::Vec3 up;
		Math3D::Vec3 position;

		Math3D::Mat4 view;
		Math3D::Mat4 perspective;
	};

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

	// object
	struct Object
	{
		struct Data
		{
			char* name;
			void* data;
		};
		struct Module
		{
			char* name;
			Data* data;
			void(*function)(void* _data);
		};
		std::vector<Data> datas;
		std::vector<Module> modules;
	};

	Camera camera;
	std::vector<Object> objects;
	std::vector<Text2D> texts2D;

	void Load();
	void Update(float _deltaTime);
	void ShutDown() {};
};

#endif