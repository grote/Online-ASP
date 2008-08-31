#hide.
#show mv(X,Y,Z).

#base.

bl(1..n).
loc(t).
loc(X) :- bl(X).
on(X,X+1,0) :- bl(X), X < n.
on(n,t,0).

#cumulative k.

1{mv(X,Y,k) : bl(X) : loc(Y) : X != Y}1.
on(X,Y,k) :- mv(X,Y,k), bl(X), loc(Y).
on(X,Y,k) :- on(X,Y,k-1), bl(X), loc(Y), not non(X,Y,k).
non(X,Y,k) :- on(X,Z,k), Y != Z, bl(X), loc(Y;Z).
:- mv(X,Y,k), on(Z,X,k-1), bl(X;Z), loc(Y).
:- mv(X,Y,k), on(Z,Y,k-1), bl(X;Y;Z).

#volatile k.

:- not on(X+1,X,k), bl(X), X < n.

#base.
