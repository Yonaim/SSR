#version 330 core

layout (location = 0) out vec4 reflectionColor;

uniform sampler2D gNormal;
uniform sampler2D colorBuffer;
uniform sampler2D depthMap;

uniform float SCR_WIDTH;
uniform float SCR_HEIGHT;
uniform mat4 invProjection;
uniform mat4 projection;

bool rayIsOutofScreen(vec2 ray)
{
	return (ray.x > 1 || ray.y > 1 || ray.x < 0 || ray.y < 0);
}

// iterationCount번 만큼 ray가 행진하고, ray와 맞닿는 오브젝트가 있다면 해당 컬러를 반환
// 레이와 만나는 가장 가까운 오브젝트를 찾아서 반환한다
vec3 TraceRay(vec3 rayPos, vec3 dir, int iterationCount)
{
	vec3 hitColor = vec3(0);
	float sampleDepth;
	float depthDiff;

	for (int i = 0; i < iterationCount; i++)
	{
		rayPos += dir;
		if (rayIsOutofScreen(rayPos.xy))
			break ;

		sampleDepth = texture(depthMap, rayPos.xy).r;
		depthDiff = rayPos.z - sampleDepth;
		if (depthDiff >= 0 && depthDiff < 0.00001)
		{
			hitColor = texture(colorBuffer, rayPos.xy).rgb;
			break ;
		}
	}
	return (hitColor);
}

void main()
{
	float maxRayDistance = 100.0f;

	vec3 position_tex; // pixel coordinates in Texture space
	position_tex.xy = vec2(gl_FragCoord.x / SCR_WIDTH, gl_FragCoord.y / SCR_HEIGHT);
	float pixelDepth = texture(gNormal, position_tex.xy).r;
	position_tex.z = pixelDepth;

	// View space ray
	vec3 normal_view = texture(gNormal, position_tex.xy).rgb;
	vec4 position_view = invProjection * vec4(position_tex * 2, 1);
	position_view /= position_view.w;
	vec3 reflection_view = normalize(reflect(position_view.xyz - vec3(0), normal_view));
	if (reflection_view.z > 0)
	{
		reflectionColor = vec4(0,0,0,1);
		return ;
	}
	vec3 rayEndPos_view = position_view.xyz + (reflection_view * maxRayDistance);

	// Texture space ray
	vec4 rayEndPos_tex;
	vec3 rayDir_tex;

	ivec2 screenSpaceStartPos = ivec2(position_tex.x * SCR_WIDTH, position_tex.y * SCR_HEIGHT);
	ivec2 screenSpaceEndPos = ivec2(rayEndPos_tex.x * SCR_WIDTH, rayEndPos_tex.y * SCR_HEIGHT);
	ivec2 screenSpaceDistance = screenSpaceEndPos - screenSpaceStartPos;
	int n_marchingStep = max(abs(screenSpaceDistance.x), abs(screenSpaceDistance.y)) / 2;
	rayDir_tex /= max(n_marchingStep, 0.001f);

	// trace the ray
	vec3 outColor = TraceRay(position_tex, rayDir_tex, n_marchingStep);
	reflectionColor = vec4(outColor, 1);
}
