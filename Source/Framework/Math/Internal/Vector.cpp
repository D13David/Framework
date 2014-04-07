#include "Vector.h"
#include "Core.h"

#include <cmath>

Vector3::Vector3()
{
}

Vector3::Vector3(float x, float y, float z)
  : x(x), y(y), z(z)
{
}

float& Vector3::operator[](int index)
{
  ASSERT(index < 3, "out of range");
  return (&x)[index];
}

float Vector3::operator[](int index) const
{
  ASSERT(index < 3, "out of range");
  return (&x)[index];
}

Vector2::Vector2()
{
}

Vector2::Vector2(float x, float y)
  : x(x), y(y)
{
}

float& Vector2::operator[](int index)
{
  ASSERT(index < 2, "out of range");
  return (&x)[index];
}

float Vector2::operator[](int index) const
{
  ASSERT(index < 2, "out of range");
  return (&x)[index];
}

Vector3 operator-(const Vector3& v)
{
  return Vector3(-v.x, -v.y, -v.z);
}

Vector3 operator+(const Vector3& a, const Vector3& b)
{
  return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3 operator-(const Vector3& a, const Vector3& b)
{
  return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector3 operator/(const Vector3& v, float s)
{
  s = 1 / s;
  return Vector3(v.x * s, v.y * s, v.z * s);
}

Vector3 operator*(const Vector3& v, float s)
{
  return Vector3(v.x * s, v.y * s, v.z * s);
}

Vector3 abs(const Vector3& v)
{
  return Vector3(abs(v.x), abs(v.y), abs(v.z));
}

float dot(const Vector3& a, const Vector3& b)
{
  return (a.x*b.x + a.y*b.y + a.z*b.z);
}

Vector3 cross(const Vector3& a, const Vector3& b)
{
  return Vector3(
      a.y * b.z - a.z * b.y,
      a.z * b.x - a.x * b.z,
      a.x * b.y - a.y * b.x
    );
}

Vector3 normalize(const Vector3& v)
{
  float len = dot(v, v);
  if (len > 0)
  {
    return v / sqrtf(len);
  }

  return Vector3();
}

float length(const Vector3& v)
{
  float len = dot(v, v);
  if (len > 0)
    return sqrtf(len);

  return 0.0f;
}

float squaredLength(const Vector3& v)
{
  return dot(v, v);
}

bool equals(const Vector3& a, const Vector3& b)
{
  return (fabs(a.x - b.x) < EPSILON && fabs(a.y - b.y) < EPSILON && fabs(a.z - b.z) < EPSILON);
}

bool equals(const Vector2& a, const Vector2& b)
{
  return (fabs(a.x - b.x) < EPSILON && fabs(a.y - b.y) < EPSILON);
}