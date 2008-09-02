#hide.
#show mv/3.

#base.

% all blocks
bl(1..n).

% all possible locations
loc(t).
loc(X) :- bl(X).

% the initial stack of blocks
on(n,t,0).
on(X,X+1,0) :- bl(X), X < n.

#cumulative k.

% plan generator
1 { mv(X,Y,k) : bl(X) : loc(Y) : X != Y } 1.
moved(X, k) :- mv(X, Y, k).

% move the block
on(X,Y,k) :- mv(X,Y,k).
% inertia
on(X,Y,k) :- on(X,Y,k-1), bl(X), loc(Y), not moved(X, k).

% a block is blocked if there is sth. on top of it
blocked(Y, k) :- bl(Y), on(X, Y, k - 1).

% blocked blocks cannot be moved
 :- mv(X, Y, k), blocked(X, k).
% nothing can be put on a blocked block
 :- mv(X, Y, k), blocked(Y, k).

#volatile k.

% the goal is the reversed stack
 :- not on(X+1,X,k), bl(X), X < n.

#base.
