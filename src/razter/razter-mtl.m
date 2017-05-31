//
//  razter-mtl.m
//  razter
//
//  Created by Shahar Sandhaus on 5/23/17.
//
//


#include <razter/razter.h>

#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_NSGL
#include <GLFW/glfw3native.h>
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <QuartzCore/QuartzCore.h>

typedef struct MTLCTX {
	NSWindow* window;
	id<MTLDevice> device;
	CAMetalLayer* metalLayer;
	id<MTLLibrary> library;
	id<MTLCommandQueue> commandQueue;
	id<MTLFunction> vertexProgram;
	id<MTLFunction> fragmentProgram;
	id<MTLRenderPipelineState> pipelineState;
	id<CAMetalDrawable> drawable;
	id<MTLCommandBuffer> cmdBuffer;
	id<MTLRenderCommandEncoder> renderEncoder;
} MTLCTX;

void rzmtlInit(RZRenderContext* ctx) {
	ctx->ctx = (MTLCTX*)malloc(sizeof(MTLCTX));
}

GLFWwindow* rzmtlCreateWindow(RZRenderContext* ctx, int width, int height, const char* title) {
	MTLCTX* mtlCTX = (MTLCTX*)ctx->ctx;
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	
	mtlCTX->window = glfwGetCocoaWindow(window);
	
	mtlCTX->device = MTLCreateSystemDefaultDevice();
	mtlCTX->metalLayer = [CAMetalLayer layer];
	mtlCTX->metalLayer.device = mtlCTX->device;
	mtlCTX->metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	mtlCTX->metalLayer.frame = CGRectMake(0, 0, width, height);
	
	[mtlCTX->metalLayer setPosition:CGPointMake([[mtlCTX->window contentView] frame].size.width/2,
								   [[mtlCTX->window contentView]frame].size.height/2)];
	
	[[mtlCTX->window contentView] setLayer:mtlCTX->metalLayer];
	[[mtlCTX->window contentView] setWantsLayer:YES];
	
	mtlCTX->library = [mtlCTX->device newDefaultLibrary];
	mtlCTX->commandQueue = [mtlCTX->device newCommandQueue];
	
	mtlCTX->vertexProgram = [mtlCTX->library newFunctionWithName:@"vertex_function"];
	mtlCTX->fragmentProgram = [mtlCTX->library newFunctionWithName:@"fragment_function"];
	
	MTLRenderPipelineDescriptor* pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
	[pipelineStateDescriptor setVertexFunction:mtlCTX->vertexProgram];
	[pipelineStateDescriptor setFragmentFunction:mtlCTX->fragmentProgram];
	pipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
	
	mtlCTX->pipelineState = [mtlCTX->device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:nil];
	
	/*
	static float quadVertexData[] =
	{
		0.5, -0.5, 0.0, 1.0,     1.0, 0.0, 0.0, 1.0,
		-0.5, -0.5, 0.0, 1.0,     0.0, 1.0, 0.0, 1.0,
		-0.5,  0.5, 0.0, 1.0,     0.0, 0.0, 1.0, 1.0,
		
		0.5,  0.5, 0.0, 1.0,     1.0, 1.0, 0.0, 1.0,
		0.5, -0.5, 0.0, 1.0,     1.0, 0.0, 0.0, 1.0,
		-0.5,  0.5, 0.0, 1.0,     0.0, 0.0, 1.0, 1.0,
	};
	*/
	
	
	return window;
}

void rzmtlClear(RZRenderContext* ctx) {
	MTLCTX* mtlCTX = (MTLCTX*)ctx->ctx;
	
	mtlCTX->drawable = [mtlCTX->metalLayer nextDrawable];
	
	mtlCTX->cmdBuffer = [mtlCTX->commandQueue commandBuffer];
	
	MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	renderPassDescriptor.colorAttachments[0].texture = mtlCTX->drawable.texture;
	renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
	renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(1, 0, 1, 1);
	renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	
	mtlCTX->renderEncoder = [mtlCTX->cmdBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
}

void rzmtlSetClearColor(RZRenderContext* ctx, float r, float g, float b, float a) {
	
}

void rzmtlSwap(RZRenderContext* ctx) {
	MTLCTX* mtlCTX = (MTLCTX*)ctx->ctx;
	
	[mtlCTX->renderEncoder endEncoding];
	
	[mtlCTX->cmdBuffer presentDrawable:mtlCTX->drawable];
	
	[mtlCTX->cmdBuffer commit];
}

RZBuffer* rzmtlAllocateBuffer(RZRenderContext* ctx, RZBufferCreateInfo* createInfo, void* data, size_t size) {
	RZBuffer* buffer = malloc(sizeof(RZBuffer));
	
	return buffer;
}

void rzmtlUpdateBuffer(RZRenderContext* ctx, RZBuffer* buffer, void* data, size_t size) {
	
}

void rzmtlBindBuffer(RZRenderContext* ctx, RZBuffer* buffer) {
	
}

void rzmtlFreeBuffer(RZRenderContext* ctx, RZBuffer* buffer) {
	
}

void rzmtlLoadPFN(RZRenderContext* ctx) {
	ctx->init = rzmtlInit;
	ctx->createWindow = rzmtlCreateWindow;
	ctx->clear = rzmtlClear;
	ctx->setClearColor = rzmtlSetClearColor;
	ctx->swap = rzmtlSwap;
	
	ctx->allocBuffer = rzmtlAllocateBuffer;
	ctx->updateBuffer = rzmtlUpdateBuffer;
	ctx->bindBuffer = rzmtlBindBuffer;
	ctx->freeBuffer = rzmtlFreeBuffer;
}
