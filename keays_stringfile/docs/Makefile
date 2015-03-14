all: html rtf

html: keays_stringfile.sgml
	mkdir -p html
	cd html && openjade -t sgml -d /home/karl/docbook-dsssl-1.78/html/docbook.dsl ../keays_stringfile.sgml

rtf: keays_stringfile.sgml
	mkdir -p rtf
	openjade -t rtf -d /home/karl/docbook-dsssl-1.78/print/docbook.dsl -o rtf/keays_stringfile.rtf keays_stringfile.sgml

clean:
	rm -rf html
	rm -rf rtf 
