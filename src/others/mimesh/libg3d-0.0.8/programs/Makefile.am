bin_PROGRAMS = g3d-stat

AM_CFLAGS = \
	-Wall -ansi -pedantic \
	-I$(top_srcdir)/include \
	@GLIB_CFLAGS@

g3d_stat_SOURCES = g3d-stat.c
g3d_stat_DEPENDENCIES =../src/libg3d.la
g3d_stat_LDADD = ../src/libg3d.la -lm @GLIB_LIBS@ @MAGIC_LIBS@
