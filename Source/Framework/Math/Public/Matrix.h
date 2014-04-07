#ifndef __Matrix_h_
#define __Matrix_h_

class Vector3;

class Matrix4
{
public:
  Matrix4();
  Matrix4(float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33);
  Matrix4(const float* ptr);

  float& operator[](int index) { return m[index]; }
  float operator[](int index) const { return m[index]; }
  operator float*() { return m; }
  const float* getPtr() const { return m; }

  static Matrix4 Zero;
  static Matrix4 Identity;

private:
  union
  {
    float m[16];
    float m_[4][4];
  };
};

Matrix4 operator*(const Matrix4 a, const Matrix4& b);
Vector3 operator*(const Matrix4 m, const Vector3& v);

#endif // __Matrix_h_