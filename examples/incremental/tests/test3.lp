#base.
#incremental(K).
d(K).
K { q(X) : d(X) } K. 
#volatile(K).
:- not d(5).
