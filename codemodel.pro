TEMPLATE = app
CONFIG -= qt

HEADERS = quickbomanager.h \
    seiscodemodel.h \
    fdct_wrapping.hpp \
    fdct_wrapping_inc.hpp \
    fdct_wrapping_inline.hpp \
    mexaux.hpp \
    nummat.hpp \
    numvec.hpp \
    offmat.hpp \
    offvec.hpp

SOURCES += \
        quickbomanager.cpp \
        main.cpp \
    seiscodemodel.cpp \
    fdct_wrapping.cpp \
    ifdct_wrapping.cpp \
    fdct_wrapping_param.cpp

include(./common.pri)

