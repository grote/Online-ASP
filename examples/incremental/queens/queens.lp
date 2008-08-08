#hide.
#show q(X,Y).

#base.

#lambda(K).

dim(K).

{q(X,K) : dim(X)}.
{q(K,Y) : dim(Y) : Y<K}.

:- q(X1,K), q(X2,Y), Y<K, abs(X1-X2)==K-Y, dim(X1), dim(X2), dim(Y).
:- q(K,Y1), q(X,Y2), Y1<K, Y2<K, X<K, K-X==abs(Y1-Y2), dim(X), dim(Y1), dim(Y2).

#delta(K).

% :- not dim(10).

:- {q(X,Y) : dim(Y)}0, dim(X).
:- 2{q(X,Y) : dim(Y)}, dim(X).
:- {q(X,Y) : dim(X)}0, dim(Y).
:- 2{q(X,Y) : dim(X)}, dim(Y).

#base.
