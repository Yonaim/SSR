#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D colorTexture;
uniform sampler2D refTexture; // reflection
uniform sampler2D specularTexture;

void main()
{
	vec3 baseColor = texture(colorTexture, texCoord).rgb;
	vec3 Ks = texture(specularTexture, texCoord).rgb; 
	vec3 finalColor = baseColor + (Ks * texture(refTexture, texCoord).rgb);
	FragColor = vec4(finalColor, 1);
}
