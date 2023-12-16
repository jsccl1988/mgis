#include "rd3d_base.h"

namespace _3Drd {

Color::Color(float red, float green, float blue, float a) {
  fRed = red;
  fGreen = green;
  fBlue = blue;
  fA = a;
}

Color::Color(void) {
  fRed = 0.;
  fGreen = 0.;
  fBlue = 0.;
  fA = 1.;
}

}  // namespace _3Drd
