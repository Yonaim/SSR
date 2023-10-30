#version 330 core

#define NUM_LIGHTS 3

layout (location = 0) out vec4 colorBuffer;

in vec2 texCoord;

uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSpecular;
uniform sampler2D depthMap;

uniform float SCR_WIDTH;
uniform float SCR_HEIGHT;
uniform mat4 invProj;

struct Light
{
	vec3 position;
	vec3 intensity; // albedo
	float constant; // constant attenuation
	float linear; // linear attenuation
	float quadratic; // quadratic attenuation
};
uniform Light lights[NUM_LIGHTS];

// screen-space 좌표에다가 p행렬의 역행렬을 적용해 view space 좌표를 구한다
vec3 computePositionInViewSpace(float z)
{
	vec2 posCanonical = vec2(gl_FragCoord.x / SCR_WIDTH, gl_FragCoord.y / SCR_HEIGHT) * 2 - 1;
	// 0~1 범위를 -1~1 범위로 매핑
	vec4 posView = invProj * vec4(posCanonical, z, 1);
	posView /= posView.w;
	return (posView.xyz);
}

void main()
{
	vec3 Kd = texture(gAlbedo, texCoord).rgb; // albedo
	vec4 spec = texture(gSpecular, texCoord);
	vec3 Ks = spec.rgb; // specular reflection
	// float specularExponent = spec.a; // specular exponent
	float depth = texture(depthMap, texCoord).r * 2 - 1; // depth
	// 0~1를 -1~1로 매핑

	vec3 norm = texture(gNormal, texCoord).rgb;
	vec3 posForColoring = computePositionInViewSpace(depth); // view space coordinates
	// gl_FragCoord의 x,y와 depth를 이용해 view space 좌표를 구함

	vec3 color = vec3(0);
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		vec3 lightDir = normalize(lights[i].position - posForColoring); // pixel-light
		vec3 viewDir = normalize(-posForColoring); // pixel-camera
		vec3 halfVec = normalize(lightDir + viewDir); // 
		
		// Blinn-phong
		vec3 ambient = 0.1f * Kd;
		vec3 diffuse = max(0, dot(norm, lightDir)) * Kd;
		vec3 specular = pow(max(0, dot(halfVec, norm)), 1000) * Ks;
		vec3 blinn = lights[i].intensity * (diffuse + specular) + ambient;

		float distance = length(lights[i].position - posForColoring);
		float attenuation = 1.0 / \
						(lights[i].constant \
						+ lights[i].linear * distance \
						+ lights[i].quadratic * (distance * distance));

		color += attenuation * blinn;
	}
	colorBuffer = vec4(color, 1);
}