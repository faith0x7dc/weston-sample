noinst_LTLIBRARIES = libWLToolKit.la

noinst_PROGRAMS =	\
	HomeScreenApp	\
	test

AM_CFLAGS = $(GCC_CFLAGS)
AM_CPPFLAGS =								\
	-DDATADIR='"$(datadir)"'				\
	-DBINDIR='"$(bindir)"'					\
	$(CLIENT_CFLAGS) $(CAIRO_EGL_CFLAGS)

libWLToolKit_la_SOURCES =	\
	Source/Display.cpp		\
	Source/Window.cpp		\
	Source/WindowEGL.cpp	\
	Source/Texture.cpp
libWLToolKit_la_CPPFLAGS = -I../clients $(AM_CPPFLAGS)
libWLToolKit_la_LIBADD = ../clients/libtoytoolkit.la $(SIMPLE_EGL_CLIENT_LIBS)

HomeScreenApp_SOURCES = 	\
	HomeScreenApp.c			\
	HomeScreenFactory.cpp	\
	HomeScreen.cpp
HomeScreenApp_CFLAGS = -I../clients
HomeScreenApp_LDADD = libWLToolKit.la

test_SOURCES = test.cpp
test_CFLAGS = -I../clients `shell pkg-config --cflags cairo`
test_LDADD = libWLToolKit.la 

