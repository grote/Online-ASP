#hide.
#show mv(X,Y,Z).

#base.

bl(1..n).
loc(t).
loc(X) :- bl(X).
on(X,X+1,0) :- bl(X), X < n.
on(n,t,0).

#incremental(K).

1{mv(X,Y,K) : bl(X) : loc(Y) : X != Y}1.
on(X,Y,K) :- mv(X,Y,K), bl(X), loc(Y).
on(X,Y,K) :- on(X,Y,K-1), bl(X), loc(Y), not non(X,Y,K).
non(X,Y,K) :- on(X,Z,K), Y != Z, bl(X), loc(Y;Z).
:- mv(X,Y,K), on(Z,X,K-1), bl(X;Z), loc(Y).
:- mv(X,Y,K), on(Z,Y,K-1), bl(X;Y;Z).
#volatile(K).
:- not on(X+1,X,K), bl(X), X < n.

#base.
