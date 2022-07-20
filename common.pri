######################################################################
# bo_template V1.0
######################################################################

defineReplace(ndp_basename) {
	fullPath = $$1
	fullBaseName = $$basename(fullPath)
	nameList = $$split(fullBaseName, ".")
	return ($$member(nameList, 0))
}

BINDIR=$$_PRO_FILE_PWD_/../bin
LIBDIR=$$BINDIR/bin/lib

CONFIG  += debug warn_off
TARGET  = $$ndp_basename($$_PRO_FILE_)

QMAKE_CXXFLAGS += -fpermissive

INCLUDEPATH += $$_PRO_FILE_PWD_
INCLUDEPATH += $$_PRO_FILE_PWD_/include

INCLUDEPATH += /usr/include/libxml2
INCLUDEPATH += $(IECO)/include/ndp/bo
INCLUDEPATH += $(IECO)/include/ndp/bdo
INCLUDEPATH += $(IECO)/include/ndp/bdo
INCLUDEPATH += $(IECO)/include/ndp/core
INCLUDEPATH += $(IECO)/include/common/ggeometry

LIBS += -L$(IECO)/libso/ndp
LIBS += -L$(IECO)/libso/common
LIBS +=  -lbomgr -lboprocess -lbointp -lboseis -lbowell -lboref -lbdomgr -lbdoseis -lbdointp -lbdoprocess -lbdowell -ldpbase -lcpool -ldpthreadlck -ldpfiles -lsqlcore -ldpplug -ldpconf -ldpcrs -ldpeav -ldpgdbmgr -ldpuomsys
LIBS += -lggeometry5
LIBS += -lxml2

#crs lib need to connect the following libs
LIBS += -L$(IECO)/support/gdal/lib/
LIBS += -lgdal
LIBS += -lfftw


#tbb
linux-icc-64 {
	LIBS += -ltbb
}
linux-gcc-64 {
	LIBS += $(SUPPORTROOT)/intelso/lib64/libtbb.so.2
}

BASE_MIDDLE_DIR = $$_PRO_FILE_PWD_/.middir
OBJECTS_DIR = $$BASE_MIDDLE_DIR/obj
UIC_DIR = $$BASE_MIDDLE_DIR/uic
RCC_DIR = $$BASE_MIDDLE_DIR/rcc
MOC_DIR = $$BASE_MIDDLE_DIR/moc

equals(TEMPLATE, app) {
DESTDIR = $$BINDIR
QMAKE_RPATHDIR += $$LIBDIR
} else {
DESTDIR = $$LIBDIR
}

#Support C++11
QMAKE_CXXFLAGS += -std=c++11


