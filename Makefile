# BSD Makefile

PROG=tagutil
SRCS=tagutil.c \
	 t_toolkit.c \
	 t_tag.c \
	 t_yaml.c \
	 t_renamer.c \
	 t_lexer.c \
	 t_parser.c \
	 t_filter.c

VERSION=2.1


# install options
DESTDIR?=/usr/local
BINDIR?=/bin
MANDIR?=/man/man


# C opts
CSTD=c99
.if defined(DEBUG)
CFLAGS=-g -O0
WARNS?=6
VERSION:=${VERSION}-debug
.endif
CFLAGS+=-I. -D'T_TAGUTIL_VERSION="${VERSION}"'

# libyaml doesn't use pkg-config :(
CFLAGS+=-I/usr/local/include
LDADD+=-L/usr/local/lib -lyaml

# Generic support with TagLib
.if defined(WITH_TAGLIB)
SRCS+=t_ftgeneric.c
TAGLIB_I!=pkg-config --cflags taglib_c
CFLAGS+=-DWITH_TAGLIB ${TAGLIB_I}
TAGLIB_L!=pkg-config --libs   taglib_c
LDADD+=${TAGLIB_L}
.endif

# FLAC support with libFLAC
.if defined(WITH_FLAC)
SRCS+=t_ftflac.c
FLAC_I!=pkg-config --cflags flac
# use -iquote instead of -I to avoid a assert.h clash
CFLAGS+=-DWITH_FLAC ${FLAC_I:S/-I/-iquote /}
FLAC_L!=pkg-config --libs   flac
LDADD+=${FLAC_L}
.endif

# Ogg/Vorbis support with libvorbis
.if defined(WITH_OGGVORBIS)
SRCS+=t_ftoggvorbis.c
OGGVORBIS_I!=pkg-config --cflags vorbisfile
CFLAGS+=-DWITH_OGGVORBIS ${OGGVORBIS_I}
OGGVORBIS_L!=pkg-config --libs   vorbisfile
LDADD+=${OGGVORBIS_L}
.endif

# MPEG layer 3 support with ID3Lib
.if defined(WITH_MPEG3)
SRCS+=t_ftmpeg3.c t_ftmpeg3_tokenize.c
t_ftmpeg3.o: t_ftmpeg3_tokenize.h
.if !defined(GPERF)
GPERF=/usr/local/bin/gperf
.endif
# no pkg-config :(
MPEG3_I=-I/usr/local/include
CFLAGS+=-DWITH_MPEG3 ${MPEG3_I}
MPEG3_L=-lid3
LDADD+=${MPEG3_L}
.endif

.include <bsd.prog.mk>


.if defined(WITH_MPEG3)
t_ftmpeg3_tokenize.c: t_ftmpeg3_tokenize.h
	build-tools/gperf-mpeg3.sh C ${GPERF} ${CC} '${MPEG3_I}' > ${.TARGET}

t_ftmpeg3_tokenize.h:
	build-tools/gperf-mpeg3.sh header > ${.TARGET}

clean-t_ftmpeg3_tokenize:
	rm -f t_ftmpeg3_tokenize.c t_ftmpeg3_tokenize.o t_ftmpeg3_tokenize.h

clean: clean-t_ftmpeg3_tokenize

.endif
