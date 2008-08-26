#const dimension=5.
#const lastime=6.

#base.

position(red,1,1,0).
position(blue,1,dimension,0).
position(green,dimension,1,0).
position(yellow,dimension,dimension,0).

barrier(1,2,south).
barrier(4,3,east).
barrier(2,5,east).

#delta(T).

%:- not position(red,3,5,T).
:- not position(red,4,4,T).
%:- not position(red,4,5,T).
%:- not position(red,1,2,T).
%:- not position(red,4,2,T).
%:- not position(red,4,2,T).
