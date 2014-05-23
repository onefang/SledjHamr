#ifndef _SCENRI_H_
#define _SCENRI_H_

#include "love.h"

typedef struct _cameraMove
{
    float	x, y, z;
    float	r, s, p;	// Roll, spin (not yaw coz y is used already), and pitch.
    float	jump;
    float	JumpSpeed, RotateSpeed, MoveSpeed;
} cameraMove;

typedef struct _vec4
{
    float   x;
    float   y;
    float   z;
    float   w;
} vec4;

typedef struct _vec3
{
    float   x;
    float   y;
    float   z;
} vec3;

typedef struct _vec2
{
    float   x;
    float   y;
} vec2;

typedef struct _vertex
{
    vec3    position;
    vec3    normal;
    vec3    tangent;
    vec4    color;
    vec3    texcoord;
} vertex;


#endif
