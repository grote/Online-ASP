#hide.
#show moved(X,Y).

#const blocks=10.

#base.

% the table
loc(0).
% the blocks
block(1..blocks).
% blocks are locations
loc(X) :- block(X).

% initial situation the blocks are stacked
on(X, X - 1, -1) :- block(X).

#lambda(T).

% move one block
1 { mv(X,Y,T) : block(X) : loc(Y) : X != Y } 1.

% mark moved blocks
moved(X, T) :- mv(X,Y,T).

% move the stone
on(X,Y,T) :- mv(X,Y,T).
% all other stones stay where they are
on(X,Y,T) :- block(X), loc(Y), not moved(X, T), on(X,Y,T - 1).

% assert incorrect moves
% only blocks on top can be moved
 :- mv(X, Y, T), on(Z, X, T-1).
% blocks can only be put on topmost blocks
 :- mv(X, Y, T), block(Y), on(Z, Y, T - 1).

#delta(T).
% all blocks have to be on the table
 :- block(X), not on(X, 0, T).

