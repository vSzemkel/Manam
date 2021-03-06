--@D:\Code\Velvet\Manam\Manam70\PostScript\DLO2019-30\CreateODJ_2019.sql
declare
   vfun_xx number;
   vmak_xx number;
   vtyp_xx number;
   vdrw_xx number;
   vis_wzorzec number := 0;
   vnazwa_typu varchar2(64) := 'ODJ-30-lat';
   vnazwa_paginy varchar2(32) := '-Reklama';
   vnazwa_produktu varchar2(6) := 'ODJ WA';
   vnaglowek varchar2(15) := 'Rekl';
begin
   -- usuni�cie starej definicji
   select min(xx) into vtyp_xx from typ_paginy where nazwa=vnazwa_typu;
   if vtyp_xx is not null then
      delete spacer_prn_fun_arg where fun_xx in (
         select mf.fun_xx from spacer_prn_makieta m,spacer_prn_mak_fun mf where m.typ_xx=vtyp_xx and m.xx=mf.mak_xx);
      delete from spacer_prn_mak_fun where mak_xx in (select xx from spacer_prn_makieta where typ_xx=vtyp_xx);
      delete from spacer_prn_fun where xx not in (select distinct fun_xx from spacer_prn_mak_fun) and xx not in (select distinct fun_xx from spacer_prn_fun_arg);
      delete from spacer_prn_makieta where typ_xx=vtyp_xx;
      delete from typ_paginy where xx=vtyp_xx;
   else
      select max(xx)+1 into vtyp_xx from typ_paginy;
   end if;
   
   -- utworzenie typu paginy
   insert into typ_paginy (xx,nazwa,init_trans_even,init_trans_odd,final_trans_even,final_trans_odd,drobne_height,pelne_pole_trans,margines_do_grzbietu,page_device_size,boundingbox)
        values (vtyp_xx, vnazwa_typu, '0 3', '0 3', '28.45 12', '28.45 12', 665, '0 25',null ,'552.76 745.51','0 0 552.76 745.51');

   -- utworzenie paginy wzorcowej
   select xx into vdrw_xx from drzewo where tytul||' '||mutacja=vnazwa_produktu;
   select xx into vmak_xx from spacer_prn_makieta where nazwa='NEXT_XX';
   update spacer_prn_makieta set xx=xx+2 where nazwa='NEXT_XX';

   insert into spacer_prn_makieta (xx,drw_xx,is_def,parity,nazwa,is_wzorzec,naglowek,typ_xx,vrt_xx)
        values (vmak_xx,vdrw_xx,0,0,vnazwa_paginy||pagina.vceven,vis_wzorzec,vnaglowek,vtyp_xx,1);
   insert into spacer_prn_makieta (xx,drw_xx,is_def,parity,nazwa,is_wzorzec,naglowek,typ_xx,vrt_xx)
        values (vmak_xx+1,vdrw_xx,0,1,vnazwa_paginy||pagina.vcodd,vis_wzorzec,vnaglowek,vtyp_xx,1);

   -- definicja funkcji paginy wzorcowej
   select max(xx) into vfun_xx from spacer_prn_fun;


   insert into spacer_prn_fun values (vfun_xx+1, 0, 'grestore', 'Linia', 'linia g�rna');
   insert into spacer_prn_fun values (vfun_xx+2, 1, 'grestore', 'Linia', 'linia g�rna');

   insert into spacer_prn_fun_arg values (vfun_xx+1, 0, 0, 'gsave');
   insert into spacer_prn_fun_arg values (vfun_xx+1, 1, 0, '.25');
   insert into spacer_prn_fun_arg values (vfun_xx+1, 2, 0, 'setlinewidth');
   insert into spacer_prn_fun_arg values (vfun_xx+1, 3, 0, '28.45');
   insert into spacer_prn_fun_arg values (vfun_xx+1, 4, 0, '722.53');
   insert into spacer_prn_fun_arg values (vfun_xx+1, 5, 0, 'moveto');
   insert into spacer_prn_fun_arg values (vfun_xx+1, 6, 0, '496');
   insert into spacer_prn_fun_arg values (vfun_xx+1, 7, 0, '0');
   insert into spacer_prn_fun_arg values (vfun_xx+1, 8, 0, 'rlineto');
   insert into spacer_prn_fun_arg values (vfun_xx+1, 9, 0, 'stroke');
   
   insert into spacer_prn_fun_arg values (vfun_xx+2, 0, 0, 'gsave');
   insert into spacer_prn_fun_arg values (vfun_xx+2, 1, 0, '.25');
   insert into spacer_prn_fun_arg values (vfun_xx+2, 2, 0, 'setlinewidth');
   insert into spacer_prn_fun_arg values (vfun_xx+2, 3, 0, '28.45');
   insert into spacer_prn_fun_arg values (vfun_xx+2, 4, 0, '722.53');
   insert into spacer_prn_fun_arg values (vfun_xx+2, 5, 0, 'moveto');
   insert into spacer_prn_fun_arg values (vfun_xx+2, 6, 0, '496');
   insert into spacer_prn_fun_arg values (vfun_xx+2, 7, 0, '0');
   insert into spacer_prn_fun_arg values (vfun_xx+2, 8, 0, 'rlineto');
   insert into spacer_prn_fun_arg values (vfun_xx+2, 9, 0, 'stroke');


   insert into spacer_prn_fun values (vfun_xx+3, 0, 'R', 'Linia dolna', 'Linia dolna gruba');
   insert into spacer_prn_fun values (vfun_xx+4, 1, 'R', 'Linia dolna', 'Linia dolna gruba');

   insert into spacer_prn_fun_arg values (vfun_xx+3, 0, 0, '1');
   insert into spacer_prn_fun_arg values (vfun_xx+3, 1, 0, '0');
   insert into spacer_prn_fun_arg values (vfun_xx+3, 2, 0, '0');
   insert into spacer_prn_fun_arg values (vfun_xx+3, 3, 0, '496');
   insert into spacer_prn_fun_arg values (vfun_xx+3, 4, 0, '3');
   insert into spacer_prn_fun_arg values (vfun_xx+3, 5, 0, '28.45');
   insert into spacer_prn_fun_arg values (vfun_xx+3, 6, 0, '690');

   insert into spacer_prn_fun_arg values (vfun_xx+4, 0, 0, '1');
   insert into spacer_prn_fun_arg values (vfun_xx+4, 1, 0, '0');
   insert into spacer_prn_fun_arg values (vfun_xx+4, 2, 0, '0');
   insert into spacer_prn_fun_arg values (vfun_xx+4, 3, 0, '496');
   insert into spacer_prn_fun_arg values (vfun_xx+4, 4, 0, '3');
   insert into spacer_prn_fun_arg values (vfun_xx+4, 5, 0, '28.45');
   insert into spacer_prn_fun_arg values (vfun_xx+4, 6, 0, '690');


   insert into spacer_prn_fun values (vfun_xx+5, 0, 'textLR', 'Numer strony', 'Numer strony');
   insert into spacer_prn_fun values (vfun_xx+6, 1, 'textLR', 'Numer strony', 'Numer strony');

   insert into spacer_prn_fun_arg values (vfun_xx+5, 0, 2, 'nr_str');
   insert into spacer_prn_fun_arg values (vfun_xx+5, 1, 0, '34.02');
   insert into spacer_prn_fun_arg values (vfun_xx+5, 2, 0, '699.5');
   insert into spacer_prn_fun_arg values (vfun_xx+5, 3, 0, '1');
   insert into spacer_prn_fun_arg values (vfun_xx+5, 4, 0, '18');
   insert into spacer_prn_fun_arg values (vfun_xx+5, 5, 0, '/Balto-Bold');

   insert into spacer_prn_fun_arg values (vfun_xx+6, 0, 2, 'nr_str');
   insert into spacer_prn_fun_arg values (vfun_xx+6, 1, 0, '518.74');
   insert into spacer_prn_fun_arg values (vfun_xx+6, 2, 0, '699.5');
   insert into spacer_prn_fun_arg values (vfun_xx+6, 3, 0, '0');
   insert into spacer_prn_fun_arg values (vfun_xx+6, 4, 0, '18');
   insert into spacer_prn_fun_arg values (vfun_xx+6, 5, 0, '/Balto-Bold');


   insert into spacer_prn_fun values (vfun_xx+7, 0, 'textLR', 'Gazeta Wyborcza', 'Gazeta Wyborcza');
   insert into spacer_prn_fun values (vfun_xx+8, 1, 'textLR', 'Gazeta Wyborcza', 'Gazeta Wyborcza');

   insert into spacer_prn_fun_arg values (vfun_xx+7, 0, 0, '(Gazeta Wyborcza)');
   insert into spacer_prn_fun_arg values (vfun_xx+7, 1, 0, '518.74');
   insert into spacer_prn_fun_arg values (vfun_xx+7, 2, 0, '708.675');
   insert into spacer_prn_fun_arg values (vfun_xx+7, 3, 0, '0');
   insert into spacer_prn_fun_arg values (vfun_xx+7, 4, 0, '7.2');
   insert into spacer_prn_fun_arg values (vfun_xx+7, 5, 0, '/Balto-Book');

   insert into spacer_prn_fun_arg values (vfun_xx+8, 0, 0, '(Gazeta Wyborcza)');
   insert into spacer_prn_fun_arg values (vfun_xx+8, 1, 0, '34.02');
   insert into spacer_prn_fun_arg values (vfun_xx+8, 2, 0, '708.675');
   insert into spacer_prn_fun_arg values (vfun_xx+8, 3, 0, '1');
   insert into spacer_prn_fun_arg values (vfun_xx+8, 4, 0, '7.2');
   insert into spacer_prn_fun_arg values (vfun_xx+8, 5, 0, '/Balto-Book');


   insert into spacer_prn_fun values (vfun_xx+9,  0, 'textLR', 'Data edycji', 'Data edycji');
   insert into spacer_prn_fun values (vfun_xx+10, 1, 'textLR', 'Data edycji', 'Data edycji');

   insert into spacer_prn_fun_arg values (vfun_xx+9, 0, 2, 'get_date');
   insert into spacer_prn_fun_arg values (vfun_xx+9, 1, 0, '518.74');
   insert into spacer_prn_fun_arg values (vfun_xx+9, 2, 0, '699.5');
   insert into spacer_prn_fun_arg values (vfun_xx+9, 3, 0, '0');
   insert into spacer_prn_fun_arg values (vfun_xx+9, 4, 0, '7.2');
   insert into spacer_prn_fun_arg values (vfun_xx+9, 5, 0, '/Balto-Book');

   insert into spacer_prn_fun_arg values (vfun_xx+10, 0, 2, 'get_date');
   insert into spacer_prn_fun_arg values (vfun_xx+10, 1, 0, '34.02');
   insert into spacer_prn_fun_arg values (vfun_xx+10, 2, 0, '699.5');
   insert into spacer_prn_fun_arg values (vfun_xx+10, 3, 0, '1');
   insert into spacer_prn_fun_arg values (vfun_xx+10, 4, 0, '7.2');
   insert into spacer_prn_fun_arg values (vfun_xx+10, 5, 0, '/Balto-Book');


   insert into spacer_prn_fun values (vfun_xx+11, 0, 'textC', 'Sekcja', 'Sekcja strony');
   insert into spacer_prn_fun values (vfun_xx+12, 1, 'textC', 'Sekcja', 'Sekcja strony');

   insert into spacer_prn_fun_arg values (vfun_xx+11, 0, 0, '(Reklama)');
   insert into spacer_prn_fun_arg values (vfun_xx+11, 1, 0, '276.45');
   insert into spacer_prn_fun_arg values (vfun_xx+11, 2, 0, '699.5');
   insert into spacer_prn_fun_arg values (vfun_xx+11, 3, 0, '19');
   insert into spacer_prn_fun_arg values (vfun_xx+11, 4, 0, '/Balto-Light');

   insert into spacer_prn_fun_arg values (vfun_xx+12, 0, 0, '(Reklama)');
   insert into spacer_prn_fun_arg values (vfun_xx+12, 1, 0, '276.45');
   insert into spacer_prn_fun_arg values (vfun_xx+12, 2, 0, '699.5');
   insert into spacer_prn_fun_arg values (vfun_xx+12, 3, 0, '19');
   insert into spacer_prn_fun_arg values (vfun_xx+12, 4, 0, '/Balto-Light');


   insert into spacer_prn_fun values(vfun_xx+13, 0, 'textLR', 'Wersja 1', 'Wersja 1 Sygnatura dol-prawo');
   insert into spacer_prn_fun values(vfun_xx+14, 1, 'textLR', 'Wersja 1', 'Wersja 1 Sygnatura dol-lewo');   
   
   insert into spacer_prn_fun_arg values(vfun_xx+13, 0, 1, 'get_wersja');
   insert into spacer_prn_fun_arg values(vfun_xx+13, 1, 1, '1');
   insert into spacer_prn_fun_arg values(vfun_xx+13, 2, 0, '528');
   insert into spacer_prn_fun_arg values(vfun_xx+13, 3, 0, '18');
   insert into spacer_prn_fun_arg values(vfun_xx+13, 4, 0, '1');
   insert into spacer_prn_fun_arg values(vfun_xx+13, 5, 0, '7');
   insert into spacer_prn_fun_arg values(vfun_xx+13, 6, 0, '/FranklinGothicPlBookCd');
   
   insert into spacer_prn_fun_arg values(vfun_xx+14, 0, 1, 'get_wersja');
   insert into spacer_prn_fun_arg values(vfun_xx+14, 1, 1, '1');
   insert into spacer_prn_fun_arg values(vfun_xx+14, 2, 0, '25');
   insert into spacer_prn_fun_arg values(vfun_xx+14, 3, 0, '18');
   insert into spacer_prn_fun_arg values(vfun_xx+14, 4, 0, '0');
   insert into spacer_prn_fun_arg values(vfun_xx+14, 5, 0, '7');
   insert into spacer_prn_fun_arg values(vfun_xx+14, 6, 0, '/FranklinGothicPlBookCd');


   insert into spacer_prn_fun values(vfun_xx+15, 0, 'textLR', 'Wersja 2', 'Wersja 2 Sygnatura dol-prawo');
   insert into spacer_prn_fun values(vfun_xx+16, 1, 'textLR', 'Wersja 2', 'Wersja 2 Sygnatura dol-lewo');   
   
   insert into spacer_prn_fun_arg values(vfun_xx+15, 0, 1, 'get_wersja');
   insert into spacer_prn_fun_arg values(vfun_xx+15, 1, 1, '2');
   insert into spacer_prn_fun_arg values(vfun_xx+15, 2, 0, '528');
   insert into spacer_prn_fun_arg values(vfun_xx+15, 3, 0, '18');
   insert into spacer_prn_fun_arg values(vfun_xx+15, 4, 0, '1');
   insert into spacer_prn_fun_arg values(vfun_xx+15, 5, 0, '7');
   insert into spacer_prn_fun_arg values(vfun_xx+15, 6, 0, '/FranklinGothicPlBookCd');
   
   insert into spacer_prn_fun_arg values(vfun_xx+16, 0, 1, 'get_wersja');
   insert into spacer_prn_fun_arg values(vfun_xx+16, 1, 1, '2');
   insert into spacer_prn_fun_arg values(vfun_xx+16, 2, 0, '25');
   insert into spacer_prn_fun_arg values(vfun_xx+16, 3, 0, '18');
   insert into spacer_prn_fun_arg values(vfun_xx+16, 4, 0, '0');
   insert into spacer_prn_fun_arg values(vfun_xx+16, 5, 0, '7');
   insert into spacer_prn_fun_arg values(vfun_xx+16, 6, 0, '/FranklinGothicPlBookCd');


   insert into spacer_prn_fun values(vfun_xx+17, 0, 'textLR', 'Wersja 3', 'Wersja 3 Sygnatura dol-prawo');
   insert into spacer_prn_fun values(vfun_xx+18, 1, 'textLR', 'Wersja 3', 'Wersja 3 Sygnatura dol-lewo');   
   
   insert into spacer_prn_fun_arg values(vfun_xx+17, 0, 1, 'get_wersja');
   insert into spacer_prn_fun_arg values(vfun_xx+17, 1, 1, '3');
   insert into spacer_prn_fun_arg values(vfun_xx+17, 2, 0, '528');
   insert into spacer_prn_fun_arg values(vfun_xx+17, 3, 0, '18');
   insert into spacer_prn_fun_arg values(vfun_xx+17, 4, 0, '1');
   insert into spacer_prn_fun_arg values(vfun_xx+17, 5, 0, '7');
   insert into spacer_prn_fun_arg values(vfun_xx+17, 6, 0, '/FranklinGothicPlBookCd');
   
   insert into spacer_prn_fun_arg values(vfun_xx+18, 0, 1, 'get_wersja');
   insert into spacer_prn_fun_arg values(vfun_xx+18, 1, 1, '3');
   insert into spacer_prn_fun_arg values(vfun_xx+18, 2, 0, '25');
   insert into spacer_prn_fun_arg values(vfun_xx+18, 3, 0, '18');
   insert into spacer_prn_fun_arg values(vfun_xx+18, 4, 0, '0');
   insert into spacer_prn_fun_arg values(vfun_xx+18, 5, 0, '7');
   insert into spacer_prn_fun_arg values(vfun_xx+18, 6, 0, '/FranklinGothicPlBookCd');


   -- po��czenie funkcji z makiet�

   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+1, 1, 1);
   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+3, 2, 1);
   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+5, 3, 1);
   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+7, 4, 1);
   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+9, 5, 1);
   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+11, 6, 1);
   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+13, 7, 1);
   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+15, 8, 0);
   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+17, 9, 0);
   
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+2, 1, 1);
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+4, 2, 1);
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+6, 3, 1);
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+8, 4, 1);
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+10, 5, 1);
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+12, 6, 1);
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+14, 7, 1);
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+16, 8, 0);
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+18, 9, 0);

   commit;
end;
/
