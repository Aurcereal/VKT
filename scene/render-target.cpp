#include "render-target.h"

void RenderTarget::CreateFromTexture(WTexture* colorTex, vk::ImageView colorView, WTexture* depthTex, vk::ImageView depthView) {
	this->colorImg = colorTex->image;
	this->colorView = colorView;
	this->depthImg = depthTex ? depthTex->image : vk::Image();
	this->depthView = depthView;
	this->dim = uvec2(colorTex->width, colorTex->height);
	this->hasDepth = depthTex != nullptr;
	this->layerCount = colorTex->arrayLayerCount;
}

void RenderTarget::CreateFromImage(vk::Image colorImg, vk::ImageView colorView, vk::Image depthImg, vk::ImageView depthView, uvec2 dim) {
	this->colorImg = colorImg;
	this->colorView = colorView;
	this->depthImg = depthImg;
	this->depthView = depthView;
	this->dim = dim;
	this->hasDepth = true;
	this->layerCount = 1;
}