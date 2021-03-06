cmake_minimum_required(VERSION 2.8)
project(popi)

#SET(CMAKE_CXX_COMPILER "gcc-4.4")
############################################################
# Enable Testing

enable_testing()

############################################################
#Tell me what commands are being executed.
set(CMAKE_VERBOSE_MAKEFILE TRUE)

# Make gcc output all the debug information it can! Enclose this in
#some sort of "if" if more than one compiler is ever supported

set(CMAKE_CXX_FLAGS "-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES -D__WXGTK__ -DEIGEN_DONT_ALIGN")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -Wno-write-strings -Wall")

SET(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS}")

#set(lapack_libraries "-Bstatic --static $ENV{HOME}/src/CLAPACK-3.2.1/lapack_LINUX.a $ENV{HOME}/src/CLAPACK-3.2.1/blas_LINUX.a $ENV{HOME}/src/CLAPACK-3.2.1/F2CLIBS/libf2c.a -Bdynamic")

set(lapack_libraries "-Bstatic $ENV{HOME}/src/CLAPACK-3.2.1/lapack_LINUX.a $ENV{HOME}/src/CLAPACK-3.2.1/blas_LINUX.a $ENV{HOME}/src/CLAPACK-3.2.1/F2CLIBS/libf2c.a -Bdynamic")

#Choose a build type:

#Default build. The worst of both worlds
# set(CMAKE_BUILD_TYPE None)

#Debug build, can use with gdb
set(CMAKE_BUILD_TYPE Debug)

#Compile a release build
#set(CMAKE_BUILD_TYPE Release)

############################################################
#Work out where all the built files are going
set(buildDir ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY})

#GDB can debug an optermised executable.
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ggdb  -Wno-write-strings ")

############################################################
# Do automatic library detection

find_package(wxWidgets COMPONENTS core gl adv aui html REQUIRED)
#find_package(Boost REQUIRED system program_options filesystem unit_test_framework) 

############################################################
# Manual wxWidgets 

# #Prefix of the wxWidgets build
# set(wxDir         	"$ENV{HOME}/src/wxWidgets-2.8.12")
# set(wxLibraryDirs 	"${wxDir}/lib")

# #set(wxSetupHDir         "${wxDir}/lib/wx/include/gtk2-unicode-release-static-2.8/")
# set(wxSetupHDir         "${wxDir}/lib/wx/include/gtk2-unicode-release-2.8")
# set(wxSetupHDir         "${wxDir}/lib/wx/include/gtk2-unicode-release-static-2.8")

# set(wxIncludeDirs 	"${wxDir}/include/" ${wxSetupHDir})

# set(wxWidgets_LIBRARIES "-L${wxLibraryDirs}" #-Wl,-Bstatic
#   -lwx_gtk2u_gl-2.8
#   -lwx_gtk2u_aui-2.8
#   -lwx_gtk2u_html-2.8
#   -lwx_gtk2u_adv-2.8
#   -lwx_baseu-2.8
#   #-Wl,-Bdynamic
# )


#Prefix of boost builds known to work
#set(Boost_version "-gcc46-mt-1_52")
#set(Boost_dir     "$ENV{HOME}/src/boost_1_52_0")

set(Boost_version "-gcc46-mt")
set(Boost_dir     "$ENV{HOME}/src/boost_1_46_1")

set(Boost_INCLUDE_DIRS ${Boost_dir})
set(Boost_LIBRARIES "-L${Boost_dir}/stage/lib/"
  -Wl,-Bstatic
  -lboost_system${Boost_version}
  -lboost_program_options${Boost_version}
  -lboost_unit_test_framework${Boost_version}
  -lboost_filesystem${Boost_version}
  -Wl,-Bdynamic
)

############################################################
#List of source files for all the targets

file(GLOB sharedSources shared/*.cpp)
file(GLOB formatSources shared/formats/*.cpp)
file(GLOB guiSources gui/*.cpp)
file(GLOB threeDeeSources 3d/*.cpp)

set(guiSources ${guiSources} ${threeDeeSources})
set(wxformbuilderSources
  ${CMAKE_CURRENT_BINARY_DIR}/auto/SpinachGUI.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/auto/easyspin.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/auto/orientdialog.cpp
)

set(easyspinSources   #For the standalone easyspin version
  ${CMAKE_CURRENT_BINARY_DIR}/auto/SpinachGUI.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/auto/easyspin.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/auto/orientdialog.cpp
  gui/EasySpinFrame.cpp
  gui/TextBitmap.cpp
  gui/OrientDialog2.cpp
  gui/Spacegroup.cpp
  gui/OrientDisplay.cpp
  gui/TextCtrlFocus.cpp
  gui/Display3D.cpp
  3d/camera.cpp
  3d/glmode.cpp
  3d/displaySettings.cpp
  easyspin/EasySpinStandalone.cpp
  #These are pulled in because of SpinachGUI.cpp from
  #wxFormBuilder. We could probably get rid of them with we modualised
  #more
  gui/LogSlider.cpp
  gui/SpinChoice.cpp
)

set(orientDialogSources   #For the standalone easyspin version
  ${CMAKE_CURRENT_BINARY_DIR}/auto/orientdialog.cpp
  gui/TextBitmap.cpp
  gui/TextCtrlFocus.cpp
  gui/OrientDialog2.cpp
  gui/OrientDisplay.cpp
  gui/Display3D.cpp
  3d/camera.cpp
  3d/glmode.cpp
  3d/displaySettings.cpp
  orientdialog/app.cpp
)


set(xmlSources
  shared/formats/tinyxml/tinyxml.cpp 
  shared/formats/tinyxml/tinyxmlerror.cpp
  shared/formats/tinyxml/tinyxmlparser.cpp)

############################################################
# Link and include directories
# If you include via absolute paths, you can move source files about
# more easily.
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(SYSTEM ${wxWidgets_INCLUDE_DIRS})
#include_directories(SYSTEM ${wxIncludeDirs}) #Manual configuration
include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(SYSTEM /usr/include/sigc++-2.0)
include_directories(SYSTEM /usr/lib/sigc++-2.0/include/)
include_directories(SYSTEM $ENV{HOME}/src/eigen-3.1.2/)
include_directories(SYSTEM $ENV{HOME}/src/CLAPACK-3.2.1/INCLUDE/)



############################################################
# Link the non graphical code into a library

add_library(spinxml STATIC ${xmlSources} ${sharedSources} ${formatSources})

############################################################
# GUI code build here

add_executable(SpinachGUI ${guiSources} ${wxformbuilderSources})
target_link_libraries(SpinachGUI
  spinxml
  ${Boost_LIBRARIES}
  sigc-2.0
  ${wxWidgets_LIBRARIES}
  GL
  GLU
  ${lapack_libraries})

############################################################
# Easyspin standalone app here

add_executable(EasySpinGUI ${easyspinSources})
target_link_libraries(EasySpinGUI
  spinxml
  ${Boost_LIBRARIES}
  sigc-2.0
  ${wxWidgets_LIBRARIES}
  GL
  GLU
  ${lapack_libraries})

############################################################
# Orient Dialog Standalone

add_executable(OrientDialog ${orientDialogSources})
target_link_libraries(OrientDialog
  spinxml
  ${Boost_LIBRARIES}
  sigc-2.0
  ${wxWidgets_LIBRARIES}
  GL
  GLU
  ${lapack_libraries})

############################################################
# Tests specified here

# add_executable(InteractionRawTest "shared/test/interaction_test.cpp")
# target_link_libraries(InteractionRawTest ${Boost_LIBRARIES} spinxml sigc-2.0)
# add_test(InteractionRawTest ${EXECUTABLE_OUTPUT_PATH}/InteractionRawTest)

# add_executable(OrientationTest "shared/test/orientation_test.cpp" )
# target_link_libraries(OrientationTest spinxml ${Boost_LIBRARIES} spinxml sigc-2.0)
# add_test(OrientationTest ${EXECUTABLE_OUTPUT_PATH}/OrientationTest)

# add_executable(SpinTest "shared/test/spin_test.cpp" )
# target_link_libraries(SpinTest spinxml ${Boost_LIBRARIES} sigc-2.0)
# add_test(SpinTest ${EXECUTABLE_OUTPUT_PATH}/SpinTest)

# add_executable(SpinSystemTest "shared/test/spinsys_test.cpp" )
# target_link_libraries(SpinSystemTest spinxml ${Boost_LIBRARIES}  sigc-2.0)
# add_test(SpinSystemTest ${EXECUTABLE_OUTPUT_PATH}/SpinSystemTest)

# add_executable(FrameTest "shared/test/frame_test.cpp" )
# target_link_libraries(FrameTest spinxml ${Boost_LIBRARIES}  sigc-2.0)
# add_test(FrameTest ${EXECUTABLE_OUTPUT_PATH}/SpinSystemTest)

# add_executable(FormatTest "shared/test/format_test.cpp" )
# target_link_libraries(FormatTest spinxml ${Boost_LIBRARIES} sigc-2.0)
# add_test(FrameTest ${EXECUTABLE_OUTPUT_PATH}/FormatTest)


############################################################
# Copy the dat directory
file(COPY data/ DESTINATION data/)
file(COPY examples/ DESTINATION examples/)