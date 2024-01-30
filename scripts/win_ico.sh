#!/bin/bash
# Requires icoutils package
# Run inside the scripts directory

declare -a apps=("valentina" "tape" "puzzle")

for app in "${apps[@]}"; do
    iconset="../src/app/${app}/share/resources/${app}.iconset"

    icotool --create "--output=../src/app/${app}/share/resources/${app}.ico" \
    --icon ${iconset}/icon_16x16.png \
    ${iconset}/icon_32x32.png \
    ${iconset}/icon_128x128.png \
    -r ${iconset}/icon_256x256.png \
    -r ${iconset}/icon_512x512.png
done

declare -a mimetypes=("i-measurements" "k-measurements" "layout" "pattern" "s-measurements")

for mimetype in "${mimetypes[@]}"; do
    iconset="../dist/win/valentina-project.assets/${mimetype}.iconset"

    icotool --create "--output=../dist/win/${mimetype}.ico" \
    --icon ${iconset}/icon_16x16.png \
    ${iconset}/icon_32x32.png \
    ${iconset}/icon_128x128.png \
    -r ${iconset}/icon_256x256.png \
    -r ${iconset}/icon_512x512.png
done
