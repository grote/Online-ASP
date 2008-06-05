#!/bin/bash

lparse $* > compare.lparse
gringo2 -l $* > compare.gringo

awk '
func seek0(fin, fout) {
	do {
		getline l < fin
		print l > fout
	} while( l != "0" )
}

func symtab(fin, fout, a, ka) {
	j = 1
	do {
		getline l < fin
		if( l != "0" ) {
			split(l, s, " ")
			a[s[2]] = s[1]
			ka[++j] = s[2]
		}
	} while( l != "0" )
	asort(ka)
	return j - 1
}

func rest(fin, fout) {
	while ((getline l < fin) > 0) {
		print l > fout
	}
}

func intersect(a, ka, la, fa, b, kb, lb, fb)
{
	ia = 1;
	ib = 1;
	while (ia <= la && ib <= lb) {
		na = ka[ia]
		nb = kb[ib]
		if (na == nb) {
			print a[na] " " na > fa
			print b[nb] " " nb > fb
			ia++;
			ib++;
		} else if (na > nb) {
			ib++;
		} else {
			ia++;
		}
	}
	print "0" > fa
	print "0" > fb
}

BEGIN {
	seek0("compare.lparse", "compare.lparse.reduced")
	seek0("compare.gringo", "compare.gringo.reduced")
	la = symtab("compare.lparse", "compare.lparse.reduced", a, ka)
	lb = symtab("compare.gringo", "compare.gringo.reduced", b, kb)
	intersect(a, ka, la, "compare.lparse.reduced", b, kb, lb, "compare.gringo.reduced")
	rest("compare.lparse", "compare.lparse.reduced")
	rest("compare.gringo", "compare.gringo.reduced")
}
'

lpeq compare.gringo.reduced compare.lparse.reduced > compare1.lp
lpeq compare.lparse.reduced compare.gringo.reduced > compare2.lp

echo "first comparison"
clasp < compare1.lp
echo "second comparison"
clasp < compare2.lp

rm compare.lparse compare.lparse.reduced 
rm compare.gringo compare.gringo.reduced 
rm compare1.lp compare2.lp

