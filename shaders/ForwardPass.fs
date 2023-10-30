#version 330 core

layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec3 gAlbedo;
layout (location = 2) out vec4 gSpecular;

in vec3 normal;
in vec3 posForColoring; // view space coordinates
in vec2 texCoord;

uniform vec3 kd; // k of diffuse
uniform vec3 ks; // k of specular
uniform float specularExponent;

uniform bool use_tex_d;
uniform sampler2D diffuseTexture;
uniform bool use_tex_s;
uniform sampler2D specularTexture;

uniform vec3 lightPosition;

void main()
{
	gNormal.xyz = normalize(normal);
	gNormal.w = posForColoring.z;
	gAlbedo = use_tex_d ? texture(diffuseTexture, texCoord).rgb : kd;
	gSpecular.rgb = use_tex_s ? texture(specularTexture, texCoord).rgb : ks;
	gSpecular.a = specularExponent;
}