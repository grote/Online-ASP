#hide.
#show q/2.

#base.

#lambda(K).
d(K).

{q(X,K)} :- not l(K-1,X), d(X), X<K.
{q(K,Y): d(Y)}1.

l(K,X) :- l(K-1,X), d(X), X<K.
l(K,X) :- q(X,K), d(X), X<K.
l(K,K) :- 1{q(K,Y): d(Y)}.

c(K,Y) :- c(K-1,Y), d(Y), Y<K.
c(K,Y) :- q(K,Y), d(Y), Y<K.
c(K,K) :- 1{q(X,K): d(X)}.

:- q(X,K), q(Z,Y), abs(X-Z)==K-Y, d(X),      d(Y), Y<K, d(Z).
:- q(K,Y), q(X,Z), K-X==abs(Y-Z), d(X), X<K, d(Y), Y<K, d(Z), Z<K.

#delta(K).

:- not l(K,X), d(X).
:- not c(K,Y), d(Y).

