#
#	http://docbook.sourceforge.net/release/xsl/current/README
#

RELNOTES=\
	release-notes-0.7.1.html \
	release-notes-0.8.html \
	release-notes-0.9.html \
	release-notes-0.9.2.html \
	release-notes-1.0.1.html \
	release-notes-1.1.html

RELNOTES_XSL=$(RELNOTES:.html=.xsl)
HTML=altusmetrum.html altos.html telemetry.html companion.html $(RELNOTES)
PDF=altusmetrum.pdf altos.pdf telemetry.pdf companion.pdf
DOC=$(HTML) $(PDF)
HTMLSTYLE=/usr/share/xml/docbook/stylesheet/docbook-xsl/html/docbook.xsl
FOSTYLE=/usr/share/xml/docbook/stylesheet/docbook-xsl/fo/docbook.xsl
PDFSTYLE=

.SUFFIXES: .xsl .html .fo .pdf

XSLTFLAGS=--stringparam section.autolabel 1 --xinclude

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

altusmetrum.html: $(RELNOTES_XSL)
altusmetrum.fo: $(RELNOTES_XSL)

indent:		altusmetrum.xsl
	xmlindent -i 2 < altusmetrum.xsl > altusmetrum.new

