// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/text_utils.h"
 

namespace gfx {

string16 RemoveAcceleratorChar(const string16& s,
                               char16 accelerator_char,
                               int* accelerated_char_pos,
                               int* accelerated_char_span) {
  bool escaped = false;
  ptrdiff_t last_char_pos = -1;
  int last_char_span = 0;    
  string16 accelerator_removed;
  accelerator_removed.reserve(s.size());
  for(size_t i=0; i<s.size(); ++i)
  {
	  if(s[i]!=accelerator_char || escaped)
	  {
		  accelerator_removed.push_back(s[i]);
		  escaped = false;
		  last_char_pos++;
	  }
	  else
	  {
		  escaped = true;
		  last_char_span = last_char_pos;
	  }
  }
  if (accelerated_char_pos)
    *accelerated_char_pos = last_char_pos;
  if (accelerated_char_span)
    *accelerated_char_span = last_char_span;

  return accelerator_removed;
}

}  // namespace gfx
