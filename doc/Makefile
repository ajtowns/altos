#
#	http://docbook.sourceforge.net/release/xsl/current/README
#

all:	telemetrum.html telemetrum.pdf

telemetrum.html:	telemetrum.xsl
	xsltproc -o telemetrum.html \
		/usr/share/xml/docbook/stylesheet/nwalsh/html/docbook.xsl \
		telemetrum.xsl

telemetrum.fo:	telemetrum.xsl
	xsltproc -o telemetrum.fo \
		/usr/share/xml/docbook/stylesheet/nwalsh/fo/docbook.xsl \
		telemetrum.xsl

telemetrum.pdf:	telemetrum.fo
	fop -fo telemetrum.fo -pdf telemetrum.pdf

clean:
	rm -f telemetrum.html telemetrum.pdf telemetrum.fo

indent:		telemetrum.xsl
	xmlindent -i 2 < telemetrum.xsl > telemetrum.new

