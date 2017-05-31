#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

//typedef struct {
//	float4x4 rotation_matrix;
//} Uniforms;

typedef struct {
	float2 position [[attribute(0)]];
	float3 color [[attribute(1)]];
	float2 uv [[attribute(2)]];
} VertexIn;

typedef struct {
	float4 position [[position]];
	half4  color;
} VertexOut;

vertex VertexOut vertex_function(VertexIn vert [[stage_in]],
								 //constant Uniforms &uniforms [[buffer(1)]],
								 uint vid [[vertex_id]]) {
	VertexOut out;
	out.position = float4(vert.position.xy, 0.0, 1.0);//uniforms.rotation_matrix * vertices[vid].position;
	out.color = half4(vert.color[0], vert.color[1], vert.color[2], 1.0);
	return out;
}

fragment half4 fragment_function(VertexOut in [[stage_in]]) {
	return in.color;
}
