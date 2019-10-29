--@d:\Velvet\PS\Paginy\TQx2019\CloneTQx_2019.sql
create or replace procedure clone_tqbby2019 (
    vdrw_xx in drzewo.xx%type,
    vmanname in space_reservation.spacer_prn_makieta.nazwa%type,   /* nazwa widoczna w Manamie, np. *Reklama */
    vnagl in space_reservation.spacer_prn_makieta.naglowek%type,   /* 'Rekl','OgPlat','OgBezPl','OgWlWyd' - naglowek dla tabeli spacer_prn_makieta */
    vsekcja in space_reservation.spacer_prn_makieta.nazwa%type,    /* nazwa sekcji, np: OG£OSZENIA */
    vtygodnik in varchar2                                          /* nazwa produktu, np: CZAS BRODNICY */
) as
    vmak_xx spacer_prn_makieta.xx%type;
    vfun_xx spacer_prn_fun.xx%type;
begin
    pagina.copy_mak('>Reklama','TQB','BY','ALL','RP');
    select max(xx)-2 into vmak_xx from spacer_prn_makieta;
    update spacer_prn_makieta set drw_xx=vdrw_xx,nazwa=vmanname||pagina.vceven,naglowek=vnagl where xx=vmak_xx;
    update spacer_prn_makieta set drw_xx=vdrw_xx,nazwa=vmanname||pagina.vcodd,naglowek=vnagl where xx=vmak_xx+1;
    pagina.clone_fun(vmak_xx,  4);
    pagina.clone_fun(vmak_xx+1,4);
    update spacer_prn_fun_arg set arg=vsekcja
     where arg='(REKLAMA)' and fun_xx>=(select max(xx)-1 from spacer_prn_fun);
    pagina.clone_fun(vmak_xx,  6);
    pagina.clone_fun(vmak_xx+1,6);
    update spacer_prn_fun_arg set arg=vtygodnik
     where arg='(CZAS BRODNICY)' and fun_xx>=(select max(xx)-1 from spacer_prn_fun);
end clone_tqbby2019;
/

declare
    vdrw_xx number;
    vmak_xx number;
    vfun_xx number;
begin
   -- TQB
   select xx into vdrw_xx from drzewo where tytul='TQB' and mutacja='BY';
   clone_tqbby2019(vdrw_xx,'>Drobne','Drob','(OG£OSZENIA DROBNE)','(CZAS BRODNICY)');
   clone_tqbby2019(vdrw_xx,'>Og³oszP³atne','Platn','(OG£OSZENIE P£ATNE)','(CZAS BRODNICY)');
   -- TQC
   select xx into vdrw_xx from drzewo where tytul='TQC' and mutacja='BY';
   clone_tqbby2019(vdrw_xx,'>Reklama','Rekl','(REKLAMA)','(CZAS CHE£MNA)');
   clone_tqbby2019(vdrw_xx,'>Drobne','Drob','(OG£OSZENIA DROBNE)','(CZAS CHE£MNA)');
   clone_tqbby2019(vdrw_xx,'>Og³oszP³atne','Platn','(OG£OSZENIE P£ATNE)','(CZAS CHE£MNA)');
   -- TQS
   select xx into vdrw_xx from drzewo where tytul='TQS' and mutacja='BY';
   clone_tqbby2019(vdrw_xx,'>Reklama','Rekl','(REKLAMA)','(CZAS ŒWIECIA)');
   clone_tqbby2019(vdrw_xx,'>Drobne','Drob','(OG£OSZENIA DROBNE)','(CZAS ŒWIECIA)');
   clone_tqbby2019(vdrw_xx,'>Og³oszP³atne','Platn','(OG£OSZENIE P£ATNE)','(CZAS ŒWIECIA)');
   -- TQR
   select xx into vdrw_xx from drzewo where tytul='TQR' and mutacja='BY';
   clone_tqbby2019(vdrw_xx,'>Reklama','Rekl','(REKLAMA)','(CZAS RYPINA)');
   clone_tqbby2019(vdrw_xx,'>Drobne','Drob','(OG£OSZENIA DROBNE)','(CZAS RYPINA)');
   clone_tqbby2019(vdrw_xx,'>Og³oszP³atne','Platn','(OG£OSZENIE P£ATNE)','(CZAS RYPINA)');

   commit;
end;
/
