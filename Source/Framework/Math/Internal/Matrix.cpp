#include "Core.h"
#include "Matrix.h"
#include "Vector.h"

Matrix4 Matrix4::Zero = Matrix4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
Matrix4 Matrix4::Identity = Matrix4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

Matrix4::Matrix4()
{
}

Matrix4::Matrix4(float m00, float m01, float m02, float m03,
  float m10, float m11, float m12, float m13,
  float m20, float m21, float m22, float m23,
  float m30, float m31, float m32, float m33)
{
  m[0] = m00; m[1] = m01; m[2] = m02; m[3] = m03;
  m[4] = m10; m[5] = m11; m[6] = m12; m[7] = m13;
  m[8] = m20; m[9] = m21; m[10] = m22; m[11] = m23;
  m[12] = m30; m[13] = m31; m[14] = m32; m[15] = m33;
}

Matrix4::Matrix4(const float* ptr)
{
  for (int i = 0; i < 16; ++i)
    m[i] = ptr[i];
}

Matrix4 operator*(const Matrix4 a, const Matrix4& b)
{
  Matrix4 mat;

  /*
    a0  a1  a2  a3        b0  b1  b2  b3
    a4  a5  a6  a7        b4  b5  b6  b7
    a8  a9  a10 a11       b8  b9  b10 b11
    a12 a13 a14 a15       b12 b13 b14 b15
  */

  mat[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12];
  mat[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13];
  mat[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14];
  mat[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15];

  mat[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12];
  mat[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13];
  mat[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14];
  mat[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15];

  mat[8] = a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12];
  mat[9] = a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13];
  mat[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14];
  mat[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15];

  mat[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12];
  mat[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13];
  mat[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
  mat[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];

  return mat;
}

Vector3 operator*(const Matrix4 m, const Vector3& v)
{
  Vector3 vec;
  vec.x = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3];
  vec.y = m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7];
  vec.z = m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11];
  return vec;
}