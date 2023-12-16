#ifndef _GL_TEXT_H
#define _GL_TEXT_H

#include "gl_prerequisites.h"

namespace _3Drd {
class GLText {
 public:
  GLText();
  virtual ~GLText(void);

  // create font
  long CreateFont(HDC hDC, const char *chType, int nHeight, int nWidth,
                  int nWeight, bool bItalic, bool bUnderline, bool bStrike,
                  ulong ulSize);
  HFONT GetFont();
  // draw text
  HRESULT DrawText(HDC hDC, float xpos, float ypos, float zpos,
                   const char *);  // 3d pos
  HRESULT DrawText(HDC hDC, float xscreen, float yscreen,
                   const char *);  // screen pos

 protected:
  GLuint m_TextTb[65536];
  HFONT m_hFont;
  int m_nHeight, m_nWidth, m_nWeight;
};
}  // namespace _3Drd
#endif  //_GL_TEXT_H