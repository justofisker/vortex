#version 450

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(push_constant) uniform UniformPushConstants {
    vec3 sunDir;
    vec3 sunColor;
} PushConstants;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNorm;

layout(location = 0) out vec4 outColor;

void main() {
    float light_atten = clamp(dot(fragNorm, PushConstants.sunDir), 0.1, 1.0);
    vec3 lighting = PushConstants.sunColor * light_atten;
    outColor = texture(texSampler, fragTexCoord) * vec4(fragColor * lighting, 1.0);
}
