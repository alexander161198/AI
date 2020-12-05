%сферические опоры
spherical_support('2.910.01', m6). 
spherical_support('2.910.02', m8). 
spherical_support('3.910.01', m12). 
spherical_support('3.910.02', m16).

%опоры с насечкой
notched_support('2.911.01', m6). 
notched_support('2.911.02', m8). 
notched_support('3.911.01', m12).

%штыри
pin('2.213.01', m6, 6, 8). 
pin('2.213.04', m8, 8, 12). 
pin('3.213.06', m12, 12, 26).

%кулачки с резьбовым отверстием
can_hole('2.913.05', '30_18', m6, 16). 
can_hole('2.913.06', '45_22', m8, 20). 
can_hole('2.913.09', '65_30', m12, 38).

%кулачки с призматическими пазами
cam_prizmatic('2.913.01', '30_18', 10, 8, 12, 3, 7). 
cam_prizmatic('2.913.02', '45_22', 12, 8, 12, 3, 7). 
cam_prizmatic('2.913.07', '65_30', 25, 12, 30, 8, 18).

%кулачковые зажимы
clamp('2.451.01', '45_30', '30_18', 29). 
clamp('2.451.02', '60_45', '45_22', 34). 
clamp('3.451.01', '60_45', '45_22', 35). 
clamp('3.451.02', '90_60', '65_30', 42).

%прокладки
gasket('2.217.01', '45_30', 1). 
gasket('2.217.07', '45_30', 2). 
gasket('2.217.09', '45_30', 3). 
gasket('2.217.10', '45_30', 5).

gasket('3.217.01', '60_45', 1). 
gasket('3.217.07', '60_45', 2). 
gasket('3.217.09', '60_45', 3). 
gasket('3.217.10', '60_45', 5).

gasket('3.107.25', '90_60', 2). 
gasket('3.107.27', '90_60', 3). 
gasket('3.107.28', '90_60', 5).

%определение приспособления как совокупности подвижной части, зажима и пакета прокладок
device(H, Type, D) :- moving_part(Type, D), clamp_device(), gaskets_device(H).

%ПРАВИЛА ПРОЕКТИРОВАНИЯ

%подвижная часть = установочный элемент(поверхность) + кулачок
moving_part(Type, D) :- surface(Type, D), cam_part(D).

%установочные элементы для разных поверхностей
surface(плоск_чист, _) :- spherical_support(Code, DM), b_setval(dm, DM), 
    write('Сферическая опора '), write(Code).
surface(плоск_черн, _) :- notched_support(Code, DM), b_setval(dm, DM), 
    write('Опора с насечкой '), write(Code).
surface(цил_верт, _) :- write('Установочный элемент не используется.'), 
    b_setval(dm, 'vert').
surface(цил_гор, _) :- write('Установочный элемент не используется.'), 
    b_setval(dm, 'hor').
surface(перфор, D) :- pin(Code, DM, Dmin, Dmax), D >= Dmin, D =< Dmax, 
    b_setval(dm, DM), write('Штырь '), write(Code).

%Кулачковая часть
cam_part(D) :- b_getval(dm, Dm), cam_type(Dm, D).

cam_type('vert', D) :- cam_prizmatic(Code, Ssize, H, _, _, Dmin, Dmax), D >= Dmin, D =< Dmax, 
    write('\nКулачок с призматическими пазами '), write(Code), 
    b_setval(height, H), b_setval(ssize, Ssize).
cam_type('hor', D) :- cam_prizmatic(Code, Ssize, H, Dmin, Dmax, _, _), D >= Dmin, D =< Dmax,   
    write('\nКулачок с призматическими пазами '), write(Code), 
    b_setval(height, H), b_setval(ssize, Ssize).
cam_type(DM, _) :- can_hole(Code, Ssize, DM, H), 
    write('\nКулачок с резьбовым отверстием '), write(Code), 
    b_setval(height, H), b_setval(ssize, Ssize), !.

%зажим
clamp_device() :- b_getval(ssize, Scam), clamp(Code, SClamp, Scam, HClamp), 
    write('\nЗажим кулачковый '), write(Code), 
    b_getval(height, H2), Sumheight is HClamp+H2, 
    b_setval(height, Sumheight), b_setval(clampsize, SClamp).

%прокладки
%30 мм - длина плиты => высота прокладок = высота зажима заготовки - (кулачка + зажима) - 30
gaskets_device(H) :- write('\nНабор прокладок '), 
    b_getval(height, CamClapHeight), GHeight is H - CamClapHeight - 30,  
    b_getval(clampsize, Csize), getgasket(GHeight, Csize, 0).
    
getgasket(1,'90_60', _) :- write('\nПодходящих прокладок нет'), !.
getgasket(0, _, _) :- !.
getgasket(Cheight, _, _) :- Cheight < 0, 
    write('\nСлишком маленькая высота'), !, fail.
    %
getgasket(6, Csize, N) :- gasket(Code, Csize, 3), gasket(Code2, Csize, 5), 
    write('\n'), write(N), write(' прокладкок высотой 5 '), write(Code2) , 
    write('\n2 прокладки высотой 3 '), write(Code), !.
getgasket(4, Csize, N) :- gasket(Code, Csize, 2), gasket(Code2, Csize, 5), 
    write('\n'), write(N), write(' прокладки высотой 5 '), write(Code2) , 
    write('\n2 прокладки высотой 2 '), write(Code), !.
getgasket(5, Csize, N) :- gasket(Code, Csize, 5), N2 is N + 1, 
    write('\n'), write(N2), write(' прокладки высотой 5 '), write(Code), !.
getgasket(Cheight, Csize, N) :- gasket(Code, Csize, Cheight), gasket(Code2, Csize, 5), 
    write('\n'), write(N), write(' прокладкок высотой 5 '), write(Code2), 
    write('\n1 прокладка высотой '), write(Cheight), write(' '), write(Code), !.
getgasket(Cheight, Csize, N) :- gasket(_, Csize, 5), 
    Newheight is Cheight - 5, N2 is N + 1, 
    getgasket(Newheight, Csize, N2).
  
  
  
  
    %getgasket(Cheight, _, _) :- Cheight < 0, 
    %write('\nСлишком маленькая высота, должна быть не меньше '), 
    %b_getval(height, Minheight), Out is Minheight + 30, write(Out), !.
