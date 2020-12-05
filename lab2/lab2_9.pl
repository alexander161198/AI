/*
53. Дано алгебраическое уравнение над целыми числами, в котором знаки операций (+, -, *, /) заменены буквами (например, Z, Y, X, ...). Написать универсальную программу, отыскивающую подстановку для букв, превращающую уравнение в тождество. Пример.
Уравнение: (3 Z 5) Y 12 = 27
Подстановка: Z = *, Y = + 
*/

solver(String) :- parse(String, Left, Ans), string_codes(Left, List),  copy_term(List, Nlist), replacement(List, Nlist, Out), check_zero(Out), string_codes(PossibleLeft, Out), correct_ans(PossibleLeft, Ans).

%проверка деления на 0
check_zero([H|T]) :- H\=47, check_zero(T).
check_zero([H|T]) :- H==47, next_to(T, Num), Num \= 48, check_zero(T).
check_zero([]).

next_to([H|_], Num) :- Num is H.

%разделение на левую часть и ответ
parse(String, Expr, Answer) :- split_string(String, "=", "", [Expr|ListEnd]), num_from_array(ListEnd, AnsAtom), Answer is AnsAtom.

num_from_array([H|_], Num) :- term_to_atom(Num, H).

%замена букв на знаки
replacement(Input, Output, Noutput) :- length(Input, Len), Len > 0, !, last(Input, Lastelem),
    selection(Output, Lastelem, New), delete(Input, Lastelem, Deleted), replacement(Deleted, New, Noutput), copy(New, Noutput).
replacement(_, _, _).

selection(A, B, Out) :- B >= 65, B =< 90, recursivereplace(B, A, 43, Out). %+
selection(A, B, Out) :- B >= 65, B =< 90, recursivereplace(B, A, 45, Out). %-
selection(A, B, Out) :- B >= 65, B =< 90, recursivereplace(B, A, 42, Out). %*
selection(A, B, Out) :- B >= 65, B =< 90, recursivereplace(B, A, 47, Out), !. %/
selection(A, B, Out) :- B =< 65, copy_term(A, Out), !.
selection(A, B, Out) :- B >= 90, copy_term(A, Out), !.
recursivereplace(Elem, List, Insert, Out) :- selectchk(Elem, List, Insert, New), !, recursivereplace(Elem, New, Insert, Out), copy(New, Out).
recursivereplace(_, _, _, _).

copy(_,R) :- is_list(R), !.
copy(L,R) :- accCp(L,R).
accCp([],[]).
accCp([H|T1],[H|T2]) :- accCp(T1,T2).

%вывод правильного ответа
correct_ans(String, Ans) :- term_to_atom(T, String), X is T, simplify(X, AnsReal), AnsReal is Ans, write(String), write(' = '), write(Ans).

%расчёт выражения слева
simplify(X,X) :- atomic(X).
simplify(+(A, B), V) :- simplify(A, VA), simplify(B, VB), simp_vals(+(VA, VB), V).
simplify(*(A, B), V) :- simplify(A, VA), simplify(B, VB), simp_vals(*(VA, VB), V).
simplify(/(A, B), V) :- simplify(A, VA), simplify(B, VB), simp_vals(/(VA, VB), V).
simplify(-(A, B), V) :- simplify(A, VA), simplify(B, VB), simp_vals(-(VA, VB), V).
simplify(-E,R) :- simplify(E, S), simp_vals(-S, R).

simp_vals(+(0, V), V).
simp_vals(+(V, 0), V).
simp_vals(+(V, -V), 0).

simp_vals(-(V, 0), V).
simp_vals(-(0, V),-V).
simp_vals(-(V, V), 0).
simp_vals(-(0), 0).
simp_vals(-(-X), X).
simp_vals(+(A, B), AB) :- number(A), number(B), AB is +(A, B).
simp_vals(A-B,AB) :- number(A),number(B), AB is -(A, B).
simp_vals(*(0, _), 0).
simp_vals(*(_, 0), 0).
simp_vals(*(V, 1), V).
simp_vals(*(1, V), V).
simp_vals(*(A, B), AB) :- number(A),number(B), AB is *(A, B).
simp_vals(/(0, _), 0).
simp_vals(/(V, 1), V).
simp_vals(/(A, B), AB) :- number(A), number(B), AB is /(A, B).

simp_vals(X, X).
