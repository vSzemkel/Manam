--@D:\Code\Velvet\Manam\Manam70\PostScript\DLO2019-30\CloneODJ_2019.sql
create or replace procedure clone_odjwa2019 (
    vdrw_xx in drzewo.xx%type,
    vmanname in space_reservation.spacer_prn_makieta.nazwa%type,   /* nazwa widoczna w Manamie, np. *Reklama */
    vnagl in space_reservation.spacer_prn_makieta.naglowek%type,   /* 'Rekl','OgPlat','OgBezPl','OgWlWyd' - naglowek dla tabeli spacer_prn_makieta */
    vsekcja in space_reservation.spacer_prn_makieta.nazwa%type     /* nazwa sekcji */
) as
    vmak_xx spacer_prn_makieta.xx%type;
    vfun_xx spacer_prn_fun.xx%type;
begin
    pagina.copy_mak('-Reklama','ODJ','WA','ALL','RP');
    select max(xx)-2 into vmak_xx from spacer_prn_makieta;
    update spacer_prn_makieta set drw_xx=vdrw_xx,nazwa=vmanname||pagina.vceven,naglowek=vnagl where xx=vmak_xx;
    update spacer_prn_makieta set drw_xx=vdrw_xx,nazwa=vmanname||pagina.vcodd,naglowek=vnagl where xx=vmak_xx+1;
    pagina.clone_fun(vmak_xx,  6);
    pagina.clone_fun(vmak_xx+1,6);
    update spacer_prn_fun_arg set arg=vsekcja
     where arg='(Reklama)' and fun_xx>=(select max(xx)-1 from spacer_prn_fun);
end clone_odjwa2019;
/

declare
    vdrw_xx number;
    vmak_xx number;
    vfun_xx number;
begin
   --WA
   select xx into vdrw_xx from drzewo where tytul='ODJ' and mutacja='WA';
   clone_odjwa2019(vdrw_xx,'-W豉sneWydawcy','WlWyd','(Og這szenie w豉sne wydawcy)');
   clone_odjwa2019(vdrw_xx,'-Og這szenie','Oglosz','(Og這szenie)');
   --BY
   pagina.copy_mak('-Reklama','ODJ','WA','ODJ','BY');
   pagina.copy_mak('-W豉sneWydawcy','ODJ','WA','ODJ','BY');
   pagina.copy_mak('-Og這szenie','ODJ','WA','ODJ','BY');
   --CZ
   pagina.copy_mak('-Reklama','ODJ','WA','ODJ','CZ');
   pagina.copy_mak('-W豉sneWydawcy','ODJ','WA','ODJ','CZ');
   pagina.copy_mak('-Og這szenie','ODJ','WA','ODJ','CZ');
   --GD
   pagina.copy_mak('-Reklama','ODJ','WA','ODJ','GD');
   pagina.copy_mak('-W豉sneWydawcy','ODJ','WA','ODJ','GD');
   pagina.copy_mak('-Og這szenie','ODJ','WA','ODJ','GD');
   --KR
   pagina.copy_mak('-Reklama','ODJ','WA','ODJ','KR');
   pagina.copy_mak('-W豉sneWydawcy','ODJ','WA','ODJ','KR');
   pagina.copy_mak('-Og這szenie','ODJ','WA','ODJ','KR');
   --LU
   pagina.copy_mak('-Reklama','ODJ','WA','ODJ','LU');
   pagina.copy_mak('-W豉sneWydawcy','ODJ','WA','ODJ','LU');
   pagina.copy_mak('-Og這szenie','ODJ','WA','ODJ','LU');
   --PL
   pagina.copy_mak('-Reklama','ODJ','WA','ODJ','PL');
   pagina.copy_mak('-W豉sneWydawcy','ODJ','WA','ODJ','PL');
   pagina.copy_mak('-Og這szenie','ODJ','WA','ODJ','PL');
   --PO
   pagina.copy_mak('-Reklama','ODJ','WA','ODJ','PO');
   pagina.copy_mak('-W豉sneWydawcy','ODJ','WA','ODJ','PO');
   pagina.copy_mak('-Og這szenie','ODJ','WA','ODJ','PO');
   --RA
   pagina.copy_mak('-Reklama','ODJ','WA','ODJ','RA');
   pagina.copy_mak('-W豉sneWydawcy','ODJ','WA','ODJ','RA');
   pagina.copy_mak('-Og這szenie','ODJ','WA','ODJ','RA');
   --RP
   pagina.copy_mak('-Reklama','ODJ','WA','ODJ','RP');
   pagina.copy_mak('-W豉sneWydawcy','ODJ','WA','ODJ','RP');
   pagina.copy_mak('-Og這szenie','ODJ','WA','ODJ','RP');
   --RZ
   pagina.copy_mak('-Reklama','ODJ','WA','ODJ','RZ');
   pagina.copy_mak('-W豉sneWydawcy','ODJ','WA','ODJ','RZ');
   pagina.copy_mak('-Og這szenie','ODJ','WA','ODJ','RZ');
   --WR
   pagina.copy_mak('-Reklama','ODJ','WA','ODJ','WR');
   pagina.copy_mak('-W豉sneWydawcy','ODJ','WA','ODJ','WR');
   pagina.copy_mak('-Og這szenie','ODJ','WA','ODJ','WR');

 end;
/

drop procedure space_reservation.clone_odjwa2019
/

declare
    vtyp_xx number;
begin
    select xx into vtyp_xx from typ_paginy where nazwa='ODJ-30-lat';

    update spacer_prn_makieta set vrt_xx=5 where typ_xx=vtyp_xx and naglowek='WlWyd';
    update spacer_prn_makieta set vrt_xx=6 where typ_xx=vtyp_xx and naglowek='Oglosz';
end;
/