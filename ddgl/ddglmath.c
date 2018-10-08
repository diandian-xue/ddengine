#define DDGL_CORE

#include "ddglmath.h"

#include <math.h>
#include <memory.h>



DDGLAPI inline float
ddgl_length_vec(ddgl_Vec * v){
    return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}


DDGLAPI inline void
ddgl_normalize_vec(ddgl_Vec * v){
    float mag = v->x * v->x + v->y * v->y + v->z * v->z;
    if (mag > 0.0f) {
        float i = 1.0f / sqrt(mag);
        v->x *= i;
        v->y *= i;
        v->z *= i;
    }
}

DDGLAPI inline void
ddgl_negation_vec(ddgl_Vec * v){
    v->x = -v->x;
    v->y = -v->y;
    v->z = -v->z;
}

DDGLAPI inline void
ddgl_addition_vec(ddgl_Vec * v1, ddgl_Vec * v2){
    v1->x += v2->x;
    v1->y += v2->y;
    v1->z += v2->z;
}

DDGLAPI inline float
ddgl_dot_product_vec(ddgl_Vec * v1, ddgl_Vec * v2){
    return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z + v1->w * v2->w;
}

DDGLAPI inline void
ddgl_cross_product_vec(ddgl_Vec * v1, ddgl_Vec * v2){
    float x = v1->y * v2->z - v1->z * v2->y;
    float y = v1->z * v2->x - v1->x * v2->z;
    float z = v1->x * v2->y - v1->y * v2->x;
    v1->x = x; v1->y = y; v1->z = z;
}

DDGLAPI inline void
ddgl_product_vec_scalar(ddgl_Vec * v, float scalar){
    v->x *= scalar;
    v->y *= scalar;
    v->z *= scalar;
    v->w *= scalar;
}

DDGLAPI inline void
ddgl_product_vec_matrix(ddgl_Vec * v, ddgl_Matrix * m){
    float x = (v->x * m->V.v11) + (v->y * m->V.v21) + (v->z * m->V.v31) + (v->w * m->V.v41);
    float y = (v->x * m->V.v12) + (v->y * m->V.v22) + (v->z * m->V.v32) + (v->w * m->V.v42);
    float z = (v->x * m->V.v13) + (v->y * m->V.v23) + (v->z * m->V.v33) + (v->w * m->V.v43);
    float w = (v->x * m->V.v14) + (v->y * m->V.v24) + (v->z * m->V.v34) + (v->w * m->V.v44);
    v->x = x; v->y = y; v->z = z; v->w = w;
}

DDGLAPI inline void
ddgl_identity_matrix(ddgl_Matrix * m){
    memset(m, 0, sizeof(ddgl_Matrix));
    m->V.v11 = 1.0f;
    m->V.v22 = 1.0f;
    m->V.v33 = 1.0f;
    m->V.v44 = 1.0f;
}

DDGLAPI inline void
ddgl_addition_matrix(ddgl_Matrix * m, float scalar){
    for (int i = 0; i < 16; i ++){
        m->A[i] += scalar;
    }
}

DDGLAPI inline void
ddgl_product_matrix_scalar(ddgl_Matrix * m, float scalar){
    for (int i = 0; i < 16; i ++){
        m->A[i] *= scalar;
    }
}

static inline float
_dot_product_matrix_row(ddgl_Matrix * m, int row, ddgl_Vec * col){
    ddgl_Vec v = { m->A[row], m->A[row + 4], m->A[row + 8],  m->A[row + 12] };
    return ddgl_dot_product_vec(&v, col);
}

static inline void
_product_matrix_row_scalar(ddgl_Matrix * m, int row, float scalar){
    m->A[row] *= scalar;
    m->A[row + 4] *= scalar;
    m->A[row + 8] *= scalar;
    m->A[row + 12] *= scalar;
}

static inline ddgl_Vec
_cross_product_matrix_row(ddgl_Matrix * m, int row, ddgl_Vec * col){
    ddgl_Vec v = { m->A[row], m->A[row + 4], m->A[row + 8],  m->A[row + 12] };
    ddgl_cross_product_vec(&v, col);
    return v;
}

DDGLAPI inline void
ddgl_product_matrix(ddgl_Matrix * m1, ddgl_Matrix * m2){
    /*
     *
     */
    float v1, v2, v3, v4;
    v1 = _dot_product_matrix_row(m1, 0, &(m2->COL[0]));
    v2 = _dot_product_matrix_row(m1, 0, &(m2->COL[1]));
    v3 = _dot_product_matrix_row(m1, 0, &(m2->COL[2]));
    v4 = _dot_product_matrix_row(m1, 0, &(m2->COL[3]));
    m1->V.v11 = v1; m1->V.v21 = v2; m1->V.v31 = v3; m1->V.v41 = v4;

    v1 = _dot_product_matrix_row(m1, 1, &(m2->COL[0]));
    v2 = _dot_product_matrix_row(m1, 1, &(m2->COL[1]));
    v3 = _dot_product_matrix_row(m1, 1, &(m2->COL[2]));
    v4 = _dot_product_matrix_row(m1, 1, &(m2->COL[3]));
    m1->V.v12 = v1; m1->V.v22 = v2; m1->V.v32 = v3; m1->V.v42 = v4;

    v1 = _dot_product_matrix_row(m1, 2, &(m2->COL[0]));
    v2 = _dot_product_matrix_row(m1, 2, &(m2->COL[1]));
    v3 = _dot_product_matrix_row(m1, 2, &(m2->COL[2]));
    v4 = _dot_product_matrix_row(m1, 2, &(m2->COL[3]));
    m1->V.v13 = v1; m1->V.v23 = v2; m1->V.v33 = v3; m1->V.v43 = v4;

    v1 = _dot_product_matrix_row(m1, 3, &(m2->COL[0]));
    v2 = _dot_product_matrix_row(m1, 3, &(m2->COL[1]));
    v3 = _dot_product_matrix_row(m1, 3, &(m2->COL[2]));
    v4 = _dot_product_matrix_row(m1, 3, &(m2->COL[3]));
    m1->V.v14 = v1; m1->V.v24 = v2; m1->V.v34 = v3; m1->V.v44 = v4;
}

DDGLAPI inline void
ddgl_translate_matrix(ddgl_Matrix * m, float x, float y, float z){
    ddgl_Vec * v4 = &(m->COL[3]);
    ddgl_Vec tv = {0};
    if (x != 0.0f){
        tv = m->COL[0];
        ddgl_product_vec_scalar(&tv, x);
        ddgl_addition_vec(v4, &tv);
    }
    if (y != 0.0f){
        tv = m->COL[1];
        ddgl_product_vec_scalar(&tv, y);
        ddgl_addition_vec(v4, &tv);
    }
    if (z != 0.0f){
        tv = m->COL[2];
        ddgl_product_vec_scalar(&tv, z);
        ddgl_addition_vec(v4, &tv);
    }
}

DDGLAPI inline void
ddgl_translate_matrix_v(ddgl_Matrix * m, ddgl_Vec * v){
    ddgl_translate_matrix(m, v->x, v->y, v->z);
}

DDGLAPI inline void
ddgl_scale_matrix(ddgl_Matrix * m, float x, float y, float z){
    if(x != 1.0f){
        _product_matrix_row_scalar(m, 0, x);
    }
    if(y != 1.0f){
        _product_matrix_row_scalar(m, 1, y);
    }
    if(z != 1.0f){
        _product_matrix_row_scalar(m, 2, z);
    }
}

DDGLAPI inline void
ddgl_scale_matrix_v(ddgl_Matrix * m, ddgl_Vec * v){
    ddgl_scale_matrix(m, v->x, v->y, v->z);
}

DDGLAPI inline void
ddgl_rotate_matrix(ddgl_Matrix * m, float x, float y, float z){
}

DDGLAPI inline void
ddgl_rotate_x_matrix(ddgl_Matrix * m, float angle){
    if (angle == 0.0f){
        return;
    }
    float c = cos(angle);
    float s = sin(angle);

    ddgl_Vec cv2 = { 0, c, s, 0};
    ddgl_Vec col2 = {
        _dot_product_matrix_row(m, 0, &cv2),
        _dot_product_matrix_row(m, 1, &cv2),
        _dot_product_matrix_row(m, 2, &cv2),
        _dot_product_matrix_row(m, 3, &cv2),
    };
    ddgl_Vec cv3 = { 0, -s, c, 0};
    ddgl_Vec col3 = {
        _dot_product_matrix_row(m, 0, &cv3),
        _dot_product_matrix_row(m, 1, &cv3),
        _dot_product_matrix_row(m, 2, &cv3),
        _dot_product_matrix_row(m, 3, &cv3),
    };

    m->COL[1] = col2;
    m->COL[2] = col3;
}

DDGLAPI inline void
ddgl_rotate_y_matrix(ddgl_Matrix * m, float y){
    if (y == 0.0f){
        return;
    }
    float c = cos(y);
    float s = sin(y);

    ddgl_Vec cv1 = {c, 0, -s, 0};
    ddgl_Vec col1 = {
        _dot_product_matrix_row(m, 0, &cv1),
        _dot_product_matrix_row(m, 1, &cv1),
        _dot_product_matrix_row(m, 2, &cv1),
        _dot_product_matrix_row(m, 3, &cv1),
    };
    ddgl_Vec cv3 = {s, 0, c, 0};
    ddgl_Vec col3 = {
        _dot_product_matrix_row(m, 0, &cv3),
        _dot_product_matrix_row(m, 1, &cv3),
        _dot_product_matrix_row(m, 2, &cv3),
        _dot_product_matrix_row(m, 3, &cv3),
    };
    m->COL[0] = col1;
    m->COL[2] = col3;
}

DDGLAPI inline void
ddgl_rotate_z_matrix(ddgl_Matrix * m, float z){
    if(z == 0.0f){
        return;
    }
    float c = cos(z);
    float s = sin(z);
    ddgl_Vec cv1 = {c, s, 0, 0};
    ddgl_Vec col1 = {
        _dot_product_matrix_row(m, 0, &cv1),
        _dot_product_matrix_row(m, 1, &cv1),
        _dot_product_matrix_row(m, 2, &cv1),
        _dot_product_matrix_row(m, 3, &cv1),
    };
    ddgl_Vec cv2 = {-s, c, 0, 0};
    ddgl_Vec col2 = {
        _dot_product_matrix_row(m, 0, &cv2),
        _dot_product_matrix_row(m, 1, &cv2),
        _dot_product_matrix_row(m, 2, &cv2),
        _dot_product_matrix_row(m, 3, &cv2),
    };
    m->COL[0] = col1;
    m->COL[1] = col2;
}

DDGLAPI inline void
ddgl_perspective_matrix(ddgl_Matrix * m,
        float fov, float aspect, float near, float far){
    float a = (1.0f / tan(fov * 0.5f));
    m->V.v11 =  a / aspect;
    m->V.v22 = a;
    m->V.v33 = (near + far) / (far - near);
    m->V.v34 = - ((2 * near * far) / (far - near));
    m->V.v44 = 0;
    m->V.v43 = -1;
}



DDGLAPI inline float
ddgl_radians(float degrees){
    return degrees * (DDGL_PI / 180.0f);
}

DDGLAPI inline float
ddgl_degrees(float radians){
    return radians * (180.0f / DDGL_PI);
}
