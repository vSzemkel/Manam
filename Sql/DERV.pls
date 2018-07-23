create or replace PACKAGE BODY                     "DERV" as
  pragma serially_reusable;

  function check_derv_cycle (
    vmak_xx in makieta.xx%type,
    vnr_porz in spacer_strona.nr_porz%type
  ) return boolean is
    vbase_mak_xx makieta.xx%type := vmak_xx;
    vbase_nr_porz spacer_strona.nr_porz%type := vnr_porz;
  begin
    for i in 1..10 loop
       select derv_mak_xx,derv_nr_porz into vbase_mak_xx,vbase_nr_porz
         from spacer_strona where mak_xx=vbase_mak_xx and nr_porz=vbase_nr_porz;
       exit when vbase_mak_xx is null;
       if vbase_mak_xx = vmak_xx and vbase_nr_porz = vnr_porz then
          return true;
       end if;
    end loop;
    return false;
  end check_derv_cycle;

  procedure dervtmpl_from (
    vfname in varchar2,
    vkiedy in varchar2,
    vretCur out sr.refCur
  ) as 
    vnr_porz number := to_number(substr(vfname,6,3));
  begin
    open vretCur for
      select d2.mutacja
        from drzewo d, makieta m, spacer_strona dst, makieta m2, drzewo d2
       where d.tytul=upper(substr(vfname,1,3)) and d.mutacja=upper(substr(vfname,4,2)) and d.xx=m.drw_xx and m.kiedy=to_date(vkiedy,sr.vfShortDate)
         and m2.kiedy=m.kiedy and dst.mak_xx=m2.xx 
         and dst.derv_nr_porz=decode(vnr_porz,m2.objetosc,0,vnr_porz) and dst.derv_mak_xx=m.xx and dst.dervlvl=sr.derv_tmpl
         and m2.drw_xx=d2.xx;
  end dervtmpl_from;
  
  /*procedure mozliwosci_derv (
    vxx in makieta.xx%type, 
    vretCur out sr.refCur
  ) as 
  begin
    open vretCur for
    select xx,opis from (
      select d2.xx,decode(d2.mutacja,' ',d2.tytul,d2.tytul||' '||d2.mutacja)||' - '||rejkod.get_opis_drzewa(d2.xx,m.kiedy) opis
        from drzewo d, drzewo d2, makieta m
       where m.xx=vxx and d.xx=m.drw_xx and sql_check_access(d2.xx,'R')>0
         and d.tytul=d2.tytul and d.xx<>d2.xx and exists (select 1 from makieta m2 where m2.drw_xx=d2.xx and m2.kiedy=m.kiedy)
    union all
      select d2.xx,decode(d2.mutacja,' ',d2.tytul,d2.tytul||' '||d2.mutacja)||' - '||rejkod.get_opis_drzewa(d2.xx,m.kiedy) opis
        from drzewo d, drzewo d2, makieta m, schemat_derv sd
       where m.xx=vxx and d.xx=m.drw_xx and sd.do_kiedy>=trunc(sysdate) and sql_check_access(d2.xx,'R')>0
         and d.xx=sd.base_drw_xx and sd.parent_drw_xx=d2.xx and exists (select 1 from makieta m2 where m2.drw_xx=d2.xx and m2.kiedy=m.kiedy)
    ) order by 2;
  end mozliwosci_derv;*/

  procedure info (
    vmak_xx in out makieta.xx%type, 
    vnr_porz in out spacer_strona.nr_porz%type,
    vretCur out sr.refCur
  ) as begin /* zwraca liste mozliwosci dziedziczenia strony, ujemne klucze dla ju¿ dziedziczacych mutacji */
    open vretCur for
    select nvl2(derv.drw_xx,-posib.xx,posib.xx),posib.opis from (
      select d2.xx,decode(d2.mutacja,' ',d2.tytul,d2.tytul||' '||d2.mutacja)||' - '||rejkod.get_opis_drzewa(d2.xx,m.kiedy) opis
        from drzewo d, drzewo d2, makieta m
       where m.xx=vmak_xx and d.xx=m.drw_xx and sql_check_access(d2.xx,'R')>0
         and d.tytul=d2.tytul and d.xx<>d2.xx and exists (select 1 from makieta m2 where m2.drw_xx=d2.xx and m2.kiedy=m.kiedy)
    union all
      select d2.xx,decode(d2.mutacja,' ',d2.tytul,d2.tytul||' '||d2.mutacja)||' - '||rejkod.get_opis_drzewa(d2.xx,m.kiedy) opis
        from drzewo d, drzewo d2, makieta m, schemat_derv sd
       where m.xx=vmak_xx and d.xx=m.drw_xx and sd.do_kiedy>=sysdate and sql_check_access(d2.xx,'R')>0
         and d.xx=sd.base_drw_xx and sd.parent_drw_xx=d2.xx and exists (select 1 from makieta m2 where m2.drw_xx=d2.xx and m2.kiedy=m.kiedy)
    ) posib, 
       (select md.drw_xx from spacer_strona d,spacer_strona b,makieta mb,makieta md 
        where b.mak_xx=vmak_xx and b.nr_porz=decode(vnr_porz,mb.objetosc,0,vnr_porz) 
          and d.derv_mak_xx=b.mak_xx and d.derv_nr_porz=b.nr_porz and d.mak_xx=md.xx and b.mak_xx=mb.xx and mb.kiedy=md.kiedy) derv
    where posib.xx=derv.drw_xx(+);
          
    select min(di.drw_xx),min(decode(derv_nr_porz,0,m.objetosc,derv_nr_porz)) into vmak_xx,vnr_porz 
      from makieta m,derv_info di where m.xx=vmak_xx and m.xx=di.mak_xx and di.nr_porz=decode(vnr_porz,m.objetosc,0,vnr_porz);
  end info;
  
  procedure derive_pages (
    vdst_mak_xx in makieta.xx%type,
    vdst_nr_porz in spacer_strona.nr_porz%type,
    vbase_drw_xx in drzewo.xx%type,
    vbase_nr in pls_integer,
    vdervlvl in spacer_strona.dervlvl%type,
    vile_kol in pls_integer
  ) as
    vi pls_integer;
    vj pls_integer;
    vmsg varchar2(1024);
    voffset pls_integer;
    vwyd_xx wydawca.xx%type;
    vbase_nr_porz pls_integer;
    vbase_mak_xx makieta.xx%type;
    vdrukarnie drzewo_mutacji.drukarnie%type;
  begin
    if vdervlvl = derv.druk then
       raise_application_error(-20001,'Ten rodzaj dziedziczenia nie jest juz dostepny');
    end if;
    -- czy dziedziczenie odwrotne
    if vbase_drw_xx < -1 then
      select b.drw_xx,dst.xx,rejkod.get_opis_drzewa(dst.drw_xx,dst.kiedy),
        (select count(1) from spacer_pub p,spacer_strona s where p.mak_xx=s.mak_xx and p.str_xx=s.str_xx and s.mak_xx=dst.xx and s.nr_porz=vbase_nr) 
        into vwyd_xx,vbase_mak_xx,vmsg,vi
        from makieta b,makieta dst
        where dst.drw_xx=-vbase_drw_xx and b.kiedy=dst.kiedy and b.xx=vdst_mak_xx;
      if vi > 0 then
         raise_application_error(-20001,'Przed dziedziczeniem nale¿y zdj¹æ og³oszenia ze zmienianych stron makiety '||vmsg);
      else
         derive_pages(vbase_mak_xx,vbase_nr,vwyd_xx,vdst_nr_porz,vdervlvl,vile_kol);
      end if;
      return;
    end if;
    -- loguj zmiane
    insert into spacer_log (mak_xx,opis)
    values (vdst_mak_xx,'Dziedziczenie typu '||vdervlvl||' od strony '||vdst_nr_porz);
    -- uniewaznij blachy
    for c in (
      select grb_xx from makingrb where mak_xx=vdst_mak_xx
       union select grb_okl_xx from makingrb where mak_xx=vdst_mak_xx and grb_okl_xx is not null
       union select g.xx from grzbiet g, split_grzbietu s, drzewo d, makieta m
              where m.xx=vdst_mak_xx and m.drw_xx=d.xx and d.tytul=s.okladka 
                and s.skladka1=g.tytul and d.mutacja=g.mutgrb and g.kiedy=m.kiedy
    ) loop
      wyprzedzeniowe.check_can_modify(c.grb_xx,2);
      delete from blacha where grb_xx=c.grb_xx;
    end loop;
    -- blokada i drukarniane
    if vdervlvl = derv.proh then
      select min(md.xx) into vbase_mak_xx from spacer_strona sb, makieta mb, spacer_strona sd, makieta md
       where mb.xx=vdst_mak_xx and sb.nr_porz>=vdst_nr_porz and sb.nr_porz<vdst_nr_porz+vile_kol
         and sb.mak_xx=mb.xx and mb.kiedy=md.kiedy and md.xx=sd.mak_xx
         and sd.derv_mak_xx=mb.xx and sd.derv_nr_porz=sb.nr_porz;
      if vbase_mak_xx is not null then
        select tytul into vmsg from mak where mak_xx=vbase_mak_xx;
        raise_application_error(-20002,'Co najmniej jedna ze wskazanych stron jest dziedziczona w makiecie '||vmsg);
      end if;
      
      update spacer_strona set dervlvl=derv.proh 
       where mak_xx=vdst_mak_xx and nr_porz>=vdst_nr_porz and nr_porz<vdst_nr_porz+vile_kol;
      return;
    elsif vdervlvl = derv.druk then
      update spacer_strona set dervlvl=derv.druk 
       where mak_xx=vdst_mak_xx and nr_porz>=vdst_nr_porz and nr_porz<vdst_nr_porz+vile_kol;
      return;
    end if;
    -- kasuj dziedziczenie
    if vdervlvl = sr.derv_none then
      -- usun stuby ogloszen
      for delcur in (
          select s.mak_xx,s.str_xx,s.x,s.y,p.sizex,p.sizey,s.xx
            from spacer_pubstub s, spacer_pub p, spacer_strona dst
           where s.xx=p.xx 
             and s.mak_xx=vdst_mak_xx
             and s.str_xx=dst.str_xx
             and dst.dervlvl=sr.derv_adds
             and dst.mak_xx=vdst_mak_xx
             and dst.nr_porz>=vdst_nr_porz
             and dst.nr_porz<vdst_nr_porz + vile_kol
    union select s.mak_xx,s.str_xx,s.x,s.y,p.sizex,p.sizey,s.xx
            from spacer_pubstub s, spacer_pub p
           where s.xx=p.xx and s.xx in (select p2.xx 
                       from spacer_strona dst,spacer_strona base,spacer_pub p2
                      where dst.dervlvl=sr.derv_adds
                        and dst.mak_xx=vdst_mak_xx
                        and dst.nr_porz>=vdst_nr_porz
                        and dst.nr_porz<vdst_nr_porz + vile_kol
                        and base.mak_xx=dst.derv_mak_xx
                        and base.nr_porz=dst.derv_nr_porz
                        and base.mak_xx=p2.mak_xx
                        and base.str_xx=p2.str_xx)
      ) loop
            spacer_set_space2(delcur.mak_xx,delcur.str_xx,delcur.x,delcur.y,delcur.sizex,delcur.sizey,sr.wolny);
            delete from spacer_pubstub where xx=delcur.xx and mak_xx=vdst_mak_xx;
      end loop;
      
      update spacer_strona 
         set dervlvl=sr.derv_none,
             derv_mak_xx=null,
             derv_nr_porz=null,
             moduly=lpad('0',length(moduly),'0')
       where mak_xx=vdst_mak_xx
         and nr_porz>=vdst_nr_porz
         and nr_porz<vdst_nr_porz + vile_kol;

      -- geste i niebazowe kraty 
      update spacer_pow 
          set moduly=lpad('0',length(moduly),'0')
        where mak_xx=vdst_mak_xx
          and str_xx in (select s.str_xx from spacer_strona s
                          where s.mak_xx=vdst_mak_xx
                            and s.nr_porz>=vdst_nr_porz
                            and s.nr_porz<vdst_nr_porz + vile_kol);
      update spacer_str_krat
         set moduly=lpad('0',length(moduly),'0')
       where mak_xx=vdst_mak_xx
         and str_xx in (select s.str_xx from spacer_strona s
                         where s.mak_xx=vdst_mak_xx
                           and s.nr_porz>=vdst_nr_porz
                           and s.nr_porz<vdst_nr_porz + vile_kol);
      return;
    end if;
    
    begin -- znajdz makiete bazowa
      select d.wyd_xx,base.xx into vwyd_xx,vbase_mak_xx
        from drzewo d, makieta base, makieta dst
       where base.kiedy=dst.kiedy
         and base.drw_xx=vbase_drw_xx
         and dst.xx=vdst_mak_xx
         and base.drw_xx=d.xx;
    exception
      when no_data_found then
         select decode(d.mutacja,' ',d.tytul,d.tytul||' '||d.mutacja)||' na '||to_char(kiedy,sr.vfShortDate) into vmsg
           from drzewo d, makieta
          where d.xx=vbase_drw_xx
            and makieta.xx=vdst_mak_xx;
         raise_application_error(-20003,'W bazie danych nie odnaleziono wskazanej makiety bazowej '||vmsg);
    end;

    -- dziedziczenia ogloszen nie wolno nadpisac
    select count(1) into vi
      from spacer_strona
     where mak_xx=vdst_mak_xx
       and nr_porz>=vdst_nr_porz
       and nr_porz<vdst_nr_porz + vile_kol
       and dervlvl=sr.derv_adds;
    if vi > 0 then
        raise_application_error(-20003,'Dla stron dziedziczonych w trybie dziedziczenia ogloszen jedyny dostepny tryb to brak dziedziczenia');
    end if;
    
    -- czy dziedziczyc ostatnia
    select count(1) into vi
      from spacer_strona
     where mak_xx=vbase_mak_xx;
    if vi = vbase_nr and vile_kol = 1 then
        vbase_nr_porz := 0;
    else
        vbase_nr_porz := vbase_nr;
    end if;
    
    -- czy istnieja strony bazowe
    select count(1) into vi
      from spacer_strona
     where mak_xx=vbase_mak_xx
       and nr_porz>=vbase_nr_porz
       and nr_porz<vbase_nr_porz + vile_kol;
    if vi < vile_kol then
        raise_application_error(-20004,'Stwierdzono brak stron bazowych. Brakuje: '||(vile_kol - vi)||'.Przyjmij, ze ostatnia strona makiety ma numer 0');
    elsif vi > vile_kol then
        raise_application_error(-20005,'Makieta bazowa ma zdublowane strony. Skontaktuj sie z administratorem');
    end if;
    
    voffset := vdst_nr_porz - vbase_nr_porz;
    -- zgodnosc krat bazowych
    if vdervlvl <> derv.fixd then
      select decode(min(dst.nr_porz),0,'ostatniej',min(dst.nr_porz)) into vmsg
        from spacer_strona base, spacer_strona dst
       where base.mak_xx=vbase_mak_xx and dst.mak_xx=vdst_mak_xx
         and base.kra_xx<>dst.kra_xx
         and dst.nr_porz=base.nr_porz + voffset
         and dst.nr_porz>=vdst_nr_porz
         and dst.nr_porz<vdst_nr_porz + vile_kol;
      if vmsg is not null then
          raise_application_error(-20006,'Wykryto niezgodnosc krat na stronie '||vmsg);
      end if;
    end if;
    
    -- dziedziczenie wielokrotne jest nielegalne
    select min(dst.nr_porz) into vi
      from spacer_strona dst
     where dst.mak_xx=vdst_mak_xx
       and (dst.nr_porz - vdst_nr_porz <> dst.derv_nr_porz - vbase_nr_porz)
       and dst.derv_mak_xx=vbase_mak_xx
       and dst.derv_nr_porz>=vbase_nr_porz
       and dst.derv_nr_porz<vbase_nr_porz + vile_kol;
    if vi is not null then
        select objetosc into vj from makieta where xx=vbase_mak_xx;
        if vj > 2 then
           raise_application_error(-20007,'Wykryto probe wielokrotnego dziedziczenia. Strona zostala juz odziedziczona w tej makiecie jako strona '||vi);
        end if;
    end if;

    -- dziedziczenie zabronione
    select count(1) into vi from spacer_strona 
     where mak_xx=vbase_mak_xx and dervlvl=derv.proh 
       and vbase_nr_porz<=nr_porz and nr_porz<vbase_nr_porz + vile_kol;
    if vi>0 then
        raise_application_error(-20008,'Wsrod wskazanych stron bazowych znajduje sie strona niedostepna do dziedziczenia');
    end if;

    -- ustal drukarnie
    select nvl(d.drukarnie,dm.drukarnie) into vdrukarnie
      from makieta m, drzewo d, drzewo_mutacji dm
     where rownum=1 and m.xx=vdst_mak_xx and m.drw_xx=d.xx and d.mutacja=dm.mutacja(+);
     
    -- przepisanie atrybutow
    if vdervlvl = sr.derv_adds then
      update spacer_strona dst
         set dst.dervlvl=vdervlvl,
             dst.derv_mak_xx=vbase_mak_xx,
             (dst.derv_nr_porz,dst.moduly,dst.sciezka,dst.str_log,dst.naglowek)=
             (select base.nr_porz,base.moduly,base.sciezka,base.str_log,base.naglowek
                from spacer_strona base
               where base.mak_xx=vbase_mak_xx
                 and dst.nr_porz=base.nr_porz + voffset)
       where dst.mak_xx=vdst_mak_xx
         and dst.nr_porz>=vdst_nr_porz
         and dst.nr_porz<vdst_nr_porz + vile_kol;
    else
      update spacer_strona dst
         set dst.dervlvl=vdervlvl,
             dst.derv_mak_xx=vbase_mak_xx,
             (dst.derv_nr_porz,dst.sciezka,dst.str_log,dst.naglowek,dst.moduly,dst.ile_kol,dst.spot,dst.drukarnie,dst.deadline,dst.wyd_xx)=
             (select base.nr_porz,base.sciezka,base.str_log,base.naglowek,base.moduly,base.ile_kol,base.spot,decode(vdervlvl,sr.derv_fixd,drukarnie,vdrukarnie),base.deadline,nvl(base.wyd_xx,vwyd_xx)
                from spacer_strona base
               where base.mak_xx=vbase_mak_xx
                 and dst.nr_porz=base.nr_porz + voffset)
       where dst.mak_xx=vdst_mak_xx
         and dst.nr_porz>=vdst_nr_porz
         and dst.nr_porz<vdst_nr_porz + vile_kol;
         
      update plan_wyd.strona set wca_xx=(select wyd_xx from spacer_strona where mak_xx=vdst_mak_xx and nr_porz=vdst_nr_porz)
       where wpr_xx=(select xx from plan_wyd.wydanie_produktu where rownum=1 and del_xx is null and nvl(odw_xx,0)=0 and nvl(do_przelicz,0)<>2 and mak_xx=vdst_mak_xx)
         and numer>=decode(vdst_nr_porz,0,(select objetosc from makieta where xx=vdst_mak_xx),vdst_nr_porz)
         and numer<decode(vdst_nr_porz,0,(select objetosc from makieta where xx=vdst_mak_xx),vdst_nr_porz) + vile_kol;
    end if;
    
    -- wyszukiwanie cykli
    for vi in 0..vile_kol-1 loop
       if check_derv_cycle(vdst_mak_xx,vdst_nr_porz+vi) then    
          rollback;
          raise_application_error(-20009,'Zdefiniowanie tego dziedziczenia utworzyloby dziedziczenie cykliczne');
       end if;
    end loop;
    
    delete from spacer_pow
          where mak_xx=vdst_mak_xx
            and str_xx in (select s.str_xx from spacer_strona s
                           where s.mak_xx=vdst_mak_xx
                             and s.nr_porz>=vdst_nr_porz
                             and s.nr_porz<vdst_nr_porz + vile_kol);
    insert into spacer_pow (mak_xx,str_xx,lp,moduly)
         select vdst_mak_xx,dst.str_xx,p.lp,p.moduly
           from spacer_pow p, spacer_strona base, spacer_strona dst
          where p.mak_xx=vbase_mak_xx
            and p.str_xx=base.str_xx
            and dst.nr_porz=base.nr_porz + voffset
            and dst.nr_porz>=vdst_nr_porz
            and dst.nr_porz<vdst_nr_porz + vile_kol
            and base.mak_xx=vbase_mak_xx 
            and dst.mak_xx=vdst_mak_xx;

    delete from spacer_str_krat
          where mak_xx=vdst_mak_xx
            and str_xx in (select s.str_xx from spacer_strona s
                           where s.mak_xx=vdst_mak_xx
                             and s.nr_porz>=vdst_nr_porz
                             and s.nr_porz<vdst_nr_porz + vile_kol);
    insert into spacer_str_krat (mak_xx,str_xx,kra_xx,lp,moduly)
         select vdst_mak_xx,dst.str_xx,p.kra_xx,p.lp,p.moduly
           from spacer_str_krat p, spacer_strona base, spacer_strona dst
          where p.mak_xx=vbase_mak_xx
            and p.str_xx=base.str_xx
            and dst.nr_porz=base.nr_porz + voffset
            and dst.nr_porz>=vdst_nr_porz
            and dst.nr_porz<vdst_nr_porz + vile_kol
            and base.mak_xx=vbase_mak_xx 
            and dst.mak_xx=vdst_mak_xx;

    -- przepisanie ogloszen
    if vdervlvl = sr.derv_adds then
        insert into spacer_pubstub (xx,mak_xx,str_xx,x,y)
             select p.xx,vdst_mak_xx,dst.str_xx,p.x,p.y
               from (select xx,mak_xx,str_xx,x,y from spacer_pub union all select xx,mak_xx,str_xx,x,y from spacer_pubstub) p,
                    spacer_strona base, spacer_strona dst
              where base.mak_xx=p.mak_xx and base.str_xx=p.str_xx
                and dst.nr_porz=base.nr_porz + voffset
                and dst.nr_porz>=vdst_nr_porz
                and dst.nr_porz<vdst_nr_porz + vile_kol
                and base.mak_xx=vbase_mak_xx 
                and dst.mak_xx=vdst_mak_xx
                and p.x>0
                and not exists (select 1 from spacer_pubstub p2 where p2.xx=p.xx and p2.mak_xx=vdst_mak_xx);
    end if;
  end derive_pages;
  
  procedure synchronize_derv (
  	vdstmak_xx in number
  ) as
    vsx sr.srtinyint;
    vsrcstat sr.srtinyint;
    vdststat sr.srtinyint;
    vsrcmakdate sr.srint;
    vdstmakdate sr.srint := makdate(vdstmak_xx);
  begin
    -- obecnosc stron bazowych
    select min(dst.nr_porz) into vsx
      from spacer_strona dst
     where dst.mak_xx=vdstmak_xx and dst.dervlvl<>sr.derv_none
       and not exists (select 1 from spacer_strona base 
             where base.mak_xx=dst.derv_mak_xx and base.nr_porz=dst.derv_nr_porz);
    if vsx is not null then
        raise_application_error(-20001,'Strona dziedziczona jako '||vsx||' zostala usunieta w makiecie bazowej');
    end if;
   
    -- usuniecie ogloszen dziedziczonych na bok makiety bazowej
    select min(nvl(p.adno,-1)) into vsx
      from spacer_pub p
     where p.x=0 and exists (select 1 from spacer_pubstub s
                              where s.mak_xx=vdstmak_xx and s.xx=p.xx);
    if vsx is not null then
        raise_application_error(-20002,'Dziedziczone ogloszenie '||vsx||' zostalo zdjete z makiety bazowej');
    end if;
          
    -- kraty stron dziedziczonych jako DERV_FIXD, DERV_TMPL
    select min(dst.nr_porz) into vsx
      from spacer_strona dst, spacer_strona base
     where dst.mak_xx=vdstmak_xx and (dst.dervlvl=sr.derv_fixd or dst.dervlvl=sr.derv_tmpl)
       and base.mak_xx=dst.derv_mak_xx and base.nr_porz=dst.derv_nr_porz
       and dst.kra_xx<>base.kra_xx;
    if vsx is not null then
        raise_application_error(-20003,'Krata strony dziedziczonej jako '||vsx||' nie jest juz zgodna z krata strony bazowej');
    end if;

    -- strony dziedziczone jako DERV_FIXD
    select min(dst.nr_porz) into vsx
      from spacer_strona dst, spacer_strona base
     where dst.mak_xx=vdstmak_xx and dst.dervlvl=sr.derv_fixd
       and base.mak_xx=dst.derv_mak_xx and base.nr_porz=dst.derv_nr_porz
       and (base.moduly<>dst.moduly or base.ile_kol<>dst.ile_kol or
            base.sciezka<>dst.sciezka or base.naglowek<>dst.naglowek or
            base.str_log<>dst.str_log or base.spot<>dst.spot or
            base.drukarnie<>dst.drukarnie or base.deadline<>dst.deadline);
    if vsx is not null then
        raise_application_error(-20004,'Strona dziedziczona jako '||vsx||' - kopia strony z makiety bazowej nie jest juz z nia zgodna');
    end if;

    -- strony dziedziczone jako DERV_TMPL
    for cur in (
        select k.szpalt_x sx,k.szpalt_y-1 sy,dst.str_xx dststr_xx,base.str_xx srcstr_xx,base.mak_xx srcmak_xx, dst.nr_porz
          from spacer_strona dst,spacer_strona base,spacer_kratka k
         where dst.mak_xx=vdstmak_xx and base.mak_xx=dst.derv_mak_xx and dst.kra_xx=k.xx
           and dst.derv_nr_porz=base.nr_porz and dst.dervlvl=sr.derv_tmpl
    ) loop

      vsrcmakdate := makdate(cur.srcmak_xx);
  
      for vx in 0..(cur.sx-1) loop
         for vy in 0..cur.sy loop
            vsrcstat := select_modul_status(cur.srcmak_xx,cur.srcstr_xx,vx+vy*cur.sx,vsrcmakdate);
            vdststat := select_modul_status(vdstmak_xx,cur.dststr_xx,vx+vy*cur.sx,vdstmakdate);

            if vsrcstat<>vdststat and vsrcstat not in (sr.wolny,sr.zablokowany) then
                raise_application_error(-20006,'Blad synchronizacji na poziomie statusow modulow na stronie '||cur.nr_porz);
            end if;
            if vsrcstat=vdststat and vsrcstat=sr.ogloszenie then
                select is_xy_covered(vdstmak_xx,cur.dststr_xx,vx+1,vy+1) into vsx from dual;
                if vsx > 0 then
                    raise_application_error(-20007,'Na stronie '||cur.nr_porz||' ogloszenia pokrywaja sie w module ('||(vx+1)||','||(vy+1)||')');
                end if;
            end if;
            if vsrcstat<>vdststat and vdststat=sr.ogloszenie then
                select is_xy_covered(vdstmak_xx,cur.dststr_xx,vx+1,vy+1) into vsx from dual;
                if vsx = 0 then
                    raise_application_error(-20008,'Na stronie '||cur.nr_porz||' modul ('||(vx+1)||','||(vy+1)||') nie jest juz blokowany poprzez dziedziczenie');
                end if;
            end if;
         end loop;
      end loop;
    end loop;
  
    for s in (select str_xx,nr_porz,derv_mak_xx,derv_nr_porz from spacer_strona where mak_xx=vdstmak_xx and dervlvl=adds) loop
      select min(nvl(p.adno,-1)) into vsx from spacer_pub p,spacer_strona base
       where base.mak_xx=s.derv_mak_xx and base.nr_porz=s.derv_nr_porz
         and p.mak_xx=base.mak_xx and p.str_xx=base.str_xx 
         and p.x>0 and p.y>0 and (p.flaga_rezerw=1 or nvl(p.czas_obow,0)<=makdate(base.mak_xx))
         and not exists (select 1 from spacer_pubstub ps where ps.mak_xx=vdstmak_xx and ps.xx=p.xx);
      if vsx is not null then 
        raise_application_error(-20009,'Trzeba powtornie odziedziczyc strone '||s.nr_porz||'. W makiecie bazowej pojawilo sie nowe ogloszenie: '||vsx);
      end if;
    end loop;
  
    raise_application_error(-20010,'Nie stwierdzono bledow');
  end synchronize_derv;
  
  procedure synchronize_kraj (
  	vdstmak_xx in number
  ) as
    vsx sr.srtinyint;
    vmaxnr_porz sr.srsmallint;
    vszpalt_x sr.srtinyint;
    vszpalt_y sr.srtinyint;
    vsrcstat sr.srtinyint;
    vdststat sr.srtinyint;
    vsrcmakdate sr.srint;
    vdstmakdate sr.srint;
    vmsg varchar2(1000);
  	vsrcmak_xx number;
  	vsrcstr_xx number;
  	vdststr_xx number;
    vkra_xx number;
  begin
  -- synchronizacji podlegaja tylko makiety
  -- tytulow XXX YY, dla ktorych istnieja makiety XXX RP
  -- wyjatek tytul TEM synchronizuje sie do WA
  select m.xx into vsrcmak_xx
    from makieta m,makieta m2,drzewo d,drzewo d2
   where m2.xx=vdstmak_xx and m.kiedy=m2.kiedy 
     and m2.drw_xx=d2.xx and m.drw_xx=d.xx
     and d.mutacja=decode(d2.tytul,'TEM','WA','RP') 
     and d2.mutacja<>decode(d2.tytul,'TEM','WA','RP')
     and d.tytul=d2.tytul;
   
  select max(t.nr_porz) into vmaxnr_porz
    from (select t2.nr_porz
            from (select s1.nr_porz from spacer_strona s1 where s1.mak_xx=vsrcmak_xx
                  union all select s2.nr_porz from spacer_strona s2 where s2.mak_xx=vdstmak_xx) t2
           group by t2.nr_porz
          having count(1)=2) t;
          
  -- kolory, naglowki i typ numeracji
  update spacer_strona s
     set (s.ile_kol,s.spot,s.sciezka,s.str_log,s.naglowek,s.nr,s.num_xx)=
         (select s2.ile_kol,s2.spot,s2.sciezka,s2.str_log,s2.naglowek,s2.nr,s2.num_xx
            from spacer_strona s2 where s2.mak_xx=vsrcmak_xx and s2.nr_porz=s.nr_porz)
   where s.nr_porz<=vmaxnr_porz
     and s.mak_xx=vdstmak_xx;
              
  -- powierzchnia
  for vi in 0..vmaxnr_porz loop
    begin
      select k.szpalt_x,k.szpalt_y - 1,k.xx,s.str_xx,s2.str_xx 
        into vszpalt_x,vszpalt_y,vkra_xx,vsrcstr_xx,vdststr_xx
        from spacer_strona s,spacer_strona s2,spacer_kratka k
       where s.mak_xx=vsrcmak_xx and s2.mak_xx=vdstmak_xx
         and s.nr_porz=s2.nr_porz and s.nr_porz=vi
         and s.kra_xx=s2.kra_xx and s.kra_xx=k.xx;
    exception
      when no_data_found then
        select decode(vi,0,'ostatnich',to_char(vi)) into vmsg from dual;
        raise_application_error(-20001,'Blad synchronizacji: niezgodnosc krat bazowych dla stron '||vmsg);
    end;  

    vsx := vszpalt_x - 1;
    vsrcmakdate := makdate(vsrcmak_xx);
    vdstmakdate := makdate(vdstmak_xx);
             
    if vkra_xx = 1.0 then
      for vx in 0..vsx loop
        for vy in 0..vszpalt_y loop
          vsrcstat := select_modul_status(vsrcmak_xx,vsrcstr_xx,vx+vy*vszpalt_x,vsrcmakdate);
          vdststat := select_modul_status(vdstmak_xx,vdststr_xx,vx+vy*vszpalt_x,vdstmakdate);
          
          if vsrcstat=vdststat and vsrcstat=sr.ogloszenie then
            select decode(vi,0,'ostatniej',to_char(vi)) into vmsg from dual;
            raise_application_error(-20002,'Blad synchronizacji: pokrywajace sie ogloszenia na stronie '||vmsg);         
          end if;
          
          if vsrcstat=sr.redakcyjny and vdststat=sr.ogloszenie then
            select decode(vi,0,'ostatniej',to_char(vi)) into vmsg from dual;
            raise_application_error(-20002,'Blad synchronizacji: ogloszenie stoi na modulach redakcyjnych na stronie '||vmsg);         
          end if;

          if vsrcstat=sr.zablokowany and vdststat=sr.ogloszenie then
            select decode(vi,0,'ostatniej',to_char(vi)) into vmsg from dual;
            raise_application_error(-20002,'Blad synchronizacji: ogloszenie stoi na modulach zablokonych na stronie '||vmsg);         
          end if;
          
          if vsrcstat=sr.ogloszenie or vsrcstat=sr.flaga_rezerw then
            vsrcstat := sr.zablokowany;
          end if;
          
          if vdststat<>sr.ogloszenie and vdststat<>sr.flaga_rezerw then
            update_modul(vdstmak_xx,vdststr_xx,vx+vy*vszpalt_x,vsrcstat);
          end if;
        end loop;
      end loop;
    else --kra_xx <> 1.0
      for vx in 0..vsx loop
        for vy in 0..vszpalt_y loop
          vsrcstat := select_modul_status_krat(vsrcmak_xx,vsrcstr_xx,vkra_xx,vx+vy*vszpalt_x,vsrcmakdate);
          vdststat := select_modul_status_krat(vdstmak_xx,vdststr_xx,vkra_xx,vx+vy*vszpalt_x,vdstmakdate);
          
          if vsrcstat=vdststat and vsrcstat=sr.ogloszenie then
            select decode(vi,0,'ostatniej',to_char(vi)) into vmsg from dual;
            raise_application_error(-20002,'Blad synchronizacji: pokrywajace sie ogloszenia na stronie '||vmsg);         
          end if;
          
          if vsrcstat=sr.redakcyjny and vdststat=sr.ogloszenie then
            select decode(vi,0,'ostatniej',to_char(vi)) into vmsg from dual;
            raise_application_error(-20002,'Blad synchronizacji: ogloszenie stoi na modulach redakcyjnych na stronie '||vmsg);         
          end if;

          if vsrcstat=sr.zablokowany and vdststat=sr.ogloszenie then
            select decode(vi,0,'ostatniej',to_char(vi)) into vmsg from dual;
            raise_application_error(-20002,'Blad synchronizacji: ogloszenie stoi na modulach zablokonych na stronie '||vmsg);         
          end if;
          
          if vsrcstat=sr.ogloszenie or vsrcstat=sr.flaga_rezerw then
            vsrcstat := sr.zablokowany;
          end if;
          
          if vdststat<>sr.ogloszenie and vdststat<>sr.flaga_rezerw then
            update_modul_krat(vdstmak_xx,vdststr_xx,vkra_xx,vx+vy*vszpalt_x,vsrcstat);
          end if;
        end loop;
      end loop;
    end if;
    end loop;
  
    commit;
  exception
    when no_data_found then
      raise_application_error(-20003,'Ta makieta nie podlega synchronizacji powierzchni. Nie istnieje dla niej makieta o zasiegu RP');  
  end synchronize_kraj;
  
  procedure set_mutred (
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type,
    vkiedy in varchar2,
    vnr_porz in spacer_strona.nr_porz%type,
    vmutred in drzewo.mutacja%type,
    vsetval in number
  ) as 
    vmak_xx makieta.xx%type;
    vlastmutred spacer_strona.mutred%type;
    vmutlen number;
    vi pls_integer := 1;
  begin
    select m.xx,s.mutred,nvl(length(s.mutred),0) into vmak_xx,vlastmutred,vmutlen
      from drzewo d, makieta m, spacer_strona s, drzewo_mutacji dm
     where d.tytul=vtytul and d.mutacja=vmutacja and d.xx=m.drw_xx
       and m.kiedy=to_date(vkiedy,sr.vfShortDate) and m.xx=s.mak_xx 
       and s.nr_porz=vnr_porz and dm.mutacja=vmutred;
    
    if vsetval = 0 then
      while vi<vmutlen loop
        if substr(vlastmutred,vi,2)=vmutred then
          vlastmutred := substr(vlastmutred,1,vi-1)||substr(vlastmutred,vi+2);
          vi := vmutlen;
        end if;
        vi := vi + 2;
      end loop;
    else
      while vi<vmutlen and substr(vlastmutred,vi,2)<vmutred loop
        vi := vi + 2;
      end loop;

      if vi>=vmutlen then
        vlastmutred := vlastmutred||vmutred;
      elsif substr(vlastmutred,vi,2)>vmutred then
        vlastmutred := substr(vlastmutred,1,vi-1)||vmutred||substr(vlastmutred,vi);
      end if;
    end if;
    
    update spacer_strona set mutred=vlastmutred
     where mak_xx=vmak_xx and nr_porz=vnr_porz;
  exception
    when no_data_found then
      raise_application_error(-20001,'Nie odnaleziono strony lub ustawiana mutacja nie jest zdefiniowana');
  end set_mutred;

  -- procedura ustawia mozliwosc dziedziczenia z makiety v_parent do v_base
  -- w wydaniach od daty biezacej do daty v_dokiedy
  procedure define_derv_nostd (
     v_parent in mak.tytul%type, 
     v_base in mak.tytul%type, 
     v_dokiedy in varchar2 default to_char(sysdate+7,sr.vfShortDate)
  ) is
     v_parent_xx number;
     v_base_xx   number;
     vd_dokiedy   date;
  begin
     if v_parent=v_base then
        raise_application_error(-20001,'Nie mozna dziedziczyc w obrebie jednej makiety.');
     elsif v_dokiedy is null then
        vd_dokiedy:=trunc(sysdate)+7;
     else
        vd_dokiedy:=to_date(v_dokiedy,sr.vfShortDate);
     end if;   
     if vd_dokiedy<trunc(sysdate) then
        raise_application_error(-20002,'Graniczna data wydania nie moze byc wczesniejsza do biezacej.');
     end if;
  
     select p.xx,b.xx into v_parent_xx,v_base_xx
       from drzewo p,drzewo b
      where p.tytul=upper(substr(v_parent,1,3)) and p.mutacja=upper(substr(v_parent,4,2))
        and b.tytul=upper(substr(v_base,1,3)) and b.mutacja=upper(substr(v_base,4,2));
  
     update space_reservation.schemat_derv
        set do_kiedy=vd_dokiedy,kto_mod=uid,kiedy_mod=current_timestamp
      where base_drw_xx=v_base_xx and parent_drw_xx=v_parent_xx;
     if sql%rowcount=0 then
        insert into space_reservation.schemat_derv(base_drw_xx,parent_drw_xx,do_kiedy)
             values (v_base_xx,v_parent_xx,vd_dokiedy);
     end if;
  exception
     when no_data_found then 
        raise_application_error(-20003,'Nie mozna odnalezc makiety o podanym kodzie.'); 
  end define_derv_nostd;

  procedure derv_deadline (
    vtytul in drzewo.tytul%type,
    vkiedy in makieta.kiedy%type
  ) as begin
    -- child to parent deadline propagation
    update spacer_strona sp set deadline=(select least(nvl(sp.deadline,sr.dozywocie),nvl(min(sch.deadline),sp.deadline))
      from spacer_strona sch,makieta mp,makieta mch
     where sp.mak_xx=mp.xx and sch.mak_xx=mch.xx and mp.kiedy=mch.kiedy 
       and sch.dervlvl=derv.fixd and sch.derv_mak_xx=mp.xx and sch.derv_nr_porz=sp.nr_porz)
    where sp.mak_xx in (select mak_xx from mak where kiedy=vkiedy and substr(tytul,1,3)=vtytul);
    -- parent to child deadline propagation
    update spacer_strona sch set deadline=nvl((select least(sp.deadline,sch.deadline)
      from spacer_strona sp,makieta mp,makieta mch
     where sp.mak_xx=mp.xx and sch.mak_xx=mch.xx and mp.kiedy=mch.kiedy 
       and sch.dervlvl=derv.fixd and sch.derv_mak_xx=mp.xx and sch.derv_nr_porz=sp.nr_porz),sch.deadline)
    where sch.mak_xx in (select mak_xx from mak where kiedy=vkiedy and substr(tytul,1,3)=vtytul);
  end derv_deadline;
end derv;