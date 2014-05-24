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

typedef struct _vertex
{
    vec3    position;
    vec3    normal;
    vec3    tangent;
    vec4    color;
    vec3    texcoord;
} vertex;


#endif
