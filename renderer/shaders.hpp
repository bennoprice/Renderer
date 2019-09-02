#pragma once

constexpr char shader[] = R"(
	struct VSOut
	{
		float4 position : SV_POSITION;
		float4 colour : COLOUR;
	};

	VSOut VS(float4 position : POSITION, float4 colour : COLOUR)
	{
		VSOut output;
		output.position = position;
		output.colour = colour;
		return output;
	}

	float4 PS(float4 position : SV_POSITION, float4 colour : COLOUR) : SV_TARGET
	{
		return colour;
	};
)";