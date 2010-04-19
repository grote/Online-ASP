#base.
% step size for agent
step(-1..1).
#hide step/1.

% possible and invalid locations
location(loc(X,Y)) :- not invalid(loc(X,Y)), grid(X,Y). 
invalid(loc(X,Y)) :- wall(X,Y).
location(noloc).
#hide location/1, invalid/1.

% possible moves
move(X,Y) :- step(X;Y), #abs(X) != #abs(Y).
#hide move/2.

% initial position
position(L, 0) :- sense(S, L, 0).
%#hide sense/3.

#cumulative t.

% extrapolate new location of agent
1 [ position(loc(X+U, Y+V), t): location(loc(X+U, Y+V)) : move(U, V) ] 1 :- position(loc(X, Y), t-1), location(loc(X, Y)).

% sensor data is trustworthy
:- sense(S, L, t), not position(L, t), S!=nodata. 

#external sense(S, L, t) : sensor(S) : location(L).

#volatile t.

% Don't stop at t if there are readings at T>t
:- sense(S, L, T), T>t.
