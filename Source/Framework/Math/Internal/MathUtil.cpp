#include "Core.h"
#include "MathUtil.h"

void makePerspectiveProjMatrix(Matrix4& mat, float fovy, float aspect, float zNear, float zFar)
{
  const float f = 1.0f / tan(fovy * 0.5f);
  const float scale = 1.0f / (zFar - zNear);

  mat = Matrix4(
    f / aspect, 0,  0,            0,
    0,          f,  0,            0,
    0,          0, zFar * scale, -zNear * zFar * scale,
    0,          0,  1,            0);
}

void makeTranslation(Matrix4& mat, float x, float y, float z)
{
  mat = Matrix4::Identity;
  mat[3] = x;
  mat[7] = y;
  mat[11] = z;
}

void makeLookAt(Matrix4& mat, const Vector3& origin, const Vector3& targetPoint, const Vector3& up)
{
  Vector3 axisZ = normalize(targetPoint - origin);
  Vector3 axisX = normalize(cross(up, axisZ));
  Vector3 axisY = cross(axisZ, axisX);

  mat[0] = axisX.x; mat[1]  = axisX.y; mat[2]  = axisX.z; mat[3]  = -dot(axisX, origin);
  mat[4] = axisY.x; mat[5]  = axisY.y; mat[6]  = axisY.z; mat[7]  = -dot(axisY, origin);
  mat[8] = axisZ.x; mat[9]  = axisZ.y; mat[10] = axisZ.z; mat[11] = -dot(axisZ, origin);
  mat[12] = 0.0f;   mat[13] = 0.0f;   mat[14]  = 0.0f;   mat[15]  = 1.0f;
}

void makeRotateX(Matrix4& mat, float angle)
{
  float c = cos(angle);
  float s = sin(angle);

  mat = Matrix4::Identity;
  mat[5] = c; mat[6] = -s;
  mat[9] = s; mat[10] = c;
}

void makeRotateY(Matrix4& mat, float angle)
{
  float c = cos(angle);
  float s = sin(angle);

  mat = Matrix4::Identity;
  mat[0] = c; mat[2] = s;
  mat[8] = -s; mat[10] = c;
}

void makeRotateZ(Matrix4& mat, float angle)
{
  float c = cos(angle);
  float s = sin(angle);

  mat = Matrix4::Identity;
  mat[0] = c; mat[1] = -s;
  mat[4] = s; mat[5] = c;
}

void setTransform(Matrix4& mat, float x, float y, float z)
{
  mat[3] = x;
  mat[7] = y;
  mat[11] = z;
}