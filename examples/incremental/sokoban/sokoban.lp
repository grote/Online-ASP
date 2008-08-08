#hide.
#show push_from(X,Y,D,K).
#show push_to(X,Y,D,K).

#lambda(K).

direction(u) :- K<2.
direction(d) :- K<2.
direction(r) :- K<2.
direction(l) :- K<2.

route(X,Y,0) :- initial_at(X,Y), K<2.
route(X,Y,0) :- route(X+1,Y,0), not box(X,Y,0), square(X,Y), K<2.
route(X,Y,0) :- route(X-1,Y,0), not box(X,Y,0), square(X,Y), K<2.
route(X,Y,0) :- route(X,Y+1,0), not box(X,Y,0), square(X,Y), K<2.
route(X,Y,0) :- route(X,Y-1,0), not box(X,Y,0), square(X,Y), K<2.
route(X,Y,K) :- at(X,Y,K), square(X,Y).
route(X,Y,K) :- route(X+1,Y,K), not box(X,Y,K), square(X,Y).
route(X,Y,K) :- route(X-1,Y,K), not box(X,Y,K), square(X,Y).
route(X,Y,K) :- route(X,Y+1,K), not box(X,Y,K), square(X,Y).
route(X,Y,K) :- route(X,Y-1,K), not box(X,Y,K), square(X,Y).

1{push_from(X,Y,D,K) : square(X,Y) : direction(D)}1.
:- push_from(X,Y,D,K), not route(X,Y,K-1), square(X,Y), direction(D).
:- push_from(X,Y,u,K), not box(X,Y+1,K-1), square(X,Y).
:- push_from(X,Y,d,K), not box(X,Y-1,K-1), square(X,Y).
:- push_from(X,Y,r,K), not box(X+1,Y,K-1), square(X,Y).
:- push_from(X,Y,l,K), not box(X-1,Y,K-1), square(X,Y).

push(X,Y,D,K) :- push_from(X,Y,D,K), square(X,Y), direction(D).
{push(X,Y+1,u,K)} :- push(X,Y,u,K), square(X,Y).
{push(X,Y-1,d,K)} :- push(X,Y,d,K), square(X,Y).
{push(X+1,Y,r,K)} :- push(X,Y,r,K), square(X,Y).
{push(X-1,Y,l,K)} :- push(X,Y,l,K), square(X,Y).
:- push(X,Y,u,K), not square(X,Y+2), square(X,Y).
:- push(X,Y,d,K), not square(X,Y-2), square(X,Y).
:- push(X,Y,r,K), not square(X+2,Y), square(X,Y).
:- push(X,Y,l,K), not square(X-2,Y), square(X,Y).
:- push(X,Y,u,K), box(X,Y+2,K-1), square(X,Y).
:- push(X,Y,d,K), box(X,Y-2,K-1), square(X,Y).
:- push(X,Y,r,K), box(X+2,Y,K-1), square(X,Y).
:- push(X,Y,l,K), box(X-2,Y,K-1), square(X,Y).

push_to(X,Y,u,K) :- push(X,Y-1,u,K), not push(X,Y,u,K), square(X,Y).
push_to(X,Y,d,K) :- push(X,Y+1,d,K), not push(X,Y,d,K), square(X,Y).
push_to(X,Y,r,K) :- push(X-1,Y,r,K), not push(X,Y,r,K), square(X,Y).
push_to(X,Y,l,K) :- push(X+1,Y,l,K), not push(X,Y,l,K), square(X,Y).

at(X,Y,K) :- push_to(X,Y,D,K), square(X,Y), direction(D).

box(X,Y,0) :- initial_box(X,Y), K<2.
box(X,Y+2,K) :- push(X,Y,u,K), not push(X,Y+1,u,K), square(X,Y).
box(X,Y-2,K) :- push(X,Y,d,K), not push(X,Y-1,d,K), square(X,Y).
box(X+2,Y,K) :- push(X,Y,r,K), not push(X+1,Y,r,K), square(X,Y).
box(X-2,Y,K) :- push(X,Y,l,K), not push(X-1,Y,l,K), square(X,Y).
box(X,Y,K) :- box(X,Y,K-1), square(X,Y),
              not push_from(X,Y-1,u,K), not push_from(X,Y+1,d,K), 
              not push_from(X-1,Y,r,K), not push_from(X+1,Y,l,K).

:- push_to(X,Y,u,K-1), push_from(X,Y,u,K), square(X,Y).
:- push_to(X,Y,d,K-1), push_from(X,Y,d,K), square(X,Y).
:- push_to(X,Y,r,K-1), push_from(X,Y,r,K), square(X,Y).
:- push_to(X,Y,l,K-1), push_from(X,Y,l,K), square(X,Y).

:- push_from(X,Y,u,K-1), push_to(X,Y1,u,K-1), push_from(X,Y1+2,d,K),
   not push(X,Y+1,d,K), square(X,Y), square(X,Y1), Y1>Y.
:- push_from(X,Y,d,K-1), push_to(X,Y1,d,K-1), push_from(X,Y1-2,u,K),
   not push(X,Y-1,u,K), square(X,Y), square(X,Y1), Y1<Y.
:- push_from(X,Y,r,K-1), push_to(X1,Y,r,K-1), push_from(X1+2,Y,l,K),
   not push(X+1,Y,l,K), square(X,Y), square(X1,Y), X1>X.
:- push_from(X,Y,l,K-1), push_to(X1,Y,l,K-1), push_from(X1-2,Y,r,K),
   not push(X-1,Y,r,K), square(X,Y), square(X1,Y), X1<X.

#delta(K).
:- target_square(X,Y), not box(X,Y,K).

#base.

