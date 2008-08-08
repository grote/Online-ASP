#base.

p(0) :- not -p(0).
-p(0) :- not p(0).
:- p(0), -p(0).

#lambda(K).

a(K) :- not -a(K).
-a(K) :- not a(K).
p(K) :- a(K).
p(K) :- p(K-1), not -p(K).
-p(K) :- -p(K-1), not p(K).
:- p(K), -p(K).
:- a(K), -a(K).

#delta(K).

:- not -p(0).
:- not p(K).
:- not -a(K).
