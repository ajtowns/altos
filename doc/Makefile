#
#	http://docbook.sourceforge.net/release/xsl/current/README
#

all:	telemetrum-doc.html telemetrum-doc.pdf

publish:	all
	cp telemetrum-doc.html \
		telemetrum-doc.pdf /home/bdale/web/altusmetrum/TeleMetrum/doc/
	(cd /home/bdale/web/altusmetrum ; echo "update docs" | git commit -F - /home/bdale/web/altusmetrum/TeleMetrum/doc/* ; git push)


telemetrum-doc.html:	telemetrum-doc.xsl
	xsltproc -o telemetrum-doc.html \
		/usr/share/xml/docbook/stylesheet/docbook-xsl/html/docbook.xsl \
		telemetrum-doc.xsl

telemetrum-doc.fo:	telemetrum-doc.xsl
	xsltproc -o telemetrum-doc.fo \
		/usr/share/xml/docbook/stylesheet/docbook-xsl/fo/docbook.xsl \
		telemetrum-doc.xsl

telemetrum-doc.pdf:	telemetrum-doc.fo
	fop -fo telemetrum-doc.fo -pdf telemetrum-doc.pdf

clean:
	rm -f telemetrum-doc.html telemetrum-doc.pdf telemetrum-doc.fo

distclean:
	rm -f telemetrum-doc.html telemetrum-doc.pdf telemetrum-doc.fo

indent:		telemetrum-doc.xsl
	xmlindent -i 2 < telemetrum-doc.xsl > telemetrum-doc.new

