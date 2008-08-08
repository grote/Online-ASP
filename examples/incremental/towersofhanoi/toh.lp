#hide.
#show put(M,N,K).

#lambda(K).

1{move(N,K)  : disk(N) : N > 3}1.
1{where(N,K) : disk(N)}1.

on(M,N,K) :- disk(M;N), move(M,K), where(N,K).
on(M,N,K) :- disk(M;N), on(M,N,K-1), not move(M,K).

:- disk(N), move(N,K-1), move(N,K).
:- disk(M;N), on(M,N,K-1), move(N,K).
:- disk(M;N), on(M,N,K-1), where(N,K).
:- disk(M;N), on(M,N,K), M <= N.

put(M,N,K) :- disk(M;N), move(M,K), where(N,K).
