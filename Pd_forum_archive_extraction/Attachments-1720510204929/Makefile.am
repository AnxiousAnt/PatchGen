
INCLUDES =\
	 -I/usr/local/include/ -I../include/

CFLAGS = \
	@FLEXT_CFLAGS@  @VORBIS_CFLAGS@   @MAD_CFLAGS@  @FLC_CFLAGS@

CXXFLAGS =  \
	 @FLEXT_CFLAGS@  @VORBIS_CFLAGS@   @MAD_CFLAGS@  @FLC_CFLAGS@

bin_PROGRAMS = readanysf~.@pd_suffix@

readanysf__@pd_suffix@_SOURCES = \
	Fifo.cpp\
	Input.cpp\
	InputFile.cpp\
	InputStream.cpp\
	ReadRaw.cpp\
	Readsf.cpp\
	main.cpp\
	ReadMad.cpp\
	ReadVorbis.cpp \
	ReadFlac.cpp

#### IMPORTANT!!! FLEXT_LIBS HAS TO CUM FIRST
readanysf__@pd_suffix@_LDADD = \
	@FLEXT_LIBS@  @PTHREAD_LIBS@  @SRC_LIBS@  @VORBIS_LIBS@   @MAD_LIBS@  @FLC_LIBS@

