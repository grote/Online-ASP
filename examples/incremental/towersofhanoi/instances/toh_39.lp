#base.

disk(1).
disk(2).
disk(3).
disk(4).
disk(5).
disk(6).
disk(7).
disk(8).
disk(9).

on(9,8,0).
on(8,7,0).
on(7,4,0).
on(4,1,0).
on(6,5,0).
on(5,2,0).

#delta(K).

:- not on(4,3,K).
:- not on(5,4,K).
:- not on(6,5,K).
:- not on(7,6,K).
:- not on(8,7,K).
:- not on(9,8,K).
