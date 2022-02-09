project('spnavcfg', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

pkgdatadir = get_option('prefix') / get_option('datadir') / meson.project_name()

executable('spnavcfg',
           'back.c',
           'spnavcfg.c',
           'front_gtk.c',
           'cfgfile.c',
           dependencies: [
             dependency('gtk+-2.0'),
             dependency('gmodule-export-2.0'),
             dependency('x11')
           ],
           c_args: [ '-DPKGDATADIR="@0@"'.format(pkgdatadir) ],
           install : true)

subdir('ui')
subdir('icons')
