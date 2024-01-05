{
  'variables': {
    'chromium_code': 1,
    'grit_base_out_dir': '<(SHARED_INTERMEDIATE_DIR)/ui',
  },
  'targets': [
    {
      'target_name': 'ui_strings',
      'type': 'none',
      'actions': [
        {
          'action_name': 'ui_strings',
          'variables': {
            'grit_grd_file': 'ui_strings.grd',
            'grit_out_dir': '<(grit_base_out_dir)/ui_strings',
          },
          'includes': [ '../../../build/grit_action.gypi' ],
        },
        {
          'action_name': 'app_locale_settings',
          'variables': {
            'grit_grd_file': 'app_locale_settings.grd',
            'grit_out_dir': '<(grit_base_out_dir)/app_locale_settings',
          },
          'includes': [ '../../../build/grit_action.gypi' ],
        },
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(grit_base_out_dir)/app_locale_settings',
          '<(grit_base_out_dir)/ui_strings',
        ],
      },
    },
  ],
  'conditions': [
    [ 'OS != "mac"', {
      'targets': [{
        'target_name': 'ui_locales_strings',
        'type': 'none',
        'dependencies': [
          'ui_strings',
        ],
        'variables': {
          'repack_path': '<(DEPTH)/tools/grit/grit/format/repack.py',
          'conditions': [
            ['OS == "ios"', {
              'pak_output': '<(PRODUCT_DIR)/locales/cn.lproj/locale.pak',
            }, {
              'pak_output': '<(PRODUCT_DIR)/locales/zh-CN.pak',
            }],
          ],
        },
        'actions': [
          {
            'action_name': 'repack_ui_unittest_strings',
            'variables': {
              'pak_inputs': [
                '<(grit_base_out_dir)/ui_strings/ui_strings_zh-CN.pak',
                '<(grit_base_out_dir)/app_locale_settings/app_locale_settings_zh-CN.pak',
              ],
            },
            'inputs': [
              '<(repack_path)',
              '<@(pak_inputs)',
            ],
            'outputs': [
              '<(pak_output)',
            ],
            'action': ['python', '<(repack_path)', '<@(_outputs)',
                       '<@(pak_inputs)'],
          },
        ],
        'copies': [
          {
            'destination': '<(PRODUCT_DIR)',
            'files': [
              '<(grit_base_out_dir)/ui_resources/ui_resources_100_percent.pak',
            ],
          },
        ],
      }],
    }],
  ],
}
