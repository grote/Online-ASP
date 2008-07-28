vertex(1..3).

edge(1,2).
edge(2,3).
edge(3,1).

color(red;green;blue).

colored(V,C) :- not colored(V,D), C != D, color(C;D), vertex(V).

:- edge(V,U), colored(V,C), colored(U,C), vertex(V;U), color(C).
