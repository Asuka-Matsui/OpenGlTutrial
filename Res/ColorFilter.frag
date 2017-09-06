#version 410

layout(location=0) in vec2 inTexCoord;

uniform sampler2D colorSampler;

out vec4 fragColor;

layout(std140) uniform PostEffectData{
	mat4x4 matColor;
}postEffect;

void main(){
	fragColor = texture(colorSampler, inTexCoord);
	//fragColor.rgb = (postEffect.matColor * vec4(fragColor.rgb, 1)).rgb;
}