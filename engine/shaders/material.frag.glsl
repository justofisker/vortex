#version 450

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNorm;

layout(location = 0) out vec4 outColor;

const vec3 sun_dir = normalize(vec3(1.0, 2.0, 0.75));
const vec3 sun_color = vec3(1.0);

void main() {
    float light_atten = clamp(dot(fragNorm, sun_dir), 0.1, 1.0);
    vec3 lighting = sun_color * light_atten;
    outColor = texture(texSampler, fragTexCoord) * vec4(fragColor * lighting, 1.0);
}
