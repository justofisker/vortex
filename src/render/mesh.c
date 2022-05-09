#include "mesh.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <volk.h>
#include <cglm/cglm.h>
#include "globals.h"
#include "util.h"

VE_MeshObject_T *VE_Render_CreateCylinderMesh(int vertex_count, float radius, float depth, VE_ProgramT *pProgram)
{
    vertex_count = vertex_count > 3 ? vertex_count : 3;

    uint32_t nVertices = vertex_count * (2 + 2) + 6;

    VE_VertexT *pVertices = malloc(sizeof(VE_VertexT) * nVertices);

    uint32_t i;
    for (i = 0; i < nVertices; ++i) {
        pVertices[i].color[0] = 1.0f;
        pVertices[i].color[1] = 1.0f;
        pVertices[i].color[2] = 1.0f;
    }

    int side_vertex_count = vertex_count + 1;
    for(i = 0; i < vertex_count + 1; ++i)
    {
        float theta = GLM_PIf * 2 / vertex_count * i;
        float x_unscaled = cosf(theta);
        float y_unscaled = sinf(theta);
        float x = x_unscaled * radius;
        float y = y_unscaled * radius;

        // Sides [0 -> verticies_count * 2 - 1]
        pVertices[i].position[0] = x;
        pVertices[i].position[1] = -.5f * depth;
        pVertices[i].position[2] = y;
        pVertices[i].normal[0] = x_unscaled;
        pVertices[i].normal[1] = 0.0f;
        pVertices[i].normal[2] = y_unscaled;
        pVertices[i].texCoord[0] = theta / (2 * GLM_PIf);
        pVertices[i].texCoord[1] = 0.f;

        pVertices[i + side_vertex_count].position[0] = x;
        pVertices[i + side_vertex_count].position[1] = .5f * depth;
        pVertices[i + side_vertex_count].position[2] = y;
        pVertices[i + side_vertex_count].normal[0] = x_unscaled;
        pVertices[i + side_vertex_count].normal[1] = 0.0f;
        pVertices[i + side_vertex_count].normal[2] = y_unscaled;
        pVertices[i + side_vertex_count].texCoord[0] = theta / (2 * GLM_PIf);
        pVertices[i + side_vertex_count].texCoord[1] = 1.f;

        // Top / Bottom Faces [verticies_count * 2 -> verticies_count * 4 - 1]
        pVertices[i + side_vertex_count * 2].position[0] = x;
        pVertices[i + side_vertex_count * 2].position[1] = -.5f * depth;
        pVertices[i + side_vertex_count * 2].position[2] = y;
        pVertices[i + side_vertex_count * 2].texCoord[0] = x_unscaled * 0.5f + 0.5f;
        pVertices[i + side_vertex_count * 2].texCoord[1] = y_unscaled * 0.5f + 0.5f;
        pVertices[i + side_vertex_count * 2].normal[0] = 0.0f;
        pVertices[i + side_vertex_count * 2].normal[1] = -1.0f;
        pVertices[i + side_vertex_count * 2].normal[2] = 0.0f;

        pVertices[i + side_vertex_count * 3].position[0] = x;
        pVertices[i + side_vertex_count * 3].position[1] = .5f * depth;
        pVertices[i + side_vertex_count * 3].position[2] = y;
        pVertices[i + side_vertex_count * 3].texCoord[0] = x_unscaled * 0.5f + 0.5f;
        pVertices[i + side_vertex_count * 3].texCoord[1] = y_unscaled * 0.5f + 0.5f;
        pVertices[i + side_vertex_count * 3].normal[0] = 0.0f;
        pVertices[i + side_vertex_count * 3].normal[1] = 1.0f;
        pVertices[i + side_vertex_count * 3].normal[2] = 0.0f;

    }
    {
        // Center vertex for Top / Bottom Faces

        pVertices[side_vertex_count * 4 + 0].position[0] = 0;
        pVertices[side_vertex_count * 4 + 0].position[1] = -0.5f * depth;
        pVertices[side_vertex_count * 4 + 0].position[2] = 0;
        pVertices[side_vertex_count * 4 + 0].normal[0] = 0.0f;
        pVertices[side_vertex_count * 4 + 0].normal[1] = -1.0f;
        pVertices[side_vertex_count * 4 + 0].normal[2] = 0.0f;
        pVertices[side_vertex_count * 4 + 0].texCoord[0] = 0.5f;
        pVertices[side_vertex_count * 4 + 0].texCoord[1] = 0.5f;
        pVertices[side_vertex_count * 4 + 1].position[0] = 0;
        pVertices[side_vertex_count * 4 + 1].position[1] = 0.5f * depth;
        pVertices[side_vertex_count * 4 + 1].position[2] = 0;
        pVertices[side_vertex_count * 4 + 1].normal[0] = 0.0f;
        pVertices[side_vertex_count * 4 + 1].normal[1] = 1.0f;
        pVertices[side_vertex_count * 4 + 1].normal[2] = 0.0f;
        pVertices[side_vertex_count * 4 + 1].texCoord[0] = 0.5f;
        pVertices[side_vertex_count * 4 + 1].texCoord[1] = 0.5f;
    }

    typedef uint16_t Index;

    uint32_t nIndices = vertex_count * (6 + 6) + 2;

    Index *pIndices = malloc(sizeof(Index) * nIndices);

    for(i = 0; i < vertex_count; ++i)
    {
        int offset = i * 6;
        pIndices[offset + 0] = i + side_vertex_count;
        pIndices[offset + 1] = i + side_vertex_count + 1;
        pIndices[offset + 2] = i + 0;
        pIndices[offset + 3] = pIndices[offset + 1];
        pIndices[offset + 4] = i + 1;
        pIndices[offset + 5] = pIndices[offset + 2];
    }
    for(i = 0; i < vertex_count; ++i)
    {
        int offset = 6 * vertex_count + i * 6;
        pIndices[offset + 0] = side_vertex_count * 4 + 1;
        pIndices[offset + 1] = side_vertex_count * 3 + 1 + i;
        pIndices[offset + 2] = side_vertex_count * 3 + 0 + i;
        pIndices[offset + 3] = side_vertex_count * 2 + 0 + i;
        pIndices[offset + 4] = side_vertex_count * 2 + 1 + i;
        pIndices[offset + 5] = side_vertex_count * 4 + 0;
    }

    VE_MeshObject_T *pMeshObject = VE_Render_CreateMeshObject(pVertices, nVertices, pIndices, nIndices, pProgram);


    free(pVertices);
    free(pIndices);

    return pMeshObject;
}

VE_MeshObject_T *VE_Render_CreatePlaneMesh(float width, float height, VE_ProgramT *pProgram) {
    VE_VertexT vertices[] = {
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0, 0.0, 1.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0, 0.0, 1.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0, 0.0, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0, 0.0, 1.0f}, {0.0f, 1.0f}},
    };

    uint16_t indices[] = {
            0, 1, 2, 2, 3, 0
    };

    return VE_Render_CreateMeshObject(vertices, sizeof(vertices) / sizeof(vertices[0]), indices, sizeof(indices) / sizeof(indices[0]), pProgram);
}

void VE_Render_UpdateMeshUniformBuffer(VE_MeshObject_T *pMeshObject, mat4 modelMatrix) {
    VE_UniformBufferObjectT ubo = {0};

    glm_mat4_copy(modelMatrix, ubo.model);

    glm_mat4_copy(VE_G_ProjectionMatrix, ubo.projection);
    glm_mat4_copy(VE_G_ViewMatrix, ubo.view);

    void *data;
    vkMapMemory(VE_G_Device, pMeshObject->pUniformBufferMemory[VE_G_CurrentFrame], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(VE_G_Device, pMeshObject->pUniformBufferMemory[VE_G_CurrentFrame]);
}

void VE_Render_SetMeshObjectTexture(VE_MeshObject_T *pMeshObject, VE_TextureT *pTexture) {
    for (uint32_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; ++i) {
        VkDescriptorImageInfo imageInfo = { 0 };
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = pTexture->imageView;
        imageInfo.sampler = pTexture->sampler;

        VkWriteDescriptorSet descriptorWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        descriptorWrite.dstSet = pMeshObject->pDescriptorSets[i];
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

VE_MeshObject_T *VE_Render_CreateMeshObject(VE_VertexT *vertices, uint32_t vertexCount, uint16_t *indices, uint32_t indexCount, VE_ProgramT *pProgram) {
    VE_MeshObject_T *pMeshObject = malloc(sizeof(VE_MeshObject_T));

    VkDeviceSize bufferSize = sizeof(VE_UniformBufferObjectT);
    for (size_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; i++) {
        VE_Render_CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               &pMeshObject->pUniformBuffer[i], &pMeshObject->pUniformBufferMemory[i]);
    }

    pMeshObject->pVertexBuffer = VE_Render_CreateVertexBuffer(vertices, vertexCount);
    pMeshObject->pIndexBuffer = VE_Render_CreateIndexBuffer(indices, indexCount);
    pMeshObject->pProgram = pProgram;

    VkDescriptorPoolSize pPoolSizes[2] = { 0 };
    pPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pPoolSizes[0].descriptorCount = VE_RENDER_MAX_FRAMES_IN_FLIGHT;
    pPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pPoolSizes[1].descriptorCount = VE_RENDER_MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    descriptorPoolCreateInfo.poolSizeCount = 2;
    descriptorPoolCreateInfo.pPoolSizes = pPoolSizes;
    descriptorPoolCreateInfo.maxSets = VE_RENDER_MAX_FRAMES_IN_FLIGHT;

    vkCreateDescriptorPool(VE_G_Device, &descriptorPoolCreateInfo, NULL, &pMeshObject->descriptorPool);

    VkDescriptorSetLayout pDescriptorSetLayouts[VE_RENDER_MAX_FRAMES_IN_FLIGHT];
    for (uint32_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; ++i)
        pDescriptorSetLayouts[i] = pProgram->pDescriptorSetLayouts[0];

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    descriptorSetAllocateInfo.descriptorPool = pMeshObject->descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = VE_RENDER_MAX_FRAMES_IN_FLIGHT;
    descriptorSetAllocateInfo.pSetLayouts = pDescriptorSetLayouts;

    vkAllocateDescriptorSets(VE_G_Device, &descriptorSetAllocateInfo, pMeshObject->pDescriptorSets);

    for (uint32_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; ++i) {
        VkDescriptorBufferInfo bufferInfo = { 0 };
        bufferInfo.buffer = pMeshObject->pUniformBuffer[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(VE_UniformBufferObjectT);

        VkWriteDescriptorSet descriptorWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        descriptorWrite.dstSet = pMeshObject->pDescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = NULL;
        descriptorWrite.pTexelBufferView = NULL;

        vkUpdateDescriptorSets(VE_G_Device, 1, &descriptorWrite, 0, NULL);
    }

    return pMeshObject;
}

void VE_Render_DestroyMeshObject(VE_MeshObject_T *pMeshObject) {
    vkQueueWaitIdle(VE_G_GraphicsQueue);
    VE_Render_DestroyBuffer(pMeshObject->pVertexBuffer);
    VE_Render_DestroyBuffer(pMeshObject->pIndexBuffer);

    vkDestroyDescriptorPool(VE_G_Device, pMeshObject->descriptorPool, NULL);

    for (uint32_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroyBuffer(VE_G_Device, pMeshObject->pUniformBuffer[i], NULL);
        vkFreeMemory(VE_G_Device, pMeshObject->pUniformBufferMemory[i], NULL);
    }
}

VE_BufferT *VE_Render_CreateVertexBuffer(VE_VertexT *vertices, uint32_t count) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize bufferSize = sizeof(vertices[0]) * count;
    VE_Render_CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(VE_G_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t) bufferSize);
    vkUnmapMemory(VE_G_Device, stagingBufferMemory);

    VE_BufferT *pVertexBuffer = malloc(sizeof(VE_BufferT));

    VE_Render_CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &pVertexBuffer->buffer, &pVertexBuffer->deviceMemory);

    VE_Render_CopyBuffer(pVertexBuffer->buffer, stagingBuffer, bufferSize);

    vkDestroyBuffer(VE_G_Device, stagingBuffer, NULL);
    vkFreeMemory(VE_G_Device, stagingBufferMemory, NULL);

    pVertexBuffer->instanceCount = count;

    return pVertexBuffer;
}

VE_BufferT *VE_Render_CreateIndexBuffer(uint16_t *indices, uint32_t count) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize bufferSize = sizeof(indices[0]) * count;
    VE_Render_CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(VE_G_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, (size_t) bufferSize);
    vkUnmapMemory(VE_G_Device, stagingBufferMemory);

    VE_BufferT *pIndexBuffer = malloc(sizeof(VE_BufferT));

    VE_Render_CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &pIndexBuffer->buffer, &pIndexBuffer->deviceMemory);

    VE_Render_CopyBuffer(pIndexBuffer->buffer, stagingBuffer, bufferSize);

    vkDestroyBuffer(VE_G_Device, stagingBuffer, NULL);
    vkFreeMemory(VE_G_Device, stagingBufferMemory, NULL);

    pIndexBuffer->instanceCount = count;

    return pIndexBuffer;
}

void VE_Render_DestroyBuffer(VE_BufferT *pBuffer) {
    vkDeviceWaitIdle(VE_G_Device);

    vkDestroyBuffer(VE_G_Device, pBuffer->buffer, NULL);
    vkFreeMemory(VE_G_Device, pBuffer->deviceMemory, NULL);

    free(pBuffer);
}
