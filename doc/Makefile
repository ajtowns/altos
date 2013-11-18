#
#	http://docbook.sourceforge.net/release/xsl/current/README
#

RELNOTES=\
	release-notes-0.7.1.html \
	release-notes-0.8.html \
	release-notes-0.9.html \
	release-notes-0.9.2.html \
	release-notes-1.0.1.html \
	release-notes-1.1.html \
	release-notes-1.1.1.html \
	release-notes-1.2.html \
	release-notes-1.2.1.html \
	release-notes-1.3.html

RELNOTES_XSL=$(RELNOTES:.html=.xsl)
HTML=altusmetrum.html altos.html telemetry.html companion.html micropeak.html $(RELNOTES)
PDF=altusmetrum.pdf altos.pdf telemetry.pdf companion.pdf micropeak.pdf
DOC=$(HTML) $(PDF)
HTMLSTYLE=/usr/share/xml/docbook/stylesheet/docbook-xsl/html/docbook.xsl
FOSTYLE=xorg-fo.xsl
PDFSTYLE=
IMAGES=telemetrum.svg telemini.svg

.SUFFIXES: .xsl .html .pdf

XSLTFLAGS=--stringparam section.autolabel 1 --xinclude

.xsl.html:
	xsltproc $(XSLTFLAGS) -o $@ $(HTMLSTYLE) $*.xsl

.xsl.pdf:
	xmlto -x $(FOSTYLE) --with-fop pdf $*.xsl

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
	rm -f $(HTML) $(PDF)

distclean:
	rm -f $(HTML) $(PDF)

altusmetrum.html: $(RELNOTES_XSL) $(IMAGES)
altusmetrum.pdf: $(RELNOTES_XSL) $(IMAGES)

$(PDF): $(FOSTYLE)

indent:		altusmetrum.xsl
	xmlindent -i 2 < altusmetrum.xsl > altusmetrum.new

