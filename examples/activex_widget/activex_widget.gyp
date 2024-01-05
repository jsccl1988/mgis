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
      'target_name': 'activex_widget',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:base_i18n', 
        '../../skia/skia.gyp:skia', 
        '../../ui/ui.gyp:ui', 
        '../../ui/ui.gyp:ui_resources', 
        '../../ui/views/views.gyp:views'
      ],
      'include_dirs': [
        '../..',
      ],
      'sources': [  
        'main.cpp',
        'demo_main.cpp',
        'demo_main.h',
        'demo_activex.cpp',
        'demo_activex.h',          
        'demo_animate.cpp',
        'demo_animate.h',  
        'demo_bubble.cpp',
        'demo_bubble.h',  
        'demo_html.cpp',
        'demo_html.h',  
        'demo_label.cpp',
        'demo_label.h',  
        'demo_silverlight.cpp',
        'demo_silverlight.h',  
        'demo_webbrowser.cpp',
        'demo_webbrowser.h',   
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
