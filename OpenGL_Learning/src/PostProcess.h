#pragma once
#include "Globals.h"
#include "Shader.h"

void PostProcessing(Shader& brightPassShader, Shader& blurShader, Shader& compositeShader, const unsigned int& quadVAO);

