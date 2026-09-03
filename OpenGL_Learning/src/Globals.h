#pragma once
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include "camera_ver2.h"
#include "Spaceship.h"

// ===== 常量 =====
inline constexpr unsigned int SCR_WIDTH = 960;
inline constexpr unsigned int SCR_HEIGHT = 600;
inline constexpr unsigned int SHADOW_WIDTH = 4096;
inline constexpr unsigned int SHADOW_HEIGHT = 4096;
inline constexpr float		  PI = 3.14159265359f;

// ===== 窗口 =====
inline int windowwidth = SCR_WIDTH;
inline int windowheight = SCR_HEIGHT;


// ===== 相机 =====
inline Camera_ver2 camera(glm::vec3(-40.0f, 10.0f, 200.0f));
inline float lastX = SCR_WIDTH / 2.0f;
inline float lastY = SCR_HEIGHT / 2.0f;
inline bool  firstMouse = true;


// ===== 输入状态 / 开关 =====
inline bool cursorLocked = true;
inline bool tabKeyPressed = false;   // 用于检测 TAB 键的上升沿
inline bool f10Pressed = false;    // 用于检测 F10 键的上升沿
inline bool f11Pressed = false;    // 用于检测 F11 键的上升沿
inline bool isFullscreen = false; // 是否全屏
inline bool shadows = true;
inline bool PCSS = false;
inline bool ssaoEnabled = true;
inline bool unify = true;


inline bool shadowKeyPressed = false;
inline bool PCSSKeyPressed = false;
inline bool ssaoKeyPressed = false;
inline bool unifyKeyPressed = false;

inline int savedX = 0, savedY = 0;
inline int savedWidth = SCR_WIDTH, savedHeight = SCR_HEIGHT;


// ===== 时间 =====
inline float deltaTime = 0.0f;
inline float lastFrame = 0.0f;


// ===== 帧缓冲 =====
inline unsigned int hdrFBO = 0, hdrColorBuffer = 0, hdrDepthRBO = 0;
inline unsigned int pingpongFBO[2] = { 0, 0 };
inline unsigned int pingpongColorbuffers[2] = { 0, 0 };
	/*G - buffer*/ 
inline unsigned int gBuffer = 0;
inline unsigned int	gPosition = 0;
inline unsigned int	gNormal = 0;
inline unsigned int	gAlbedo = 0;
inline unsigned int	gPBR = 0;
inline unsigned int gDepthRBO = 0;

// ===== 阴影 =====
inline unsigned int depthCubeMap = 0, depthCubeFBO = 0;
inline float shadow_near = 1.0f;
inline float shadow_far = 1000.0f;
inline unsigned int shadowColorMap = 0;

// ===== SSAO =====
inline unsigned int ssaoFBO = 0, ssaoBlurFBO = 0;
inline unsigned int ssaoColorBuffer = 0, ssaoColorBufferBlur = 0;
inline std::vector<glm::vec3> ssaoKernel;
inline unsigned int noiseTexture = 0;

// ===== ship / view mode =====
inline Spaceship ship;
inline ViewMode currentMode = MODE_FREE;

// ===== third-person follow params =====
inline float followDistance = 80.0f;	// camera distance behind the ship
inline float followHeight = 25.0f;		// camera height above the ship
inline float followSmooth = 2.0f;		// follow damping (higher = snappier, lower = softer)，10质感偏硬，换5有滞后感
inline float orbitYaw = 0.0f;    // mode-3 look-around: horizontal orbit angle
inline float orbitPitch = 0.0f;  // mode-3 look-around: vertical orbit angle

// ===== 尘埃星环 =====
inline unsigned int ringVAO = 0, ringVBO = 0, ringEBO = 0;
inline unsigned int ringIndexCount = 0;
inline float ringInner = 150.0f;   // 内半径（占位，待调）
inline float ringOuter = 260.0f;   // 外半径（占位，待调）
inline float ringThickness = 15.0f;   // 环的厚度（±15 单位，很扁但可见）