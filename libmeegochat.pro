VERSION = 0.1.0
unix { 
    PREFIX = /usr
    LIBDIR = $$PREFIX/lib
    INCLUDEDIR = $$PREFIX/include/libmeegochat
}
TEMPLATE = lib
TARGET = meegochat
MOC_DIR = .moc
OBJECTS_DIR = .obj
MGEN_OUTDIR = .gen
DEPENDPATH += .

CONFIG += link_pkgconfig mobility debug
MOBILITY += contacts
PKGCONFIG += QtContacts TelepathyQt4
SUBDIRS += tests
QT += dbus
#INCLUDEPATH += /usr/include/telepathy-1.0
#LIBS += -ltelepathy-qt4
INSTALL_HEADERS = meegochatcontact.h \
    meegochataccount.h \
    meegochataccountmanager.h \
    meegochataccountmodel.h \
    meegochatcontactmodel.h \
    meegochatcontactmanager.h \
    meegochatclienthandler.h \
    meegochatmessage.h \
    meegochatcontactsortfilterproxymodel.h
HEADERS += $$STYLE_HEADERS \
    $$INSTALL_HEADERS
SOURCES += meegochatcontact.cpp \
    meegochataccount.cpp \
    meegochataccountmanager.cpp \
    meegochataccountmodel.cpp \
    meegochatcontactmodel.cpp \
    meegochatcontactmanager.cpp \
    meegochatcontactsortfilterproxymodel.cpp \
    meegochatclienthandler.cpp \
    meegochatmessage.cpp
target.path = $$LIBDIR
headers.files = $$INSTALL_HEADERS
headers.path = $$INCLUDEDIR
pcfile.files = libmeegochat.pc
pcfile.path = $$LIBDIR/pkgconfig
INSTALLS += target \
    headers \
    pcfile
OTHER_FILES += README \
    libmeegochat.pc
