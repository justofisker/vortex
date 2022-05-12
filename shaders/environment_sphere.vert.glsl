#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNorm;

void main() {
    // Ignore camera translation
    mat4 newView = ubo.view;
    newView[3][0] = 0;
    newView[3][1] = 0;
    newView[3][2] = 0;

    gl_Position = ubo.proj * newView * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragNorm = (ubo.model * vec4(inNormal, 0.0)).xyz;
}
