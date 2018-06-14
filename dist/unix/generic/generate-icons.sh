#!/bin/bash

ARRAY=(16 22 24 32 36 48 64 72 96 128 192 256)

for i in ${ARRAY[*]}; do 
   mkdir hicolor/"$i"x"$i"
   mkdir hicolor/"$i"x"$i"/apps
   mkdir hicolor/"$i"x"$i"/status
   inkscape hicolor/scalable/apps/crow-translate.svg -e hicolor/"$i"x"$i"/apps/crow-translate.png -w $i -h $i
   inkscape hicolor/scalable/status/crow-translate-tray-light.svg -e hicolor/"$i"x"$i"/status/crow-translate-tray-light.png -w $i -h $i
   inkscape hicolor/scalable/status/crow-translate-tray-dark.svg -e hicolor/"$i"x"$i"/status/crow-translate-tray-dark.png -w $i -h $i
done
