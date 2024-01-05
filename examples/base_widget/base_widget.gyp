# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'chromium_code': 1,
  }, 
  'target_defaults': {
    'conditions': [
      ['OS=="win"', {'sources/': [
        ['include', '_(win)\\.cc$'],
        ['include', '/win/'],
        ['include', '/win_[^/]*\\.cc$'],
      ]}],
      ['use_aura==1', {
        'sources/': [ ['exclude', '_win\\.(h|cc)$'],
                      ['exclude', '_x\\.(h|cc)$'] ],
        'dependencies': [ '../aura/aura.gyp:aura', ],
      }],
    ],
  }, 
  'targets': [  
  {
      'target_name': 'base_widget',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:base_i18n', 
        '../../skia/skia.gyp:skia', 
        '../../ui/ui.gyp:ui', 
        '../../ui/ui.gyp:ui_resources', 
        '../../ui/views/views.gyp:views',
        '../extra_views/extra_views.gyp:*',
      ],
      'include_dirs': [
        '../..',
      ],
      'sources': [  
        'examples_main.cc',
        'example_base.cc',
        'example_base.h',
        'examples_window.cc',
        'examples_window.h',  
        'native_table_example.cc',
        'native_table_example.h',
        'tree_table_view_example.cc',
        'tree_table_view_example.h',
      ],
      'conditions': [
        ['OS=="win"', {
          'link_settings': {
            'libraries': [
              '-limm32.lib',
              '-loleacc.lib',
            ]
          },
          'msvs_settings': { 
            'VCLinkerTool': {
              'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
            },
          }, 
        }], 
        ['OS=="win"', {
          'sources/': [
            # This is needed because the aura rule strips it from the default
            # sources list. 
          ],
        }],
      ],
    },  # target_name: base_widget          
  ],
}
