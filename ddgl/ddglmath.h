#pragma once

#include "ddglconfig.h"

#define DDGL_PI 3.1415926535898f

typedef struct tag_ddgl_Size{
	float width, height, depth;
}ddgl_Size;

typedef struct tag_ddgl_Color{
    unsigned char r, g, b, a;
}ddgl_Color;

typedef struct tag_ddgl_UV{
    float u, v;
}ddgl_UV;

typedef struct tag_ddgl_Vec{
    float x, y, z, w;
}ddgl_Vec;

typedef struct tag_ddgl_Vertex{
    ddgl_Vec position;
    ddgl_Color color;
    ddgl_UV uv;
}ddgl_Vertex;

typedef union tag_ddgl_Matrix{
    float A[16];
    struct {
        float
            v11, v12, v13, v14,
            v21, v22, v23, v24,
            v31, v32, v33, v34,
            v41, v42, v43, v44;
    } V;
    ddgl_Vec COL[4];
}ddgl_Matrix;

DDGLAPI inline float
ddgl_length_vec(ddgl_Vec * v);

DDGLAPI inline void
ddgl_normalize_vec(ddgl_Vec * v);

DDGLAPI inline void
ddgl_negation_vec(ddgl_Vec * v);

DDGLAPI inline void
ddgl_addition_vec(ddgl_Vec * v1, ddgl_Vec * v2);

DDGLAPI inline float
ddgl_dot_product_vec(ddgl_Vec * v1, ddgl_Vec * v2);

DDGLAPI inline void
ddgl_cross_product_vec(ddgl_Vec * v1, ddgl_Vec * v2);

DDGLAPI inline void
ddgl_product_vec_scalar(ddgl_Vec * v, float scalar);

DDGLAPI inline void
ddgl_product_vec_matrix(ddgl_Vec * v, ddgl_Matrix * m);



DDGLAPI inline void
ddgl_identity_matrix(ddgl_Matrix * m);

DDGLAPI inline void
ddgl_addition_matrix(ddgl_Matrix * m1, float scalar);

DDGLAPI inline void
ddgl_product_matrix_scalar(ddgl_Matrix * m, float scalar);

DDGLAPI inline void
ddgl_product_matrix(ddgl_Matrix * m1, ddgl_Matrix * m2);

DDGLAPI inline void
ddgl_translate_matrix(ddgl_Matrix * m, float x, float y, float z);

DDGLAPI inline void
ddgl_translate_matrix_v(ddgl_Matrix * m, ddgl_Vec * v);

DDGLAPI inline void
ddgl_scale_matrix(ddgl_Matrix * m, float x, float y, float z);

DDGLAPI inline void
ddgl_scale_matrix_v(ddgl_Matrix * m, ddgl_Vec * v);

DDGLAPI inline void
ddgl_rotate_matrix(ddgl_Matrix * m, float x, float y, float z);

DDGLAPI inline void
ddgl_rotate_x_matrix(ddgl_Matrix * m, float x);

DDGLAPI inline void
ddgl_rotate_y_matrix(ddgl_Matrix * m, float y);

DDGLAPI inline void
ddgl_rotate_z_matrix(ddgl_Matrix * m, float z);

DDGLAPI inline void
ddgl_perspective_matrix(ddgl_Matrix * m,
        float fov, float aspect, float near, float far);



DDGLAPI inline float
ddgl_radians(float degrees);

DDGLAPI inline float
ddgl_degrees(float radians);
