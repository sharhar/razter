#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef struct {
	matrix_float4x4 view;
	matrix_float4x4 proj;
} Transforms;

typedef struct {
	float2 position [[attribute(0)]];
	float2 uv [[attribute(1)]];
} VertexIn;

typedef struct {
	float4 position [[position]];
	float2 uv;
} VertexOut;

vertex VertexOut vertex_function(VertexIn vert [[stage_in]],
								 constant Transforms &trans [[buffer(1)]],
								 uint vid [[vertex_id]]) {
	VertexOut out;
	out.position = trans.proj * trans.view * float4(vert.position.xy, 0.0, 1.0);
	out.uv = vert.uv;
	return out;
}

fragment half4 fragment_function(VertexOut in [[stage_in]],
								texture2d<float> texture [[texture(0)]],
								sampler samplr [[sampler(0)]]) {
	float4 finalColor = texture.sample(samplr, in.uv);
	
	return half4(finalColor);
}
