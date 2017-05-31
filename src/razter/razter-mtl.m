#include <razter/razter.h>

#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_NSGL
#include <GLFW/glfw3native.h>
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

typedef struct MTCTX {
	NSWindow* window;
	id<MTLDevice> device;
	CAMetalLayer* metalLayer;
	id<MTLCommandQueue> commandQueue;
	id<CAMetalDrawable> drawable;
	id<MTLCommandBuffer> commandBuffer;
	id<MTLRenderCommandEncoder> renderEncoder;
	
	float clearR, clearG, clearB, clearA;
} MTCTX;

typedef struct MTBuffer {
	id<MTLBuffer> buffer;
} MTBuffer;

typedef struct MTShader {
	id<MTLLibrary> library;
	id<MTLFunction> vertexProgram;
	id<MTLFunction> fragmentProgram;
	id<MTLRenderPipelineState> pipelineState;
} MTShader;


void rzmtInit(RZRenderContext* ctx) {
	ctx->ctx = (MTCTX*)malloc(sizeof(MTCTX));
}

GLFWwindow* rzmtCreateWindow(RZRenderContext* ctx, int width, int height, const char* title) {
	MTCTX* mtCTX = (MTCTX*)ctx->ctx;
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	
	mtCTX->window = glfwGetCocoaWindow(window);
	
	mtCTX->device = MTLCreateSystemDefaultDevice();
	mtCTX->metalLayer = [CAMetalLayer layer];
	mtCTX->metalLayer.device = mtCTX->device;
	mtCTX->metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	mtCTX->metalLayer.frame = CGRectMake(0, 0, width, height);
	
	[mtCTX->metalLayer setPosition:CGPointMake([[mtCTX->window contentView] frame].size.width/2,
								   [[mtCTX->window contentView]frame].size.height/2)];
	
	[[mtCTX->window contentView] setLayer:mtCTX->metalLayer];
	[[mtCTX->window contentView] setWantsLayer:YES];
	
	mtCTX->commandQueue = [mtCTX->device newCommandQueue];
	
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

void rzmtClear(RZRenderContext* ctx) {
	MTCTX* mtCTX = (MTCTX*)ctx->ctx;
	
	mtCTX->drawable = [mtCTX->metalLayer nextDrawable];
	
	mtCTX->commandBuffer = [mtCTX->commandQueue commandBuffer];
	
	MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	renderPassDescriptor.colorAttachments[0].texture = mtCTX->drawable.texture;
	renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
	renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(mtCTX->clearR, mtCTX->clearG, mtCTX->clearB, mtCTX->clearA);
	renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	
	mtCTX->renderEncoder = [mtCTX->commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
}

void rzmtSetClearColor(RZRenderContext* ctx, float r, float g, float b, float a) {
	MTCTX* mtCTX = (MTCTX*)ctx->ctx;
	
	mtCTX->clearR = r;
	mtCTX->clearG = g;
	mtCTX->clearB = b;
	mtCTX->clearA = a;
}

void rzmtSwap(RZRenderContext* ctx) {
	MTCTX* mtCTX = (MTCTX*)ctx->ctx;
	
	[mtCTX->renderEncoder endEncoding];
	
	[mtCTX->commandBuffer presentDrawable:mtCTX->drawable];
	
	[mtCTX->commandBuffer commit];
}

RZBuffer* rzmtAllocateBuffer(RZRenderContext* ctx, RZBufferCreateInfo* createInfo, void* data, size_t size) {
	MTCTX* mtCTX = (MTCTX*)ctx->ctx;
	MTBuffer* buffer = malloc(sizeof(MTBuffer));
	
	buffer->buffer = [mtCTX->device newBufferWithBytes:data length:size options:MTLResourceOptionCPUCacheModeDefault];
	
	return buffer;
}

void rzmtUpdateBuffer(RZRenderContext* ctx, RZBuffer* buffer, void* data, size_t size) {
	
}

void rzmtBindBuffer(RZRenderContext* ctx, RZBuffer* buffer) {
	MTCTX* mtCTX = (MTCTX*)ctx->ctx;
	MTBuffer* mtBuffer = (MTBuffer*)buffer;
	
	[mtCTX->renderEncoder setVertexBuffer:mtBuffer->buffer offset:0 atIndex:0];
}

void rzmtFreeBuffer(RZRenderContext* ctx, RZBuffer* buffer) {
	
}

RZShader* rzmtCreateShader(RZRenderContext* ctx, RZShaderCreateInfo* createInfo) {
	MTCTX* mtCTX = (MTCTX*)ctx->ctx;
	
	MTShader* shader = malloc(sizeof(MTShader));
	
	size_t size;
	char* source = rzReadFileFromPath("res/shader.metal", &size);
	
	shader->library = [mtCTX->device newLibraryWithSource:@(source) options:nil error:nil];
	
	shader->vertexProgram = [shader->library newFunctionWithName:@(createInfo->vertData)];
	shader->fragmentProgram = [shader->library newFunctionWithName:@(createInfo->fragData)];
	
	MTLVertexDescriptor* vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
	
	for (uint32_t i = 0; i < createInfo->vertexAttribDesc->count;i++) {
		uint32_t size = createInfo->vertexAttribDesc->sizes[i];
		size_t offset = createInfo->vertexAttribDesc->offsets[i];
		
		if (size == 1) {
			vertexDescriptor.attributes[i].format = MTLVertexFormatFloat;
		} else if (size == 2) {
			vertexDescriptor.attributes[i].format = MTLVertexFormatFloat2;
		} else if (size == 3) {
			vertexDescriptor.attributes[i].format = MTLVertexFormatFloat3;
		} else if (size == 4) {
			vertexDescriptor.attributes[i].format = MTLVertexFormatFloat4;
		}
		
		vertexDescriptor.attributes[i].bufferIndex = 0;
		vertexDescriptor.attributes[i].offset = offset;
	}
 
	vertexDescriptor.layouts[0].stride = createInfo->vertexAttribDesc->stride;
	vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
	
	MTLRenderPipelineDescriptor* pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
	[pipelineStateDescriptor setVertexFunction:shader->vertexProgram];
	[pipelineStateDescriptor setFragmentFunction:shader->fragmentProgram];
	pipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
	[pipelineStateDescriptor setVertexDescriptor:vertexDescriptor];
	
	
	shader->pipelineState = [mtCTX->device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:nil];

	return shader;
}

void rzmtBindShader(RZRenderContext* ctx, RZShader* shader) {
	MTCTX* mtCTX = (MTCTX*)ctx->ctx;
	MTShader* mtShader = (MTShader*)shader;
	
	[mtCTX->renderEncoder setRenderPipelineState:mtShader->pipelineState];
}

void rzmtDestroyShader(RZRenderContext* ctx, RZShader* shader) {
	
}

void rzmtDraw(RZRenderContext* ctx, uint32_t firstVertex, uint32_t vertexCount) {
	MTCTX* mtCTX = (MTCTX*)ctx->ctx;
	
	[mtCTX->renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:firstVertex vertexCount:vertexCount];
}


void rzmtLoadPFN(RZRenderContext* ctx) {
	ctx->init = rzmtInit;
	ctx->createWindow = rzmtCreateWindow;
	ctx->clear = rzmtClear;
	ctx->setClearColor = rzmtSetClearColor;
	ctx->swap = rzmtSwap;
	
	ctx->allocBuffer = rzmtAllocateBuffer;
	ctx->updateBuffer = rzmtUpdateBuffer;
	ctx->bindBuffer = rzmtBindBuffer;
	ctx->freeBuffer = rzmtFreeBuffer;
	
	ctx->createShader = rzmtCreateShader;
	ctx->bindShader = rzmtBindShader;
	ctx->destroyShader = rzmtDestroyShader;
	
	ctx->draw = rzmtDraw;
}
