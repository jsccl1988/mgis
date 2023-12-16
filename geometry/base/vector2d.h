
#ifndef GEOMETRY_BASE_VECTOR2D_H
#define GEOMETRY_BASE_VECTOR2D_H

#include "core/core.h"
#include "geometry/export.h"

namespace core {
class Vector2D {
 public:
  union {
    struct {
      float x, y;
    };
    float v[2];
  };

 public:
  // construct
  Vector2D(void);
  Vector2D(float x, float y);

  // function
  inline void Set(float x, float y);
  inline float GetLength(void);           // length
  inline float GetSqrLength(void) const;  // square length
  inline void Negate(void);               // Vector4 mult -1
  inline void Normalize(void);            // normalize
  inline float AngleWith(Vector2D &v);    // angle in rad
  inline void Difference(const Vector2D &v1,
                         const Vector2D &v2);       // from v1 to v2
  void Rotate(const Vector2D &vAxis, float theta);  // rotate by an axis
  inline double CrossProduct(const Vector2D &v1);   // cross product

  void operator+=(const Vector2D &v);           // operator +=
  void operator-=(const Vector2D &v);           // operator -=
  void operator*=(float f);                     // scale Vector4
  void operator/=(float f);                     // scale down
  void operator+=(float f);                     // add scalar
  void operator-=(float f);                     // subtract scalar
  float operator*(const Vector2D &v) const;     // dot product
  Vector2D operator*(float f) const;            // scale Vector4
  Vector2D operator/(float f) const;            // scalar divide
  Vector2D operator+(float f) const;            // add scalar
  Vector2D operator-(float f) const;            // scale down
  Vector2D operator+(const Vector2D &v) const;  // addition
  Vector2D operator-(const Vector2D &v) const;  // subtraction
};

// construct
Vector2D::Vector2D(void) { x = 0, y = 0; }

Vector2D::Vector2D(float _x, float _y) {
  x = _x;
  y = _y;
}

void Vector2D::Set(float _x, float _y) {
  x = _x;
  y = _y;
}

// function to be called
inline float Vector2D::GetLength(void) {
  float f = 0;
  f = (float)sqrt(x * x + y * y);
  return f;
}

inline float Vector2D::GetSqrLength(void) const { return (x * x + y * y); }

inline void Vector2D::Negate(void) {
  x = -x;
  y = -y;
  ;
}

inline float Vector2D::AngleWith(Vector2D &v) {
  return (float)acos(((*this) * v) / (this->GetLength() * v.GetLength()));
}

inline void Vector2D::Normalize(void) {
  float f = (float)sqrt(x * x + y * y);

  if (f != 0.0f) {
    x /= f;
    y /= f;
    ;
  }
}

void Vector2D::Rotate(const Vector2D &vAxis, float theta) {
  // Calculate the sine and cosine of the angle once
  float cosTheta = (float)cos(theta);
  float sinTheta = (float)sin(theta);

  float _x = vAxis.x;
  float _y = vAxis.y;

  Vector2D vNew;
  // Find the new x position for the new rotated point
  vNew.x = (cosTheta + (1 - cosTheta) * _x * _x) * x;
  vNew.x += ((1 - cosTheta) * _x * _y) * y;

  // Find the new y position for the new rotated point
  vNew.y = ((1 - cosTheta) * _x * _y) * x;
  vNew.y += (cosTheta + (1 - cosTheta) * _y * _y) * y;

  *this = vNew;
}

inline double Vector2D::CrossProduct(const Vector2D &v1)  // cross product
{
  return x * v1.y - y * v1.x;
}

/**
 * Get vector from v1 to v2.
 */
inline void Vector2D::Difference(const Vector2D &v1, const Vector2D &v2) {
  x = v2.x - v1.x;
  y = v2.y - v1.y;
}

void Vector2D::operator+=(const Vector2D &v) {
  x += v.x;
  y += v.y;
}

Vector2D Vector2D::operator+(const Vector2D &v) const {
  return Vector2D(x + v.x, y + v.y);
}

void Vector2D::operator-=(const Vector2D &v) {
  x -= v.x;
  y -= v.y;
}

Vector2D Vector2D::operator-(const Vector2D &v) const {
  return Vector2D(x - v.x, y - v.y);
}

void Vector2D::operator*=(float f) {
  x *= f;
  y *= f;
}

void Vector2D::operator/=(float f) {
  x /= f;
  y /= f;
}

Vector2D Vector2D::operator*(float f) const { return Vector2D(x * f, y * f); }

Vector2D Vector2D::operator/(float f) const { return Vector2D(x / f, y / f); }

void Vector2D::operator+=(float f) {
  x += f;
  y += f;
}

void Vector2D::operator-=(float f) {
  x -= f;
  y -= f;
}

Vector2D Vector2D::operator+(float f) const { return Vector2D(x + f, y + f); }
Vector2D Vector2D::operator-(float f) const { return Vector2D(x - f, y - f); }
float Vector2D::operator*(const Vector2D &v) const {
  return (v.x * x + v.y * y);
}

}  // namespace core

#endif  // GEOMETRY_BASE_VECTOR2D_H