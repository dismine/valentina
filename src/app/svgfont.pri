INSTALL_SVG_FONTS += $$files($$PWD/share/svgfonts/*.svg, true)

copyToDestdir($$INSTALL_SVG_FONTS, $$shell_path($${OUT_PWD}/$$DESTDIR/svgfonts))
