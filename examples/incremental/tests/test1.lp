#base.
p(0).

#cumulative k.
p(k) :- p(k-1).

#volatile k.
 :- p(4).

