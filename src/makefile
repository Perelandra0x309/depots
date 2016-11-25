NAME = Depots
APP_MIME_SIG = application/x-vnd.BH-Depots
TYPE = APP
SRCS = AddRepoWindow.cpp \
	DepotsView.cpp \
	DepotsWindow.cpp \
	DepotsSettings.cpp \
	Depots.cpp
LIBS := be localestub $(STDCPPLIBS)
OPTIMIZE := FULL
RDEFS = Depots.rdef
LOCALES = en
TARGET_DIR := .

## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine

