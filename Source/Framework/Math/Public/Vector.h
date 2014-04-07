#ifndef __Vector_h_
#define __Vector_h_

class Vector3
{
public:
  Vector3();
  Vector3(float x, float y, float z);

  float& operator[](int index);
  float operator[](int index) const;

  float x, y, z;
};

class Vector2
{
public:
  Vector2();
  Vector2(float x, float y);

  float& operator[](int index);
  float operator[](int index) const;

  float x, y;
};

Vector3 operator-(const Vector3& v);
Vector3 operator+(const Vector3& a, const Vector3& b);
Vector3 operator-(const Vector3& a, const Vector3& b);
Vector3 operator*(const Vector3& v, float s);
Vector3 operator/(const Vector3& v, float s);

Vector3 abs(const Vector3& v);
float dot(const Vector3& a, const Vector3& b);
Vector3 cross(const Vector3& a, const Vector3& b);
Vector3 normalize(const Vector3& v);
float length(const Vector3& v);
float squaredLength(const Vector3& v);

bool equals(const Vector3& a, const Vector3& b);
bool equals(const Vector2& a, const Vector2& b);

#endif // __Vector_h_