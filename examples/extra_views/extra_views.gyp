# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
  },
  'targets': [   
    {
      'target_name': 'extra_views',
      'type': '<(component)',
      'dependencies': [       
        '../../base/base.gyp:base',
        '../../base/base.gyp:base_i18n',     
        '../../ui/views/views.gyp:views',		
        '../../skia/skia.gyp:skia',          
        '../../ui/ui.gyp:ui', 
        '../../ui/ui.gyp:ui_resources', 
      ],
      'defines': [
        'EXTRA_VIEWS_IMPLEMENTATION',
      ],
      'sources': [
        'extra_views_export.h',
        'table/table_view_win.cc',
        'table/table_view_win.h',
        'treetable/tree_table_header.cc',        
        'treetable/tree_table_header.h',
        'treetable/tree_table_model.cc',
        'treetable/tree_table_model.h',
        'treetable/tree_table_node_iterator.h',
        'treetable/tree_table_node_model.h',
        'treetable/tree_table_view.cc',        
        'treetable/tree_table_view.h',
        'treetable/tree_table_view_controller.cc',        
        'treetable/tree_table_view_controller.h',                
        'treetable/tree_table_view_selector.cc',
        'treetable/tree_table_view_selector.h',     
      ],
      'conditions': [
        ['OS=="win"', {
          'link_settings': {
            'libraries': [
              '-limm32.lib',
              '-loleacc.lib',
              '-lComctl32.lib',
            ]
          },          
        },],
      ],
    },  
  ],
}
