#include "Headers/VKPipelineBuild.h"

VkPipelineBuild::VkPipelineBuild()
{
	_shaderModules.resize(2);
	_shaderStages.resize(2);
}

Pipelines VkPipelineBuild::buildPipeline(VkDevice device, VkRenderPass pass)
{
	Pipelines pipeline;
	VkResult result = vkCreatePipelineLayout(device, &_pipelineLayoutInfo, nullptr, &pipeline.layout);
	if (result != VK_SUCCESS)
	{
		throw MessageBox(NULL, "Failed to create pipeline layout!", "Error", MB_OK);
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.stageCount = _shaderStages.size();
	pipelineInfo.pStages = _shaderStages.data();
	pipelineInfo.pVertexInputState = &_vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &_inputAssembly;
	pipelineInfo.pViewportState = &_viewportState;
	pipelineInfo.pRasterizationState = &_rasterizer;
	pipelineInfo.pMultisampleState = &_multisampling;
	pipelineInfo.pDepthStencilState = &_depthStencil;
	pipelineInfo.pColorBlendState = &_colorBlending;
	pipelineInfo.layout = pipeline.layout;
	pipelineInfo.renderPass = pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	return pipeline;
}

void VkPipelineBuild::addShaderModule(VkShaderModule shaderModule, shader shaderWrite)
{
	_shaderModules.at(shaderWrite) = shaderModule;
}

void VkPipelineBuild::setShaderStages(VkShaderStageFlagBits stage[2])
{
	VkPipelineShaderStageCreateInfo shaderStageInfo = {};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.pNext = nullptr;
	shaderStageInfo.stage = stage[0];
	shaderStageInfo.module = _shaderModules.at(VERTEX_SHADER);
	shaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStageInfo2 = {};
	shaderStageInfo2.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo2.pNext = nullptr;
	shaderStageInfo2.stage = stage[1];
	shaderStageInfo2.module = _shaderModules.at(FRAGMENT_SHADER);
	shaderStageInfo2.pName = "main";

	_shaderStages.at(0) = shaderStageInfo;
	_shaderStages.at(1) = shaderStageInfo2;
}

void VkPipelineBuild::setVertexInputInfo()
{
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(VertexStruct);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	_bindingDescription = bindingDescription;
	VkVertexInputAttributeDescription attributeDescription[4] = {};
	attributeDescription[0].binding = 0;
	attributeDescription[0].location = 0;
	attributeDescription[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescription[0].offset = offsetof(VertexStruct, vertices);

	_attributeDescriptions.push_back(attributeDescription[0]);

	attributeDescription[1].binding = 0;
	attributeDescription[1].location = 1;
	attributeDescription[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;;
	attributeDescription[1].offset = offsetof(VertexStruct, normals);

	_attributeDescriptions.push_back(attributeDescription[1]);

	attributeDescription[2].binding = 0;
	attributeDescription[2].location = 2;
	attributeDescription[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescription[2].offset = offsetof(VertexStruct, colors);

	_attributeDescriptions.push_back(attributeDescription[2]);

	attributeDescription[3].binding = 0;
	attributeDescription[3].location = 3;
	attributeDescription[3].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescription[3].offset = offsetof(VertexStruct, uv);

	_attributeDescriptions.push_back(attributeDescription[3]);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.pNext = nullptr;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = _attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = _attributeDescriptions.data();
	vertexInputInfo.pVertexBindingDescriptions = &_bindingDescription;

	_vertexInputInfo = vertexInputInfo;
}

void VkPipelineBuild::setInputAssembly(VkPrimitiveTopology topology)
{
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.pNext = nullptr;
	inputAssembly.topology = topology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	_inputAssembly = inputAssembly;
}

void VkPipelineBuild::setViewport()
{
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;

	viewportState.viewportCount = 1;
	viewportState.pViewports = &_viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &_scissor;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;

	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &_colorBlendAttachment;

	_viewportState = viewportState;
	_colorBlending = colorBlending;
}

void VkPipelineBuild::buildViewport(int width, int height)
{
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	_viewport = viewport;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = { (uint32_t)width, (uint32_t)height };
	_scissor = scissor;
}

void VkPipelineBuild::setScissor()
{
	
}

void VkPipelineBuild::setRasterizer(VkPolygonMode polygonMode)
{
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.pNext = nullptr;

	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	rasterizer.polygonMode = polygonMode;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	rasterizer.depthBiasEnable = false;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	_rasterizer = rasterizer;
}

void VkPipelineBuild::setColorBlendAttachment()
{
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	_colorBlendAttachment = colorBlendAttachment;
}

void VkPipelineBuild::setMultisampling()
{
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.pNext = nullptr;

	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	_multisampling = multisampling;
}

void VkPipelineBuild::setPipelineLayout()
{
	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(ViewProjection);

	_pushConstantRange = pushConstantRange;

	VkPipelineLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;

	layoutInfo.flags = 0;
	layoutInfo.setLayoutCount = 0;
	layoutInfo.pSetLayouts = nullptr;
	layoutInfo.pushConstantRangeCount = 1;
	layoutInfo.pPushConstantRanges = &_pushConstantRange;

	_pipelineLayoutInfo = layoutInfo;
}

void VkPipelineBuild::setDepthStencilState() 
{
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.pNext = nullptr;

	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_FALSE;

	_depthStencil = depthStencil;
}
