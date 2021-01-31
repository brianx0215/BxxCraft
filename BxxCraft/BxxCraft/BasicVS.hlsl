struct PSInput {
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

PSInput main(
	float4 pos : POSITION,
	float4 color : COLOR) {
	PSInput result;
	result.pos = pos;
	result.color = color;
	return result;
}