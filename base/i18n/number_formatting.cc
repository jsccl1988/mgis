// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/i18n/number_formatting.h"

#include "base/format_macros.h"
#include "base/logging.h"
#include "base/lazy_instance.h"
#include "base/memory/scoped_ptr.h"
#include "base/stringprintf.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"

namespace base {


// A simple wrapper around icu::NumberFormat that allows for resetting it
// (as LazyInstance does not).

string16 FormatNumber(int64 number) {
  
   return UTF8ToUTF16(StringPrintf("%" PRId64, number));

}

string16 FormatDouble(double number, int fractional_digits) {

    // As a fallback, just return the raw number in a string.
    return UTF8ToUTF16(StringPrintf("%f", number));
 
}



}  // namespace base
