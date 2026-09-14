#pragma once
#include "Globals.h"

#include "Shader.h"



void FramebufferInit();

void HDRenvLoad(float* data, int width, int height);

void CubemapSet();

void HDRConvert(Shader& equirectangularToCubemapShader);

void IrradianceMAP_Create_Init(Shader& irradianceShader);

void PrefilterMAP_Create_Init(Shader& prefilterShader);

void BrdfLUTTextureGenerate(Shader& brdfShader);

void InitPBR(float* data, int width, int height, Shader& equirectangularToCubemapShader, Shader& irradianceShader,
    Shader& prefilterShader, Shader& brdfShader);