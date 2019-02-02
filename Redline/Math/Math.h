#pragma once
#define _USE_MATH_DEFINES
#include <cmath>

#define DEG_TO_RAD ((float)M_PI / 180.0f)
#define RAD_TO_DEG (180.0f / (float)M_PI)

#define M_PIF ((float)M_PI)

#include "mathfu/glsl_mappings.h"

float Saturate(float val);

mathfu::vec3 Reflect(const mathfu::vec3& surfaceToViewer, const mathfu::vec3& surfaceNormal);

//Given a start and a end and a target position, return the 'time' value that lerp would use
//to produce value val between a and b
float UnLerp(float a, float b, float val);

#define PERCEIVED_BRIGHTNESS_RED 0.299f
#define PERCEIVED_BRIGHTNESS_GREEN 0.587f
#define PERCEIVED_BRIGHTNESS_BLUE 0.114f

float CalculateLuminance(const mathfu::vec4& color);

//Extract the pitch, yaw and roll of a quaternion. returns them in that order in the vec3
mathfu::vec3 ExtractPitchYawRoll(const mathfu::quat& rotation);

//Decompose a 4x4 transform matrix into it's consituant components.
//The rotation and scale that are extracted may not be the same values that
//where used to create the matrix, but do represent the same transform
void DecomposeMatrix(const mathfu::mat4& matrix,
	mathfu::vec3& outPosition,
	mathfu::quat& outRotation,
	mathfu::vec3& outScale);