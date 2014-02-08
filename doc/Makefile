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
	release-notes-1.3.html \
	release-notes-1.3.1.html \
	release-notes-1.3.2.html

PICTURES=\
	altosui.png \
	ascent.png \
	configure-altimeter.png \
	configure-altosui.png \
	configure-groundstation.png \
	configure-pyro.png \
	descent.png \
	device-selection.png \
	easymini-top.jpg \
	fire-igniter.png \
	graph-configure.png \
	graph-map.png \
	graph.png \
	graph-stats.png \
	landed.png \
	launch-pad.png \
	load-maps.png \
	scan-channels.png \
	site-map.png \
	table.png \
	telemega-v1.0-top.jpg \
	telemetrum-v1.1-thside.jpg \
	telemini-v1-top.jpg \
	telemini-v2-top.jpg
SVG=\
	easymini-outline.svg \
	telemega-outline.svg \
	telemetrum.svg \
	telemini.svg

RELNOTES_XSL=$(RELNOTES:.html=.xsl)
HTML=altusmetrum.html altos.html telemetry.html companion.html micropeak.html $(RELNOTES)
PDF=altusmetrum.pdf altos.pdf telemetry.pdf companion.pdf micropeak.pdf
HTMLSTYLE=/usr/share/xml/docbook/stylesheet/docbook-xsl/html/docbook.xsl
FOSTYLE=xorg-fo.xsl
TEMPLATES=titlepage.templates.xsl
PDFSTYLE=
IMAGES=$(PICTURES) $(SVG)
DOC=$(HTML) $(PDF) $(PICTURES)

.SUFFIXES: .xml .xsl .html .pdf

XSLTFLAGS=--stringparam section.autolabel 1 --xinclude

.xsl.html:
	xsltproc $(XSLTFLAGS) -o $@ $(HTMLSTYLE) $*.xsl

.xsl.pdf:
	xmlto -x $(FOSTYLE) --with-fop pdf $*.xsl

.xml.xsl:
	xsltproc --output $@ /usr/share/xml/docbook/stylesheet/docbook-xsl/template/titlepage.xsl $*.xml

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
	rm -f $(HTML) $(PDF) $(TEMPLATES)

distclean:
	rm -f $(HTML) $(PDF)

altusmetrum.html: $(RELNOTES_XSL) $(IMAGES)
altusmetrum.pdf: $(RELNOTES_XSL) $(IMAGES)

$(PDF): $(FOSTYLE) $(TEMPLATES)

indent:		altusmetrum.xsl
	xmlindent -i 2 < altusmetrum.xsl > altusmetrum.new

