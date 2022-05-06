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

VkShaderModule VE_Render_CreateShaderModule(uint32_t* pData, uint32_t size) {
    VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    createInfo.codeSize = size;
    createInfo.pCode = pData;

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    vkCreateShaderModule(VE_G_Device, &createInfo, NULL, &shaderModule);

    return shaderModule;
}

static void VE_Render_CreateProgramAtLocation(VE_ProgramT *pProgram, const char *pVertexPath, const char *pFragmentPath) {
    VkDeviceSize bufferSize = sizeof(VE_UniformBufferObjectT);
    for (size_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; i++) {
        VE_Render_CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               &pProgram->pUniformBuffer[i], &pProgram->pUniformBufferMemory[i]);
    }

    VE_ProgramSourceT programSource = { 0 };
    programSource.pVertexSource =  (uint32_t*) VE_Util_ReadFile(pVertexPath, &programSource.vertexLength);
    programSource.pFragmentSource = (uint32_t*) VE_Util_ReadFile(pFragmentPath, &programSource.fragmentLength);

    VE_Render_CreateProgramLayout(pProgram, &programSource);

    VkDescriptorPoolSize pPoolSizes[2] = { 0 };
    pPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pPoolSizes[0].descriptorCount = VE_RENDER_MAX_FRAMES_IN_FLIGHT;
    pPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pPoolSizes[1].descriptorCount = VE_RENDER_MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    descriptorPoolCreateInfo.poolSizeCount = 2;
    descriptorPoolCreateInfo.pPoolSizes = pPoolSizes;
    descriptorPoolCreateInfo.maxSets = VE_RENDER_MAX_FRAMES_IN_FLIGHT;

    vkCreateDescriptorPool(VE_G_Device, &descriptorPoolCreateInfo, NULL, &pProgram->descriptorPool);

    VkDescriptorSetLayout pDescriptorSetLayouts[VE_RENDER_MAX_FRAMES_IN_FLIGHT];
    for (uint32_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; ++i)
        pDescriptorSetLayouts[i] = pProgram->descriptorSetLayout;

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    descriptorSetAllocateInfo.descriptorPool = pProgram->descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = VE_RENDER_MAX_FRAMES_IN_FLIGHT;
    descriptorSetAllocateInfo.pSetLayouts = pDescriptorSetLayouts;

    vkAllocateDescriptorSets(VE_G_Device, &descriptorSetAllocateInfo, pProgram->pDescriptorSets);

    for (uint32_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; ++i) {
        VkDescriptorBufferInfo bufferInfo = { 0 };
        bufferInfo.buffer = pProgram->pUniformBuffer[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(VE_UniformBufferObjectT);

        VkWriteDescriptorSet descriptorWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        descriptorWrite.dstSet = pProgram->pDescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = NULL;
        descriptorWrite.pTexelBufferView = NULL;

        vkUpdateDescriptorSets(VE_G_Device, 1, &descriptorWrite, 0, NULL);
    }

    pProgram->renderPass = VE_Render_CreateRenderpass();

    VE_Render_CreateGraphicsPipeline(pProgram, &programSource);

    VkImageView framebufferAttachments[] = { VK_NULL_HANDLE, VE_G_DepthImageView };

    VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    framebufferInfo.renderPass = pProgram->renderPass;
    framebufferInfo.attachmentCount = sizeof(framebufferAttachments) / sizeof(framebufferAttachments[0]);
    framebufferInfo.pAttachments = framebufferAttachments;
    framebufferInfo.width = VE_G_SwapchainExtent.width;
    framebufferInfo.height = VE_G_SwapchainExtent.height;
    framebufferInfo.layers = 1;

    pProgram->pFramebuffers = malloc(sizeof(VkFramebuffer) * VE_G_SwapchainImageCount);
    for (uint32_t i = 0; i < VE_G_SwapchainImageCount; ++i) {
        framebufferAttachments[0] = VE_G_pSwapchainImageViews[i];
        vkCreateFramebuffer(VE_G_Device, &framebufferInfo, NULL, &pProgram->pFramebuffers[i]);
    }

    pProgram->pVertexPath = pVertexPath;
    pProgram->pFragmentPath = pFragmentPath;

    free(programSource.pVertexSource);
    free(programSource.pFragmentSource);
}

VE_ProgramT *VE_Render_CreateProgram(const char *pVertexPath, const char *pFragmentPath) {
    VE_G_ppPrograms[VE_G_ProgramCount] = malloc(sizeof(VE_ProgramT));
    VE_Render_CreateProgramAtLocation(VE_G_ppPrograms[VE_G_ProgramCount], pVertexPath, pFragmentPath);
    return VE_G_ppPrograms[VE_G_ProgramCount++];
}

static void VE_Render_DestroyProgramAtLocation(VE_ProgramT *pProgram) {
    for (uint32_t i = 0; i < VE_G_SwapchainImageCount; ++i)
        vkDestroyFramebuffer(VE_G_Device, pProgram->pFramebuffers[i], NULL);
    free(pProgram->pFramebuffers);
    vkDestroyPipeline(VE_G_Device, pProgram->pipeline, NULL);
    vkDestroyPipelineLayout(VE_G_Device, pProgram->layout, NULL);
    vkDestroyRenderPass(VE_G_Device, pProgram->renderPass, NULL);
    vkDestroyDescriptorSetLayout(VE_G_Device, pProgram->descriptorSetLayout, NULL);
    vkDestroyDescriptorPool(VE_G_Device, pProgram->descriptorPool, NULL);

    for (uint32_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroyBuffer(VE_G_Device, pProgram->pUniformBuffer[i], NULL);
        vkFreeMemory(VE_G_Device, pProgram->pUniformBufferMemory[i], NULL);
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

    VE_Render_DestroyProgramAtLocation(pProgram);

    free(pProgram);

    if (--VE_G_ProgramCount) {
        VE_G_ppPrograms[programIndex] = VE_G_ppPrograms[VE_G_ProgramCount];
    }
}

void VE_Render_DestroyAllPrograms(char freeMemory) {
    vkDeviceWaitIdle(VE_G_Device);

    for (uint32_t i = 0; i < VE_G_ProgramCount; ++i) {
        VE_Render_DestroyProgramAtLocation((VE_G_ppPrograms[i]));
        if (freeMemory) free(VE_G_ppPrograms[i]);
    }
    VE_G_ProgramCount = 0;
}

void VE_Render_RecreateAllPrograms() {
    uint32_t programCount = VE_G_ProgramCount;
    VE_Render_DestroyAllPrograms(0);
    for (uint32_t i = 0; i < programCount; ++i) {
        VE_Render_CreateProgramAtLocation(VE_G_ppPrograms[i], VE_G_ppPrograms[i]->pVertexPath, VE_G_ppPrograms[i]->pFragmentPath);
    }
    VE_G_ProgramCount = programCount;
}

void VE_Render_UpdateUniformBuffer(VE_ProgramT *pProgram) {
    float time = SDL_GetTicks() / 1000.f;

    VE_UniformBufferObjectT ubo = { 0 };
    glm_mat4_identity(ubo.model);
    glm_rotate(ubo.model, time * GLM_PI_2f, (vec3){0.f, 0.f, 1.f});
    glm_lookat((vec3){2.f, 2.f, 2.f}, (vec3){0.f, 0.f, 0.f}, (vec3){0.0f, 0.0f, 1.0f}, ubo.view);
    glm_perspective(glm_rad(60.f), VE_G_SwapchainExtent.width / (float)VE_G_SwapchainExtent.height, 0.1f, 10.f, ubo.projection);
    ubo.projection[1][1] *= -1;

    void* data;
    vkMapMemory(VE_G_Device, pProgram->pUniformBufferMemory[VE_G_CurrentFrame], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(VE_G_Device, pProgram->pUniformBufferMemory[VE_G_CurrentFrame]);
}

void VE_Render_SetProgramSampler(VE_ProgramT *pProgram, VE_TextureT *pTexture) {
    for (uint32_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; ++i) {
        VkDescriptorImageInfo imageInfo = { 0 };
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = pTexture->imageView;
        imageInfo.sampler = pTexture->sampler;

        VkWriteDescriptorSet descriptorWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        descriptorWrite.dstSet = pProgram->pDescriptorSets[i];
        descriptorWrite.dstBinding = 1;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = NULL;
        descriptorWrite.pImageInfo = &imageInfo;
        descriptorWrite.pTexelBufferView = NULL;

        vkUpdateDescriptorSets(VE_G_Device, 1, &descriptorWrite, 0, NULL);
    }
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

        vkCreateDescriptorSetLayout(VE_G_Device, &createInfo, NULL, &pProgram->descriptorSetLayout); // TODO: Allow programs to have multiple descriptor sets

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

    VkVertexInputAttributeDescription pAttributeDescriptions[3] = { 0 };
    pAttributeDescriptions[0].binding = 0;
    pAttributeDescriptions[0].location = 0;
    pAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    pAttributeDescriptions[0].offset = offsetof(VE_VertexT, pos);
    pAttributeDescriptions[1].binding = 0;
    pAttributeDescriptions[1].location = 1;
    pAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    pAttributeDescriptions[1].offset = offsetof(VE_VertexT, color);
    pAttributeDescriptions[2].binding = 0;
    pAttributeDescriptions[2].location = 2;
    pAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    pAttributeDescriptions[2].offset = offsetof(VE_VertexT, texCoord);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 3;
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
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &pProgram->descriptorSetLayout;
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
    pipelineInfo.renderPass = pProgram->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    vkCreateGraphicsPipelines(VE_G_Device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pProgram->pipeline);

    vkDestroyShaderModule(VE_G_Device, vertexShaderModule, NULL);
    vkDestroyShaderModule(VE_G_Device, fragmentShaderModule, NULL);
}

VkRenderPass VE_Render_CreateRenderpass() {
    VkAttachmentDescription colorAttachment = { 0 };
    colorAttachment.format = VE_G_SwapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment = { 0 };
    depthAttachment.format = VE_G_DepthImageFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = { 0 };
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = { 0 };
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = { 0 };
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = { 0 };
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    renderPassInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkRenderPass renderPass;
    vkCreateRenderPass(VE_G_Device, &renderPassInfo, NULL, &renderPass);

    return renderPass;
}