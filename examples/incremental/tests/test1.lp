#base.
p(0).

#lambda(X).
p(X) :- p(X-1).

#delta(X).
 :- not p(4).

