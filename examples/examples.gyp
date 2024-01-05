# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'examples',
      'type': 'none',
      'dependencies': [
        'base_widget/base_widget.gyp:*',
        'activex_widget/activex_widget.gyp:*',  
        'extra_views/extra_views.gyp:*',
      ],
    },
  ],
}
