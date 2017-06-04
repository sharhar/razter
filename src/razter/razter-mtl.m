#include <razter/razter.h>

#define GLFW_EXPOSE_NATIVE_COCOA
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

void rzmtInit(RZRenderContext* ctx) {
	ctx->ctx = (MTCTX*)malloc(sizeof(MTCTX));
}

GLFWwindow* rzmtCreateWindow(MTCTX* ctx, int width, int height, const char* title) {
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	
	ctx->window = glfwGetCocoaWindow(window);
	
	ctx->device = MTLCreateSystemDefaultDevice();
	ctx->metalLayer = [CAMetalLayer layer];
	ctx->metalLayer.device = ctx->device;
	ctx->metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	ctx->metalLayer.frame = CGRectMake(0, 0, width, height);
	
	[ctx->metalLayer setPosition:CGPointMake([[ctx->window contentView] frame].size.width/2,
											 [[ctx->window contentView]frame].size.height/2)];
	
	[[ctx->window contentView] setLayer:ctx->metalLayer];
	[[ctx->window contentView] setWantsLayer:YES];
	
	ctx->commandQueue = [ctx->device newCommandQueue];
	
	return window;
}

void rzmtClear(MTCTX* ctx) {
	ctx->drawable = [ctx->metalLayer nextDrawable];
	
	ctx->commandBuffer = [ctx->commandQueue commandBuffer];
	
	MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	renderPassDescriptor.colorAttachments[0].texture = ctx->drawable.texture;
	renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
	renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(ctx->clearR, ctx->clearG, ctx->clearB, ctx->clearA);
	renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	
	ctx->renderEncoder = [ctx->commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
}

void rzmtSetClearColor(MTCTX* ctx, float r, float g, float b, float a) {
	ctx->clearR = r;
	ctx->clearG = g;
	ctx->clearB = b;
	ctx->clearA = a;
}

void rzmtSwap(MTCTX* ctx) {
	[ctx->renderEncoder endEncoding];
	
	[ctx->commandBuffer presentDrawable:ctx->drawable];
	
	[ctx->commandBuffer commit];
}

RZBuffer* rzmtAllocateBuffer(MTCTX* ctx, RZBufferCreateInfo* createInfo, void* data, size_t size) {
	MTBuffer* buffer = malloc(sizeof(MTBuffer));
	
	buffer->buffer = [ctx->device newBufferWithBytes:data length:size options:MTLResourceOptionCPUCacheModeDefault];
	
	return buffer;
}

void rzmtUpdateBuffer(RZRenderContext* ctx, MTBuffer* buffer, void* data, size_t size) {
	
}

void rzmtBindBuffer(MTCTX* ctx, MTBuffer* buffer) {
	[ctx->renderEncoder setVertexBuffer:buffer->buffer offset:0 atIndex:0];
}

void rzmtFreeBuffer(MTCTX* ctx, MTBuffer* buffer) {
	
}

RZShader* rzmtCreateShader(MTCTX* ctx, RZShaderCreateInfo* createInfo) {
	MTShader* shader = malloc(sizeof(MTShader));
	
	shader->descriptorCount = createInfo->descriptorCount;
	shader->descriptors = malloc(sizeof(RZUniformDescriptor) * shader->descriptorCount);
	memcpy(shader->descriptors, createInfo->descriptors, sizeof(RZUniformDescriptor) * shader->descriptorCount);
	
	size_t size;
	char* source = rzReadFileFromPath("res/shader.metal", &size);
	
	shader->library = [ctx->device newLibraryWithSource:@(source) options:nil error:nil];
	
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
	
	NSError* error = nil;
	shader->pipelineState = [ctx->device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
	
	if (error != nil) {
		NSLog(@"%@", error);
	}

	return shader;
}

void rzmtBindShader(MTCTX* ctx, MTShader* shader) {
	[ctx->renderEncoder setRenderPipelineState:shader->pipelineState];
}

void rzmtDestroyShader(MTCTX* ctx, MTShader* shader) {
	
}

void rzmtDraw(MTCTX* ctx, uint32_t firstVertex, uint32_t vertexCount) {
	[ctx->renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:firstVertex vertexCount:vertexCount];
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

void rzmtBindUniform(MTCTX* ctx, MTShader* shader, MTUniform* uniform) {
	for(uint32_t i = 0; i < uniform->count;i++) {
		RZUniformType type = uniform->types[i];
		
		if(type == RZ_UNIFORM_TYPE_BUFFER) {
			id<MTLBuffer> buffer = (id<MTLBuffer>)uniform->uniforms[i];
			[ctx->renderEncoder setVertexBuffer:buffer offset:0 atIndex:uniform->indexes[i]];
		} else if(type == RZ_UNIFORM_TYPE_SAMPLED_IMAGE) {
			MTTexture* texture = (MTTexture*)uniform->uniforms[i];
			[ctx->renderEncoder setFragmentTexture:texture->texture atIndex:uniform->indexes[i]];
			[ctx->renderEncoder setFragmentSamplerState:texture->sampler atIndex:uniform->indexes[i]];
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

RZTexture* rzmtCreateTexture(MTCTX* ctx, RZTextureCreateInfo* createInfo) {
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
	
	ctx->createUniform = rzmtCreateUniform;
	ctx->bindUniform = rzmtBindUniform;
	ctx->uniformData = rzmtUniformData;
	ctx->destroyUniform = rzmtDestroyUniform;
	
	ctx->createTexture = rzmtCreateTexture;
	ctx->destroyTexture = rzmtDestroyTexture;
}
