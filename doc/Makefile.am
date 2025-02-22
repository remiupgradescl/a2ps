# Process this file with automake to create Makefile.in
#
# Copyright 1995-2017 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

## Process this file with automake to produce Makefile.in
info_TEXINFOS = a2ps.texi regex.texi
a2ps_TEXINFOS = a2ps.texi $(generated_texinfos) version.texi

TEXI2DVIFLAGS = -b -c -t "@setchapternewpage odd"

# The a2ps of this package
xa2ps = $(top_builddir)/src/a2ps
defs = $(top_builddir)/tests/defs
A2PS = a2ps

generated_texinfos = \
encoding.texi prologue.texi sheets.texi

MAINTAINERCLEANFILES = $(generated_texinfos)
CLEANFILES = a2ps.tmp a2ps.tps

EXTRA_DIST = $(generated_texinfos) .a2psrc

# This is really the minimum form of dependency...
sheets.texi: $(top_srcdir)/sheets/Makefile.am
	. $(defs) ; LC_ALL= LC_COLLATE=fr \
	$(xa2ps) --list=ssh-texi > sheets.texi || rm sheets.texi

encoding.texi: $(top_srcdir)/encoding/Makefile.am
	. $(defs) ; LC_ALL= LC_COLLATE=fr \
	$(xa2ps) --list=edf-texi > encoding.texi || rm encoding.texi

prologue.texi: $(top_srcdir)/ps/Makefile.am
	. $(defs) ; LC_ALL= LC_COLLATE=fr \
	$(xa2ps) --list=pro-texi > prologue.texi || rm prologue.texi

#
# Various other outputs formats
#
docs: doc-a4 doc-us book-a4 book-us cover-a4 cover-us

usdvi = $(PACKAGE)-$(VERSION).us.dvi
a4dvi = $(PACKAGE)-$(VERSION).a4.dvi

$(a4dvi): a2ps.texi
	$(TEXI2DVI) $(TEXI2DVIFLAGS) -t @afourpaper a2ps.texi
	mv a2ps.dvi $(a4dvi)

$(usdvi): a2ps.texi
	$(TEXI2DVI) $(TEXI2DVIFLAGS) a2ps.texi
	mv a2ps.dvi $(usdvi)

doc-a4: $(a4dvi)
	$(A2PS) -=make-doc -MA4 $(a4dvi)
	gzip -c $(a4dvi) > doc-$(a4dvi).gz

doc-us: $(usdvi)
	$(A2PS) -=make-doc -MLetter $(usdvi)
	gzip -c $(usdvi) > doc-$(usdvi).gz

book-a4: $(a4dvi)
	$(A2PS) -=make-book -MA4 $(a4dvi)

book-us: $(usdvi)
	$(A2PS) -=make-book -MLetter $(usdvi)

cover-a4: $(a4dvi)
	$(A2PS) -=make-cover -MA4 $(a4dvi)

cover-us: $(usdvi)
	$(A2PS) -=make-cover -MLetter $(usdvi)

# Note: I use a2html which puts better background and hlines.
# you can put texi2html without any problems.
html-local: a2ps.texi
	$(MAKEINFO) -E /tmp/a2ps.texi -o /dev/null a2ps.texi
	a2html -expandinfo -split_chapter -number /tmp/a2ps.texi
