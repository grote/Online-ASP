#base.
p(0).

#incremental(X).
p(X) :- p(X-1).

#volatile(X).
 :- not p(4).

