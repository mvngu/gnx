include doxygen.mk

if DX_COND_html
gnxdoc_HTML = @DX_DOCDIR@/html
$(gnxdoc_HTML): doxygen-doc
html-local:
	make doxygen-doc
endif DX_COND_html

MOSTLYCLEANFILES = $(DX_CLEANFILES)
