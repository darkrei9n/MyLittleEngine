#pragma once


#include "../../../Renderer/Headers/APIBase.h"
#include <Windows.h>
#include <vulkan/vulkan.h>
#include "../../../Utility/Headers/MathStructures.h"
#include "../../Headers/Camera.h"

typedef struct {
	VkPipeline pipeline;
	VkPipelineLayout layout;
	std::string name;
} Pipelines;

typedef struct
{
	Vector4 vertices;
	Vector4 normals;
	Vector4 colors;
	Vector2 uv;

} VertexStruct;

enum shader {
	VERTEX_SHADER = 0,
	FRAGMENT_SHADER = 1
};

class VkPipelineBuild
{
	std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
	std::vector<VkShaderModule> _shaderModules;

	VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo _inputAssembly;

	VkViewport _viewport;
	VkRect2D _scissor;
	VkPipelineRasterizationStateCreateInfo _rasterizer;
	VkPipelineColorBlendAttachmentState _colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo _multisampling;
	VkPipelineLayoutCreateInfo _pipelineLayoutInfo;
	VkPipelineViewportStateCreateInfo _viewportState;
	VkPipelineColorBlendStateCreateInfo _colorBlending;
	VkPipelineDepthStencilStateCreateInfo _depthStencil;
	VkVertexInputBindingDescription _bindingDescription;
	std::vector<VkVertexInputAttributeDescription> _attributeDescriptions;
	VkPushConstantRange _pushConstantRange;

public:
	VkPipelineBuild();
	Pipelines buildPipeline(VkDevice device, VkRenderPass pass);
	void addShaderModule(VkShaderModule shaderModule, shader shaderWrite);
	void setShaderStages(VkShaderStageFlagBits stage[2]);
	void setVertexInputInfo();
	void setInputAssembly(VkPrimitiveTopology topology);
	void setViewport();
	void buildViewport(int width, int height);
	void setScissor();
	void setRasterizer(VkPolygonMode polygonMode);
	void setColorBlendAttachment();
	void setMultisampling();
	void setPipelineLayout();
	void setDepthStencilState();
};