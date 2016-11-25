NAME = Depots
APP_MIME_SIG = application/x-vnd.BH-Depots
TYPE = APP
SRCS = AddRepoWindow.cpp \
	DepotsView.cpp \
	DepotsWindow.cpp \
	DepotsSettings.cpp \
	Depots.cpp
LIBS := be localestub tracker /boot/system/develop/lib/libcolumnlistview.a $(STDCPPLIBS)
OPTIMIZE := FULL
RDEFS = Depots.rdef
LIBPATHS = /boot/system/develop/lib
SYSTEM_INCLUDE_PATHS = /boot/system/develop/headers/private/interface
LOCALES = en
TARGET_DIR := .

## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine

