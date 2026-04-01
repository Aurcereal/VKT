#pragma once

#include "defines.h"

#include "scene/texture.h"

class RenderTarget {
public:
	// no, delete: void CreateFromImages(vk::raii::Image* colorImg, vk::raii::ImageView* colorView, vk::raii::Image* depthImg, vk::raii::ImageView* depthView);
	void CreateFromTexture(WTexture* colorTex, vk::ImageView colorView, WTexture* depthTex, vk::ImageView depthView);
	void CreateFromImage(vk::Image colorImg, vk::ImageView colorView, vk::Image depthImg, vk::ImageView depthView, uvec2 dim);
	// void Create(uvec2 dim); will create textures first, for deferred rendering, but then we're actually OWNING the image so we'll need to create a texture (uPtr Texture = nullptr)
private:
	friend class WRenderPass;
	
	vk::Image colorImg;
	vk::ImageView colorView;

	vk::Image depthImg;
	vk::ImageView depthView;
	bool hasDepth;

	uint32_t layerCount;
	uvec2 dim;
	
};