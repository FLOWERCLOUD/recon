project(visualsfm)

set(VSFM_URL http://ccwu.me/vsfm/download/VisualSFM_osx_64bit.zip)
set(VSFM_SRCZIP ${CMAKE_CURRENT_BINARY_DIR}/VisualSFM.zip)
set(VSFM_SRC ${CMAKE_CURRENT_BINARY_DIR}/VisualSFM)

if(NOT EXISTS ${VSFM_SRCZIP})
  file(DOWNLOAD ${VSFM_URL} ${VSFM_SRCZIP} SHOW_PROGRESS)
endif()

if(NOT EXISTS ${VSFM_SRC})
  execute_process(COMMAND unzip ${VSFM_SRCZIP} -d ${VSFM_SRC})
endif()

#add_custom_command

