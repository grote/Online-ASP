% Ricochet Robots (Rasende Roboter) by Alex Randolph
% ---
% http://en.wikipedia.org/wiki/Ricochet_Robot
% http://www.ricochetrobots.com
%
% ASP encoding by Torsten Schaub
%
% --- remarks
%   - (1,1) is north-west corner
%   - boards are specified in terms of
%     - robot positions at time 0:       pos/4
%     - barriers relative to directions: barrier/3
%   - a goal is a single robot position,
%      eg. :- not position(red,6,7,42),
%     given in the respective board description files,
%      eg. board16-1.

%#domain time(T).

%time(1..lastime).

#base.

dim(1..dimension).

robot(red).
robot(blue).
robot(green).
robot(yellow).

direction(north).
direction(east).
direction(south).
direction(west).

% blockages
blocked(I,J,D) :- barrier(I,J,D),direction(D),dim(I;J).

blocked(1..dimension,1           ,north).
blocked(   dimension,1..dimension,east ).
blocked(1..dimension,   dimension,south).
blocked(           1,1..dimension,west ).

blocked(I+1,J,west ) :- blocked(I,J,east ),dim(I;J),I<dimension.
blocked(I,J+1,north) :- blocked(I,J,south),dim(I;J),J<dimension.
blocked(I-1,J,east ) :- blocked(I,J,west ),dim(I;J),I>1.
blocked(I,J-1,south) :- blocked(I,J,north),dim(I;J),J>1.

% this should simplify sth.
dir(west, -1,  0).
dir(east,  1,  0).
dir(north, 0, -1).
dir(south, 0,  1).

#incremental(T).

% which places are reachable by a robot
reachable(R, I + DI, J + DJ, D, T) :- not blocked2(I, J, T), dir(D, DI, DJ), go(R, D, T), position(R, I, J, T - 1), dim(I;J).
reachable(R, I + DI, J + DJ, D, T) :- not blocked2(I, J, T), reachable(R, I, J, D, T), dir(D, DI, DJ), go(R, D, T), dim(I;J).

% no crossing of blockages or robots
blocked2(I, J, T) :- blocked(I, J, D), go(R, D, T).
blocked2(I - DI, J - DJ, T) :- position(R, I, J, T - 1), selectDir(D, T), dir(D, DI, DJ).

% move it (no stopping if there is neither a blockage nor a robot)
position(R, I, J, T) :- reachable(R, I, J, D, T), not reachable(R, I + DI, J + DJ, D, T), dir(D, DI, DJ), go(R, D, T).

% one has to be moved
moved(T) :- reachable(R, I, J, D, T).
 :- not moved(T).

% inertia
position(R, I, J, T) :- position(R, I, J, T - 1), not selectRobot(R, T), robot(R), dim(I;J).

% plan generator
1 { selectRobot(R, T) : robot(R) } 1.
1 { selectDir(D,T)    : direction(D) } 1.

go(R, D, T) :- selectRobot(R, T), selectDir(D,T).

#hide.
%#show moved/4.
#show go/3.
%#show position/4.
%#show reachable/4.

