#
#	http://docbook.sourceforge.net/release/xsl/current/README
#

HTML=altusmetrum.html altos.html telemetry.html
PDF=altusmetrum.pdf altos.pdf telemetry.pdf
DOC=$(HTML) $(PDF)
HTMLSTYLE=/usr/share/xml/docbook/stylesheet/docbook-xsl/html/docbook.xsl
FOSTYLE=/usr/share/xml/docbook/stylesheet/docbook-xsl/fo/docbook.xsl
PDFSTYLE=

.SUFFIXES: .xsl .html .fo .pdf

XSLTFLAGS=--stringparam section.autolabel 1

.xsl.html:
	xsltproc $(XSLTFLAGS) -o $@ $(HTMLSTYLE) $*.xsl

.xsl.fo:
	xsltproc $(XSLTFLAGS) -o $@ $(FOSTYLE) $*.xsl

.fo.pdf:
	fop -fo $*.fo -pdf $@

all:	$(HTML) $(PDF)

install:	all

publish:	$(DOC)
	cp $(DOC) /home/bdale/web/altusmetrum/AltOS/doc/
	(cd /home/bdale/web/altusmetrum ; \
	 git add /home/bdale/web/altusmetrum/AltOS/doc/* ; \
	 echo "update docs" | \
	 git commit -F - /home/bdale/web/altusmetrum/AltOS/doc/* ; \
	 git push)

clean:
	rm -f $(HTML) $(PDF) *.fo

distclean:
	rm -f $(HTML) $(PDF) *.fo

indent:		altusmetrum.xsl
	xmlindent -i 2 < altusmetrum.xsl > altusmetrum.new

