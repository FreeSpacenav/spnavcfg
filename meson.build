project('spnavcfg', ['c', 'cpp'],
  default_options: ['warning_level=3', 'cpp_std=c++17'],
  meson_version : '>=0.63.0',
)

qt6 = import('qt6')

qt6modules = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])
libspnav = dependency('spnav', version: '>1.1')
libx11 = dependency('x11')

moc_ui = qt6.compile_moc(headers: ['src/ui.h'])
ui_headers = qt6.compile_ui(
  sources: ['ui/about.ui', 'ui/bnmaprow.ui', 'ui/spnavcfg.ui']
)
resources = qt6.compile_resources(sources: ['ui/spnavcfg.qrc'])

executable('spnavcfg',
    sources: [
      'src/main.cc',
      'src/spnavcfg.c',
      'src/ui.cc',
      moc_ui,
      ui_headers,
      resources,
    ],
    dependencies: [qt6modules, libspnav, libx11],
    install: true,
)

datadir = get_option('datadir')

foreach icon_size: ['48x48', '128x128', '256x256']
  install_data('icons/spnavcfg-@0@.png'.format(icon_size),
    rename: 'spnavcfg.png',
    install_dir: datadir / 'icons/hicolor' / icon_size / 'apps',
  )
endforeach

install_data('icons/spnavcfg.desktop',
  install_dir: datadir / 'applications',
)
