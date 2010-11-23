#
#	http://docbook.sourceforge.net/release/xsl/current/README
#

HTML=telemetrum-doc.html altosui-doc.html altos.html
PDF=telemetrum-doc.pdf altosui-doc.pdf altos.pdf
DOC=$(HTML) $(PDF)
HTMLSTYLE=/usr/share/xml/docbook/stylesheet/docbook-xsl/html/docbook.xsl
FOSTYLE=/usr/share/xml/docbook/stylesheet/docbook-xsl/fo/docbook.xsl
PDFSTYLE=

.SUFFIXES: .xsl .html .fo .pdf

.xsl.html:
	xsltproc -o $@ $(HTMLSTYLE) $*.xsl

.xsl.fo:
	xsltproc -o $@ $(FOSTYLE) $*.xsl

.fo.pdf:
	fop -fo $*.fo -pdf $@

all:	$(HTML) $(PDF)

publish:	$(DOC)
	cp $(DOC)telemetrum-doc.html home/bdale/web/altusmetrum/TeleMetrum/doc/
	(cd /home/bdale/web/altusmetrum ; echo "update docs" | git commit -F - /home/bdale/web/altusmetrum/TeleMetrum/doc/* ; git push)

clean:
	rm -f *.html *.pdf *.fo

distclean:
	rm -f *.html *.pdf *.fo

indent:		telemetrum-doc.xsl
	xmlindent -i 2 < telemetrum-doc.xsl > telemetrum-doc.new

