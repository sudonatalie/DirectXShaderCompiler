// SHADER vertex vtex_shader HLSL
struct VS_OUTPUT {
  float4 pos : SV_POSITION;
  float4 color : COLOR;
};

VS_OUTPUT vsmain(float4 pos : POSITION,
               float4 color : COLOR) {
  VS_OUTPUT vout;
  vout.pos = pos;
  vout.color = color;
  return vout;
}
//END

//SHADER fragment frag_shader HLSL
float4 psmain(float4 color : COLOR) : SV_TARGET {
  return color;
}
//END