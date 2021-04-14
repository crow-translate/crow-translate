find_program(LINUXDEPLOY_EXECUTABLE
  NAMES linuxdeploy linuxdeploy-x86_64.AppImage
  HINTS ${CPACK_PACKAGE_DIRECTORY} ${CPACK_LINUXDEPLOY_ROOT}
)

execute_process(COMMAND ${CMAKE_COMMAND} -E env EXTRA_QT_PLUGINS=multimediawidgets\;svg VERSION=${CPACK_PACKAGE_VERSION}
  ${LINUXDEPLOY_EXECUTABLE} --appdir=${CPACK_TEMPORARY_DIRECTORY} --plugin=qt --plugin=gstreamer --output=appimage
)
