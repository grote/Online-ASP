% Some optimizations

#incremental(T).

% --- forbid redundant return moves
:- go(R,east,T - 1),go(R,west,T).
:- go(R,west,T - 1),go(R,east,T).
:- go(R,south,T - 1),go(R,north,T).
:- go(R,north,T - 1),go(R,south,T).


%:- not position(red,           1,        1,T),time(T). % don't move red    robot
%:- not position(green, dimension,        1,T),time(T). % don't move green  robot
%:- not position(blue,          1,dimension,T),time(T). % don't move blue   robot
%:- not position(yellow,dimension,dimension,T),time(T). % don't move yellow robot
