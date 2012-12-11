AM_CPPFLAGS = @CPPFLAGS@
AM_LDFLAGS  = @LDFLAGS@
 
bin_PROGRAMS = 2nexml

#noinst_HEADERS = 

2nexml_SOURCES = \
	2nexml.cpp


2nexml_LDADD =  $(LDADD) @TO_NEXML_LIBS@

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	  mv -f 2nexml$(EXEEXT) 2nexml-$(VERSION)$(EXEEXT) && \
	  $(LN_S) 2nexml-$(VERSION)$(EXEEXT) 2nexml$(EXEEXT)

