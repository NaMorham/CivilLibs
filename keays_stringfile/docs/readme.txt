This directory contains the source needed to build the documentation
for keays::stringfile. In addition to this you will need a docbook 4.2
compatible transformation engine. I'm using openjade under cygwin on 
WindowsXP but this should be possible under other environments as well.

I'm using:
	*Cygwin
	*OpenJade
	*GNU make
	*docbook-dsssl-1.78
	
Hint: http://ourworld.compuserve.com/homepages/hoenicka_markus/ntsgml.html
This might come in handy if you are trying to setup a similar system.

I'm using the following catalog environment variables:
export SGML_CATALOG_FILES="/home/karl/docbook-dsssl-1.78/dtd/docbook.cat:/home/karl/docbook-dsssl-1.78/catalog:/home/karl/docbook-dsssl-1.78/dsssl/catalog"