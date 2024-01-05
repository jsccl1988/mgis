# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'All',
      'type': 'none',
      'xcode_create_dependents_test_runner': 1,
      'dependencies': [
        '../base/base.gyp:*',
        '../testing/gmock.gyp:*',
        '../testing/gtest.gyp:*',
        '../third_party/zlib/zlib.gyp:*',
        '../ui/ui.gyp:*',
        '../mviews.gyp:*',
      ],
      'conditions': [
        ['OS!="ios"', {
          'dependencies': [
            '../skia/skia.gyp:*',
            '../third_party/libpng/libpng.gyp:*',
            '../third_party/modp_b64/modp_b64.gyp:*',
            '<(libjpeg_gyp_path):*',
          ],
        }],
        ['os_posix==1 and OS!="android" and OS!="ios"', {
          'dependencies': [
            '../third_party/yasm/yasm.gyp:*#host',
          ],
        }],
        ['OS=="mac"', {
          'dependencies': [
            '../third_party/ocmock/ocmock.gyp:*',
          ],
        }],
        ['OS=="linux"', {
          'dependencies': [
            '../courgette/courgette.gyp:*',
            '../dbus/dbus.gyp:*',
            '../sandbox/sandbox.gyp:*',
          ],
          'conditions': [

            ['chromeos==0', {
              'dependencies': [
                '../third_party/cros_dbus_cplusplus/cros_dbus_cplusplus.gyp:*',
                '../third_party/libmtp/libmtp.gyp:*',
                '../third_party/mtpd/mtpd.gyp:*',
              ],
            }],
          ],
        }],
        ['use_x11==1', {
          'dependencies': [
            '../tools/xdisplaycheck/xdisplaycheck.gyp:*',
          ],
        }],
        ['toolkit_uses_gtk==1', {
          'dependencies': [
            '../tools/gtk_clipboard_dump/gtk_clipboard_dump.gyp:*',
          ],
        }],
      ],
    }, # target_name: All
  ],
  'conditions': [

    ['OS=="win"', {
      'targets': [
        # These targets are here so the build bots can use them to build
        # subsets of a full tree for faster cycle times.


      ],  # targets
      'conditions': [
        ['branding=="Chrome"', {
          'targets': [
            {
              'target_name': 'chrome_official_builder',
              'type': 'none',
              'dependencies': [
                '../chrome/chrome.gyp:chromedriver',
                '../chrome/chrome.gyp:crash_service',
                '../chrome/chrome.gyp:interactive_ui_tests',
                '../chrome/chrome.gyp:policy_templates',
                '../chrome/chrome.gyp:reliability_tests',
                '../chrome/chrome.gyp:automated_ui_tests',
                '../chrome/installer/mini_installer.gyp:mini_installer',
                '../chrome_frame/chrome_frame.gyp:npchrome_frame',
                '../courgette/courgette.gyp:courgette',
                '../cloud_print/cloud_print.gyp:cloud_print',
                '../remoting/remoting.gyp:remoting_webapp',
                '../third_party/widevine/cdm/widevine_cdm.gyp:widevinecdmadapter',
              ],
              'conditions': [
                # If you change this condition, make sure you also change it
                # in chrome_tests.gypi
                ['enable_automation==1 and (OS=="mac" or (os_posix==1 and target_arch==python_arch))', {
                  'dependencies': [
                    '../chrome/chrome.gyp:pyautolib',
                  ],
                }],
                ['internal_pdf', {
                  'dependencies': [
                    '../pdf/pdf.gyp:pdf',
                  ],
                }], # internal_pdf
                ['target_arch=="ia32"', {
                  'dependencies': [
                    '../chrome/chrome.gyp:crash_service_win64',
                    '../courgette/courgette.gyp:courgette64',
                  ],
                }],
                ['component != "shared_library" and wix_exists == "True" and \
                    sas_dll_exists == "True"', {
                  'dependencies': [
                    '../remoting/remoting.gyp:remoting_host_installation',
                  ],
                }], # component != "shared_library"
                ['target_arch=="x64"', {
                  'dependencies!': [
                    '../chrome_frame/chrome_frame.gyp:npchrome_frame',
                  ],
                  'defines': [
                    'OMIT_CHROME_FRAME',
                  ],
                }], # target_arch=="x64"
              ]
            },
          ], # targets
        }], # branding=="Chrome"
       ], # conditions
    }], # OS="win"
    ['use_aura==1', {
      'targets': [
        {
          'target_name': 'aura_builder',
          'type': 'none',
          'dependencies': [
            '../cc/cc_tests.gyp:cc_unittests',
            '../components/components.gyp:components_unittests',
            '../ui/app_list/app_list.gyp:*',
            '../ui/aura/aura.gyp:*',
            '../ui/message_center/message_center.gyp:*',
            '../ui/ui.gyp:ui_unittests',
            '../ui/views/views.gyp:views',
            '../ui/views/views.gyp:views_examples_with_content_exe',
            '../ui/views/views.gyp:views_unittests',
            '../ui/keyboard/keyboard.gyp:*',
          ],
          'conditions': [
            ['OS=="win"', {
              'dependencies': [
                '../chrome/chrome.gyp:crash_service',
                '../chrome_frame/chrome_frame.gyp:npchrome_frame',
              ],
            }],
            ['OS=="win" and target_arch!="x64"', {
              'dependencies': [
                '../chrome_frame/chrome_frame.gyp:chrome_frame_net_tests',
                '../chrome_frame/chrome_frame.gyp:chrome_frame_perftests',
                '../chrome_frame/chrome_frame.gyp:chrome_frame_reliability_tests',
                '../chrome_frame/chrome_frame.gyp:chrome_frame_tests',
                '../chrome_frame/chrome_frame.gyp:chrome_frame_unittests',
              ],
            }],
            ['OS=="win" and target_arch=="x64"', {
              'dependencies!': [
                '../chrome_frame/chrome_frame.gyp:npchrome_frame',
              ],
              'defines': [
                'OMIT_CHROME_FRAME',
              ],
            }],
            ['OS=="win" and target_arch=="ia32"', {
              'dependencies': [
                '../chrome/chrome.gyp:crash_service_win64',
              ],
            }],
            ['use_ash==1', {
              'dependencies': [
                '../ash/ash.gyp:ash_shell',
                '../ash/ash.gyp:ash_unittests',
              ],
            }],
            ['OS=="linux"', {
              # Tests that currently only work on Linux.
              'dependencies': [
                '../base/base.gyp:base_unittests',
              ],
            }],
            ['OS=="mac"', {
              # Exclude dependencies that are not currently implemented.
              'dependencies!': [
                '../chrome/chrome.gyp:chrome',
                '../chrome/chrome.gyp:unit_tests',
                '../device/device.gyp:device_unittests',
                '../ui/views/views.gyp:views_unittests',
              ],
            }],
            ['chromeos==1', {
              'dependencies': [
                '../chromeos/chromeos.gyp:chromeos_unittests',
              ],
            }],
          ],
        },
      ],  # targets
    }], # "use_aura==1"
  ], # conditions
}
