#hide.
#show push_from(X,Y,K).
#show push_to(X,Y,K).

#base.

% convert my instances
square(X, Y)        :- field(X, Y).
target_square(X, Y) :- target(X, Y).
initial_box(X, Y)   :- stone(X, Y).
initial_at(X, Y)    :- start(X, Y).

#lambda(K).

box(X,Y,0) :- target_square(X,Y), K < 2.

direction(u) :- K < 2.
direction(d) :- K < 2.
direction(r) :- K < 2.
direction(l) :- K < 2.

1{push_to(X,Y,-K) : square(X,Y)}1.
:- push_to(X,Y,-K), not box(X,Y,-K+1), square(X,Y).

1{push_dir(D,-K) : direction(D)}1.
:- push_to(X,Y,-K), push_dir(u,-K), square(X,Y), not square(X,Y-1).
:- push_to(X,Y,-K), push_dir(d,-K), square(X,Y), not square(X,Y+1).
:- push_to(X,Y,-K), push_dir(r,-K), square(X,Y), not square(X-1,Y).
:- push_to(X,Y,-K), push_dir(l,-K), square(X,Y), not square(X+1,Y).

push(X,Y-1,u,-K) :- push_to(X,Y,-K), push_dir(u,-K), square(X,Y).
push(X,Y+1,d,-K) :- push_to(X,Y,-K), push_dir(d,-K), square(X,Y).
push(X-1,Y,r,-K) :- push_to(X,Y,-K), push_dir(r,-K), square(X,Y).
push(X+1,Y,l,-K) :- push_to(X,Y,-K), push_dir(l,-K), square(X,Y).
{push(X,Y-1,u,-K)} :- push(X,Y,u,-K), square(X,Y).
{push(X,Y+1,d,-K)} :- push(X,Y,d,-K), square(X,Y).
{push(X-1,Y,r,-K)} :- push(X,Y,r,-K), square(X,Y).
{push(X+1,Y,l,-K)} :- push(X,Y,l,-K), square(X,Y).

:- push(X,Y,D,-K), box(X,Y,-K+1), square(X,Y), direction(D).
:- push(X,Y,u,-K), square(X,Y), not square(X,Y-1).
:- push(X,Y,d,-K), square(X,Y), not square(X,Y+1).
:- push(X,Y,r,-K), square(X,Y), not square(X-1,Y).
:- push(X,Y,l,-K), square(X,Y), not square(X+1,Y).

push_from(X,Y,-K) :- push(X,Y,u,-K), not push(X,Y-1,u,-K), square(X,Y).
push_from(X,Y,-K) :- push(X,Y,d,-K), not push(X,Y+1,d,-K), square(X,Y).
push_from(X,Y,-K) :- push(X,Y,r,-K), not push(X-1,Y,r,-K), square(X,Y).
push_from(X,Y,-K) :- push(X,Y,l,-K), not push(X+1,Y,l,-K), square(X,Y).

box(X,Y,-K) :- push_from(X,Y,-K), square(X,Y).
box(X,Y,-K) :- box(X,Y,-K+1), not push_to(X,Y,-K), square(X,Y).

at(X,Y-1,-K) :- push_from(X,Y,-K), push_dir(u,-K), square(X,Y).
at(X,Y+1,-K) :- push_from(X,Y,-K), push_dir(d,-K), square(X,Y).
at(X-1,Y,-K) :- push_from(X,Y,-K), push_dir(r,-K), square(X,Y).
at(X+1,Y,-K) :- push_from(X,Y,-K), push_dir(l,-K), square(X,Y).

:- at(X,Y,-K), box(X,Y,-K+1), square(X,Y).

route(X,Y,-K) :- at(X,Y,-K), square(X,Y).
route(X,Y,-K) :- route(X+1,Y,-K), not box(X,Y,-K), square(X,Y).
route(X,Y,-K) :- route(X-1,Y,-K), not box(X,Y,-K), square(X,Y).
route(X,Y,-K) :- route(X,Y+1,-K), not box(X,Y,-K), square(X,Y).
route(X,Y,-K) :- route(X,Y-1,-K), not box(X,Y,-K), square(X,Y).

:- push_to(X,Y,-K), push_dir(u,-K), not route(X,Y-1,-K+1), square(X,Y), K > 1.
:- push_to(X,Y,-K), push_dir(d,-K), not route(X,Y+1,-K+1), square(X,Y), K > 1.
:- push_to(X,Y,-K), push_dir(r,-K), not route(X-1,Y,-K+1), square(X,Y), K > 1.
:- push_to(X,Y,-K), push_dir(l,-K), not route(X+1,Y,-K+1), square(X,Y), K > 1.

:- push_to(X,Y,-K), push_from(X,Y,-K+1), push_dir(D,-K), push_dir(D,-K+1),
   direction(D), square(X,Y), K > 1.

% :- push_from(X,Y1,-K), push_to(X,Y2,-K), push_from(X,Y2,-K+1), push_dir(d,-K+1), not push(X,Y1,d,-K+1),
%    square(X,Y1), square(X,Y2), Y1 < Y2, K > 1.
% :- push_from(X,Y1,-K), push_to(X,Y2,-K), push_from(X,Y2,-K+1), push_dir(u,-K+1), not push(X,Y1,u,-K+1),
%    square(X,Y1), square(X,Y2), Y1 > Y2, K > 1.
% :- push_from(X1,Y,-K), push_to(X2,Y,-K), push_from(X2,Y,-K+1), push_dir(l,-K+1), not push(X1,Y,l,-K+1),
%    square(X1,Y), square(X2,Y), X1 < X2, K > 1.
% :- push_from(X1,Y,-K), push_to(X2,Y,-K), push_from(X2,Y,-K+1), push_dir(r,-K+1), not push(X1,Y,r,-K+1),
%    square(X1,Y), square(X2,Y), X1 > X2, K > 1.

#delta(K).

:- initial_box(X,Y), not box(X,Y,-K).
:- initial_at(X,Y), not route(X,Y,-K).

#base.
