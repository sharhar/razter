#include <razter/razter.h>

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

typedef struct MTCTX {
	id<MTLDevice> device;
} MTCTX;

typedef struct MTFrameBuffer {
	id<MTLTexture> texture;
	
	float clearR, clearG, clearB, clearA;
} MTFrameBuffer;

typedef struct MTBuffer {
	id<MTLBuffer> buffer;
} MTBuffer;

typedef struct MTShader {
	id<MTLLibrary> vertLibrary;
	id<MTLLibrary> fragLibrary;
	id<MTLFunction> vertexProgram;
	id<MTLFunction> fragmentProgram;
	id<MTLRenderPipelineState> pipelineState;
	RZUniformDescriptor* descriptors;
	uint32_t descriptorCount;
} MTShader;

typedef struct MTUniform {
	void** uniforms;
	uint32_t count;
	RZUniformType* types;
	size_t* sizes;
	uint32_t* indexes;
} MTUniform;

typedef struct MTTexture {
	uint32_t width;
	uint32_t height;
	id<MTLTexture> texture;
	id<MTLSamplerState> sampler;
} MTTexture;

typedef struct MTCommandQueue {
	id<MTLCommandQueue> queue;
} MTCommandQueue;

typedef struct MTCommandBuffer {
	id<MTLCommandBuffer> cmdBuffer;
	id<MTLRenderCommandEncoder> renderEncoder;
	id<MTLBlitCommandEncoder> blitEncoder;
} MTCommandBuffer;

typedef struct MTSwapChain {
	NSWindow* window;
	CAMetalLayer* metalLayer;
	id<CAMetalDrawable> drawable;
	MTCommandQueue* queue;
	MTFrameBuffer* backBuffer;
} MTSwapChain;

void rzmtInit(RZRenderContext* ctx) {
	ctx->ctx = (MTCTX*)malloc(sizeof(MTCTX));
}

void rzmtInitContext(RZRenderContext* ctx, GLFWwindow* window, MTSwapChain** pSwapChain, RZBool debug, uint32_t queueCount, MTCommandQueue*** pQueues) {
	ctx->ctx = malloc(sizeof(MTCTX));
	
	MTCTX* mctx = (MTCTX*)ctx->ctx;
	
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	
	mctx->device = MTLCreateSystemDefaultDevice();
	
	MTSwapChain* swapChain = malloc(sizeof(MTSwapChain));
	
	swapChain->window = glfwGetCocoaWindow(window);
	
	swapChain->metalLayer = [CAMetalLayer layer];
	swapChain->metalLayer.device = mctx->device;
	swapChain->metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	swapChain->metalLayer.frame = CGRectMake(0, 0, width, height);
	swapChain->metalLayer.framebufferOnly = NO;
	
	[swapChain->metalLayer setPosition:CGPointMake([[swapChain->window contentView] frame].size.width/2,
											 [[swapChain->window contentView]frame].size.height/2)];
	
	[[swapChain->window contentView] setLayer:swapChain->metalLayer];
	[[swapChain->window contentView] setWantsLayer:YES];
	
	MTFrameBuffer* backBuffer = malloc(sizeof(MTFrameBuffer));
	
	MTLTextureDescriptor* textureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
																						   width:width
																						  height:height
																					   mipmapped:NO];
	
	textureDesc.usage = MTLTextureUsageRenderTarget;
	
	backBuffer->texture =[mctx->device newTextureWithDescriptor:textureDesc];
	
	swapChain->backBuffer = backBuffer;
	
	MTCommandQueue** queues = malloc(sizeof(MTCommandQueue*) * queueCount);
	
	for(uint32_t i = 0; i < queueCount;i++) {
		queues[i] = malloc(sizeof(MTCommandQueue));
		
		queues[i]->queue = [mctx->device newCommandQueue];
	}
	
	swapChain->queue = queues[0];
	
	id<MTLCommandBuffer> setupCmdBuffer = [queues[0]->queue commandBuffer];
	
	MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	renderPassDescriptor.colorAttachments[0].texture = backBuffer->texture;
	renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
	renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 1);
	renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	
	id<MTLRenderCommandEncoder> renderEncoder = [setupCmdBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
	
	[renderEncoder endEncoding];
	
	[setupCmdBuffer commit];
	
	*pQueues = queues;
	*pSwapChain = swapChain;
}

RZFrameBuffer* rzmtGetBackBuffer(MTSwapChain* swapChain) {
	return swapChain->backBuffer;
}

void rzmtSetClearColor(MTSwapChain* swapChain, float r, float g, float b, float a) {
	swapChain->backBuffer->clearR = r;
	swapChain->backBuffer->clearG = g;
	swapChain->backBuffer->clearB = b;
	swapChain->backBuffer->clearA = a;
}

void rzmtPresent(MTCTX* ctx, MTSwapChain* swapChain) {
	swapChain->drawable = [swapChain->metalLayer nextDrawable];
	id<MTLCommandBuffer> cmdBuffer = [swapChain->queue->queue commandBuffer];
	
	id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer blitCommandEncoder];
	
	MTLOrigin origin = MTLOriginMake(0, 0, 0);
	MTLSize size = MTLSizeMake(800, 600, 1);
	
	[blitEncoder copyFromTexture:swapChain->backBuffer->texture
					 sourceSlice:0
					 sourceLevel:0
					sourceOrigin:origin
					  sourceSize:size
					   toTexture:swapChain->drawable.texture
				destinationSlice:0
				destinationLevel:0
			   destinationOrigin:origin];
	
	[blitEncoder endEncoding];
	
	[cmdBuffer presentDrawable:swapChain->drawable];
	[cmdBuffer commit];
}

RZBuffer* rzmtAllocateBuffer(MTCTX* ctx, MTCommandQueue* queue, RZBufferCreateInfo* createInfo, void* data, size_t size) {
	MTBuffer* buffer = malloc(sizeof(MTBuffer));
	
	buffer->buffer = [ctx->device newBufferWithBytes:data length:size options:MTLResourceOptionCPUCacheModeDefault];
	
	return buffer;
}

void rzmtUpdateBuffer(RZRenderContext* ctx, MTBuffer* buffer, void* data, size_t size) {
	
}

void rzmtBindBuffer(MTCTX* ctx, MTCommandBuffer* cmdBuffer, MTBuffer* buffer) {
	[cmdBuffer->renderEncoder setVertexBuffer:buffer->buffer offset:0 atIndex:0];
}

void rzmtFreeBuffer(MTCTX* ctx, MTBuffer* buffer) {
	
}

RZShader* rzmtCreateShader(MTCTX* ctx, RZShaderCreateInfo* createInfo) {
	MTShader* shader = malloc(sizeof(MTShader));
	
	shader->descriptorCount = createInfo->descriptorCount;
	shader->descriptors = malloc(sizeof(RZUniformDescriptor) * shader->descriptorCount);
	memcpy(shader->descriptors, createInfo->descriptors, sizeof(RZUniformDescriptor) * shader->descriptorCount);
	
	size_t vSize = createInfo->vertSize;
	char* vertSource = createInfo->vertData;
	size_t fSize = createInfo->fragSize;
	char* fragSource = createInfo->fragData;
	
	if(createInfo->isPath == RZ_TRUE) {
		vertSource = rzReadFileFromPath(createInfo->vertData, &vSize);
		fragSource = rzReadFileFromPath(createInfo->fragData, &fSize);
	}
	
	shader->vertLibrary = [ctx->device newLibraryWithSource:@(vertSource) options:nil error:nil];
	shader->vertexProgram = [shader->vertLibrary newFunctionWithName:@(createInfo->vertFunction)];
	
	shader->fragLibrary = [ctx->device newLibraryWithSource:@(fragSource) options:nil error:nil];
	shader->fragmentProgram = [shader->fragLibrary newFunctionWithName:@(createInfo->fragFunction)];
	
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
	
	NSError* error = nil;
	shader->pipelineState = [ctx->device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
	
	if (error != nil) {
		NSLog(@"%@", error);
	}

	return shader;
}

void rzmtBindShader(MTCTX* ctx, MTCommandBuffer* cmdBuffer, MTShader* shader) {
	[cmdBuffer->renderEncoder setRenderPipelineState:shader->pipelineState];
}

void rzmtDestroyShader(MTCTX* ctx, MTShader* shader) {
	
}

void rzmtDraw(MTCTX* ctx, MTCommandBuffer* cmdBuffer, uint32_t firstVertex, uint32_t vertexCount) {
	[cmdBuffer->renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:firstVertex vertexCount:vertexCount];
}


RZUniform* rzmtCreateUniform(MTCTX* ctx, MTShader* shader) {
	MTUniform* uniform = malloc(sizeof(MTUniform));
	
	uniform->count = shader->descriptorCount;
	uniform->uniforms = malloc(sizeof(void*) * uniform->count);
	uniform->types = malloc(sizeof(RZUniformType) * uniform->count);
	uniform->indexes = malloc(sizeof(uint32_t) * uniform->count);
	uniform->sizes = malloc(sizeof(size_t) * uniform->count);
	
	for (uint32_t i = 0; i < uniform->count;i++) {
		uniform->types[i] = shader->descriptors[i].type;
		uniform->indexes[i] = shader->descriptors[i].index;
		
		if (uniform->types[i] == RZ_UNIFORM_TYPE_BUFFER) {
			uniform->uniforms[i] = (void*)[ctx->device newBufferWithLength:shader->descriptors[i].bufferSize options:MTLResourceOptionCPUCacheModeDefault];
			uniform->sizes[i] = shader->descriptors[i].bufferSize;
		}
		
	}
	
	return uniform;
}

void rzmtBindUniform(MTCTX* ctx, MTCommandBuffer* cmdBuffer, MTShader* shader, MTUniform* uniform) {
	for(uint32_t i = 0; i < uniform->count;i++) {
		RZUniformType type = uniform->types[i];
		
		if(type == RZ_UNIFORM_TYPE_BUFFER) {
			id<MTLBuffer> buffer = (id<MTLBuffer>)uniform->uniforms[i];
			[cmdBuffer->renderEncoder setVertexBuffer:buffer offset:0 atIndex:uniform->indexes[i]];
		} else if(type == RZ_UNIFORM_TYPE_SAMPLED_IMAGE) {
			MTTexture* texture = (MTTexture*)uniform->uniforms[i];
			[cmdBuffer->renderEncoder setFragmentTexture:texture->texture atIndex:uniform->indexes[i]];
			[cmdBuffer->renderEncoder setFragmentSamplerState:texture->sampler atIndex:uniform->indexes[i]];
		}
	}
}

void rzmtUniformData(MTCTX* ctx, MTUniform* uniform, uint32_t index, void* data) {
	if(uniform->types[index] == RZ_UNIFORM_TYPE_BUFFER) {
		id<MTLBuffer> buffer = (id<MTLBuffer>)uniform->uniforms[index];
		void* mapped = [buffer contents];
		memcpy(mapped, data, uniform->sizes[index]);
	} else if (uniform->types[index] == RZ_UNIFORM_TYPE_SAMPLED_IMAGE) {
		uniform->uniforms[index] = data;
	}
}

void rzmtDestroyUniform(MTCTX* ctx, RZUniform* uniform) {
	
}

RZTexture* rzmtCreateTexture(MTCTX* ctx, MTCommandQueue* queue, RZTextureCreateInfo* createInfo) {
	MTTexture* texture = malloc(sizeof(MTTexture));
	
	MTLPixelFormat* pixelFormat = MTLPixelFormatRGBA32Float;
	
	if(createInfo->componentType == RZ_COMPONENT_TYPE_FLOAT_32) {
		if(createInfo->componentsPerPixel == 1) {
			pixelFormat = MTLPixelFormatR32Float;
		} else if(createInfo->componentsPerPixel == 2) {
			pixelFormat = MTLPixelFormatRG32Float;
		} else if(createInfo->componentsPerPixel == 4) {
			pixelFormat = MTLPixelFormatRGBA32Float;
		}
	} else if (createInfo->componentType == RZ_COMPONENT_TYPE_INT_32) {
		if(createInfo->componentsPerPixel == 1) {
			pixelFormat = MTLPixelFormatR32Uint;
		} else if(createInfo->componentsPerPixel == 2) {
			pixelFormat = MTLPixelFormatRG32Uint;
		} else if(createInfo->componentsPerPixel == 4) {
			pixelFormat = MTLPixelFormatRGBA32Uint;
		}
	} else if (createInfo->componentType == RZ_COMPONENT_TYPE_INT_8) {
		if(createInfo->componentsPerPixel == 1) {
			pixelFormat = MTLPixelFormatR8Unorm;
		} else if(createInfo->componentsPerPixel == 2) {
			pixelFormat = MTLPixelFormatRG8Unorm;
		} else if(createInfo->componentsPerPixel == 4) {
			pixelFormat = MTLPixelFormatRGBA8Unorm;
		}
	}
	
	MTLTextureDescriptor* textureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat
																						  width:createInfo->width
																						 height:createInfo->height
																					  mipmapped:YES];
	
	texture->texture = [ctx->device newTextureWithDescriptor:textureDesc];
	
	NSUInteger bytesPerRow = createInfo->bytesPerComponent * createInfo->componentsPerPixel * createInfo->width;
	
	MTLRegion region = MTLRegionMake2D(0, 0, createInfo->width, createInfo->height);
	[texture->texture replaceRegion:region mipmapLevel:0 withBytes:createInfo->data bytesPerRow:bytesPerRow];
	
	MTLSamplerDescriptor* samplerDescriptor = [MTLSamplerDescriptor new];
	samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
	samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
	samplerDescriptor.sAddressMode = MTLSamplerAddressModeClampToEdge;
	samplerDescriptor.tAddressMode = MTLSamplerAddressModeClampToEdge;
	
	texture->sampler = [ctx->device newSamplerStateWithDescriptor:samplerDescriptor];
	
	return texture;
}

void rzmtDestroyTexture(MTCTX* ctx, RZTexture* texture) {
	
}

RZCommandBuffer* rzmtCreateCommandBuffer(MTCTX* ctx, MTCommandQueue* queue) {
	MTCommandBuffer* cmdBuffer = malloc(sizeof(MTCommandBuffer));
	return cmdBuffer;
}

void rzmtStartCommandBuffer(MTCTX* ctx, MTCommandQueue* queue, MTCommandBuffer* cmdBuffer) {
	cmdBuffer->cmdBuffer = [queue->queue commandBuffer];
}

void rzmtStartRender(MTCTX* ctx, MTFrameBuffer* frameBuffer, MTCommandBuffer* cmdBuffer) {
	MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	renderPassDescriptor.colorAttachments[0].texture = frameBuffer->texture;
	renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
	renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(frameBuffer->clearR, frameBuffer->clearG, frameBuffer->clearB, frameBuffer->clearA);
	renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	
	cmdBuffer->renderEncoder = [cmdBuffer->cmdBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
}

void rzmtEndRender(MTCTX* ctx, MTFrameBuffer* frameBuffer, MTCommandBuffer* cmdBuffer) {
	[cmdBuffer->renderEncoder endEncoding];
}

void rzmtEndCommandBuffer(MTCTX* ctx, MTCommandBuffer* cmdBuffer) {
	
}

void rzmtExecuteCommandBuffer(MTCTX* ctx, MTCommandQueue* queue, MTCommandBuffer* cmdBuffer) {
	[cmdBuffer->cmdBuffer commit];
	[cmdBuffer->cmdBuffer waitUntilCompleted];
}

void rzmtLoadPFN(RZRenderContext* ctx) {
	ctx->initContext = rzmtInitContext;
	ctx->getBackBuffer = rzmtGetBackBuffer;
	ctx->setClearColor = rzmtSetClearColor;
	ctx->present = rzmtPresent;
	
	ctx->allocBuffer = rzmtAllocateBuffer;
	ctx->updateBuffer = rzmtUpdateBuffer;
	ctx->bindBuffer = rzmtBindBuffer;
	ctx->freeBuffer = rzmtFreeBuffer;
	
	ctx->createShader = rzmtCreateShader;
	ctx->bindShader = rzmtBindShader;
	ctx->destroyShader = rzmtDestroyShader;
	
	ctx->draw = rzmtDraw;
	
	ctx->createUniform = rzmtCreateUniform;
	ctx->bindUniform = rzmtBindUniform;
	ctx->uniformData = rzmtUniformData;
	ctx->destroyUniform = rzmtDestroyUniform;
	
	ctx->createTexture = rzmtCreateTexture;
	ctx->destroyTexture = rzmtDestroyTexture;
	
	ctx->createCommandBuffer = rzmtCreateCommandBuffer;
	ctx->startCommandBuffer = rzmtStartCommandBuffer;
	ctx->startRender = rzmtStartRender;
	ctx->endRender = rzmtEndRender;
	ctx->endCommandBuffer = rzmtEndCommandBuffer;
	ctx->executeCommandBuffer = rzmtExecuteCommandBuffer;
}
