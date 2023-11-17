#!/bin/bash

# The script creates SVG templates for all cursors.
# Before run create PNG icon of tool!
# Use SVG templates for creating PNG cursors.

# IMPORTANT!!! Run script inside 'scripts' directory.

# usage:
# $ ./generate_tool_cursor.sh

# Notes.
# No, template_cursor.svg is not broken! It is just template, no point open it.
# After creating SVG templates open them by Inkscape, previewer doesn't show pictures inside svg file linked by url.
# I have spent hour before understand why i don't see PNG inside SVG in Nautilus.

PATTERN=*@2x.png
TOOLICONPATH=../src/app/valentina/share/resources/toolicon/ # PNG tool icon should be here
OUTPATH=../src/app/valentina/share/resources/toolcursor

COLOR_SCHEMES=("light" "dark")

for scheme in "${COLOR_SCHEMES[@]}"
do
  # Create a subdirectory for the current scheme if it doesn't exist
  mkdir -p "$OUTPATH/$scheme"

  TOOLICONS=`ls $TOOLICONPATH${scheme}/$PATTERN`

  for var in $TOOLICONS
  do
	  basename=${var##*/} # remove the path from a path-string
	  basename=${basename%.png} # remove the extension from a path-string
	  basename=${basename%@2x} # remove optional @2x suffix
	  if [ ! -f $basename@2x.png ]; then # always prefere hidpi version
		  sed "s/<<basename>>/$basename@2x/" $OUTPATH/svg/template_cursor_${scheme}.svg > $OUTPATH/svg/${scheme}/${basename}_cursor.svg
	  else
		  sed "s/<<basename>>/$basename/" $OUTPATH/svg/template_cursor_${scheme}.svg > $OUTPATH/svg/${scheme}/${basename}_cursor.svg
	  fi

    # Generate PNG files from the SVGs
    inkscape --export-filename "$OUTPATH/$scheme/${basename}_cursor@2x.png" --export-width 64 --export-height 64 "$OUTPATH/svg/$scheme/${basename}_cursor.svg"
    inkscape --export-filename "$OUTPATH/$scheme/${basename}_cursor.png" --export-width 32 --export-height 32 "$OUTPATH/svg/$scheme/${basename}_cursor.svg"
  done
done
