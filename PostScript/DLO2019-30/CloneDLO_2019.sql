--@D:\Velvet\PS\Paginy\DLO2019-30\CloneDLO_2019.sql
create or replace procedure clone_dlowa2019 (
    vdrw_xx in drzewo.xx%type,
    vmanname in space_reservation.spacer_prn_makieta.nazwa%type,   /* nazwa widoczna w Manamie, np. *Reklama */
    vnagl in space_reservation.spacer_prn_makieta.naglowek%type,   /* 'Rekl','OgPlat','OgBezPl','OgWlWyd' - naglowek dla tabeli spacer_prn_makieta */
    vsekcja in space_reservation.spacer_prn_makieta.nazwa%type     /* nazwa sekcji */
) as
    vmak_xx spacer_prn_makieta.xx%type;
    vfun_xx spacer_prn_fun.xx%type;
begin
    pagina.copy_mak('-Reklama','DLO','WA','ALL','RP');
    select max(xx)-2 into vmak_xx from spacer_prn_makieta;
    update spacer_prn_makieta set drw_xx=vdrw_xx,nazwa=vmanname||pagina.vceven,naglowek=vnagl where xx=vmak_xx;
    update spacer_prn_makieta set drw_xx=vdrw_xx,nazwa=vmanname||pagina.vcodd,naglowek=vnagl where xx=vmak_xx+1;
    pagina.clone_fun(vmak_xx,  6);
    pagina.clone_fun(vmak_xx+1,6);
    update spacer_prn_fun_arg set arg=vsekcja
     where arg='(Reklama)' and fun_xx>=(select max(xx)-1 from spacer_prn_fun);
end clone_dlowa2019;
/

declare
    vdrw_xx number;
    vmak_xx number;
    vfun_xx number;
begin
   --WA
   select xx into vdrw_xx from drzewo where tytul='DLO' and mutacja='WA';
   clone_dlowa2019(vdrw_xx,'-Drobne','Drob','(Og這szenia drobne)');
   clone_dlowa2019(vdrw_xx,'-Nekrologi','Nekr','(Nekrologi)');
   clone_dlowa2019(vdrw_xx,'-Nekrologi-Drobne','NeDr','(Nekrologi/Og這szenia drobne)');
   clone_dlowa2019(vdrw_xx,'-W豉sneWydawcy','WlWyd','(Og這szenie w豉sne wydawcy)');
   clone_dlowa2019(vdrw_xx,'-Og這szBezp豉tne','Bezpl','(Og這szenie bezp豉tne)');
   clone_dlowa2019(vdrw_xx,'-Og這szP豉tne','Platn','(Og這szenie p豉tne)');
   clone_dlowa2019(vdrw_xx,'-Og這szenie','Oglosz','(Og這szenie)');
   clone_dlowa2019(vdrw_xx,'-Og這szenia-Reklamy','OglRekl','(Og這szenia i reklamy)');
   --BI
   pagina.copy_mak('-Reklama','DLO','WA','DLO','BI');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','BI');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','BI');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','BI');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','BI');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','BI');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','BI');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','BI');
   --BS
   pagina.copy_mak('-Reklama','DLO','WA','DLO','BS');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','BS');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','BS');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','BS');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','BS');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','BS');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','BS');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','BS');
   --CZ
   pagina.copy_mak('-Reklama','DLO','WA','DLO','CZ');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','CZ');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','CZ');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','CZ');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','CZ');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','CZ');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','CZ');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','CZ');
   --GD
   pagina.copy_mak('-Reklama','DLO','WA','DLO','GD');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','GD');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','GD');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','GD');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','GD');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','GD');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','GD');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','GD');
   --KA
   pagina.copy_mak('-Reklama','DLO','WA','DLO','KA');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','KA');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','KA');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','KA');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','KA');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','KA');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','KA');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','KA');
   select xx into vdrw_xx from drzewo where tytul='DLO' and mutacja='KA';
   clone_dlowa2019(vdrw_xx,'-NFZ','NFZ','(Informacja �l零kiego OW NFZ)');
   --KI
   pagina.copy_mak('-Reklama','DLO','WA','DLO','KI');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','KI');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','KI');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','KI');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','KI');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','KI');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','KI');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','KI');
   --KR
   pagina.copy_mak('-Reklama','DLO','WA','DLO','KR');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','KR');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','KR');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','KR');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','KR');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','KR');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','KR');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','KR');
   --LO
   pagina.copy_mak('-Reklama','DLO','WA','DLO','LO');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','LO');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','LO');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','LO');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','LO');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','LO');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','LO');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','LO');
   --LU
   pagina.copy_mak('-Reklama','DLO','WA','DLO','LU');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','LU');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','LU');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','LU');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','LU');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','LU');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','LU');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','LU');
   --OL
   pagina.copy_mak('-Reklama','DLO','WA','DLO','OL');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','OL');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','OL');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','OL');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','OL');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','OL');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','OL');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','OL');
   --OP
   pagina.copy_mak('-Reklama','DLO','WA','DLO','OP');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','OP');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','OP');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','OP');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','OP');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','OP');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','OP');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','OP');
   --PL
   pagina.copy_mak('-Reklama','DLO','WA','DLO','PL');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','PL');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','PL');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','PL');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','PL');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','PL');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','PL');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','PL');
   --PO
   pagina.copy_mak('-Reklama','DLO','WA','DLO','PO');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','PO');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','PO');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','PO');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','PO');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','PO');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','PO');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','PO');
   --RA
   pagina.copy_mak('-Reklama','DLO','WA','DLO','RA');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','RA');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','RA');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','RA');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','RA');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','RA');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','RA');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','RA');
   --RZ
   pagina.copy_mak('-Reklama','DLO','WA','DLO','RZ');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','RZ');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','RZ');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','RZ');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','RZ');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','RZ');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','RZ');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','RZ');
   --SZ
   pagina.copy_mak('-Reklama','DLO','WA','DLO','SZ');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','SZ');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','SZ');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','SZ');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','SZ');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','SZ');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','SZ');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','SZ');
   --TO
   pagina.copy_mak('-Reklama','DLO','WA','DLO','TO');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','TO');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','TO');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','TO');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','TO');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','TO');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','TO');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','TO');
   --WR
   pagina.copy_mak('-Reklama','DLO','WA','DLO','WR');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','WR');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','WR');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','WR');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','WR');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','WR');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','WR');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','WR');
   --ZI
   pagina.copy_mak('-Reklama','DLO','WA','DLO','ZI');
   pagina.copy_mak('-Nekrologi','DLO','WA','DLO','ZI');
   pagina.copy_mak('-Nekrologi-Drobne','DLO','WA','DLO','ZI');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DLO','ZI');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DLO','ZI');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DLO','ZI');
   pagina.copy_mak('-Og這szenie','DLO','WA','DLO','ZI');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DLO','ZI');
   --TCG
   pagina.copy_mak('-Reklama','DLO','WA','TCG','GD');
   pagina.copy_mak('-Reklama','DLO','WA','TCG','KA');
   pagina.copy_mak('-Reklama','DLO','WA','TCG','KR');
   pagina.copy_mak('-Reklama','DLO','WA','TCG','LO');
   pagina.copy_mak('-Reklama','DLO','WA','TCG','LU');
   pagina.copy_mak('-Reklama','DLO','WA','TCG','PO');
   pagina.copy_mak('-Reklama','DLO','WA','TCG','R2');
   pagina.copy_mak('-Reklama','DLO','WA','TCG','RP');
   pagina.copy_mak('-Reklama','DLO','WA','TCG','WA');
   pagina.copy_mak('-Reklama','DLO','WA','TCG','WR');

   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TCG','GD');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TCG','KA');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TCG','KR');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TCG','LO');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TCG','LU');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TCG','PO');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TCG','R2');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TCG','RP');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TCG','WA');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TCG','WR');
   --TBP
   pagina.copy_mak('-Reklama','DLO','WA','TBP','BI');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','BY');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','CZ');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','GD');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','KA');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','KI');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','KR');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','LO');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','LU');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','OL');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','OP');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','PL');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','PO');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','RA');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','RP');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','RZ');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','SZ');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','WA');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','WR');
   pagina.copy_mak('-Reklama','DLO','WA','TBP','ZI');

   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','BI');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','BY');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','CZ');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','GD');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','KA');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','KI');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','KR');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','LO');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','LU');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','OL');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','OP');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','PL');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','PO');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','RA');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','RP');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','RZ');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','SZ');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','WA');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','WR');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','TBP','ZI');
   --DGW RP
   pagina.copy_mak('-Reklama','DLO','WA','DGW','RP');
   pagina.copy_mak('-W豉sneWydawcy','DLO','WA','DGW','RP');
   pagina.copy_mak('-Og這szBezp豉tne','DLO','WA','DGW','RP');
   pagina.copy_mak('-Og這szP豉tne','DLO','WA','DGW','RP');
   pagina.copy_mak('-Og這szenie','DLO','WA','DGW','RP');
   pagina.copy_mak('-Og這szenia-Reklamy','DLO','WA','DGW','RP');
   -- podmieniamy umiejscowienie napisu z centrowanego na LR
   select m.xx into vmak_xx from spacer_prn_makieta m,drzewo d 
    where m.drw_xx=d.xx and d.tytul='DGW' and d.mutacja='RP' and m.parity=0 and m.nazwa like '%nia-Reklamy%';
   pagina.clone_fun(vmak_xx,  6);
   pagina.clone_fun(vmak_xx+1,6);
   select f.xx into vfun_xx from spacer_prn_fun f,spacer_prn_mak_fun mf 
    where f.xx=mf.fun_xx and mf.mak_xx=vmak_xx and lp=6;
   update spacer_prn_fun set nazwa='textLR' where xx in (vfun_xx,vfun_xx+1);
   update spacer_prn_fun_arg set arg_no=arg_no+1 where arg_no>=3 and fun_xx in (vfun_xx,vfun_xx+1);
   insert into spacer_prn_fun_arg (fun_xx,arg_no,arg_type,arg) values (vfun_xx,3,0,'1');
   insert into spacer_prn_fun_arg (fun_xx,arg_no,arg_type,arg) values (vfun_xx+1,3,0,'0');
   update spacer_prn_fun_arg set arg='99.7' where fun_xx=vfun_xx and arg_no=1;
   update spacer_prn_fun_arg set arg='693.75' where fun_xx=vfun_xx+1 and arg_no=1;
   pagina.copy_mak('-Og這szenia-Reklamy','DGW','RP','TBP','BI');
   --logo DGW
   select max(xx) into vfun_xx from spacer_prn_fun;
   insert into spacer_prn_fun values (vfun_xx+1, 0, 'logo', 'logo wyborcza', 'logo wyborcza');
   insert into spacer_prn_fun values (vfun_xx+2, 1, 'logo', 'logo wyborcza', 'logo wyborcza');

   insert into spacer_prn_fun_arg values (vfun_xx+1, 0, 0, '#wyborcza.eps');
   insert into spacer_prn_fun_arg values (vfun_xx+1, 1, 0, '404.6');
   insert into spacer_prn_fun_arg values (vfun_xx+1, 2, 0, '1089.7');
   
   insert into spacer_prn_fun_arg values (vfun_xx+2, 0, 0, '#wyborcza.eps');
   insert into spacer_prn_fun_arg values (vfun_xx+2, 1, 0, '164.5');
   insert into spacer_prn_fun_arg values (vfun_xx+2, 2, 0, '1089.7');


   insert into spacer_prn_fun values (vfun_xx+3, 0, 'logo', 'logo komunikaty', 'logo komunikaty');
   insert into spacer_prn_fun values (vfun_xx+4, 1, 'logo', 'logo komunikaty', 'logo komunikaty');

   insert into spacer_prn_fun_arg values (vfun_xx+3, 0, 0, '#komunikaty.eps');
   insert into spacer_prn_fun_arg values (vfun_xx+3, 1, 0, '524');
   insert into spacer_prn_fun_arg values (vfun_xx+3, 2, 0, '1089.7');
   
   insert into spacer_prn_fun_arg values (vfun_xx+4, 0, 0, '#komunikaty.eps');
   insert into spacer_prn_fun_arg values (vfun_xx+4, 1, 0, '283.9');
   insert into spacer_prn_fun_arg values (vfun_xx+4, 2, 0, '1089.7');


   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+1, 10, 1);
   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+3, 11, 1);
   
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+2, 10, 1);
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+4, 11, 1);
   --logo TBP
   select m.xx into vmak_xx from spacer_prn_makieta m,drzewo d 
    where m.drw_xx=d.xx and d.tytul='TBP' and d.mutacja='BI' and m.parity=0 and m.nazwa like '%nia-Reklamy%';

   insert into spacer_prn_fun values (vfun_xx+5, 0, 'logo', 'logo golden', 'logo golden');
   insert into spacer_prn_fun values (vfun_xx+6, 1, 'logo', 'logo golden', 'logo golden');

   insert into spacer_prn_fun_arg values (vfun_xx+5, 0, 0, '#golden.eps');
   insert into spacer_prn_fun_arg values (vfun_xx+5, 1, 0, '344.2');
   insert into spacer_prn_fun_arg values (vfun_xx+5, 2, 0, '1089.7');
   
   insert into spacer_prn_fun_arg values (vfun_xx+6, 0, 0, '#golden.eps');
   insert into spacer_prn_fun_arg values (vfun_xx+6, 1, 0, '145.5');
   insert into spacer_prn_fun_arg values (vfun_xx+6, 2, 0, '1089.7');


   insert into spacer_prn_fun values (vfun_xx+7, 0, 'logo', 'logo praca', 'logo praca');
   insert into spacer_prn_fun values (vfun_xx+8, 1, 'logo', 'logo praca', 'logo praca');

   insert into spacer_prn_fun_arg values (vfun_xx+7, 0, 0, '#praca.eps');
   insert into spacer_prn_fun_arg values (vfun_xx+7, 1, 0, '446');
   insert into spacer_prn_fun_arg values (vfun_xx+7, 2, 0, '1089.7');
   
   insert into spacer_prn_fun_arg values (vfun_xx+8, 0, 0, '#praca.eps');
   insert into spacer_prn_fun_arg values (vfun_xx+8, 1, 0, '250');
   insert into spacer_prn_fun_arg values (vfun_xx+8, 2, 0, '1089.7');


   insert into spacer_prn_fun values (vfun_xx+9, 0, 'logo', 'logo komunikaty', 'logo komunikaty');
   insert into spacer_prn_fun values (vfun_xx+10, 1, 'logo', 'logo komunikaty', 'logo komunikaty');

   insert into spacer_prn_fun_arg values (vfun_xx+9, 0, 0, '#komunikaty.eps');
   insert into spacer_prn_fun_arg values (vfun_xx+9, 1, 0, '546.95');
   insert into spacer_prn_fun_arg values (vfun_xx+9, 2, 0, '1089.7');
   
   insert into spacer_prn_fun_arg values (vfun_xx+10, 0, 0, '#komunikaty.eps');
   insert into spacer_prn_fun_arg values (vfun_xx+10, 1, 0, '348.25');
   insert into spacer_prn_fun_arg values (vfun_xx+10, 2, 0, '1089.7');


   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+5, 10, 1);
   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+7, 11, 1);
   insert into spacer_prn_mak_fun values (vmak_xx, vfun_xx+9, 12, 1);
   
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+6, 10, 1);
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+8, 11, 1);
   insert into spacer_prn_mak_fun values (vmak_xx+1, vfun_xx+10, 12, 1);

   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','BY');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','CZ');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','GD');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','KA');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','KI');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','KR');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','LO');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','LU');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','OL');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','OP');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','PL');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','PO');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','RA');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','RP');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','RZ');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','SZ');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','WA');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','WR');
   pagina.copy_mak('-Og這szenia-Reklamy','TBP','BI','TBP','ZI');
 end;
/

drop procedure space_reservation.clone_dlowa2019
/

declare
    vtyp_xx number;
begin
    select xx into vtyp_xx from typ_paginy where nazwa='DLO-30-lat';
    
    update spacer_prn_makieta set vrt_xx=1 where typ_xx=vtyp_xx;
    update spacer_prn_makieta set vrt_xx=3 where typ_xx=vtyp_xx and naglowek='Platn';
    update spacer_prn_makieta set vrt_xx=4 where typ_xx=vtyp_xx and naglowek='Bezpl';
    update spacer_prn_makieta set vrt_xx=5 where typ_xx=vtyp_xx and naglowek='WlWyd';
    update spacer_prn_makieta set vrt_xx=6 where typ_xx=vtyp_xx and naglowek='Oglosz';
end;
/