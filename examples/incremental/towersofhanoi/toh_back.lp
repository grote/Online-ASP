#hide.
#show put(M,N,K).

#lambda(K).

1{on(M,N,0) : disk(N) : M > N}1 :- disk(M), M > 3, K < 2.
 {on(M,N,0) : disk(M) : M > N : M > 3}1 :- disk(N), K < 2.
:- goal_on(M,N), not on(M,N,0), K < 2.

1{move(N,-K) : disk(N) : N > 3}1.
1{from(N,-K) : disk(N)}1.

on(M,N,-K) :- disk(M;N), move(M,-K), from(N,-K), M > 3.
on(M,N,-K) :- disk(M;N), on(M,N,-K+1), not move(M,-K), M > N, M > 3.

:- disk(N), move(N,-K+1), move(N,-K), N > 3.
:- disk(M;N), on(M,N,-K+1), from(N,-K), M > N, M > 3.
:- disk(M;N), on(M,N,-K+1), move(N,-K), M > N, N > 3.
:- disk(M;N), on(M,N,-K), M <= N.

put(M,N,-K) :- disk(M;N), move(M,-K), on(M,N,-K+1).

#delta(K).

:- init_on(M,N), not on(M,N,-K).

#base.
