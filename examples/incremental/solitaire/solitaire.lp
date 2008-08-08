% Solitaire
%
% Original file under Copyright Martin Brain 16/10/06, see COPYING for licence
% Modified on 04/21/08
%

#hide.
#show move(T,D,X,Y).

#base.

% 2x2 squares in the corner aren't used
location(1,3..5).
location(2,3..5).
location(3..5,1..7).
location(6,3..5).
location(7,3..5).

% Moves can be made in one of four directions
direction(up).
direction(down).
direction(left).
direction(right).

% Each location is either full or empty
status(full).
status(empty).

% To start with all except the center is full
state(1,full,1..2,3..5).
state(1,full,3,1..7).
state(1,full,4,1..3).
state(1,empty,4,4).
state(1,full,4,5..7).
state(1,full,5,1..7).
state(1,full,6..7,1..7).

% Note that simply relying on "not status(full)" would give a more ocmpact representation


#lambda(K).

% Can move a full location over a full location to an empty one.
possibleMove(K,up,X,Y) :-  state(K,full,X,Y), state(K,full,X,Y-1), state(K,empty,X,Y-2),
                           location(X,Y),     location(X,Y-1),      location(X,Y-2).

possibleMove(K,down,X,Y) :-  state(K,full,X,Y), state(K,full,X,Y+1), state(K,empty,X,Y+2),
                             location(X,Y),     location(X,Y+1),      location(X,Y+2).

possibleMove(K,left,X,Y) :-  state(K,full,X,Y), state(K,full,X-1,Y), state(K,empty,X-2,Y),
                             location(X,Y),     location(X-1,Y),      location(X-2,Y).

possibleMove(K,right,X,Y) :-  state(K,full,X,Y), state(K,full,X+1,Y), state(K,empty,X+2,Y),
                              location(X,Y),     location(X+1,Y),      location(X+2,Y).


% At each time step choose a move
1{move(K,D,X,Y) : direction(D) : location(X,Y)}1.

% A move must be possible
:- move(K,D,X,Y), not possibleMove(K,D,X,Y), direction(D), location(X,Y).

% Now need to looK at the effect of moves
% (section location parameter to cut grounding size)
state(K+1,empty,X,Y) :- move(K,up,X,Y), location(X,Y).
state(K+1,empty,X,Y-1) :- move(K,up,X,Y), location(X,Y), location(X,Y-1).
state(K+1,full,X,Y-2) :- move(K,up,X,Y), location(X,Y), location(X,Y-2).

state(K+1,empty,X,Y) :- move(K,down,X,Y), location(X,Y).
state(K+1,empty,X,Y+1) :- move(K,down,X,Y), location(X,Y), location(X,Y+1).
state(K+1,full,X,Y+2) :- move(K,down,X,Y), location(X,Y), location(X,Y+2).

state(K+1,empty,X,Y) :- move(K,left,X,Y), location(X,Y).
state(K+1,empty,X-1,Y) :- move(K,left,X,Y), location(X,Y), location(X-1,Y).
state(K+1,full,X-2,Y) :- move(K,left,X,Y), location(X,Y), location(X-2,Y).

state(K+1,empty,X,Y) :- move(K,right,X,Y), location(X,Y).
state(K+1,empty,X+1,Y) :- move(K,right,X,Y), location(X,Y), location(X+1,Y).
state(K+1,full,X+2,Y) :- move(K,right,X,Y), location(X,Y), location(X+2,Y).


changed(K+1,X,Y) :- move(K,up,X,Y), location(X,Y).
changed(K+1,X,Y-1) :- move(K,up,X,Y), location(X,Y), location(X,Y-1).
changed(K+1,X,Y-2) :- move(K,up,X,Y), location(X,Y), location(X,Y-2).

changed(K+1,X,Y) :- move(K,down,X,Y), location(X,Y).
changed(K+1,X,Y+1) :- move(K,down,X,Y), location(X,Y), location(X,Y+1).
changed(K+1,X,Y+2) :- move(K,down,X,Y), location(X,Y), location(X,Y+2).

changed(K+1,X,Y) :- move(K,left,X,Y), location(X,Y).
changed(K+1,X-1,Y) :- move(K,left,X,Y), location(X,Y), location(X-1,Y).
changed(K+1,X-2,Y) :- move(K,left,X,Y), location(X,Y), location(X-2,Y).

changed(K+1,X,Y) :- move(K,right,X,Y), location(X,Y).
changed(K+1,X+1,Y) :- move(K,right,X,Y), location(X,Y), location(X+1,Y).
changed(K+1,X+2,Y) :- move(K,right,X,Y), location(X,Y), location(X+2,Y).

state(K+1,S,X,Y) :- not changed(K+1,X,Y), state(K,S,X,Y), status(S), location(X,Y).

#delta(K).
:- {state(K,empty,X,Y) : location(X,Y)}n-1.

#base.

