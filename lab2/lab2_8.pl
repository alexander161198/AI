% pass - проход между 2мя комнатами в лабиринте
pass(v1, v2).
pass(v2, v3).
pass(v2, v4).
pass(v3, v4).
pass(v4, v6).
pass(v5, v6).
pass(v4, v7).
pass(v6, v7).
pass(v5, v8).
pass(v7, v8).
pass(v8, v9).
pass(v9, v10).
pass(v6, v10).

link(V1, V2) :- pass(V1, V2); pass(V2, V1).

prolong_way([H|T], [New, H|T]) :- link(H, New), not(member(New, [H|T])).

bfs([[H|T] | _], H, [H|T]).

bfs([H|T], Finish, Way) :- findall(W, prolong_way(H, W), Ways), append(T, Ways, NewWays), bfs(NewWays, Finish, Way).

print([]) :- !.
print([H|T]) :- write(H), write("  lenth = "), list_length(H, L), write(L), nl, print(T).

list_length(Xs,L) :- list_length(Xs,0,L) .

list_length([], L, L).
list_length([_|Xs], T, L) :- T1 is T+1 , list_length(Xs,T1,L).

path(From, To) :- findall(Path, bfs([[From]], To, Path), PathList), print(PathList).
