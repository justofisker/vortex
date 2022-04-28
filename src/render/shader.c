#include "shader.h"

#include <volk.h>
#include <SDL.h>

#include "globals.h"

#include <stdio.h>

static char *VE_Util_ReadFile(const char *path, uint32_t *size) {
    FILE* file;
    file = fopen(path, "rb");
    if(!file)
    {
        printf("Failed to open %s", path);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    if(size)
        *size = (int)fsize;
    fseek(file, 0, SEEK_SET);
    char* content = malloc(fsize + 1);
    fread(content, 1, fsize, file);
    fclose(file);
    content[fsize] = 0;
    return content;
}

VkShaderModule VE_Render_CreateShaderModule(uint32_t* pData, uint32_t size) {
    VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    createInfo.codeSize = size;
    createInfo.pCode = pData;

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    vkCreateShaderModule(VE_G_Device, &createInfo, NULL, &shaderModule);

    return shaderModule;
}

static void VE_Render_CreateProgramAtLocation(VE_ProgramT *pProgram, const char *pVertexPath, const char *pFragmentPath) {
    uint32_t vertexSourceSize;
    uint32_t *pVertexSource =  (uint32_t*) VE_Util_ReadFile(pVertexPath, &vertexSourceSize);
    uint32_t fragmentSourceSize;
    uint32_t *pFragmentSource = (uint32_t*) VE_Util_ReadFile(pFragmentPath, &fragmentSourceSize);
    VkShaderModule vertexShaderModule = VE_Render_CreateShaderModule(pVertexSource, vertexSourceSize);
    VkShaderModule fragmentShaderModule = VE_Render_CreateShaderModule(pFragmentSource, fragmentSourceSize);

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertexShaderModule;
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragmentShaderModule;
    fragmentShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = NULL;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = NULL;

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
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = NULL;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;

    vkCreatePipelineLayout(VE_G_Device, &pipelineLayoutInfo, NULL, &pProgram->layout);

    VkAttachmentDescription colorAttachment = { 0 };
    colorAttachment.format = VE_G_SwapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = { 0 };
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = { 0 };
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = { 0 };
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    vkCreateRenderPass(VE_G_Device, &renderPassInfo, NULL, &pProgram->renderPass);

    VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = NULL;
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
    free(pVertexSource);
    free(pFragmentSource);

    VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    framebufferInfo.renderPass = pProgram->renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.width = VE_G_SwapchainExtent.width;
    framebufferInfo.height = VE_G_SwapchainExtent.height;
    framebufferInfo.layers = 1;

    pProgram->pFramebuffers = malloc(sizeof(VkFramebuffer) * VE_G_SwapchainImageCount);
    for (uint32_t i = 0; i < VE_G_SwapchainImageCount; ++i) {
        framebufferInfo.pAttachments = &VE_G_pSwapchainImageViews[i];
        vkCreateFramebuffer(VE_G_Device, &framebufferInfo, NULL, &pProgram->pFramebuffers[i]);
    }

    pProgram->pVertexPath = pVertexPath;
    pProgram->pFragmentPath = pFragmentPath;
}

VE_ProgramT *VE_Render_CreateProgram(const char *pVertexPath, const char *pFragmentPath) {
    VE_G_ppPrograms[VE_G_ProgramCount] = malloc(sizeof(VE_ProgramT));
    VE_Render_CreateProgramAtLocation(VE_G_ppPrograms[VE_G_ProgramCount], pVertexPath, pFragmentPath);
    return VE_G_ppPrograms[VE_G_ProgramCount++];
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
    for (uint32_t i = 0; i < VE_G_SwapchainImageCount; ++i)
        vkDestroyFramebuffer(VE_G_Device, pProgram->pFramebuffers[i], NULL);
    free(pProgram->pFramebuffers);
    vkDestroyPipeline(VE_G_Device, pProgram->pipeline, NULL);
    vkDestroyPipelineLayout(VE_G_Device, pProgram->layout, NULL);
    vkDestroyRenderPass(VE_G_Device, pProgram->renderPass, NULL);

    free(pProgram);

    if (--VE_G_ProgramCount) {
        VE_G_ppPrograms[programIndex] = VE_G_ppPrograms[VE_G_ProgramCount];
        VE_G_ppPrograms[VE_G_ProgramCount] = NULL;
    } else {
        VE_G_ppPrograms[programIndex] = NULL;
    }
}

void VE_Render_DestroyAllPrograms(char freeMemory) {
    for (uint32_t i = 0; i < VE_G_ProgramCount; ++i) {
        for (uint32_t j = 0; j < VE_G_SwapchainImageCount; ++j)
            vkDestroyFramebuffer(VE_G_Device, VE_G_ppPrograms[i]->pFramebuffers[j], NULL);
        free(VE_G_ppPrograms[i]->pFramebuffers);
        vkDestroyPipeline(VE_G_Device, VE_G_ppPrograms[i]->pipeline, NULL);
        vkDestroyPipelineLayout(VE_G_Device, VE_G_ppPrograms[i]->layout, NULL);
        vkDestroyRenderPass(VE_G_Device, VE_G_ppPrograms[i]->renderPass, NULL);
        if (freeMemory) free(VE_G_ppPrograms[i]);
    }
    VE_G_ProgramCount = 0;
}

void VE_Render_RecreateAllPrograms() {
    uint32_t programCount = VE_G_ProgramCount;
    const char *shaderPaths[VE_RENDER_MAX_PROGRAMS][2];
    for (uint32_t i = 0; i < programCount; ++i) {
        shaderPaths[i][0] = VE_G_ppPrograms[i]->pVertexPath;
        shaderPaths[i][1] = VE_G_ppPrograms[i]->pFragmentPath;
    }
    VE_Render_DestroyAllPrograms(0);
    for (uint32_t i = 0; i < programCount; ++i) {
        VE_Render_CreateProgramAtLocation(VE_G_ppPrograms[i], shaderPaths[i][0], shaderPaths[i][1]);
    }
    VE_G_ProgramCount = programCount;
}
