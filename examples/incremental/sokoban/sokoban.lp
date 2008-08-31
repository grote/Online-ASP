#hide.
#show push_from(X,Y,D,K).
#show push_to(X,Y,D,K).

#base.

% convert my instances
square(X, Y)        :- field(X, Y).
target_square(X, Y) :- target(X, Y).
initial_box(X, Y)   :- stone(X, Y).
initial_at(X, Y)    :- start(X, Y).

direction(u).
direction(d).
direction(r).
direction(l).

route(X,Y,0) :- initial_at(X,Y).
route(X,Y,0) :- route(X+1,Y,0), not box(X,Y,0), square(X,Y).
route(X,Y,0) :- route(X-1,Y,0), not box(X,Y,0), square(X,Y).
route(X,Y,0) :- route(X,Y+1,0), not box(X,Y,0), square(X,Y).
route(X,Y,0) :- route(X,Y-1,0), not box(X,Y,0), square(X,Y).

box(X,Y,0) :- initial_box(X,Y).

#cumulative k.

route(X,Y,k) :- at(X,Y,k), square(X,Y).
route(X,Y,k) :- route(X+1,Y,k), not box(X,Y,k), square(X,Y).
route(X,Y,k) :- route(X-1,Y,k), not box(X,Y,k), square(X,Y).
route(X,Y,k) :- route(X,Y+1,k), not box(X,Y,k), square(X,Y).
route(X,Y,k) :- route(X,Y-1,k), not box(X,Y,k), square(X,Y).

1{push_from(X,Y,D,k) : square(X,Y) : direction(D)}1.
:- push_from(X,Y,D,k), not route(X,Y,k-1), square(X,Y), direction(D).
:- push_from(X,Y,u,k), not box(X,Y+1,k-1), square(X,Y).
:- push_from(X,Y,d,k), not box(X,Y-1,k-1), square(X,Y).
:- push_from(X,Y,r,k), not box(X+1,Y,k-1), square(X,Y).
:- push_from(X,Y,l,k), not box(X-1,Y,k-1), square(X,Y).

push(X,Y,D,k) :- push_from(X,Y,D,k), square(X,Y), direction(D).
{push(X,Y+1,u,k)} :- push(X,Y,u,k), square(X,Y).
{push(X,Y-1,d,k)} :- push(X,Y,d,k), square(X,Y).
{push(X+1,Y,r,k)} :- push(X,Y,r,k), square(X,Y).
{push(X-1,Y,l,k)} :- push(X,Y,l,k), square(X,Y).
:- push(X,Y,u,k), not square(X,Y+2), square(X,Y).
:- push(X,Y,d,k), not square(X,Y-2), square(X,Y).
:- push(X,Y,r,k), not square(X+2,Y), square(X,Y).
:- push(X,Y,l,k), not square(X-2,Y), square(X,Y).
:- push(X,Y,u,k), box(X,Y+2,k-1), square(X,Y).
:- push(X,Y,d,k), box(X,Y-2,k-1), square(X,Y).
:- push(X,Y,r,k), box(X+2,Y,k-1), square(X,Y).
:- push(X,Y,l,k), box(X-2,Y,k-1), square(X,Y).

push_to(X,Y,u,k) :- push(X,Y-1,u,k), not push(X,Y,u,k), square(X,Y).
push_to(X,Y,d,k) :- push(X,Y+1,d,k), not push(X,Y,d,k), square(X,Y).
push_to(X,Y,r,k) :- push(X-1,Y,r,k), not push(X,Y,r,k), square(X,Y).
push_to(X,Y,l,k) :- push(X+1,Y,l,k), not push(X,Y,l,k), square(X,Y).

at(X,Y,k) :- push_to(X,Y,D,k), square(X,Y), direction(D).

box(X,Y+2,k) :- push(X,Y,u,k), not push(X,Y+1,u,k), square(X,Y).
box(X,Y-2,k) :- push(X,Y,d,k), not push(X,Y-1,d,k), square(X,Y).
box(X+2,Y,k) :- push(X,Y,r,k), not push(X+1,Y,r,k), square(X,Y).
box(X-2,Y,k) :- push(X,Y,l,k), not push(X-1,Y,l,k), square(X,Y).
box(X,Y,k) :- box(X,Y,k-1), square(X,Y),
              not push_from(X,Y-1,u,k), not push_from(X,Y+1,d,k), 
              not push_from(X-1,Y,r,k), not push_from(X+1,Y,l,k).

:- push_to(X,Y,u,k-1), push_from(X,Y,u,k), square(X,Y).
:- push_to(X,Y,d,k-1), push_from(X,Y,d,k), square(X,Y).
:- push_to(X,Y,r,k-1), push_from(X,Y,r,k), square(X,Y).
:- push_to(X,Y,l,k-1), push_from(X,Y,l,k), square(X,Y).

#volatile k.
:- target_square(X,Y), not box(X,Y,k).

#base.

