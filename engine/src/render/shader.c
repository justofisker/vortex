#include "shader.h"
#include <volk.h>
#include <SDL.h>
#include "globals.h"
#include <stdio.h>
#include <string.h>
#include <cglm/cglm.h>
#include "util.h"
#include <spirv_reflect.h>
#include <assert.h>
#include "render.h"

VkShaderModule VE_Render_CreateShaderModule(uint32_t* pData, uint32_t size) {
    VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    createInfo.codeSize = size;
    createInfo.pCode = pData;

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    vkCreateShaderModule(VE_G_Device, &createInfo, NULL, &shaderModule);

    return shaderModule;
}

static void VE_Render_CreateProgramAtLocation(VE_ProgramT *pProgram, const char *pVertexPath, const char *pFragmentPath, char newLayout) {
    VE_ProgramSourceT programSource = { 0 };
    programSource.pVertexSource =  (uint32_t*) VE_Util_ReadFile(pVertexPath, &programSource.vertexLength);
    programSource.pFragmentSource = (uint32_t*) VE_Util_ReadFile(pFragmentPath, &programSource.fragmentLength);

    if (newLayout)
        VE_Render_CreateProgramLayout(pProgram, &programSource);

    VE_Render_CreateGraphicsPipeline(pProgram, &programSource);

    pProgram->pVertexPath = pVertexPath;
    pProgram->pFragmentPath = pFragmentPath;

    free(programSource.pVertexSource);
    free(programSource.pFragmentSource);
}

VE_ProgramT *VE_Render_CreateProgram(const char *pVertexPath, const char *pFragmentPath) {
    VE_G_ppPrograms[VE_G_ProgramCount] = malloc(sizeof(VE_ProgramT));
    VE_Render_CreateProgramAtLocation(VE_G_ppPrograms[VE_G_ProgramCount], pVertexPath, pFragmentPath, 1);
    return VE_G_ppPrograms[VE_G_ProgramCount++];
}

static void VE_Render_DestroyProgramAtLocation(VE_ProgramT *pProgram, char destroyLayout) {
    vkDestroyPipeline(VE_G_Device, pProgram->pipeline, NULL);
    vkDestroyPipelineLayout(VE_G_Device, pProgram->layout, NULL);
    if (destroyLayout) {
        for (uint32_t i = 0; i < pProgram->descriptorSetLayoutCount; ++i)
            vkDestroyDescriptorSetLayout(VE_G_Device, pProgram->pDescriptorSetLayouts[i], NULL);
    }
}

void VE_Render_DestroyProgram(VE_ProgramT *pProgram) {
    uint32_t programIndex = UINT32_MAX;
    for (uint32_t i = 0; i < VE_G_ProgramCount; ++i) {
        if (VE_G_ppPrograms[i] == pProgram) {
            programIndex = i;
            break;
        }
    }
    if (programIndex == UINT32_MAX) return;

    vkDeviceWaitIdle(VE_G_Device);

    VE_Render_DestroyProgramAtLocation(pProgram, 1);

    free(pProgram);

    if (--VE_G_ProgramCount) {
        VE_G_ppPrograms[programIndex] = VE_G_ppPrograms[VE_G_ProgramCount];
    }
}

void VE_Render_DestroyAllPrograms(char freeMemory) {
    vkDeviceWaitIdle(VE_G_Device);

    for (uint32_t i = 0; i < VE_G_ProgramCount; ++i) {
        VE_Render_DestroyProgramAtLocation((VE_G_ppPrograms[i]), freeMemory);
        if (freeMemory) free(VE_G_ppPrograms[i]);
    }
    VE_G_ProgramCount = 0;
}

void VE_Render_RecreateAllPrograms() {
    uint32_t programCount = VE_G_ProgramCount;
    VE_Render_DestroyAllPrograms(0);
    for (uint32_t i = 0; i < programCount; ++i) {
        VE_Render_CreateProgramAtLocation(VE_G_ppPrograms[i], VE_G_ppPrograms[i]->pVertexPath, VE_G_ppPrograms[i]->pFragmentPath, 0);
    }
    VE_G_ProgramCount = programCount;
}

void VE_Render_CreateProgramLayout(VE_ProgramT *pProgram, VE_ProgramSourceT *pProgramSource) {
    SpvReflectShaderModule vertModule = { 0 };
    SpvReflectResult result = spvReflectCreateShaderModule(pProgramSource->vertexLength, pProgramSource->pVertexSource, &vertModule);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    uint32_t vertexDescriptorSetCount;
    result = spvReflectEnumerateDescriptorSets(&vertModule, &vertexDescriptorSetCount, NULL);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    SpvReflectDescriptorSet **pVertexDescriptorSets = malloc(sizeof(SpvReflectDescriptorSet*) * vertexDescriptorSetCount);
    result = spvReflectEnumerateDescriptorSets(&vertModule, &vertexDescriptorSetCount, pVertexDescriptorSets);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    SpvReflectShaderModule fragModule = { 0 };
    result = spvReflectCreateShaderModule(pProgramSource->fragmentLength, pProgramSource->pFragmentSource, &fragModule);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    uint32_t fragmentDescriptorSetCount;
    result = spvReflectEnumerateDescriptorSets(&fragModule, &fragmentDescriptorSetCount, NULL);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    SpvReflectDescriptorSet **pFragmentDescriptorSets = malloc(sizeof(SpvReflectDescriptorSet*) * fragmentDescriptorSetCount);
    result = spvReflectEnumerateDescriptorSets(&fragModule, &fragmentDescriptorSetCount, pFragmentDescriptorSets);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    assert(vertexDescriptorSetCount == 1);
    assert(fragmentDescriptorSetCount == 1);

    uint32_t descriptorSetCount = max(pVertexDescriptorSets[vertexDescriptorSetCount - 1]->set, pFragmentDescriptorSets[fragmentDescriptorSetCount - 1]->set) + 1;
    uint32_t vertexSetIndex = 0;
    uint32_t fragmentSetIndex = 0;

    pProgram->pDescriptorSetLayouts = malloc(sizeof(VkDescriptorSetLayout) * descriptorSetCount);
    pProgram->descriptorSetLayoutCount = descriptorSetCount;

    for (uint32_t i = 0; i < descriptorSetCount; ++i) {
        VkDescriptorSetLayoutCreateInfo createInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};

        SpvReflectDescriptorSet *pVertexSet = vertexDescriptorSetCount > vertexSetIndex ? pVertexDescriptorSets[vertexSetIndex] : NULL;
        SpvReflectDescriptorSet *pFragmentSet = fragmentDescriptorSetCount > fragmentSetIndex ? pFragmentDescriptorSets[fragmentSetIndex] : NULL;

        if (pVertexSet && pVertexSet->set != i) pVertexSet = NULL;
        if (pFragmentSet && pFragmentSet->set != i) pFragmentSet = NULL;

        uint32_t bindingCount = max(
                pVertexSet ? pVertexSet->bindings[pVertexSet->binding_count - 1]->binding : 0,
                pFragmentSet ? pFragmentSet->bindings[pFragmentSet->binding_count - 1]->binding : 0
                ) + 1;

        VkDescriptorSetLayoutBinding *pBindings = malloc(sizeof(VkDescriptorSetLayoutBinding) * bindingCount);

        uint32_t vertexBindingIndex = 0;
        uint32_t fragmentBindingIndex = 0;

        for (uint32_t j = 0; j < bindingCount; ++j) {
            SpvReflectDescriptorSet *pSet = NULL;
            uint32_t bindingIndex = 0;
            if (pVertexSet && pVertexSet->binding_count > vertexBindingIndex && pVertexSet->bindings[vertexBindingIndex]->binding == j) {
                pSet = pVertexSet;
                bindingIndex = vertexBindingIndex;
            } else {
                pSet = pFragmentSet;
                bindingIndex = fragmentBindingIndex;
            }

            pBindings[j].binding = pSet->bindings[bindingIndex]->binding;
            pBindings[j].descriptorType = pSet->bindings[bindingIndex]->descriptor_type;
            pBindings[j].descriptorCount = 1;
            pBindings[j].pImmutableSamplers = NULL;
            for (uint32_t k = 0; k < pSet->bindings[bindingIndex]->array.dims_count; ++k) {
                pBindings[j].descriptorCount *= pSet->bindings[bindingIndex]->array.dims[k];
            }
            pBindings[j].stageFlags = 0;
            if (pVertexSet && pVertexSet->binding_count > vertexBindingIndex && pVertexSet->bindings[vertexBindingIndex]->binding == j) {
                pBindings[j].stageFlags |= vertModule.shader_stage;
                ++vertexBindingIndex;
            }
            if (pFragmentSet && pFragmentSet->binding_count > fragmentBindingIndex && pFragmentSet->bindings[fragmentBindingIndex]->binding == j) {
                pBindings[j].stageFlags |= fragModule.shader_stage;
                ++fragmentBindingIndex;
            }
        }

        createInfo.pBindings = pBindings;
        createInfo.bindingCount = bindingCount;

        vkCreateDescriptorSetLayout(VE_G_Device, &createInfo, NULL, &pProgram->pDescriptorSetLayouts[i]);

        free((void*) createInfo.pBindings);

        if (pVertexSet)
            ++vertexSetIndex;
        if (pFragmentSet)
            ++fragmentSetIndex;
    }

    free(pVertexDescriptorSets);
    free(pFragmentDescriptorSets);
    spvReflectDestroyShaderModule(&vertModule);
    spvReflectDestroyShaderModule(&fragModule);
}

void VE_Render_CreateGraphicsPipeline(VE_ProgramT *pProgram, VE_ProgramSourceT *pProgramSource) {
    VkShaderModule vertexShaderModule = VE_Render_CreateShaderModule(pProgramSource->pVertexSource, pProgramSource->vertexLength);
    VkShaderModule fragmentShaderModule = VE_Render_CreateShaderModule(pProgramSource->pFragmentSource, pProgramSource->fragmentLength);

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertexShaderModule;
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragmentShaderModule;
    fragmentShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

    VkVertexInputBindingDescription bindingDescription = { 0 };
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(VE_VertexT);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription pAttributeDescriptions[4] = { 0 };
    pAttributeDescriptions[0].binding = 0;
    pAttributeDescriptions[0].location = 0;
    pAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    pAttributeDescriptions[0].offset = offsetof(VE_VertexT, position);
    pAttributeDescriptions[1].binding = 0;
    pAttributeDescriptions[1].location = 1;
    pAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    pAttributeDescriptions[1].offset = offsetof(VE_VertexT, color);
    pAttributeDescriptions[2].binding = 0;
    pAttributeDescriptions[2].location = 2;
    pAttributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    pAttributeDescriptions[2].offset = offsetof(VE_VertexT, normal);
    pAttributeDescriptions[3].binding = 0;
    pAttributeDescriptions[3].location = 3;
    pAttributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
    pAttributeDescriptions[3].offset = offsetof(VE_VertexT, texCoord);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = sizeof(pAttributeDescriptions) / sizeof(pAttributeDescriptions[0]);
    vertexInputInfo.pVertexAttributeDescriptions = pAttributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) VE_G_SwapchainExtent.width;
    viewport.height = (float) VE_G_SwapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = { 0 };
    scissor.offset = (VkOffset2D){ 0, 0 };
    scissor.extent = VE_G_SwapchainExtent;

    VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = NULL;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = { 0 };
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { 0 };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = pProgram->descriptorSetLayoutCount;
    pipelineLayoutInfo.pSetLayouts = pProgram->pDescriptorSetLayouts;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;

    vkCreatePipelineLayout(VE_G_Device, &pipelineLayoutInfo, NULL, &pProgram->layout);

    VkPipelineDepthStencilStateCreateInfo depthStencil = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front;
    depthStencil.back;

    VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = NULL;
    pipelineInfo.layout = pProgram->layout;
    pipelineInfo.renderPass = VE_G_RenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    vkCreateGraphicsPipelines(VE_G_Device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pProgram->pipeline);

    vkDestroyShaderModule(VE_G_Device, vertexShaderModule, NULL);
    vkDestroyShaderModule(VE_G_Device, fragmentShaderModule, NULL);
}
