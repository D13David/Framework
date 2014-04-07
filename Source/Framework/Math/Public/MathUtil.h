#ifndef __MathUtil_h_
#define __MathUtil_h_

class Matrix4;
class Vector3;

#undef max

static const float PI = 3.14159265f;
static const float HALF_PI = PI * 0.5f;
static const float TWO_PI = PI * 2.0f;
static const float DEG2RAD = PI / 180.0f;
static const float RAD2DEG = 180.0f / PI;
static const float EPSILON = 1e-6f;

void makePerspectiveProjMatrix(Matrix4& mat, float fovy, float aspect, float zn, float zf);
void makeTranslation(Matrix4& mat, float x, float y, float z);
void makeLookAt(Matrix4& mat, const Vector3& origin, const Vector3& targetPoint, const Vector3& up);

void makeRotateX(Matrix4& mat, float angle);
void makeRotateY(Matrix4& mat, float angle);
void makeRotateZ(Matrix4& mat, float angle);

void setTransform(Matrix4& mat, float x, float y, float z);

template<typename T>
inline T max(const T& a, const T& b)
{
  return (a > b) ? a : b;
}

template<typename T>
inline T clamp(const T& value, const T& min, const T& max)
{
  return value < min ? min : 
    value > max ? max : value;
}

template<typename T>
inline T clampAngle360(const T& value)
{
  if (value > 360.0f)
    return value - 360.0f;

  return value;
}

template<typename T>
inline T clampAngle180(const T& value)
{
  if (value > 180.0f)
    return value - 180;

  return value;
}


inline unsigned short convertFloatToHalf(float p_Value)
{
  unsigned int Result;

  unsigned int IValue = ((unsigned int *)(&p_Value))[0];
  unsigned int Sign = (IValue & 0x80000000U) >> 16U;
  IValue = IValue & 0x7FFFFFFFU;

  if (IValue > 0x47FFEFFFU)
  {
    Result = 0x7FFFU;
  }
  else
  {
    if (IValue < 0x38800000U)
    {
      unsigned int Shift = 113U - (IValue >> 23U);
      IValue = (0x800000U | (IValue & 0x7FFFFFU)) >> Shift;
    }
    else
    {
      IValue += 0xC8000000U;
    }

    Result = ((IValue + 0x0FFFU + ((IValue >> 13U) & 1U)) >> 13U)&0x7FFFU; 
  }
  return (unsigned short)(Result|Sign);
}

inline float CryConvertHalfToFloat(unsigned short Value)
{
  unsigned int Mantissa;
  unsigned int Exponent;
  unsigned int Result;

  Mantissa = (unsigned int)(Value & 0x03FF);

  if ((Value & 0x7C00) != 0)  // The value is normalized
  {
    Exponent = (unsigned int)((Value >> 10) & 0x1F);
  }
  else if (Mantissa != 0)     // The value is denormalized
  {
    // Normalize the value in the resulting float
    Exponent = 1;

    do
    {
      Exponent--;
      Mantissa <<= 1;
    } while ((Mantissa & 0x0400) == 0);

    Mantissa &= 0x03FF;
  }
  else                        // The value is zero
  {
    Exponent = (unsigned int)-112;
  }

  Result = ((Value & 0x8000) << 16) | // Sign
    ((Exponent + 112) << 23) | // Exponent
    (Mantissa << 13);          // Mantissa

  return *(float*)&Result;
}

#endif // __MathUtil_h_