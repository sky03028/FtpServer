TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    session.cpp \
    service.cpp \
    socketsource.cpp \
    utils.cpp \
    threadpool.cpp \
    ftpwarpper.cpp \
    ftpsession.cpp \
    FtpService/ftpsession.cpp \
    FtpService/ftpwarpper.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    session.h \
    service.h \
    socketsource.h \
    utils.h \
    threadpool.h \
    ftpwarpper.h \
    ftpcodes.h \
    ftpsession.h \
    CodeConverter.h \
    FtpService/ftpcodes.h \
    FtpService/ftpsession.h \
    FtpService/ftpwarpper.h \
    lualib/lauxlib.h \
    lualib/lua.h \
    lualib/luaconf.h \
    lualib/lualib.h


LIBS += libws2_32
