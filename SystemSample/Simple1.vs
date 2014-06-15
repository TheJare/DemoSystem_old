#include "Simple1.sh"

float4x4 mWorldViewProj;  // World * View * Projection transformation

struct VS_INPUT
{
    float4 pos      : POSITION;
	float4 diffuse  : COLOR0;
	float2 t0       : TEXCOORD0;
};

VS_OUTPUT Simple1( VS_INPUT IN )
{
    VS_OUTPUT OUT;
	
    // Transform the vertex into projection space. 
//    float4 v = { IN.pos.x, IN.pos.y, IN.pos.z, 1 };
//    OUT.pos = mul( v, mWorldViewProj );
    OUT.pos = mul( IN.pos, mWorldViewProj );
    
    OUT.diffuse = IN.diffuse;
    OUT.t0 = IN.t0;
    
    return OUT;
}
