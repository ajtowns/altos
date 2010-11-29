#
#	http://docbook.sourceforge.net/release/xsl/current/README
#

HTML=altusmetrum.html altos.html
PDF=altusmetrum.pdf altos.pdf
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
	cp $(DOC) /home/bdale/web/altusmetrum/AltOS/doc/
	(cd /home/bdale/web/altusmetrum ; \
	 git add /home/bdale/web/altusmetrum/AltOS/doc/* ; \
	 echo "update docs" | \
	 git commit -F - /home/bdale/web/altusmetrum/AltOS/doc/* ; \
	 git push)

clean:
	rm -f *.html *.pdf *.fo

distclean:
	rm -f *.html *.pdf *.fo

indent:		altusmetrum.xsl
	xmlindent -i 2 < altusmetrum.xsl > altusmetrum.new

