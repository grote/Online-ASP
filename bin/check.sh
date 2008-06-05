#!/bin/bash

# this belongs by far to the most evil shell scripts i have ever written :)

function symtab()
{
	begin=$[$(cat $1 | grep -n "^0$" | head -n 1 | grep -o "^[0-9]*") + 1]
	end=$[$(tail -n +$begin $1 | grep -n "^0$" | head -n 1 | grep -o "^[0-9]*") - 1]
	cat $1 | tail -n +$begin | head -n $end | sed -e "s/[0-9]* //" | sort > $1.symtab
}

lparse $* > compare.lparse
gringo2 -l $* > compare.gringo

symtab compare.lparse
symtab compare.gringo

diff --suppress-common-lines compare.gringo.symtab compare.lparse.symtab | grep "> " | sed -e 's/>//' -e 's/$/$/' > diff1.txt
diff --suppress-common-lines compare.gringo.symtab compare.lparse.symtab | grep "< " | sed -e 's/<//' -e 's/$/$/' > diff2.txt

if [ -s diff1.txt ]; then
	grep -v -f diff1.txt compare.lparse > compare.lparse.reduced
else
	cp compare.lparse compare.lparse.reduced
fi

if [ -s diff2.txt ]; then
	grep -v -f diff2.txt compare.gringo > compare.gringo.reduced
else
	cp compare.gringo compare.gringo.reduced
fi

lpeq compare.gringo.reduced compare.lparse.reduced > compare1.lp
lpeq compare.lparse.reduced compare.gringo.reduced > compare2.lp

echo "first comparison"
clasp < compare1.lp
echo "second comparison"
clasp < compare2.lp

#rm compare.lparse compare.lparse.reduced compare.lparse.symtab 
#rm compare.gringo compare.gringo.reduced compare.gringo.symtab 
#rm compare1.lp compare2.lp
#rm diff1.txt diff2.txt
