#hide.
#show place(X, Y, N, C1, C2, C3, C4).

#base.

stones(1..256).
field(1..16, 1..16).

%%%%%%%%%%%%%%%%%% generate %%%%%%%%%%%%%%%%%%%%%%

% all possible rotations of a piece
stone(N, C1, C2, C3, C4) :- piece(N, C1, C2, C3, C4).
stone(N, C1, C2, C3, C4) :- piece(N, C2, C3, C4, C1).
stone(N, C1, C2, C3, C4) :- piece(N, C3, C4, C1, C2).
stone(N, C1, C2, C3, C4) :- piece(N, C4, C1, C2, C3).

% choose every piece exactly 1 time
1 { place(X, Y, N, C1, C2, C3, C4) : stone(N, C1, C2, C3, C4) : field(X, Y) } 1 :- stones(N).

% choose a piece for every field
1 { place(X, Y, N, C1, C2, C3, C4) : stone(N, C1, C2, C3, C4) } 1 :- field(X, Y).

% the left/right/top/bottom colors of a selected place
colL(X,Y,C1) :- place(X, Y, N, C1, C2, C3, C4).
colR(X,Y,C3) :- place(X, Y, N, C1, C2, C3, C4).
colT(X,Y,C2) :- place(X, Y, N, C1, C2, C3, C4).
colB(X,Y,C4) :- place(X, Y, N, C1, C2, C3, C4).

%%%%%%%%%%%%%%%%%% test %%%%%%%%%%%%%%%%%%%%%%

#cumulative t.
check(t, t, 1..t).
check(t, 1..t, t).

 :- check(t,X,Y), colL(X,Y,C1), colR(X+1,Y,C2), C1 != C2.
 :- check(t,X,Y), colT(X,Y,C1), colB(X,Y+1,C2), C1 != C2.

#base.

 :- colL(0,Y,C), C != 0.
 :- colR(16,Y,C), C != 0.
 :- colT(X,0,C), C != 0.
 :- colB(X,16,C), C != 0.

%%%%%%%%%%%%%%%%%% instance %%%%%%%%%%%%%%%%%%%%%%

piece(1, 1, 17, 0, 0).
piece(2, 1, 5, 0, 0).
piece(3, 9, 17, 0, 0).
piece(4, 17, 9, 0, 0).
piece(5, 2, 1, 0, 1).
piece(6, 10, 9, 0, 1).
piece(7, 6, 1, 0, 1).
piece(8, 6, 13, 0, 1).
piece(9, 11, 17, 0, 1).
piece(10, 7, 5, 0, 1).
piece(11, 15, 9, 0, 1).
piece(12, 8, 5, 0, 1).
piece(13, 8, 13, 0, 1).
piece(14, 21, 5, 0, 1).
piece(15, 10, 1, 0, 9).
piece(16, 18, 17, 0, 9).
piece(17, 14, 13, 0, 9).
piece(18, 19, 13, 0, 9).
piece(19, 7, 9, 0, 9).
piece(20, 15, 9, 0, 9).
piece(21, 4, 5, 0, 9).
piece(22, 12, 1, 0, 9).
piece(23, 12, 13, 0, 9).
piece(24, 20, 1, 0, 9).
piece(25, 21, 1, 0, 9).
piece(26, 2, 9, 0, 17).
piece(27, 2, 17, 0, 17).
piece(28, 10, 17, 0, 17).
piece(29, 18, 17, 0, 17).
piece(30, 7, 13, 0, 17).
piece(31, 15, 9, 0, 17).
piece(32, 20, 17, 0, 17).
piece(33, 8, 9, 0, 17).
piece(34, 8, 5, 0, 17).
piece(35, 16, 13, 0, 17).
piece(36, 22, 5, 0, 17).
piece(37, 18, 1, 0, 5).
piece(38, 3, 13, 0, 5).
piece(39, 11, 13, 0, 5).
piece(40, 19, 9, 0, 5).
piece(41, 19, 17, 0, 5).
piece(42, 15, 1, 0, 5).
piece(43, 15, 9, 0, 5).
piece(44, 15, 17, 0, 5).
piece(45, 4, 1, 0, 5).
piece(46, 20, 5, 0, 5).
piece(47, 8, 5, 0, 5).
piece(48, 16, 5, 0, 5).
piece(49, 2, 13, 0, 13).
piece(50, 10, 1, 0, 13).
piece(51, 10, 9, 0, 13).
piece(52, 6, 1, 0, 13).
piece(53, 7, 5, 0, 13).
piece(54, 4, 5, 0, 13).
piece(55, 4, 13, 0, 13).
piece(56, 8, 17, 0, 13).
piece(57, 16, 1, 0, 13).
piece(58, 16, 13, 0, 13).
piece(59, 21, 9, 0, 13).
piece(60, 22, 17, 0, 13).
piece(61, 6, 18, 2, 2).
piece(62, 14, 7, 2, 2).
piece(63, 10, 3, 2, 10).
piece(64, 2, 8, 2, 18).
piece(65, 18, 22, 2, 18).
piece(66, 14, 14, 2, 18).
piece(67, 11, 10, 2, 18).
piece(68, 20, 6, 2, 18).
piece(69, 22, 8, 2, 18).
piece(70, 3, 7, 2, 3).
piece(71, 7, 12, 2, 3).
piece(72, 14, 18, 2, 11).
piece(73, 15, 4, 2, 11).
piece(74, 20, 15, 2, 11).
piece(75, 8, 3, 2, 11).
piece(76, 14, 15, 2, 19).
piece(77, 19, 15, 2, 19).
piece(78, 3, 16, 2, 7).
piece(79, 20, 3, 2, 7).
piece(80, 16, 21, 2, 7).
piece(81, 19, 18, 2, 15).
piece(82, 18, 18, 2, 4).
piece(83, 11, 4, 2, 4).
piece(84, 18, 19, 2, 12).
piece(85, 6, 14, 2, 12).
piece(86, 8, 12, 2, 12).
piece(87, 16, 20, 2, 12).
piece(88, 2, 21, 2, 20).
piece(89, 6, 22, 2, 20).
piece(90, 4, 16, 2, 20).
piece(91, 11, 12, 2, 8).
piece(92, 19, 15, 2, 8).
piece(93, 19, 4, 2, 8).
piece(94, 4, 21, 2, 8).
piece(95, 12, 14, 2, 8).
piece(96, 21, 3, 2, 21).
piece(97, 4, 19, 2, 22).
piece(98, 20, 8, 2, 22).
piece(99, 21, 6, 2, 22).
piece(100, 22, 21, 2, 22).
piece(101, 12, 15, 10, 10).
piece(102, 12, 16, 10, 10).
piece(103, 16, 19, 10, 10).
piece(104, 22, 6, 10, 10).
piece(105, 4, 15, 10, 18).
piece(106, 3, 8, 10, 6).
piece(107, 19, 8, 10, 6).
piece(108, 4, 15, 10, 6).
piece(109, 16, 11, 10, 6).
piece(110, 15, 12, 10, 14).
piece(111, 12, 15, 10, 14).
piece(112, 20, 19, 10, 3).
piece(113, 20, 16, 10, 3).
piece(114, 14, 4, 10, 11).
piece(115, 7, 12, 10, 11).
piece(116, 12, 11, 10, 11).
piece(117, 22, 16, 10, 11).
piece(118, 3, 21, 10, 19).
piece(119, 16, 12, 10, 7).
piece(120, 8, 22, 10, 15).
piece(121, 14, 22, 10, 4).
piece(122, 6, 16, 10, 20).
piece(123, 14, 19, 10, 20).
piece(124, 20, 15, 10, 20).
piece(125, 12, 22, 10, 8).
piece(126, 21, 15, 10, 8).
piece(127, 14, 6, 10, 16).
piece(128, 19, 21, 10, 16).
piece(129, 4, 3, 10, 16).
piece(130, 20, 8, 10, 16).
piece(131, 6, 20, 10, 21).
piece(132, 12, 14, 10, 21).
piece(133, 14, 16, 10, 22).
piece(134, 11, 4, 10, 22).
piece(135, 4, 3, 10, 22).
piece(136, 16, 20, 10, 22).
piece(137, 20, 7, 18, 18).
piece(138, 6, 3, 18, 6).
piece(139, 6, 11, 18, 6).
piece(140, 6, 12, 18, 6).
piece(141, 19, 21, 18, 6).
piece(142, 15, 6, 18, 6).
piece(143, 16, 12, 18, 6).
piece(144, 21, 21, 18, 6).
piece(145, 3, 4, 18, 14).
piece(146, 18, 12, 18, 3).
piece(147, 18, 22, 18, 3).
piece(148, 3, 14, 18, 3).
piece(149, 15, 12, 18, 3).
piece(150, 6, 11, 18, 19).
piece(151, 4, 22, 18, 19).
piece(152, 11, 11, 18, 7).
piece(153, 11, 19, 18, 7).
piece(154, 22, 16, 18, 7).
piece(155, 7, 7, 18, 4).
piece(156, 7, 12, 18, 4).
piece(157, 22, 7, 18, 4).
piece(158, 7, 16, 18, 20).
piece(159, 8, 6, 18, 20).
piece(160, 21, 21, 18, 8).
piece(161, 6, 20, 18, 16).
piece(162, 14, 20, 18, 16).
piece(163, 15, 11, 18, 22).
piece(164, 4, 16, 18, 22).
piece(165, 3, 4, 6, 14).
piece(166, 4, 8, 6, 14).
piece(167, 3, 3, 6, 11).
piece(168, 11, 15, 6, 19).
piece(169, 19, 21, 6, 19).
piece(170, 4, 8, 6, 7).
piece(171, 20, 16, 6, 7).
piece(172, 21, 11, 6, 7).
piece(173, 15, 15, 6, 15).
piece(174, 12, 20, 6, 15).
piece(175, 7, 21, 6, 4).
piece(176, 7, 19, 6, 12).
piece(177, 14, 4, 6, 20).
piece(178, 12, 16, 6, 8).
piece(179, 8, 15, 6, 8).
piece(180, 7, 16, 6, 16).
piece(181, 11, 16, 6, 21).
piece(182, 7, 11, 6, 21).
piece(183, 19, 8, 14, 14).
piece(184, 22, 7, 14, 3).
piece(185, 19, 12, 14, 11).
piece(186, 8, 8, 14, 11).
piece(187, 21, 7, 14, 19).
piece(188, 14, 21, 14, 7).
piece(189, 3, 19, 14, 7).
piece(190, 16, 19, 14, 7).
piece(191, 3, 3, 14, 15).
piece(192, 15, 20, 14, 15).
piece(193, 11, 7, 14, 4).
piece(194, 21, 11, 14, 12).
piece(195, 21, 22, 14, 12).
piece(196, 22, 15, 14, 12).
piece(197, 11, 22, 14, 20).
piece(198, 19, 8, 14, 20).
piece(199, 20, 20, 14, 20).
piece(200, 19, 3, 14, 8).
piece(201, 21, 8, 14, 16).
piece(202, 22, 7, 14, 16).
piece(203, 12, 19, 14, 21).
piece(204, 12, 8, 14, 21).
piece(205, 16, 3, 14, 21).
piece(206, 22, 21, 14, 21).
piece(207, 22, 7, 3, 3).
piece(208, 19, 22, 3, 11).
piece(209, 8, 15, 3, 11).
piece(210, 11, 19, 3, 7).
piece(211, 16, 15, 3, 7).
piece(212, 3, 16, 3, 15).
piece(213, 8, 8, 3, 4).
piece(214, 3, 20, 3, 12).
piece(215, 4, 22, 3, 12).
piece(216, 22, 21, 3, 12).
piece(217, 19, 15, 3, 20).
piece(218, 4, 12, 3, 16).
piece(219, 11, 4, 3, 21).
piece(220, 11, 16, 3, 22).
piece(221, 21, 21, 3, 22).
piece(222, 21, 22, 3, 22).
piece(223, 12, 22, 11, 11).
piece(224, 20, 7, 11, 11).
piece(225, 16, 15, 11, 11).
piece(226, 19, 15, 11, 7).
piece(227, 12, 12, 11, 7).
piece(228, 19, 8, 11, 4).
piece(229, 7, 22, 11, 20).
piece(230, 16, 8, 11, 20).
piece(231, 12, 20, 11, 8).
piece(232, 12, 21, 11, 8).
piece(233, 19, 20, 19, 19).
piece(234, 16, 4, 19, 7).
piece(235, 7, 4, 19, 4).
piece(236, 7, 20, 19, 4).
piece(237, 12, 15, 19, 4).
piece(238, 4, 16, 19, 12).
piece(239, 15, 22, 19, 20).
piece(240, 21, 15, 19, 20).
piece(241, 7, 21, 19, 8).
piece(242, 4, 21, 19, 8).
piece(243, 15, 12, 7, 15).
piece(244, 20, 8, 7, 15).
piece(245, 22, 20, 7, 4).
piece(246, 16, 22, 7, 21).
piece(247, 21, 22, 15, 15).
piece(248, 12, 4, 15, 4).
piece(249, 4, 21, 15, 12).
piece(250, 16, 21, 15, 20).
piece(251, 22, 8, 4, 4).
piece(252, 8, 12, 4, 12).
piece(253, 16, 20, 12, 8).
piece(254, 21, 16, 20, 16).
piece(255, 16, 22, 20, 22).
piece(256, 21, 22, 8, 22).


