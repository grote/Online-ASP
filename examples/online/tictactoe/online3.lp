ext_mark(2,1,x,0).
#endstep.
ext_mark(1,1,o,1).
#endstep.
ext_mark(2,2,x,2).
#endstep.
ext_mark(2,0,o,3).
#endstep.
ext_mark(0,2,x,4).
#endstep.
% player x can not win after this move
ext_mark(1,2,o,5).
#stop.
