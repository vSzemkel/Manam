create or replace PACKAGE BODY                   "GRB" as
/******************** CHECK_GRB_ACCESS *********************/
  function check_grb_access(
    vtytul in grzbiet.tytul%type,
    vdostep in varchar2
  ) return boolean
  is
    vcc pls_integer;
  begin
  select nvl(min(1),0) into vcc
    from spacer_users u
   where xx=uid 
     and (nvl(manam_vip,0)=1
        or exists (select 1 from spacer_uprawnienia a, drzewo d
                    where d.xx=a.drw_xx and d.tytul=vtytul
                      and nvl(a.do_kiedy,sysdate)>=sysdate
                      and a.oso_xx=u.xx and instr(a.dostep,vdostep)>0));
    return vcc > 0;
  end check_grb_access;

/******************** INIT_GRZBIET *********************/
  function init_grzbiet(vmak_xx in makieta.xx%type)
  return grzbiet.xx%type is
    vcc pls_integer;
    vgrb_xx grzbiet.xx%type;
    vmsg varchar2(30);
    blad exception;
    vobszar grzbiet.obszar%type;
    vdrukarnie grzbiet.drukarnie%type;
    pragma exception_init(blad,-8002); --CURRVAL is not yet defined
  begin
    insert into grzbiet (tytul,mutgrb,kiedy,objetosc,naklad,depeszowy,przefalc,deadline,ded_xx,tec_xx,strony)
         select d.tytul,d.mutacja,m.kiedy,m.objetosc,decode(d.tytul,'DGW',0,m.naklad),d.depeszowy,d.przefalc,
       (select min(termin_dostarczenia) from produkt p where d.tytul=p.tytul and d.mutacja=p.mutacja and m.kiedy=p.kiedy),
       (select min(xx) from dane_edycji de where de.kiedy=m.kiedy and de.tpr_xx=d.tpr_xx),
       (select nvl(nvl(min(p.tec_xx),d.tec_xx),1) from produkt p where d.tytul=p.tytul and d.mutacja=p.mutacja and m.kiedy=p.kiedy),
       cast( multiset (select mak_xx,nr_porz,nr_porz,1 from spacer_strona
          where mak_xx=vmak_xx) as grzbiet_strony)
           from makieta m, drzewo d, drzewo_mutacji dm
          where m.xx=vmak_xx and m.drw_xx=d.xx and d.mutacja=dm.mutacja(+)
            and (m.objetosc in (12,20) or bitand(nvl(d.drukarnie,dm.drukarnie),cdrukag)=0 or mod(m.objetosc,2*d.ppp)=0);
    if sql%rowcount<>1 then
      select tytul||' '||kiedy into vmsg from mak where mak_xx=vmak_xx;
      raise_application_error(-20002,'Grzbiet '||vmsg||' nie zostal utworzony. Sprawdz czy ma odpowiednia objetosc.');
    end if;
    
    select grzbiet_xx.currval into vgrb_xx from dual;
    select reguly.zastosuj_reguly_kolp(vgrb_xx,
           (select max(p.obszar) from default_plan_grzbietu p,drzewo d,grzbiet g 
             where g.xx=vgrb_xx and p.dow=to_char(g.kiedy,'d') and p.drw1_xx=d.xx and p.drw2_xx is null and d.tytul=g.tytul and d.mutacja=g.mutgrb)) 
      into vobszar from dual;
    update grzbiet g set g.obszar=vobszar where g.xx=vgrb_xx and g.ded_xx is not null;

    insert into makingrb (mak_xx,grb_xx)
         values (vmak_xx,vgrb_xx);
    -- dla makiet dziennikow, tygodnikow i zewnetrznych ustawiamy drukarnie
    select count(1) into vcc from grzbiet where xx=vgrb_xx and substr(tytul,1,1) in ('D','T','Z');
    if vcc>0 then
      select nvl(nvl(d.drukarnie,dm.drukarnie),0) into vdrukarnie from drzewo d,drzewo_mutacji dm,grzbiet g where g.xx=vgrb_xx and d.mutacja=dm.mutacja(+) and d.tytul=g.tytul and d.mutacja=g.mutgrb;
      update grzbiet g set g.drukarnie=vdrukarnie where g.xx=vgrb_xx;
      update spacer_strona s set s.drukarnie=nvl(s.drukarnie,0)-bitand(nvl(s.drukarnie,0),vdrukarnie)+vdrukarnie 
       where s.mak_xx=oklgrb(vgrb_xx) and s.dervlvl<>derv.fixd;
    end if;
    
    return vgrb_xx;
  exception
    when dup_val_on_index then
        select tytul||' '||kiedy into vmsg from mak where mak_xx=vmak_xx;
        raise_application_error(-20003,'Grzbiet '||vmsg||' juz istnieje lub makieta ma zdublowane strony.');
    when blad then
        select tytul||' '||kiedy into vmsg from mak where mak_xx=vmak_xx;
        raise_application_error(-20004,'Makieta '||vmsg||' nie istnieje lub ma nieprawidlowa objetosc');
  end init_grzbiet;

/********************* SET_MUTGRB ***********************/
  function set_mutgrb(vgrb_xx in grzbiet.xx%type)
  return grzbiet.mutgrb%type is
    vmut drzewo.mutacja%type;
    vdrukarnie grzbiet.drukarnie%type;
    nested_null exception;    
    pragma exception_init(nested_null,-22908);
  begin
	  select mutacja into vmut from (
  		select  /*+ cardinality(g 5) */ dm.mutacja,g.nr 
	      from drzewo d, makieta m, drzewo_mutacji dm,
		         (select mak_xx,min(dst_nr_porz) nr from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx) g
	     where d.xx=m.drw_xx and m.xx=g.mak_xx and d.mutacja=dm.mutacja 
         and dm.poziom=(
	       		select /*+ cardinality(g 100) */ max(poziom) from drzewo_mutacji dm2,drzewo d, makieta m, 
		               (select mak_xx from table(select strony from grzbiet where xx=vgrb_xx)) g
       			 where dm2.mutacja=d.mutacja and d.xx=m.drw_xx and m.xx=g.mak_xx
         ) order by g.nr
    	) where rownum=1;    
    
    select drukarnie into vdrukarnie from drzewo_mutacji where mutacja=vmut;
    update grzbiet set mutgrb=vmut,
      depeszowy=(select /*+ cardinality(g 100) */ max(depeszowy) from drzewo d, makieta m,
       (select mak_xx from table(select strony from grzbiet where xx=vgrb_xx)) g
         where m.kiedy=grzbiet.kiedy and m.xx=g.mak_xx and m.drw_xx=d.xx)
     where xx=vgrb_xx 
       and (select poziom from drzewo_mutacji where mutacja=vmut) >= 
           (select poziom from drzewo_mutacji dm,grzbiet g where dm.mutacja=g.mutgrb and g.xx=vgrb_xx);
    if sql%rowcount = 0 then
       select mutgrb into vmut from grzbiet where xx=vgrb_xx;
    end if;

    return vmut;
  exception
    when too_many_rows then
        rollback;
        raise_application_error(-20001,'Nie mozna ustalic jednoznacznie mutacji grzbietu');
    when dup_val_on_index then
        rollback;
        raise_application_error(-20002,'Grzbiet z mutacja '||vmut||' juz istnieje');
    when nested_null then
        null;
    when no_data_found then
        select mutgrb into vmut from grzbiet where xx=vgrb_xx;
        return vmut; --mutacja spoza drzewa mutacji (ZKA)
  end set_mutgrb;

/************************ OKLGRB *************************/
  function oklgrb(vgrb_xx grzbiet.xx%type) return makieta.xx%type
  as
    vmak_xx makieta.xx%type;
  begin
    select mak_xx into vmak_xx
      from table(select strony from grzbiet where xx=vgrb_xx)
     where dst_nr_porz=0;
    return vmak_xx;
  exception
    when no_data_found then
      return null;
  end oklgrb;

/********************** ENCODE_OBSZAR ***********************/
  function encode_obszar(vgrb_xx grzbiet.xx%type) return varchar2
  as
    vob varchar2(1024) := '';
  begin
    for c in (
      select d.mutacja||f.sym ob from drzewo_mutacji d,forma_sprzedazy f,kolp_grb k
       where k.grb_xx=vgrb_xx and k.mut_xx=d.xx and bitand(k.forma_sprzed,f.bit_code)>0 order by 1
    ) loop
      vob := vob||c.ob;
    end loop;
    return vob;
  end encode_obszar;

/********************** GET_MUTRED ***********************/
  function get_mutred( vrootmut drzewo_mutacji.mutacja%type) return varchar2
  as
    vmutred varchar2(200) := '';
  begin
    for mr in (
        select mutacja from drzewo_mutacji
         where mutacja<>vrootmut and czy_redakcyjna=1
         start with mutacja=vrootmut connect by prior xx=root_xx
    ) loop
      vmutred := mr.mutacja||vmutred;
    end loop;
    return vmutred;
  end get_mutred;

/*********************** GET_ZUZYCIE ************************/
  function get_zuzycie(vgrb_xx grzbiet.xx%type) return drzewo.zuz_xx%type
  as 
    vzuz_xx drzewo.zuz_xx%type;
  begin
    select /*+ cardinality (gs,40) */ d.zuz_xx into vzuz_xx
      from drzewo d,makieta m,table(select strony from grzbiet where xx=vgrb_xx) gs 
     where gs.dst_nr_porz=0 and gs.mak_xx=m.xx and m.drw_xx=d.xx;
    return vzuz_xx;
  exception
    when no_data_found then
      return null;
  end get_zuzycie;

/*********************** GET_WAGA ************************/
  function get_waga(vgrb_xx grzbiet.xx%type) return number
  as
    vwaga number;
  begin
    select round(sum(fp.sizex*fp.sizey*pa.gramatura*pg.ile_stron)/20000) into vwaga
      from papier_ads pa,format_papieru fp,
         ( select /*+ cardinality(gs 40) */ s.pap_xx, count(1) ile_stron from table(select strony from grzbiet where xx=vgrb_xx) gs,spacer_strona s
            where gs.mak_xx=s.mak_xx and gs.base_nr_porz=s.nr_porz group by s.pap_xx) pg
     where pa.xx=pg.pap_xx and pa.for_xx=fp.xx;
    return vwaga;
  end get_waga;

/********************** GET_WYDAW ***********************/
  function get_wydaw(vgrb_xx grzbiet.xx%type) return varchar2
  as
    vwydaw varchar2(100) := '';
  begin
    for mr in (
      select '-'||nvl(p.wydawcy,mm.mutacja) sym from produkt p,
        (select  /*+ cardinality(gs 3) */ gs.nr,m.kiedy,d.tytul,decode(d.zuz_xx,8,'NG',d.mutacja) mutacja from grzbiet g, makieta m, drzewo d, 
           (select mak_xx,min(dst_nr_porz) nr from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx) gs
         where g.xx=vgrb_xx and g.kiedy=m.kiedy and m.xx=gs.mak_xx and m.drw_xx=d.xx) mm
      where mm.kiedy=p.kiedy(+) and mm.tytul=p.tytul(+) and mm.mutacja=p.mutacja(+)
      order by mm.nr
    ) loop
      vwydaw := vwydaw||mr.sym;
    end loop;
    return substr(vwydaw,2);
  end get_wydaw;

/********************** GET_ZSYLA ***********************/
  function get_zsyla(vgrb_xx grzbiet.xx%type) return varchar2
  as
    vzsyla varchar2(100) := '';
  begin
    for mr in (
        select  /*+ cardinality(gs 44) */'-'||w.sym sym 
          from grzbiet g, makieta m, wydawca w, drzewo d, table(g.strony) gs
         where g.xx=vgrb_xx and g.kiedy=m.kiedy and m.xx=gs.mak_xx and m.drw_xx=d.xx 
         and ((m.wyd_xx is not null and m.wyd_xx=w.xx) or (m.wyd_xx is null and d.wyd_xx=w.xx))
         group by w.sym,mak_xx order by min(dst_nr_porz)
    ) loop
      vzsyla := vzsyla||mr.sym;
    end loop;
    return substr(vzsyla,2);
  end get_zsyla;

/********************** GET_ZSYRED ***********************/
  function get_zsyred(vgrb_xx grzbiet.xx%type) return varchar2
  as
    vzsyla varchar2(100) := '';
  begin
    for mr in (
        select  /*+ cardinality(gs 44) */'-'||w.sym sym 
          from grzbiet g, makieta m, wydawca w, drzewo d, table(g.strony) gs
         where g.xx=vgrb_xx and g.kiedy=m.kiedy and m.xx=gs.mak_xx and m.drw_xx=d.xx 
         and ((nvl(m.zsy_red,m.wyd_xx) is not null and nvl(m.zsy_red,m.wyd_xx)=w.xx) or (nvl(m.zsy_red,m.wyd_xx) is null and d.wyd_xx=w.xx))
         group by w.sym,mak_xx order by min(dst_nr_porz)
    ) loop
      vzsyla := vzsyla||mr.sym;
    end loop;
    return substr(vzsyla,2);
  end get_zsyred;

/********************** GET_SZYCIE ***********************/
  function get_szycie(vgrb_xx grzbiet.xx%type) return varchar2
  as
    vszycie varchar2(64) := '';
  begin
    for sz in (
        select /*+ cardinality(g 44) */ '-'||decode(nvl(mg.szycie,m.szycie),null,'NIE',0,'NIE','TAK') szycie from makieta m, makingrb mg,
         table (select strony from grzbiet where xx=vgrb_xx) g
         where m.xx=mg.mak_xx(+) and mg.grb_xx=vgrb_xx and m.xx=g.mak_xx group by nvl(mg.szycie,m.szycie),m.xx order by min(dst_nr_porz)
    ) loop
      vszycie := vszycie||sz.szycie;
    end loop;
    return substr(vszycie,2);
  end get_szycie;

/********************** GET_PODPIS ***********************/
  function get_podpis(vgrb_xx grzbiet.xx%type) return varchar2
  as
    vszycie varchar2(64) := '';
  begin
    for sz in (
        select /*+ cardinality(g 44) */ '-'||u.nazwisko podpis from makieta m, 
         table (select strony from grzbiet where xx=vgrb_xx) g, spacer_users u
         where m.xx=g.mak_xx and upper(m.podpis_red)=upper(u.loginname) 
         group by m.xx,u.nazwisko order by min(dst_nr_porz)
    ) loop
      vszycie := vszycie||sz.podpis;
    end loop;
    return substr(vszycie,2);
  end get_podpis;

/********************** GET_DERV ***********************/
  function get_derv(vgrb_xx grzbiet.xx%type) return varchar2
  as
    vderv varchar2(2048) := '';
    vdervOglInfo varchar2(254);
    vdervTmpInfo varchar2(254);
    vdervFixInfo varchar2(254);
    vdervPrhInfo varchar2(254);
    vdervDruInfo varchar2(254);
  begin
    for c_mak in (
        select /*+ cardinality(g 44) */ m.tytul,m.mak_xx,m.objetosc 
          from mak m, table (select strony from grzbiet where xx=vgrb_xx) g
         where m.mak_xx=g.mak_xx group by m.tytul,m.mak_xx,m.objetosc order by min(g.dst_nr_porz)
    ) loop
      vderv := vderv||c_mak.tytul;
      select vformat.cecha_stron(cursor(select decode(dervlvl,1,1,0) 
        from spacer_strona s, mak m where s.mak_xx=m.mak_xx and m.mak_xx=c_mak.mak_xx
       order by decode(s.nr_porz,0,c_mak.objetosc,s.nr_porz))), 
             vformat.cecha_stron(cursor(select decode(dervlvl,2,1,0) 
        from spacer_strona s, mak m where s.mak_xx=m.mak_xx and m.mak_xx=c_mak.mak_xx
       order by decode(s.nr_porz,0,c_mak.objetosc,s.nr_porz))), 
             vformat.concat_list(cursor(
            select vformat.cecha_stron(cursor(select case when dervlvl=3 and derv_mak_xx=m.xx then 1 else 0 end
              from spacer_strona s where s.mak_xx=c_mak.mak_xx order by decode(s.nr_porz,0,c_mak.objetosc,s.nr_porz)))||' ('||d.tytul||' '||d.mutacja||')' 
              from drzewo d,makieta m,(select derv_mak_xx xx,min(nr_porz) pocz from spacer_strona where mak_xx=c_mak.mak_xx group by derv_mak_xx) drv
            where d.xx=m.drw_xx and m.xx=drv.xx order by decode(drv.pocz,0,c_mak.objetosc,drv.pocz)),'; '),
             /*vformat.cecha_stron(cursor(select decode(dervlvl,3,1,0) 
        from spacer_strona s, mak m where s.mak_xx=m.mak_xx and m.mak_xx=c_mak.mak_xx
       order by decode(s.nr_porz,0,c_mak.objetosc,s.nr_porz))),*/
             vformat.cecha_stron(cursor(select decode(dervlvl,4,1,0) 
        from spacer_strona s, mak m where s.mak_xx=m.mak_xx and m.mak_xx=c_mak.mak_xx
       order by decode(s.nr_porz,0,c_mak.objetosc,s.nr_porz))), 
             vformat.cecha_stron(cursor(select decode(dervlvl,5,1,0) 
        from spacer_strona s, mak m where s.mak_xx=m.mak_xx and m.mak_xx=c_mak.mak_xx
       order by decode(s.nr_porz,0,c_mak.objetosc,s.nr_porz)))
        into vdervOglInfo,vdervTmpInfo,vdervFixInfo,vdervPrhInfo,vdervDruInfo from dual;
      if vdervOglInfo is null and vdervTmpInfo is null and vdervFixInfo is null and vdervPrhInfo is null and vdervDruInfo is null then
        vderv := vderv||' brak stron dziedziczonych;';
      else
        vderv := vderv||' -';
      end if;
      if vdervOglInfo is not null then
        vderv := vderv||' ogloszenia: '||vdervOglInfo||';';
      end if;
      if vdervTmpInfo is not null then
        vderv := vderv||' do uzupelnienia: '||vdervTmpInfo||';';
      end if;
      if vdervFixInfo is not null then
        vderv := vderv||' pelne: '||vdervFixInfo||';';
      end if;
      if vdervPrhInfo is not null then
        vderv := vderv||' nie do dziedziczenia: '||vdervPrhInfo||';';
      end if;
      if vdervDruInfo is not null then
        vderv := vderv||' do uzupelnienia w drukarni: '||vdervDruInfo||';';
      end if;
    end loop;
    return vderv;
  end get_derv;

/********************** GET_PPP ***********************/
  function get_ppp(vgrb_xx grzbiet.xx%type) return varchar2
  as
    vppp varchar2(20) := '';
    vtyt1ch char;
  begin
    select min(substr(tytul,1,1)) into vtyt1ch from grzbiet g 
     where xx=vgrb_xx and (select count(distinct mak_xx) from table(g.strony))=1;
    if vtyt1ch in ('U','Y') then
        select d.ppp into vppp from drzewo d, grzbiet g 
         where g.xx=vgrb_xx and d.tytul=g.tytul and d.mutacja=g.mutgrb;
        return vppp;
    end if;
    
    for pp in (
        select /*+ cardinality(g 44) */ '-'||d.ppp ppp from makieta m, drzewo d,
         table (select strony from grzbiet where xx=vgrb_xx) g
         where m.drw_xx=d.xx and m.xx=g.mak_xx group by d.ppp,m.xx order by min(g.dst_nr_porz)
    ) loop
      vppp := vppp||pp.ppp;
    end loop;
    return substr(vppp,2);
  end get_ppp;

/********************** GET_MAKORDER ***********************/
  function get_makorder( vgrb_xx grzbiet.xx%type) return varchar2
  as
    vret varchar2(256);
  begin
    select vformat.concat_list(cursor(select tytul||' '||mutacja||' '||rejkod.get_opis_drzewa(d.xx,m.kiedy) from drzewo d,makieta m,(select mak_xx,min(dst_nr_porz) nr from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx) t where t.mak_xx=m.xx and m.drw_xx=d.xx order by nr),'-')
      into vret from dual;
    return vret;
  end get_makorder;

/********************** GET_DEDINFO ***********************/
  function get_dedinfo( vgrb_xx grzbiet.xx%type) return varchar2
  as
    vlp binary_integer := 1;
    vret varchar2(4000);
  begin
    for c in (select mak_xx from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx) loop
       vret := vret || '-<a href=''/spacer/dinfo.aspx?m='||c.mak_xx||'''>'||vlp||'</a>';
       vlp := vlp + 1;
    end loop;
    return substr(vret,2);
  end get_dedinfo;

/********************** GET_TYTMAKORDER ***********************/
  function get_tytmakorder(vgrb_xx in grzbiet.xx%type) return varchar2
  as
    vret varchar2(256);
  begin
    select vformat.concat_list(cursor(select tytul||' '||mutacja from drzewo d,makieta m,(select mak_xx,min(dst_nr_porz) nr from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx) t where t.mak_xx=m.xx and m.drw_xx=d.xx order by nr),'-')
      into vret from dual;
    return vret;
  end get_tytmakorder;

/********************** GET_OPISORDER ***********************/
  function get_opisorder(vgrb_xx in grzbiet.xx%type) return varchar2
  as
    vtyt1ch char;
    vret varchar2(256);
    vret2 varchar2(256);
  begin
    select substr(tytul,1,1) into vtyt1ch from grzbiet where xx=vgrb_xx;
    if vtyt1ch in ('U','Y') then
      select rejkod.get_opis_drzewa(d.xx,g.kiedy) into vret
        from drzewo d, grzbiet g where g.xx=vgrb_xx and g.tytul=d.tytul and g.mutgrb=d.mutacja;
      select vformat.concat_list(cursor(select rejkod.get_opis_drzewa(d.xx,m.kiedy) from drzewo d,makieta m,(select mak_xx,min(dst_nr_porz) nr from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx) t where t.mak_xx=m.xx and m.drw_xx=d.xx and nr>0 order by nr),'-')
        into vret2 from dual;
      if vret2 is not null then
         vret := vret||' - '||vret2;
      end if;
    else
      select vformat.concat_list(cursor(select rejkod.get_opis_drzewa(d.xx,m.kiedy) from drzewo d,makieta m,(select mak_xx,min(dst_nr_porz) nr from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx) t where t.mak_xx=m.xx and m.drw_xx=d.xx order by nr),'-')
        into vret from dual;
    end if;
    return vret;
  end get_opisorder;

/********************** GET_UWAGIORDER ***********************/
  function get_uwagiorder(vgrb_xx in grzbiet.xx%type) return varchar2
  as
    vret varchar2(1024);
  begin
    select vformat.concat_list(cursor(select decode(grzbietowanie,null,null,tytul||' '||mutacja||': '||grzbietowanie) from drzewo d,makieta m,(select mak_xx,min(dst_nr_porz) nr from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx) t where t.mak_xx=m.xx and m.drw_xx=d.xx order by nr),';')
      into vret from dual;
    return vret;
  end get_uwagiorder;

/********************** GET_OBJETOSCI ***********************/
  function get_objetosci( vgrb_xx grzbiet.xx%type) return varchar2
  as
    vret varchar2(256);
  begin
    select vformat.concat_list(cursor(select objetosc from makieta m,(
       select mak_xx,dst_nr_porz nr from table(select strony from grzbiet where xx=vgrb_xx) s where s.base_nr_porz=(select min(base_nr_porz) from table(select strony from grzbiet where xx=vgrb_xx) s2 where s.mak_xx=s2.mak_xx and base_nr_porz>0)
    ) t where t.mak_xx=m.xx order by nr),'-') into vret from dual;
    
    /*select vformat.concat_list(cursor(select objetosc from makieta m,(select mak_xx,min(dst_nr_porz) nr from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx) t where t.mak_xx=m.xx order by nr),'-')
      into vret from dual;  ta wersja przeklamuje przy zdublowanych makietach*/
    return vret;
  end get_objetosci;

/********************** GET_MAKODP ***********************/
  function get_makodp( vgrb_xx grzbiet.xx%type) return varchar2
  as
    vret varchar2(256);
  begin
    select vformat.concat_list(cursor(select initcap(kto_makietuje) from makieta m,(select mak_xx,min(dst_nr_porz) nr from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx) t where m.xx=t.mak_xx order by nr),'<br>')
      into vret from dual;
    return vret;
  end get_makodp;

/********************** GET_COLOR ***********************/
  function get_color( vgrb_xx grzbiet.xx%type) return varchar2 as
    vret varchar2(128);
    vpocz pls_integer := null;
    vobj grzbiet.objetosc%type;
    vincolor boolean := false;
  begin
    select objetosc into vobj from grzbiet where xx=vgrb_xx;

    select vformat.cecha_stron(cursor(select /*+ cardinality(t 44) */ ile_kol-1
      from spacer_strona s,(select mak_xx,base_nr_porz,decode(dst_nr_porz,0,vobj,dst_nr_porz) dst_nr_porz
  	  from table(select strony from grzbiet where xx=vgrb_xx)) t
      where t.mak_xx=s.mak_xx and t.base_nr_porz=s.nr_porz order by t.dst_nr_porz
    )) into vret from dual;

    return vret;
  end get_color;

/********************** GET_PAPER ***********************/
  function get_paper(vgrb_xx in grzbiet.xx%type) return varchar2 
  as
    vpapier varchar2(512);
  begin 
    select szerokosc||'x'||wysokosc||' [mm]' into vpapier 
      from plan_wyd.wydanie_produktu wp,makieta m,drzewo d 
     where m.xx=decode(sign(vgrb_xx),1,oklgrb(vgrb_xx),-vgrb_xx) and m.drw_xx=d.xx and wp.data_edycji=m.kiedy and wp.kod_produktu=d.tytul||d.mutacja;
        
    return vpapier; 
  exception
    when others then return null;
  end get_paper;
  
/********************** GET_WYDAW_STR ***********************/
  function get_wydaw_str(vgrb_xx in grzbiet.xx%type) return varchar2 
  as
    vcc number; 
    vobj grzbiet.objetosc%type;
    vwydawca varchar2(512);
    vnullwyd_xx wydawca.xx%type;
  begin 
    if vgrb_xx > 0 then
      for c in (
        select /*+ cardinality(g 44) */ distinct d.wyd_xx from drzewo d,makieta m,table(select strony from grzbiet where xx=vgrb_xx) g where d.xx=m.drw_xx and m.xx=g.mak_xx and d.wyd_xx is not null
        union select wyd_xx from (
        select /*+ cardinality(g 44) */ wyd_xx from spacer_strona s,table(select strony from grzbiet where xx=vgrb_xx) g
         where s.mak_xx=g.mak_xx and s.nr_porz=g.base_nr_porz and s.wyd_xx is not null
         group by s.wyd_xx order by min(g.dst_nr_porz))
      ) loop
        select vwydawca||' '||w.sym||': '||vformat.cecha_stron(cursor(
                  select /*+ cardinality(g 44) */ decode(nvl(wyd_xx,(select d.wyd_xx from drzewo d,makieta m where d.xx=m.drw_xx and m.xx=s.mak_xx)),c.wyd_xx,1,0)
                    from spacer_strona s,table(select strony from grzbiet where xx=vgrb_xx) g
                   where s.mak_xx=g.mak_xx and s.nr_porz=g.base_nr_porz 
                   order by decode(g.dst_nr_porz,0,vobj,g.dst_nr_porz)  
               ))||';'
          into vwydawca from wydawca w where w.xx=c.wyd_xx;
      end loop;
    else --wersja dla makiety
      select m.objetosc,d.wyd_xx into vobj,vnullwyd_xx from drzewo d,makieta m where d.xx=m.drw_xx and m.xx=-vgrb_xx;

      for c in (
        select vnullwyd_xx wyd_xx,count(1) cnt from spacer_strona s where s.mak_xx=-vgrb_xx and s.wyd_xx is null having count(1)>0
        union select wyd_xx,cnt from (
        select wyd_xx,count(1) cnt from spacer_strona s where s.mak_xx=-vgrb_xx and s.wyd_xx is not null group by wyd_xx)
      ) loop
        select vwydawca||' '||w.sym||' ('||c.cnt||'): '||vformat.cecha_stron(cursor(
                  select /*+ cardinality(g 44) */ decode(nvl(wyd_xx,vnullwyd_xx),c.wyd_xx,1,0)
                    from spacer_strona s where s.mak_xx=-vgrb_xx
                   order by decode(s.nr_porz,0,vobj,s.nr_porz)  
               ))||';'
          into vwydawca from wydawca w where w.xx=c.wyd_xx;
      end loop;
    end if;
        
    return vwydawca; 
  exception
    when others then return null;
  end get_wydaw_str;

/******************** MUTRED_FITNESS *********************/
  function mutred_fitness(
    vstr drzewo.mutacja%type, 
    vred drzewo.mutacja%type, 
    vgrb drzewo.mutacja%type
  ) return number as 
    vstrlen pls_integer;
    vredlen pls_integer;
    vret number;
  begin
    select
    (select nvl(min(poziom),-1) from drzewo_mutacji where mutacja=vstr start with mutacja=vgrb connect by xx=prior root_xx),
    (select nvl(min(poziom),-1) from drzewo_mutacji where mutacja=vred start with mutacja=vgrb connect by xx=prior root_xx)
    into vstrlen, vredlen from dual;
  
    if vstrlen>=0 and vredlen>=0 then
      return vredlen - vstrlen;
    else
      return -1;
    end if;
  end mutred_fitness;

/******************** PLATECODE_IN_MAK *********************/
  function platecode_in_mak (
    vcode in blacha.code%type, 
    vmak_xx in makieta.xx%type
  ) return pls_integer as 
    vcc pls_integer;
    vtytmut varchar2(5);
    vnr_porz spacer_strona.nr_porz%type;
  begin 
    select instr(vcode,tytul||mutacja) into vcc
      from drzewo d, makieta m where m.xx=vmak_xx and m.drw_xx=d.xx;
    if vcc>0 then
      return 1;
    end if;
  
    select nvl(min(instr(vcode,tytul||mutacja)),0) into vcc
      from drzewo d, makieta m, makieta base, spacer_strona s, spacer_strona base_str
     where m.xx=vmak_xx and s.mak_xx=m.xx and base_str.mak_xx=base.xx
       and decode(base_str.nr_porz,0,base.objetosc,base_str.nr_porz)=to_number(substr(vcode,1,3)) 
       and s.dervlvl=sr.derv_fixd and s.derv_nr_porz=base_str.nr_porz
       and s.derv_mak_xx=base.xx and base.drw_xx=d.xx;
    return vcc;
  exception
    when no_data_found then
      return 0;  
  end platecode_in_mak;

/******************** DELETE_GRZBIET *********************/
  procedure delete_grzbiet(vgrb_xx in grzbiet.xx%type)
  is
    vkiedy grzbiet.kiedy%type;
    vtytul grzbiet.tytul%type;
    vmutgrb grzbiet.mutgrb%type;
    vgrb_dirty_xx grzbiet.xx%type := -1;
    vmak_split_xx makieta.xx%type;
    nested_null exception;
    pragma exception_init(nested_null,-22908);
  begin
    wyprzedzeniowe.check_can_modify(vgrb_xx);
    update makingrb set grb_okl_xx=null
        where mak_xx in (select /*+ cardinality (gs 40) */ mak_xx from table(select strony from grzbiet where xx=vgrb_xx) gs)
          and grb_okl_xx=vgrb_xx
        returning mak_xx,grb_xx into vmak_split_xx,vgrb_dirty_xx;
    if vgrb_dirty_xx>0 then
      wyprzedzeniowe.check_can_modify(vgrb_dirty_xx);
      update grzbiet set dirty=vmak_split_xx where xx=vgrb_dirty_xx;
    end if;
    delete from makingrb
        where mak_xx in (select /*+ cardinality (gs 40) */ mak_xx from table(select strony from grzbiet where xx=vgrb_xx) gs)
          and grb_xx=vgrb_xx;
    delete from table(select strony from grzbiet where xx=vgrb_xx);
    delete from blacha where grb_xx in(vgrb_xx,vgrb_dirty_xx);
    delete from kolp_grb where grb_xx=vgrb_xx;
    delete from podtytul where grb_xx=vgrb_xx;
    delete from grzbiet where xx=vgrb_xx returning tytul,mutgrb,kiedy into vtytul,vmutgrb,vkiedy;
    delete from grzbiet_wydawcy where tytul=vtytul and mutgrb=vmutgrb and kiedy=vkiedy;
    if vtytul='DGW' then
      update live.ag_naklad_plan@promix set status_flag=' ',data_importu=sysdate
       where part_code_alt='DGW' and tch_data_wydania=vkiedy;
      update grzbiet set naklad=0 where tytul='DGW' and kiedy=vkiedy;
    end if;
  exception
    when nested_null then
        null;
  end delete_grzbiet;

/******************** ZATWIERDZ_GRZBIET *********************/
  procedure zatwierdz_grzbiet (
    vgrb_xx in out grzbiet.xx%type,
    vflag in pls_integer,
    vprzefalc in grzbiet.przefalc%type,
    vdepeszowy in grzbiet.depeszowy%type,
    vszycie in varchar2,
    vuwagi in grzbiet.uwagi%type
  ) as begin
    if vgrb_xx < 0 then
      vgrb_xx := init_grzbiet(-vgrb_xx);
    end if;

    wyprzedzeniowe.set_accept_flag(vgrb_xx,vflag);

    update grzbiet set
       depeszowy=decode(vdepeszowy,null,depeszowy,0,null,vdepeszowy),
       przefalc=decode(vprzefalc,null,przefalc,0,null,vprzefalc),
       uwagi=vuwagi
     where xx=vgrb_xx;

    update makingrb mg set szycie=nvl((select szyj
      from (select mak_xx,substr(vszycie,rownum,1) szyj 
              from (select mak_xx,min(dst_nr_porz) 
                      from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx order by 2)) t
             where t.mak_xx=mg.mak_xx),szycie)
     where mak_xx in (select /*+ cardinality (gs 40) */ mak_xx from table(select strony from grzbiet where xx=vgrb_xx) gs) and grb_xx=vgrb_xx;

    update makieta m set szycie=nvl((select szyj
      from (select mak_xx,substr(vszycie,rownum,1) szyj 
              from (select mak_xx,min(dst_nr_porz) 
                      from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx order by 2)) t
             where t.mak_xx=m.xx),szycie)
     where xx in (select /*+ cardinality (gs 40) */ mak_xx from table(select strony from grzbiet where xx=vgrb_xx) gs);

    insert into spacer_log (mak_xx,opis) values (-vgrb_xx,'Zatwierdzenie grzbietu := '||vflag);
  end zatwierdz_grzbiet;

/******************** KTO_ZATWIERDZIL *********************/
  procedure kto_zatwierdzil (
    vgrb_xx in grzbiet.xx%type,
    vretCur in out sr.refCur
  ) as begin
    open vretCur for select
      null,
      null,
      decode(acc_print_ldrz,null,null,(select imie||' '||nazwisko||' tel. '||tel from spacer_users where xx=acc_print_ldrz)) l,
      decode(acc_print_cdrz,null,null,(select imie||' '||nazwisko||' tel. '||tel from spacer_users where xx=acc_print_cdrz)) c,
      decode(acc_print_org,null,null,(select imie||' '||nazwisko||' tel. '||tel from spacer_users where xx=acc_print_org)) o
      from grzbiet
     where xx=vgrb_xx;
  end kto_zatwierdzil;

/******************** UPDATE_INFO *********************/
  procedure update_info (
    vgrb_xx in grzbiet.xx%type,
    vprzefalc in number,
    vdrukarnie in number,
    vdeadline in varchar2,
    vnaklad in number,
    vgramatura in number,
    vtyp_pap_xx in number default null,
    vkol_pap_xx in number default null
  ) as 
    vi number;
    vmsg varchar2(6);
  begin
    update grzbiet
       set przefalc=decode(vprzefalc,0,null,1),drukarnie=decode(vdrukarnie,0,null,vdrukarnie),
           deadline=decode(vdeadline,'01/01/2004 00:00',null,to_date(vdeadline,sr.vfLongDate)),
           naklad=vnaklad,
           gramatura=decode(vgramatura,0,null,vgramatura)
     where xx=vgrb_xx;
     
    for c_m in (select distinct mak_xx from table(select strony from grzbiet where xx=vgrb_xx)) loop
      update spacer_strona s set s.drukarnie=0 where s.mak_xx=c_m.mak_xx and s.dervlvl<>derv.fixd
         and exists (select 1 from table(select strony from grzbiet where xx=vgrb_xx) g 
                      where g.base_nr_porz=s.nr_porz and g.mak_xx=s.mak_xx);
      for c_g in (
        select gr2.grb_xx,nvl(gr.drukarnie,0) druk from grzbiet gr,
        (select grb_xx from makingrb where mak_xx=c_m.mak_xx
         union all select grb_okl_xx from makingrb where mak_xx=c_m.mak_xx and grb_okl_xx is not null
         union all select g.xx from grzbiet g,drzewo d,split_grzbietu s,makieta m
         where g.kiedy=m.kiedy and d.xx=m.drw_xx and m.xx=c_m.mak_xx
           and g.mutgrb=d.mutacja and g.tytul=s.skladka1 and s.okladka=d.tytul
         union all select g.xx from grzbiet g,drzewo d,split_grzbietu s,makieta m
         where g.kiedy=m.kiedy and d.xx=m.drw_xx and m.xx=c_m.mak_xx
           and g.mutgrb=d.mutacja and g.tytul=s.skladka2 and s.okladka=d.tytul) gr2
        where gr.xx=gr2.grb_xx
      ) loop
        update spacer_strona s set drukarnie=drukarnie-bitand(drukarnie,c_g.druk)+c_g.druk 
         where s.dervlvl<>derv.fixd and (mak_xx,nr_porz) in (select /*+ cardinality (gs 40) */ mak_xx,base_nr_porz
          from table(select strony from grzbiet where xx=c_g.grb_xx) gs);      
      end loop;

      if vdrukarnie>0 then
        select min(decode(s.nr_porz,0,m.objetosc,s.nr_porz)) into vi 
          from papier_ads pa, spacer_strona s, makieta m
         where pa.xx=s.pap_xx and s.mak_xx=m.xx and m.xx=c_m.mak_xx
           and ((bitand(vdrukarnie,grb.cDrukAG)>0 and pa.druk_obcy=1)
            or (bitand(vdrukarnie,grb.cDrukAG)=0 and pa.druk_obcy is null))
           and exists (select 1 from table(select strony from grzbiet where xx=vgrb_xx) g where g.base_nr_porz=s.nr_porz and g.mak_xx=m.xx);
        if vi is not null then
          rollback;
          select tytul into vmsg from mak where mak_xx=c_m.mak_xx;
          raise_application_error(-20001,'Rodzaj papieru na '||vi||' stronie '||vmsg||' i wybrana drukarnia ('||vdrukarnie||') nie sa ze soba zgodne');
        end if;
      end if;      
    end loop;
     
    commit;
  end update_info;

/******************** SET_MUTGRB_EXPLICIT *********************/
  procedure set_mutgrb_explicit (
    vgrb_xx in grzbiet.xx%type,
    vmutgrb in drzewo_mutacji.mutacja%type
  ) as
    vcc pls_integer;
    vmak_xx makieta.xx%type;
    vobszar grzbiet.obszar%type;
    vmak_cap makieta.objetosc%type;
    vgrb_cap grzbiet.objetosc%type;
    vsplit_clone boolean := false;
  begin
    vmak_xx := oklgrb(vgrb_xx);
    select objetosc into vmak_cap from makieta where xx=vmak_xx;
    select objetosc into vgrb_cap from grzbiet where xx=vgrb_xx;
    if vmak_cap > vgrb_cap then
        select count(1) into vcc from table(select strony from grzbiet where xx=vgrb_xx)
         where mak_xx=vmak_xx and base_nr_porz=0 and dst_nr_porz=0;
        if vcc = 1 then --makieta wieksza niz grzbiet zawierajacy jej okladke jako swoja
            select count(distinct mak_xx) into vcc from table(select strony from grzbiet where xx=vgrb_xx);
            if vcc = 1 then
                vsplit_clone := true;
            end if;
        end if;
    end if;

    select min(reguly.zastosuj_reguly_kolp(vgrb_xx,
           (select p.obszar from default_plan_grzbietu p,drzewo d,grzbiet g 
             where g.xx=vgrb_xx and p.dow=to_char(g.kiedy,'d') and p.drw1_xx=d.xx and p.drw2_xx is null and d.tytul=g.tytul and p.mutgrb=vmutgrb))) 
      into vobszar from dual;
    update grzbiet set mutgrb=vmutgrb,obszar=nvl(vobszar,obszar),acc_print_cdrz=null,acc_print_org=null
     where xx=vgrb_xx;

    merge into kod_grzbietu k
    using (select g.tytul,vmutgrb mutacja from grzbiet g where xx=vgrb_xx) t
       on (k.tytul=t.tytul and k.mutgrb=t.mutacja)
     when matched then update set last_ed=sysdate
     when not matched then insert (k.tytul,k.mutgrb,k.first_ed,k.last_ed)
                           values (t.tytul,t.mutacja,sysdate,sysdate);         

    if vsplit_clone then
    begin
        vmak_xx := init_grzbiet(vmak_xx);
        delete from table(select strony from grzbiet where xx=vmak_xx)
        where dst_nr_porz>vgrb_cap/2 and dst_nr_porz<=vmak_cap-vgrb_cap/2;
        update table(select strony from grzbiet where xx=vmak_xx)
           set dst_nr_porz=dst_nr_porz-vmak_cap+vgrb_cap
         where dst_nr_porz>vmak_cap/2;
        update grzbiet set objetosc=vgrb_cap where xx=vmak_xx;
    exception
      when others then
        null;
    end;
    end if;

    commit;
  exception
    when dup_val_on_index then
        raise_application_error(-20002,'Grzbiet z mutacja '||vmutgrb||' na ten dzien juz istnieje');
  end set_mutgrb_explicit;

/******************** REFRESH_GRZBIET *********************/
  procedure refresh_grzbiet(
    vgrb_xx in grzbiet.xx%type,
    vmak_xx in makieta.xx%type
  ) is
    vi sr.srsmallint;
    voklmak_xx makieta.xx%type;
    vgrb_okl_xx grzbiet.xx%type;
    vmutgrb grzbiet.mutgrb%type;
    vorgmutgrb grzbiet.mutgrb%type;
    vgrb_aux_xx grzbiet.xx%type := vgrb_xx;
    vmaklist spacer_numlist := spacer_numlist();
    nested_null exception;
    pragma exception_init(nested_null,-22908);
  begin
    select count(1) into vi
      from grzbiet g, split_grzbietu s
     where g.xx=vgrb_xx and g.tytul in (s.skladka1,s.skladka2);
    if vi > 0 then
      delete_grzbiet(vgrb_xx);
      return;
    end if;

    select mutgrb into vorgmutgrb from grzbiet where xx=vgrb_xx;
    select min(grb_okl_xx) into vgrb_okl_xx
      from makingrb where mak_xx=nvl(vmak_xx,oklgrb(vgrb_xx)) and grb_xx=vgrb_xx;
    if vgrb_okl_xx is not null then -- makieta pofragmentowana
        voklmak_xx := oklgrb(vgrb_xx);
        update grzbiet set dirty=null
         where dirty=nvl(vmak_xx,voklmak_xx) and xx=vgrb_okl_xx;
        shrink_capacity(vgrb_okl_xx,0,0,vmutgrb);
    end if;

    for cur in (
       select mak_xx from table(select strony from grzbiet where xx=vgrb_xx)
        where exists (select 1 from makieta where xx=mak_xx)
        group by mak_xx order by min(dst_nr_porz)
    ) loop
        vmaklist.extend;
        vmaklist(vmaklist.last) := cur.mak_xx;
    end loop;

    select min(grb_xx) into vgrb_okl_xx
      from makingrb
     where mak_xx=vmaklist(vmaklist.last) and grb_okl_xx=vgrb_xx;
    if vgrb_okl_xx is not null then -- grzbiet pofragmentowany
        refresh_grzbiet(vgrb_okl_xx,vmaklist(vmaklist.last));
        vmaklist.trim;
    end if;

    update grzbiet set objetosc=0 where xx=vgrb_xx;
    delete from makingrb where grb_xx=vgrb_xx and mak_xx in (
        select /*+ cardinality (gs 40) */ mak_xx from table(select strony from grzbiet where xx=vgrb_xx) gs);
    delete from table(select strony from grzbiet where xx=vgrb_xx);
    for vi in 1..vmaklist.last loop
        select drw_xx into vgrb_okl_xx from makieta where xx=vmaklist(vi);
        expand_capacity(vgrb_aux_xx,vgrb_okl_xx,0,vmutgrb);
    end loop;

    if vorgmutgrb<>vmutgrb then
        set_mutgrb_explicit(vgrb_xx, vorgmutgrb);
    end if;

    update grzbiet set dirty=null where xx=vgrb_xx;
    commit;
  exception
    when nested_null then
        null;
  end refresh_grzbiet;

/******************** DROP_SYSLOCK *********************/
  procedure drop_syslock(vgrb_xx in grzbiet.xx%type)
  is begin
    update grzbiet set holder=null
     where xx=vgrb_xx and holder=uid;
  end drop_syslock;

/******************** SHRINK_CAPACITY *********************/
  procedure shrink_capacity (
    vgrb_xx in out grzbiet.xx%type,
    vdrw_xx in sr.srtinyint,
    vcount in sr.srsmallint,
    vmutgrb out grzbiet.mutgrb%type
  ) is
    vbase_mak_xx makieta.xx%type;
    vcc sr.srsmallint;
    vdst_half_cap number;
  begin    
    begin
      select objetosc/2 into vdst_half_cap
        from grzbiet where xx=vgrb_xx;
    exception
      when no_data_found then
        vgrb_xx := init_grzbiet(-vgrb_xx);
        select objetosc/2 into vdst_half_cap
          from grzbiet where xx=vgrb_xx;
    end;

    wyprzedzeniowe.check_can_modify(vgrb_xx);

    if vcount = 0 then -- usun
      if vdrw_xx = 0 then -- pojedynczy ze srodka
        select count(distinct mak_xx) into vcc
          from table(select strony from grzbiet where xx=vgrb_xx);
        if vcc = 1 then
            select count(1) into vcc
              from grzbiet g, makieta m, drzewo d, table(g.strony) s
             where g.xx=vgrb_xx and d.xx=m.drw_xx and m.xx=s.mak_xx and
                   s.dst_nr_porz=0 and (g.tytul<>d.tytul or s.base_nr_porz=0);
            if vcc > 0 then -- grzbiet po wydzielaniu
              delete_grzbiet(vgrb_xx);
              vmutgrb := null;
              return;
            else
              raise_application_error(-20001,'Ten grzbiet nie zawiera insertow');
            end if;
        end if;

        select mak_xx into vbase_mak_xx
          from table(select strony from grzbiet where xx=vgrb_xx)
         where dst_nr_porz=vdst_half_cap;

        delete from table(select strony from grzbiet where xx=vgrb_xx)
         where mak_xx=vbase_mak_xx;
        vcc := sql%rowcount;

        update table(select strony from grzbiet where xx=vgrb_xx)
           set dst_nr_porz=dst_nr_porz-vcc
         where dst_nr_porz>vdst_half_cap;
        update grzbiet set objetosc=objetosc-vcc where xx=vgrb_xx;
        -- usuwajac okladki pofragmentowanej makiety oznacz grzbiet
        delete from makingrb
            where mak_xx=vbase_mak_xx and grb_xx=vgrb_xx;
        update makingrb set grb_okl_xx=null
            where mak_xx=vbase_mak_xx and grb_okl_xx=vgrb_xx
                returning grb_xx into vcc;
        update grzbiet set dirty=vbase_mak_xx where xx=vcc;
      else -- wskazany produkt
        begin
          select min(sg.dst_nr_porz),sg.mak_xx into vcc, vbase_mak_xx
            from table(select strony from grzbiet where xx=vgrb_xx) sg
           where exists (select 1 from makieta m, grzbiet g
                where m.drw_xx=vdrw_xx and m.kiedy=g.kiedy and g.xx=vgrb_xx and m.xx=sg.mak_xx)
           group by mak_xx;
          if vcc = 0 then
            raise_application_error(-20002,'Nie usuwa sie okladki grzbietu - usun caly grzbiet');
          end if;

          delete from makingrb where mak_xx=vbase_mak_xx and vgrb_xx in (grb_xx,grb_okl_xx);
          delete from table(select strony from grzbiet where xx=vgrb_xx) where mak_xx=vbase_mak_xx;
          update table(select strony from grzbiet where xx=vgrb_xx) sg
             set dst_nr_porz=(select count(1) from table(select strony from grzbiet where xx=vgrb_xx) sg2 where sg2.dst_nr_porz<sg.dst_nr_porz);
          update grzbiet set objetosc=(select count(1) from table(select strony from grzbiet where xx=vgrb_xx)) where xx=vgrb_xx;
        exception
          when no_data_found then
            raise_application_error(-20003,'Wskazana makieta nie nalezy do tego grzbietu');
        end;
      end if;
    else -- usun okladki -- to jest wywolywane tylko z expand_capacity
        select mak_xx into vbase_mak_xx
          from table(select strony from grzbiet where xx=vgrb_xx)
         where dst_nr_porz=0;
        select count(1) into vcc
          from table(select strony from grzbiet where xx=vgrb_xx)
         where mak_xx=vbase_mak_xx;
        if vcc<=4*vcount then
            raise_application_error(-20004,'Usuniecie zadeklarowanej liczby kartek z wierzchu grzbietu spowodowaloby usuniecie wszystkich stron makiety bazowej grzbietu');
        end if;
        delete from table(select strony from grzbiet where xx=vgrb_xx)
         where dst_nr_porz<=2*vcount;
        delete from table(select strony from grzbiet where xx=vgrb_xx)
         where dst_nr_porz>2*(vdst_half_cap-vcount);
        update table(select strony from grzbiet where xx=vgrb_xx)
           set dst_nr_porz=0 where dst_nr_porz=2*(vdst_half_cap-vcount);
        update table(select strony from grzbiet where xx=vgrb_xx)
           set dst_nr_porz=dst_nr_porz-2*vcount where dst_nr_porz<>0;
        update grzbiet set objetosc=objetosc-4*vcount where xx=vgrb_xx;
    end if;

    vmutgrb := set_mutgrb(vgrb_xx);
    delete from blacha where grb_xx=vgrb_xx;

    commit;
  end shrink_capacity;

/******************** EXPAND_CAPACITY *********************/
  procedure expand_capacity (
    vgrb_xx in out grzbiet.xx%type,
    vdrw_xx in drzewo.xx%type,
    vcount in sr.srsmallint,
    vmutgrb out grzbiet.mutgrb%type
  ) is
    vmsg varchar2(30);
    vbase_cap number;
    vcc sr.srsmallint;
    vdst_half_cap number;
    vbase_ppp drzewo.ppp%type;
    vbase_mak_xx makieta.xx%type;
    vgrb_split_xx grzbiet.xx%type;
    vsplit_clone boolean := false;
    vdrukarnie grzbiet.drukarnie%type;
  begin
    if vgrb_xx < 0 then
        vgrb_xx := init_grzbiet(-vgrb_xx);
    end if;

    wyprzedzeniowe.check_can_modify(vgrb_xx);

    begin
        select base.xx, base.objetosc, g.objetosc/2, d.ppp, g.drukarnie
          into vbase_mak_xx, vbase_cap, vdst_half_cap, vbase_ppp, vdrukarnie
          from makieta base, grzbiet g, drzewo d
         where base.drw_xx=vdrw_xx and g.xx=vgrb_xx
           and g.kiedy=base.kiedy and d.xx=base.drw_xx;
        if vbase_cap not in (12,20) and mod(vbase_cap,2*vbase_ppp)<>0 and bitand(vdrukarnie,cDrukAG)>0 then
            raise_application_error(-20001,'Makieta wskazanego tytulu na dzien edycji edytowanego grzbietu ma objetosc niepodzielna przez 4');
        end if;
    exception
        when no_data_found then
            raise_application_error(-20002,'Makieta wskazanego tytulu na dzien edycji edytowanego grzbietu nie istnieje');
    end;

    /*select min(decode(s.nr_porz,0,m.objetosc,s.nr_porz)) into vcc from spacer_strona s, makieta m
     where m.xx=s.mak_xx and s.mak_xx=vbase_mak_xx and s.pap_xx is null;
    if vcc is not null then
      select tytul||' '||kiedy into vmsg from mak where mak_xx=vbase_mak_xx;
      raise_application_error(-20001,'Na makiecie '||vmsg||' nie okreslono papieru dla strony '||vcc);
    end if;*/

    vgrb_split_xx := oklgrb(vgrb_xx);
    if vgrb_split_xx is not null then
      select objetosc into vcc from makieta where xx=vgrb_split_xx;
      if vcc > 2*vdst_half_cap then
          select count(1) into vcc from table(select strony from grzbiet where xx=vgrb_xx)
           where mak_xx=vgrb_split_xx and base_nr_porz=0 and dst_nr_porz=0;
          if vcc = 1 then --makieta wieksza niz grzbiet zawierajacy jej okladke jako swoja
            select count(distinct mak_xx) into vcc from table(select strony from grzbiet where xx=vgrb_xx);
            if vcc = 1 then
                vsplit_clone := true;
            end if;
          end if;
      end if;
    end if;

    /*select count(1) into vcc
      from table(select strony from grzbiet where xx=vgrb_xx)
     where mak_xx=vbase_mak_xx;
    if vcc > 0 and not (vcc=vbase_cap and vcc=2*vdst_half_cap) then
        raise_application_error(-20004,'Operacja rozszerzenia grzbietu prowadzilaby do duplikacji stron');
    end if;*/

    select count(1) into vcc
      from makingrb mg, table(select strony from grzbiet where xx=vgrb_xx) g
     where mg.grb_okl_xx=vgrb_xx and mg.mak_xx=g.mak_xx and g.dst_nr_porz=vdst_half_cap;
    if vcc > 0 then -- fragment w srodku
        select count(distinct mak_xx) into vcc
          from table(select strony from grzbiet where xx=vgrb_xx);
        if vcc > 1 then
            raise_application_error(-20005,'Nie wolno rozszerzac grzbietu, ktory w srodku ma niepelna makiete');
        end if;
    end if;

    -- ogranicz objetosc
    if (vcount = 0.0 and vbase_cap + 2*vdst_half_cap > maxCap)
    or ( vcount > 0.0 and 4*vcount + 2*vdst_half_cap > maxCap) then
        select bitand(drukarnie,(select sum(xx) from spacer_drukarnie where agorowa is null)) into vcc from grzbiet where xx=vgrb_xx;
        if vcc = 0 then
           raise_application_error(-20006,'Powiekszenie objetosci jest zbyt duze.'||chr(13)||'Maksymalna objetosc grzbietu drukarnianego w Agorze wynosi '||maxCap);
        end if;
    end if;

    if vcount = 0.0 then -- wstaw wszystko
        update table(select strony from grzbiet where xx=vgrb_xx)
           set dst_nr_porz=dst_nr_porz+vbase_cap
         where dst_nr_porz>vdst_half_cap;
        insert into table(select strony from grzbiet where xx=vgrb_xx)
             select mak_xx,decode(nr_porz,0,decode(vdst_half_cap,0,0,vbase_cap),nr_porz)+vdst_half_cap,nr_porz,1
               from spacer_strona where mak_xx=vbase_mak_xx;
        update grzbiet set objetosc=objetosc+vbase_cap where xx=vgrb_xx;
        insert into makingrb (mak_xx,grb_xx)
             values (vbase_mak_xx,vgrb_xx);
    else -- wstaw strony z zewnatrz base do srodka dst
        if vbase_cap <= 4*vcount then
            raise_application_error(-20007,'Liczba stron na kartkach, ktore maja byc wstawione musi byc mniejsza od objetosci makiety zrodlowej');
        end if;
        -- wewnetrzny fragment utworzy nowy grzbiet
        vgrb_split_xx := init_grzbiet(vbase_mak_xx);
        shrink_capacity(vgrb_split_xx,0,vcount,vmutgrb);
        vsplit_clone := false;
        -- wstaw okladki do srodka
        update table(select strony from grzbiet where xx=vgrb_xx)
           set dst_nr_porz=dst_nr_porz+vcount*4
         where dst_nr_porz>vdst_half_cap;
        insert into table(select strony from grzbiet where xx=vgrb_xx)
             select mak_xx,nr_porz+vdst_half_cap,nr_porz,1
               from spacer_strona
              where mak_xx=vbase_mak_xx and nr_porz>0 and nr_porz<=2*vcount;
        insert into table(select strony from grzbiet where xx=vgrb_xx)
             --select mak_xx,vdst_half_cap+2*vcount+decode(nr_porz,0,vbase_cap,nr_porz)-(vbase_cap-2*vcount),nr_porz,1
             select mak_xx,vdst_half_cap+4*vcount+decode(nr_porz,0,decode(vdst_half_cap,0,vbase_cap-4*vcount,vbase_cap),nr_porz)-vbase_cap,nr_porz,1
               from spacer_strona where mak_xx=vbase_mak_xx and (nr_porz=0 or nr_porz>vbase_cap-2*vcount);
        update grzbiet set objetosc=objetosc+4*vcount where xx=vgrb_xx;
        update makingrb set grb_okl_xx=vgrb_xx
         where mak_xx=vbase_mak_xx and grb_xx=vgrb_split_xx;
    end if;

    if vsplit_clone then
    begin
        vbase_mak_xx := oklgrb(vgrb_xx);
        vgrb_split_xx := init_grzbiet(vbase_mak_xx);
        select objetosc into vbase_cap from makieta where xx=vbase_mak_xx;
        delete from table(select strony from grzbiet where xx=vgrb_split_xx)
        where dst_nr_porz>vdst_half_cap and dst_nr_porz<=vbase_cap-vdst_half_cap;
        update table(select strony from grzbiet where xx=vgrb_split_xx)
           set dst_nr_porz=dst_nr_porz-vbase_cap+2*vdst_half_cap
         where dst_nr_porz>vbase_cap/2;
        update grzbiet set objetosc=2*vdst_half_cap where xx=vgrb_split_xx;
    exception
      when others then
        null;
    end;
      select mutgrb into vmutgrb from grzbiet where xx=vgrb_xx;
    else
      vmutgrb := set_mutgrb(vgrb_xx);
    end if;
    delete from blacha where grb_xx=vgrb_xx;

    commit;
  end expand_capacity;

/******************** SPLIT_CAPACITY *********************/
  procedure split_capacity (
    vgrb_xx in out grzbiet.xx%type,
    vdrw_xx in drzewo.xx%type,
    vcount in sr.srsmallint,
    vmutgrb out grzbiet.mutgrb%type
  ) as
    vdst_cap number;
    vcc sr.srsmallint;
    vmak_xx makieta.xx%type;
    vgrb_inner_xx grzbiet.xx%type;
  begin
    if vgrb_xx < 0 then
        vgrb_xx := init_grzbiet(-vgrb_xx);
    end if;

    wyprzedzeniowe.check_can_modify(vgrb_xx);

    select count(1) into vcc
      from makieta m, grzbiet g
     where m.drw_xx=vdrw_xx and m.kiedy=g.kiedy and g.xx=vgrb_xx;
    if vcc > 0 then
      raise_application_error(-20001,'Nie mozna nadac wydzielanej czesci grzbietu nazwy makiety zalozonej w dniu edycji.'||chr(13)||'Usun makiete lub zmien nazwe wydzielane czesci.');
    end if;

    select mak_xx,count(1) into vmak_xx,vcc
      from table(select strony from grzbiet where xx=vgrb_xx)
     where mak_xx=(select mak_xx from table(select strony from grzbiet where xx=vgrb_xx) gs, grzbiet g
                    where gs.dst_nr_porz=g.objetosc/2 and g.xx=vgrb_xx)
     group by mak_xx;
    if 4*vcount >= vcc then
      raise_application_error(-20002,'Nie mozna wydzielac nowego grzbietu z grzbietu, ktory zawiera wiecej niz jedna makiete, gdy wewnatrzna makieta jest niewieksza niz czesc wydzielana');
    end if;

    select objetosc into vdst_cap
      from grzbiet where xx=vgrb_xx;
    if vcount < 1 or 4*vcount >= vdst_cap then
      raise_application_error(-20003,'Proba wydzielenia czesci o nieprawidlowej wielkosci.');
    end if;

  begin
    insert into grzbiet (tytul,mutgrb,kiedy,objetosc,depeszowy,przefalc,drukarnie,strony,obszar,ded_xx)
         select d.tytul,d.mutacja,m.kiedy,4*vcount,d.depeszowy,d.przefalc,nvl(d.drukarnie,dm.drukarnie),grzbiet_strony(),
                g.obszar,g.ded_xx
           from drzewo d, makieta m, drzewo_mutacji dm, grzbiet g
          where d.xx=vdrw_xx and m.xx=vmak_xx and d.mutacja=dm.mutacja and g.xx=vgrb_xx;
    merge into kod_grzbietu k
    using (select d.tytul,d.mutacja from drzewo d where d.xx=vdrw_xx) t
       on (k.tytul=t.tytul and k.mutgrb=t.mutacja)
     when matched then update set last_ed=sysdate
     when not matched then insert (k.tytul,k.mutgrb,k.first_ed,k.last_ed)
                           values (t.tytul,t.mutacja,sysdate,sysdate);         
  exception
    when dup_val_on_index then
      raise_application_error(-20005,'Wydzielany grzbiet juz istnieje.');
  end;

    select grzbiet_xx.currval into vgrb_inner_xx from dual;
    insert into table(select strony from grzbiet where xx=vgrb_inner_xx) (mak_xx,dst_nr_porz,base_nr_porz,mutczas)
    select vmak_xx,decode(dst_nr_porz,vdst_cap/2+2*vcount,0,dst_nr_porz-vdst_cap/2+2*vcount),base_nr_porz,1
      from table(select strony from grzbiet where xx=vgrb_xx)
     where dst_nr_porz>=vdst_cap/2-2*vcount+1 and dst_nr_porz<=vdst_cap/2 + 2*vcount;

    delete from table(select strony from grzbiet where xx=vgrb_xx)
          where dst_nr_porz>=vdst_cap/2-2*vcount+1 and dst_nr_porz<=vdst_cap/2 + 2*vcount;
    update table(select strony from grzbiet where xx=vgrb_xx)
       set dst_nr_porz=dst_nr_porz-4*vcount
     where dst_nr_porz>vdst_cap/2;
    update grzbiet set objetosc=objetosc-4*vcount where xx=vgrb_xx;
    update makingrb set grb_okl_xx=vgrb_inner_xx where mak_xx=vmak_xx and grb_xx=vgrb_xx;

    vmutgrb := set_mutgrb(vgrb_xx);
    delete from blacha where grb_xx=vgrb_xx;

    commit;
  end split_capacity;

/******************** UPDATE_MUTCZAS *********************/
  procedure update_mutczas (
    vgrb_xx in out grzbiet.xx%type,
    vcount in sr.srsmallint,
    vnr_porzArr sr.numlist
  ) is
    vi sr.srsmallint;
  begin
    if vgrb_xx < 0 then
        vgrb_xx := init_grzbiet(-vgrb_xx);
    end if;

    for vi in 1..vcount loop
      update table(select strony from grzbiet where xx=vgrb_xx) g
         set mutczas=vnr_porzArr(vi)
       where dst_nr_porz=vi-1;
    end loop;
    commit;
  end update_mutczas;

/******************** OPEN_GRZBIET *********************/
  procedure open_grzbiet(
      vtytul in drzewo.tytul%type,
      vmutacja in drzewo.mutacja%type,
      vkiedy in char,
      visro in out number,  -- typ dostepu: 1 tylko odczyt, 0 z blokowaniem, -1 ret <==> nowy
      vretCur out sr.refCur)
  is
    vmsg varchar2(200);
    vgrb_xx number;
    vsl number;
    vnoaccess exception;
  begin
    --jezeli ogladacz,dealer lub admin to tylko odczyt
    select decode(gru_xx,0,1,1,1,16,1,visro)
      into visro from spacer_users where xx=uid;
    if to_date(vkiedy,sr.vfshortDate) + 0.25 < sysdate and uid<>515 then
       visro := 1;
    end if;
    
    --sekcja krytyczna
    if visro = 0 then
      lock table spacer_token in exclusive mode;
    end if;

    select grzbiet.xx, decode(user#,null,0,uid,0,user#)
      into vgrb_xx, vsl
      from grzbiet, v$session
     where grzbiet.tytul=vtytul
       and grzbiet.mutgrb=vmutacja
       and kiedy=to_date(vkiedy,sr.vfShortDate)
       and user#(+)=nvl(holder,0)
       and upper(program(+))='MANAM.EXE'
       and status(+)<>'KILLED';

    if not check_grb_access(vtytul,'R') then
        raise vnoaccess;
    elsif visro=0 and not check_grb_access(vtytul,'G') then
        visro := 1;
        commit;
    end if;

    if vsl > 0 then -- zablokowane przez zalogowanego uzytkownika
        select 'Grzbiet jest uzywany przez uzytkownika '||loginname||' tel. '||tel
          into vmsg
          from spacer_users where xx=vsl;
    elsif visro = 0 then
        update grzbiet set holder=uid where xx=vgrb_xx;
    end if;

    if visro = 0 then
      commit;
    end if;

    open vretCur for
      select vgrb_xx,
             0,
             g.objetosc,
             vmsg,
             get_opisorder(g.xx)
        from grzbiet g
       where g.xx=vgrb_xx;

exception
    when vnoaccess then
        rollback;
        raise_application_error(-20001, 'Uprawnienia uzytkownika '||user||' do tytulu '||vtytul||' '||vmutacja||' sa zbyt male');
    when no_data_found then
        rollback;
        visro := 1; -- nowy tylko do zmian
        if not check_grb_access(vtytul,'G') then
          raise_application_error(-20002, 'Uprawnienia uzytkownika '||user||' nie pozwalaja na utworzenie grzbietu '||vtytul||vmutacja);
        end if;
        begin
          select m.objetosc/2*d.ppp into vsl
            from makieta m, drzewo d
           where m.drw_xx=d.xx and m.kiedy=to_date(vkiedy,sr.vfShortDate)
             and d.tytul=vtytul and d.mutacja=vmutacja;
          if trunc(vsl)<>vsl then
              raise_application_error(-20003, 'Makieta '||vtytul||vmutacja||' na '||vkiedy||' nie moze byc podstawa grzbietu.'||chr(13)||'Jej objetosc nie jest wielokrotnoscia liczby stron produkowanych z jednej blachy.');
          end if;
        exception
          when no_data_found then
              raise_application_error(-20004, 'Grzbiet i makieta '||vtytul||vmutacja||' na '||vkiedy||' nie istnieja.');
        end;
        space_reservation.open_makieta(vtytul,vmutacja,vkiedy,visro,vretCur);
        visro := -1; --grzbiet bez klucza
    when too_many_rows then -- zalogowany wiecej niz raz
        if visro = 0 then
          rollback;
        end if;
      select grzbiet.xx,loginname
        into vgrb_xx,vmsg
        from grzbiet, spacer_users
       where tytul=vtytul
         and mutgrb=vmutacja
         and kiedy=to_date(vkiedy,sr.vfShortDate)
         and holder=spacer_users.xx;

      if user=upper(vmsg) then
          vmsg := 'Uzytkownik '||user||' juz otworzyl ten grzbiet w innej sesji Manamu';
      else
          vmsg := 'Grzbiet '||vtytul||vmutacja||' na '||vkiedy||' jest blokowana przez uzytkownika: '||vmsg;
      end if;

      open vretCur for
        select g.xx,
               0,
               g.objetosc,
               vmsg,
               get_opisorder(g.xx)
          from grzbiet g
         where g.xx=vgrb_xx;
  end open_grzbiet;

/******************** GET_PLATECODE ***********************/
  procedure get_platecode (
    vgrb_xx in grzbiet.xx%type,
    vmutgrb in grzbiet.mutgrb%type,
    vgrb_nr_porz in spacer_strona.nr_porz%type,
    vpcode out plateCodeType,
    vfcode out plateCodeType,
    vpagina out plateCodeType,
    vile_kol out spacer_strona.ile_kol%type,
    vdiff_sep in out pls_integer -- 0:liczy dla obu wersji fixd|colo, zwraca 1, gdy rozne; 1:liczy tylko dla fixd
  ) as
    vmak_xx makieta.xx%type;
    vcolor_mak_xx makieta.xx%type;
    vmutstr drzewo.mutacja%type;
    vmutred spacer_strona.mutred%type; 
    vfitmutred drzewo.mutacja%type := null;
    vfitlevel pls_integer := -1;
    vbase_nr_porz spacer_strona.nr_porz%type;
  begin
    select g.mak_xx,g.base_nr_porz,to_char(g.base_nr_porz,'FM000')
      into vmak_xx,vbase_nr_porz,vfcode
      from table(select strony from grzbiet where xx=vgrb_xx) g
     where g.dst_nr_porz=vgrb_nr_porz;
    if vfcode = '000' then
        select to_char(objetosc,'FM000')
        into vfcode
        from makieta where xx=vmak_xx;
    end if;

    /*select mak_xx,nr_porz into vmak_xx,vbase_nr_porz
        from spacer_strona where nvl(dervlvl,derv.none)<>derv.fixd
       start with mak_xx=vmak_xx and nr_porz=vbase_nr_porz
     connect by mak_xx = prior derv_mak_xx and nr_porz = prior derv_nr_porz and prior dervlvl=derv.fixd;*/
    get_root_strona(vmak_xx,vbase_nr_porz);
    if vdiff_sep = 0 then
      vcolor_mak_xx := vmak_xx;
      get_root_strona_color(vcolor_mak_xx,vbase_nr_porz);
      if vmak_xx<>vcolor_mak_xx then
        vmak_xx := vcolor_mak_xx;
        vdiff_sep := 1;
      end if; 
    end if;
  
    begin  
      select to_char(decode(s.nr_porz,0,m.objetosc,s.nr_porz),'FM000')||d.tytul, substr(to_char(s.nr,'000'),2),
             d.mutacja, s.mutred, s.ile_kol
        into vpcode, vpagina, vmutstr, vmutred, vile_kol
        from spacer_strona s, makieta m, drzewo d
       where s.mak_xx=m.xx and m.drw_xx=d.xx
         and m.xx=vmak_xx and s.nr_porz=vbase_nr_porz;
    exception
      when no_data_found then
        raise_application_error(-20001,'Dziedziczenie strony '||vgrb_nr_porz||' odnosi sie do usunietej makiety bazowej: '||vmak_xx);
    end;

    if vmutgrb <> vmutstr and vmutred is not null then
       for vi in 0..length(vmutred)/2 loop
          vmak_xx := mutred_fitness(vmutstr,substr(vmutred,2*vi+1,2),vmutgrb);
          if vmak_xx>vfitlevel then
            vfitlevel := vmak_xx;
            vfitmutred := substr(vmutred,2*vi+1,2);
          end if;
          
       end loop;
      if vfitmutred is not null then
        vmutstr := vfitmutred;
      end if;
    end if;
    
    vpcode := vpcode||vmutstr;
exception
  when no_data_found then
     raise_application_error(-20002,'Nie odnaleziono blachy');
end get_platecode;

/******************** MERGE_PLATES ***********************/
  procedure merge_plates (
    vgrb_xx in grzbiet.xx%type,
    vlp1 in blacha.lp%type,
    vlp2 in blacha.lp%type,
    vtargetlp in blacha.lp%type,
    vkiedy in grzbiet.kiedy%type
  ) as
    vile_kol blacha.ile_kol%type;
  begin
    select max(ile_kol) into vile_kol
      from blacha
     where grb_xx=vgrb_xx and lp in (vlp1,vlp2);

    /*insert into blacha (grb_xx,lp,reference_id,code,ile_kol,sepcode,fizpages,pagina)
         select vgrb_xx,vtargetlp,min(b.reference_id),b1.code||b2.code,vile_kol,s.sepcode,b1.fizpages||b2.fizpages,b1.pagina||b2.pagina
              from blacha b,grzbiet g,blacha_sepcode s,pivot p,
                   (select code,fizpages,pagina from blacha where grb_xx=vgrb_xx and lp=vlp1 and rownum=1) b1,
                   (select code,fizpages,pagina from blacha where grb_xx=vgrb_xx and lp=vlp2 and rownum=1) b2
             where p.x<vile_kol and b.grb_xx=g.xx and g.kiedy=vkiedy
               and b.code=b1.code||b2.code and b.sepcode=s.sepcode
               and s.ile_kol=vile_kol and s.ref_id_offset=p.x
             group by b1.code||b2.code,s.sepcode,b1.fizpages||b2.fizpages,b1.pagina||b2.pagina;
    if sql%rowcount < vile_kol then
       insert into blacha (grb_xx,lp,reference_id,code,ile_kol,sepcode,fizpages,pagina)
       select vgrb_xx,vtargetlp,null,b1.code||b2.code,vile_kol,s.sepcode,b1.fizpages||b2.fizpages,b1.pagina||b2.pagina
         from blacha_sepcode s,pivot p,
              (select code,fizpages,pagina from blacha where grb_xx=vgrb_xx and lp=vlp1 and rownum=1) b1,
              (select code,fizpages,pagina from blacha where grb_xx=vgrb_xx and lp=vlp2 and rownum=1) b2
        where p.x<vile_kol and s.ile_kol=vile_kol and s.ref_id_offset=p.x
          and not exists (select 1 from blacha b3 where b3.grb_xx=vgrb_xx and b3.lp=vtargetlp and b3.code=b1.code||b2.code and b3.sepcode=s.sepcode);
    end if;*/
    insert into blacha (grb_xx,lp,reference_id,code,ile_kol,sepcode,fizpages,pagina)
         select vgrb_xx,vtargetlp,min(b.reference_id),b1.code||b2.code,vile_kol,s.sepcode,b1.fizpages||b2.fizpages,b1.pagina||b2.pagina
              from blacha b,grzbiet g,blacha_sepcode s,
                   (select code,sepcode,fizpages,pagina from blacha where grb_xx=vgrb_xx and lp=vlp1) b1,
                   (select code,sepcode,fizpages,pagina from blacha where grb_xx=vgrb_xx and lp=vlp2) b2
             where s.ile_kol=vile_kol and b.grb_xx=g.xx and g.kiedy=vkiedy
               and b.code=b1.code||b2.code and b.sepcode=s.sepcode
               and b.sepcode=b1.sepcode and b1.sepcode=b2.sepcode
             group by b1.code||b2.code,s.sepcode,b1.fizpages||b2.fizpages,b1.pagina||b2.pagina;
    if sql%rowcount < vile_kol then    
       insert into blacha (grb_xx,lp,reference_id,code,ile_kol,sepcode,fizpages,pagina)
       select vgrb_xx,vtargetlp,null,b1.code||b2.code,vile_kol,s.sepcode,b1.fizpages||b2.fizpages,b1.pagina||b2.pagina
         from blacha_sepcode s,
              (select code,sepcode,fizpages,pagina from blacha where grb_xx=vgrb_xx and lp=vlp1) b1,
              (select code,sepcode,fizpages,pagina from blacha where grb_xx=vgrb_xx and lp=vlp2) b2
        where s.ile_kol=vile_kol and b1.sepcode=b2.sepcode and s.sepcode=b1.sepcode
          and not exists (select 1 from blacha b3 where b3.grb_xx=vgrb_xx and b3.lp=vtargetlp and b3.code=b1.code||b2.code and b3.sepcode=s.sepcode);
    end if;
  end merge_plates;

/******************** ENUM_PLATES ***********************/
   procedure enum_plates (vgrb_xx in grzbiet.xx%type)
   as
    vlockCur sr.refCur;
    vis4 boolean := false;
    vcapacity pls_integer;
    vplate# pls_integer;
    vldiff_sep pls_integer;
    vrdiff_sep pls_integer;
    vppp drzewo.ppp%type;
    vkiedy grzbiet.kiedy%type;
    vmutgrb grzbiet.mutgrb%type;
    vref_id blacha.reference_id%type;
    vlile_kol spacer_strona.ile_kol%type;
    vrile_kol spacer_strona.ile_kol%type;
    vlnr_porz spacer_strona.nr_porz%type;
    vrnr_porz spacer_strona.nr_porz%type;
    vnr_plate spacer_strona.nr_porz%type;
    vpage1code plateCodeType;
    vpage2code plateCodeType;
    vpage3code plateCodeType;
    vpage4code plateCodeType;
    vfiz1code plateCodeType;
    vfiz2code plateCodeType;
    vpagina1code plateCodeType;
    vpagina2code plateCodeType;
  begin
    select min(ppp) into vppp from grzbiet g, drzewo d, makieta m, table(g.strony) gs
     where g.xx=vgrb_xx and g.kiedy=m.kiedy and gs.mak_xx=m.xx 
       and ((substr(g.tytul,1,1)<>'U' and m.drw_xx=d.xx)
        or (substr(g.tytul,1,1)='U' and g.tytul=d.tytul and d.mutacja=g.mutgrb));

    select objetosc, objetosc/vppp - 1, mutgrb, kiedy
      into vcapacity, vplate#, vmutgrb, vkiedy
      from grzbiet where xx=vgrb_xx;
    if mod(vcapacity,2*vppp)<>0 then
        raise_application_error(-20001,'Grzbietu o objetosci '||vcapacity||' nie da sie wydrukowac po '||vppp||' stron na blasze');
    end if;

    lock table blacha in exclusive mode; -- mutex_begin
    delete from blacha where grb_xx=vgrb_xx and (lp>vplate#+1 or length(code)<>8*vppp);

    if vppp = 1 then
      for vi in 0..vplate# loop
         select count(1) into vlnr_porz
           from blacha where grb_xx=vgrb_xx and lp=decode(vi,0,vplate#+1,vi);
         if vlnr_porz = 0 then
            vlnr_porz := vi;
            vldiff_sep := 0;
            get_platecode(vgrb_xx,vmutgrb,vlnr_porz,vpage1code,vfiz1code,vpagina1code,vlile_kol,vldiff_sep);
            for vj in 1..vlile_kol loop
                insert into blacha (grb_xx,lp,reference_id,code,ile_kol,sepcode,fizpages,pagina)
                select vgrb_xx,decode(vi,0,vplate#+1,vi),min(b.reference_id),vpage1code,vlile_kol,s.sepcode,vfiz1code,vpagina1code
                  from blacha b,grzbiet g,blacha_sepcode s
                 where b.grb_xx=g.xx and g.kiedy=vkiedy
                   and b.code=vpage1code and b.sepcode=s.sepcode
                   and s.ile_kol=vlile_kol and s.ref_id_offset=vj-1
                 group by s.sepcode;
                if sql%rowcount = 0 then
                    insert into blacha (grb_xx,lp,reference_id,code,ile_kol,sepcode,fizpages,pagina)
                    select vgrb_xx,decode(vi,0,vplate#+1,vi),null,vpage1code,vlile_kol,s.sepcode,vfiz1code,vpagina1code
                      from blacha_sepcode s
                     where s.ile_kol=vlile_kol and s.ref_id_offset=vj-1;
                end if;
            end loop;
            if vldiff_sep=1 then -- dziedziczenie koloru
              vlnr_porz := vi;
              get_platecode(vgrb_xx,vmutgrb,vlnr_porz,vpage1code,vfiz1code,vpagina1code,vlile_kol,vldiff_sep);
              update blacha set code=vpage1code,fizpages=vfiz1code,pagina=vpagina1code,
                                reference_id=(select min(b.reference_id) from blacha b,grzbiet g 
                                 where g.kiedy=vkiedy and g.xx=b.grb_xx and b.code=vpage1code and b.sepcode='K')
               where grb_xx=vgrb_xx and lp=decode(vi,0,vplate#+1,vi) and sepcode='K';
            end if;
         end if;
      end loop;

      if get_ppp(vgrb_xx)='1-2' then
            select /*+ cardinality (s 40) */ count(1) into vlnr_porz
              from makieta m,drzewo d,table(select strony from grzbiet where xx=vgrb_xx) s 
             where m.drw_xx=d.xx and m.xx=s.mak_xx and d.ppp=1;
            if vlnr_porz = 2 then -- produkt A3 z okladka A2
               select min(vsize(code)) into vlnr_porz from blacha where grb_xx=vgrb_xx and lp=vcapacity/2;
               if vlnr_porz=8 then
                  for vi in 2..vcapacity/2 loop
                     merge_plates(vgrb_xx,vi,vcapacity-vi+1,-vi,vkiedy);
                  end loop;
                  delete from blacha where grb_xx=vgrb_xx and lp between 2 and vcapacity-1;
                  update blacha set lp=-lp where grb_xx=vgrb_xx and lp<0;
               else 
                  delete from blacha where grb_xx=vgrb_xx and lp between vcapacity/2+1 and vcapacity-1;
               end if;
               update blacha set lp=vcapacity/2+1 where grb_xx=vgrb_xx and lp=vcapacity;
               update blacha set lp=lp+1 where grb_xx=vgrb_xx and lp>1;
               update blacha set lp=2 where grb_xx=vgrb_xx and lp=vcapacity/2+2;
            end if;
      end if;
    elsif vppp = 2 or vppp = 4 then
      if vppp = 4 then
        select max(lp) into vppp from blacha where grb_xx=vgrb_xx;
        if vppp = 1+vplate# then
          rollback;
          return;
        else
          vis4 := true;
          vppp := 2;
          vplate# := 2*(vplate#+1)-1;
          delete from blacha where grb_xx=vgrb_xx;
        end if;
      end if;

      for vi in 0..vplate# loop
         if vi = 0 then
            vlnr_porz := 0;
            vrnr_porz := 1;
            vnr_plate := 1;
         else
            vlnr_porz := vppp*vi;
            vrnr_porz := vcapacity - vppp*(vi - 1) - 1;
            if vlnr_porz < vrnr_porz then
              vnr_plate := vlnr_porz;
            else
              vnr_plate := vrnr_porz;
            end if;
         end if;
         select count(1) into vlile_kol
           from blacha where grb_xx=vgrb_xx and lp=vnr_plate;
         if vlile_kol = 0 then
            vldiff_sep := 0;
            vrdiff_sep := 0;
            get_platecode(vgrb_xx,vmutgrb,vlnr_porz,vpage1code,vfiz1code,vpagina1code,vlile_kol,vldiff_sep);
            get_platecode(vgrb_xx,vmutgrb,vrnr_porz,vpage2code,vfiz2code,vpagina2code,vrile_kol,vrdiff_sep);
            if vlile_kol < vrile_kol then
              vlile_kol := vrile_kol;
            end if;
            insert into blacha (grb_xx,lp,reference_id,code,ile_kol,sepcode,fizpages,pagina)
            select vgrb_xx,vnr_plate,min(b.reference_id),vpage1code||vpage2code,vlile_kol,s.sepcode,vfiz1code||vfiz2code,vpagina1code||vpagina2code
              from blacha b,grzbiet g,blacha_sepcode s,pivot p
             where p.x<vlile_kol and b.grb_xx=g.xx and g.kiedy=vkiedy
               and b.code=vpage1code||vpage2code and b.sepcode=s.sepcode
               and s.ile_kol=vlile_kol and s.ref_id_offset=p.x
             group by s.sepcode;
            if sql%rowcount < vlile_kol then
                insert into blacha (grb_xx,lp,reference_id,code,ile_kol,sepcode,fizpages,pagina)
                select vgrb_xx,vnr_plate,null,vpage1code||vpage2code,vlile_kol,s.sepcode,vfiz1code||vfiz2code,vpagina1code||vpagina2code
                  from blacha_sepcode s,pivot p
                 where p.x<vlile_kol and s.ile_kol=vlile_kol and s.ref_id_offset=p.x
                   and not exists (select 1 from blacha b2 where b2.grb_xx=vgrb_xx and lp=vnr_plate and b2.sepcode=s.sepcode);
            end if;
            if vldiff_sep>0 or vrdiff_sep>0 then -- dziedziczenie koloru
              if vldiff_sep>0 then
                get_platecode(vgrb_xx,vmutgrb,vlnr_porz,vpage1code,vfiz1code,vpagina1code,vlile_kol,vldiff_sep);
              end if;
              if vrdiff_sep>0 then
                get_platecode(vgrb_xx,vmutgrb,vrnr_porz,vpage2code,vfiz2code,vpagina2code,vrile_kol,vrdiff_sep);
              end if;
              update blacha set code=vpage1code||vpage2code,fizpages=vfiz1code||vfiz2code,pagina=vpagina1code||vpagina2code,
                                reference_id=(select min(b.reference_id) from blacha b,grzbiet g 
                                 where g.kiedy=vkiedy and g.xx=b.grb_xx and b.code=vpage1code||vpage2code and b.sepcode='K')
               where grb_xx=vgrb_xx and lp=vnr_plate and sepcode='K';
            end if;
         end if;
      end loop;

      if vis4 then
          vplate# := (vplate#+1)/2-1;
          for vi in 0..vplate# loop
            merge_plates(vgrb_xx,vi+1,2*(vplate#+1)-vi,-vi-1,vkiedy);
          end loop;
          delete from blacha where grb_xx=vgrb_xx and lp>0;
          update blacha set lp=-lp where grb_xx=vgrb_xx;
      elsif vppp=2 then 
          select count(distinct ppp) into vlnr_porz from blacha b,drzewo d 
           where grb_xx=vgrb_xx and tytul=substr(code,4,3) and mutacja=substr(code,7,2);
          if vlnr_porz > 1 then
            for c in (select /*+ cardinality(g 44) */ distinct tytul||mutacja tytul,m.xx mak_xx
                        from drzewo d, makieta m, blacha b,
                      table(select strony from grzbiet where xx=vgrb_xx) g
                      where d.xx=m.drw_xx and m.xx=g.mak_xx and b.grb_xx=vgrb_xx and d.ppp=4 
                        and length(b.code)=16 and platecode_in_mak(b.code,m.xx)>0) loop
              select count(1) into vlnr_porz from blacha where grb_xx=vgrb_xx and length(code)=32 and platecode_in_mak(code,c.mak_xx)>0;
              if vlnr_porz>0 then
                  delete from blacha where grb_xx=vgrb_xx and length(code)=16 and platecode_in_mak(code,c.mak_xx)>0;
              else
                select min(lp),max(lp) into vlnr_porz,vrnr_porz
                  from blacha where grb_xx=vgrb_xx and platecode_in_mak(code,c.mak_xx)>0;
                while vlnr_porz<vrnr_porz loop
                  merge_plates(vgrb_xx,vlnr_porz,vrnr_porz,-vlnr_porz,vkiedy);
                  vlnr_porz := vlnr_porz + 1;
                  vrnr_porz := vrnr_porz - 1;
                end loop;
                delete from blacha where grb_xx=vgrb_xx and lp>0 and platecode_in_mak(code,c.mak_xx)>0;
                update blacha set lp=-lp where grb_xx=vgrb_xx and lp<0;
                update blacha b 
                 set lp=(select count(distinct b2.lp) from blacha b2 where b2.grb_xx=vgrb_xx and b2.lp<=b.lp)
                 where b.grb_xx=vgrb_xx;
              end if;
            end loop;
          end if;
      end if;

      delete from blacha where grb_xx=vgrb_xx and lp>(select /*+ cardinality(g 3) */ sum(g.objetosc/d.ppp) 
             from makieta m, drzewo d, (select count(1) objetosc,mak_xx from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx) g 
            where m.drw_xx=d.xx and m.xx=g.mak_xx);
            
    end if;
    
    commit; -- mutex_end
  end enum_plates;

/******************** DESC_GRZBIET ***********************/
  procedure desc_grzbiet (
      vtytul in drzewo.tytul%type,
      vmutacja in drzewo.mutacja%type,
      vkiedy in char,
      vretCur out sr.refCur
  ) as
      vgrb_xx grzbiet.xx%type;
  begin
    select grzbiet.xx into vgrb_xx
      from grzbiet
     where grzbiet.tytul=vtytul
       and grzbiet.mutgrb=vmutacja
       and kiedy=to_date(vkiedy,sr.vfShortDate);

    enum_plates(vgrb_xx);

    open vretCur for
        select b.lp,
               to_char(g.kiedy,'DDMM')||'_'||to_char(b.lp,'FM00')||g.tytul||'_'||g.mutgrb||'_'||b.sepcode||b.ile_kol||'v01' fname,
               --to_char(g.kiedy,'DDMM')||'_'||to_char(b.lp,'FM00')||g.tytul||'_'||decode(k.mi,k.ma,k.mi,g.mutgrb)||'_'||b.sepcode||b.ile_kol||'v01' fname,
               to_char(b.reference_id,'FM000000000000') refid,
               code,
               fizpages,
               pagina,
               nvl((select decode(max(s.rozkladowka),null,'NIE','TAK') from spacer_strona s 
                 where s.nr_porz in (to_number(substr(b.code,1,3)),to_number(substr(b.code,9,3))) 
                   and s.mak_xx in (select mak_xx from (select distinct mak_xx from table(select strony from grzbiet where xx=vgrb_xx)) where platecode_in_mak(substr(b.code,1,8),mak_xx)>0)),'N/A') rozkladowka
          from blacha b, grzbiet g,
               (select min(m) mi,max(m) ma,reference_id,sepcode from (select distinct substr(code,8*x+7,2) m, reference_id,sepcode from blacha,pivot where grb_xx=vgrb_xx and 8*x+7 < length(code)) group by reference_id,sepcode) k
         where b.grb_xx=g.xx and g.xx=vgrb_xx
           and k.reference_id=b.reference_id and k.sepcode=b.sepcode
         order by b.lp;
  exception
    when no_data_found then
       raise_application_error(-20001,'Nie odnaleziono grzbietu '||vtytul||' '||vmutacja||' '||vkiedy);
  end desc_grzbiet;

/******************** PRINT_EXPORT ***********************/
  procedure print_export (
    vgrb_xx in grzbiet.xx%type,
    vretCur out sr.refCur
  ) as
    vopis drzewo.opis%type;
    vfname varchar2(8);
    vkiedy number;
    vppp number;
  begin
    begin
      select rejkod.get_opis_drzewa(d.xx,g.kiedy)||' ('||g.mutgrb||')', to_number(to_char(g.kiedy,'RRRRMMDD')), to_char(g.kiedy,'DD')||d.tytul||'_'||g.mutgrb, d.ppp
        into vopis, vkiedy, vfname, vppp
        from drzewo d, grzbiet g
       where d.tytul=g.tytul and d.mutacja=g.mutgrb and g.xx=vgrb_xx;
    exception
      when no_data_found then
      select rejkod.get_opis_drzewa(d.xx,g.kiedy)||' ('||g.mutgrb||')', to_number(to_char(g.kiedy,'RRRRMMDD')), to_char(g.kiedy,'DD')||d.tytul||'_'||g.mutgrb, d.ppp
        into vopis, vkiedy, vfname, vppp
        from drzewo d, makieta m, grzbiet g, table(g.strony) s
       where d.xx=m.drw_xx and m.xx=s.mak_xx
         and s.dst_nr_porz=0 and g.xx=vgrb_xx;
    end;

    enum_plates(vgrb_xx);

    open vretCur for
       select * from (
          select -vppp lp, count(distinct(lp)) ile_kol, vkiedy reference_id, vopis sepcode, vfname remark
            from blacha where grb_xx=vgrb_xx
       union all select lp, ile_kol, b.reference_id, b.sepcode, to_char(g.kiedy,'DDMM')||'_'||to_char(lp,'FM00')||g.tytul||'_'||decode(k.mi,k.ma,k.mi,g.mutgrb)||'_'
            from blacha b, grzbiet g,
                (select min(m) mi,max(m) ma,reference_id,sepcode from (select distinct substr(code,8*x+7,2) m, reference_id,sepcode from blacha,pivot where grb_xx=vgrb_xx and 8*x+7 < length(code)) group by reference_id,sepcode) k
           where b.grb_xx=g.xx and g.xx=vgrb_xx
             and k.reference_id=b.reference_id and k.sepcode=b.sepcode
       ) order by lp,reference_id;

  exception
    when no_data_found then
       if vgrb_xx < 0 then
          vkiedy := init_grzbiet(-vgrb_xx);
          commit;
          raise_application_error(-20001,'Nie zainicjowano grzbietu. Sprobuj wyeksportowac dane ponownie.');
       else
          raise_application_error(-20002,'Nie odnaleziono grzbietu grb_xx='||vgrb_xx);
       end if;
  end print_export;

/******************** PRIME_PLIST***********************/
  procedure prime_plist (
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type,
    vkiedy in varchar2,
    vretCur out sr.refCur
  ) as
      vgrb_xx grzbiet.xx%type;
      vkiedy_tmp number;
  begin
    select min(grzbiet.xx) into vgrb_xx
      from grzbiet
     where grzbiet.tytul=vtytul
       and grzbiet.mutgrb=vmutacja
       and kiedy=to_date(vkiedy,sr.vfshortdate);
    if vgrb_xx is null then
       raise_application_error(-20002,'Nie odnaleziono grzbietu tytul='||vtytul||' '||vmutacja||' na '||vkiedy);
    end if;
    
    enum_plates(vgrb_xx);

    open vretCur for                           
          select lp, ile_kol, b.reference_id, b.sepcode, b.pagina, to_char(g.kiedy,'DDMM')||'_'||to_char(lp,'FM00')||g.tytul||'_'||g.mutgrb||'_' remark
          --select lp, ile_kol, b.reference_id, b.sepcode, b.pagina, to_char(g.kiedy,'DDMM')||'_'||to_char(lp,'FM00')||g.tytul||'_'||decode(k.mi,k.ma,k.mi,g.mutgrb)||'_' remark
            from blacha b, grzbiet g,
                (select min(m) mi,max(m) ma,reference_id,sepcode from (select distinct substr(code,8*x+7,2) m, reference_id,sepcode from blacha,pivot where grb_xx=vgrb_xx and 8*x+7 < length(code)) group by reference_id,sepcode) k
           where b.grb_xx=g.xx and g.xx=vgrb_xx
             and k.reference_id=b.reference_id and k.sepcode=b.sepcode          
       order by lp, reference_id;
  end prime_plist;   

/********************* FIND_MATCH ************************/
  procedure find_match (
    vfname in varchar2, --TTTMMDDMM_NNN
    vyear in varchar2, --YY
    vxmltext out varchar2
  ) as
    vlp blacha.lp%type;
    vmak_xx makieta.xx%type;
    vgrb_xx grzbiet.xx%type;
    vgrb_okl_xx grzbiet.xx%type;
    vcapacity grzbiet.objetosc%type;
    vmakcap makieta.objetosc%type;
    vnr_porz spacer_strona.nr_porz%type;
    vmatch_nr_porz spacer_strona.nr_porz%type;
    vmatchdervfilename varchar2(64);
  begin --nie uwzglednia ppp<>2
      select m.xx, max(mg.grb_xx), max(mg.grb_okl_xx), max(g.objetosc), m.objetosc
        into vmak_xx, vgrb_xx, vgrb_okl_xx, vcapacity, vmakcap
        from drzewo d, makieta m, makingrb mg, grzbiet g
       where d.xx=m.drw_xx and m.xx=mg.mak_xx and mg.grb_xx=g.xx
         and d.tytul=substr(vfname,1,3) and d.mutacja=substr(vfname,4,2)
         and m.kiedy=to_date(substr(vfname,6,2)||'/'||substr(vfname,8,2)||'/'||vyear,'DD/MM/RRRR')
       group by m.xx,m.objetosc;
  begin
      select decode(g.base_nr_porz,0,1,1,vmakcap,vmakcap - g.base_nr_porz + 1),
             decode(g.dst_nr_porz,0,1,least(g.dst_nr_porz,decode(g.dst_nr_porz,0,1,1,vcapacity,vcapacity - g.dst_nr_porz + 1)))
        into vmatch_nr_porz, vlp
        from table(select strony from grzbiet where xx=vgrb_xx) g
       where g.mak_xx=vmak_xx and g.base_nr_porz=decode(to_number(substr(vfname,11,3)),vmakcap,0,to_number(substr(vfname,11,3)));
  exception
    when no_data_found then -- fragmentacja
       if vgrb_okl_xx is not null then
          select objetosc into vcapacity from grzbiet where xx=vgrb_okl_xx;
          select decode(g.dst_nr_porz,0,1,1,0,vcapacity - g.dst_nr_porz + 1),
                 decode(g.dst_nr_porz,0,1,least(g.dst_nr_porz,decode(g.dst_nr_porz,0,1,1,vcapacity,vcapacity - g.dst_nr_porz + 1)))
            into vmatch_nr_porz, vlp
            from table(select strony from grzbiet where xx=vgrb_okl_xx) g
           where g.mak_xx=vmak_xx and g.base_nr_porz=decode(to_number(substr(vfname,11,3)),vmakcap,0,to_number(substr(vfname,11,3)));
          select base_nr_porz into vmatch_nr_porz from table(select strony from grzbiet where xx=vgrb_okl_xx) g where dst_nr_porz=vmatch_nr_porz;
          vgrb_xx := vgrb_okl_xx;
       end if;
  end;

   select tytul||mutacja||to_char(kiedy,'DDMM')||'_'||to_char(decode(nr_porz,0,vmakcap,nr_porz),'FM000')
     into vmatchdervfilename from drzewo d, makieta m, spacer_strona s
    where d.xx=m.drw_xx and m.xx=s.mak_xx 
      and m.xx=get_root_makieta(vmak_xx,decode(vmatch_nr_porz,vmakcap,0,vmatch_nr_porz))
      and s.nr_porz=get_root_nr_porz(vmak_xx,decode(vmatch_nr_porz,vmakcap,0,vmatch_nr_porz));
/*   select tytul||mutacja||to_char(kiedy,'DDMM')||'_'||to_char(decode(nr_porz,0,vmakcap,nr_porz),'FM000')
     into vmatchdervfilename from drzewo d, makieta m,
  (select mak_xx,nr_porz from spacer_strona
    where nvl(dervlvl,sr.derv_none)<sr.derv_fixd
    start with mak_xx=vmak_xx and nr_porz=decode(vmatch_nr_porz,vmakcap,0,vmatch_nr_porz)
  connect by mak_xx = prior derv_mak_xx and nr_porz = prior derv_nr_porz and prior dervlvl=sr.derv_fixd) s
    where d.xx=m.drw_xx and m.xx=s.mak_xx;*/

      select '<?xml version="1.0"?>
<match arg="'||vfname||'" y="'||vyear||'">
<tiff>'||to_char(g.kiedy,'DDMM')||'_'||to_char(b.lp,'FM00')||g.tytul||'_'||decode(k.mi,k.ma,k.mi,g.mutgrb)||'</tiff>
<platecode>'||b.code||'</platecode>
<matchfilename>'||substr(vfname,1,10)||to_char(vmatch_nr_porz,'FM000')||'</matchfilename>
<matchdervname>'||vmatchdervfilename||'</matchdervname>
</match>' into vxmltext from blacha b, grzbiet g,
               (select min(m) mi,max(m) ma,reference_id from (select distinct substr(code,8*x+7,2) m, reference_id from blacha,pivot where grb_xx=vgrb_xx and 8*x+7 < length(code)) group by reference_id) k
         where rownum=1 and grb_xx=vgrb_xx and g.xx=vgrb_xx and lp=vlp and k.reference_id=b.reference_id;
  exception
    when no_data_found then
        raise_application_error(-20001,'Nie odnaleziono makiety lub makieta nie zostala umieszczona w zadnym grzbiecie');
  end find_match;

/********************* ZAMOWIENIE_NA_DRUK ************************/
  procedure zamowienie_na_druk (
    vkiedy in varchar2,
    vmut_xx in drzewo_mutacji.xx%type,
    vtyp in number,
    vtytul in drzewo.tytul%type,
    vdrukarnie in pls_integer,
    vdepesz in pls_integer,
    vretCur out sr.refCur,
    vzasCur out sr.refCur
  ) as begin
    open vretCur for
      select g.xx,decode(de.xx,null,'Niedziela','(Ty. '||to_char(g.kiedy+7-to_number(to_char(g.kiedy,'d')),'ww')||decode(de.nr_wyd,null,null,' Nr ed. '||de.nr_wyd)||') '||to_char(g.kiedy,sr.vfShortDate)) kiedy,
             g.data_druku,g.tytul||' '||g.mutgrb||' ('||get_makorder(g.xx)||') '||rejkod.get_opis_drzewa(d.xx,g.kiedy) tytul,g.mutgrb||decode(g.obszar,null,null,' ('||g.obszar||')')||decode(g.pre_xx,null,null,' - '||(select opis from forma_sprzedazy where xx=g.pre_xx)) zasieg,
             g.objetosc||' ('||get_objetosci(g.xx)||')' objetosc,get_szycie(g.xx) szycie,decode(g.przefalc,null,'NIE','TAK') przefalc,g.naklad,g.gramatura,get_color(g.xx) color,decode(g.depeszowy,null,'NIE','TAK') depeszowy,get_paper(g.xx) papier,get_uwagiorder(g.xx)||' '||g.uwagi uwagi,nvl(g.acc_print_cdrz,0) is_accepted,
             decode(g.acc_print_ldrz,null,'','LDRZ: '||(select nazwisko||' ' from spacer_users where xx=g.acc_print_ldrz))||
             decode(g.acc_print_cdrz,null,'','DOD: '||(select nazwisko||' ' from spacer_users where xx=g.acc_print_cdrz))||
             decode(g.acc_print_org,null,'','ORG: '||(select nazwisko||' ' from spacer_users where xx=g.acc_print_org)) kto_acc
        from grzbiet g, makieta m, drzewo d, dane_edycji de
       where g.kiedy>=to_date(vkiedy,sr.vfShortDate)
         and decode(vtyp,1,data_zamowienia(g.xx),3,trunc(g.deadline),g.kiedy)=to_date(vkiedy,sr.vfShortDate)
         and m.xx=oklgrb(g.xx) and m.drw_xx=d.xx
         and bitand(nvl(g.drukarnie,vdrukarnie),vdrukarnie)>0
         and m.kiedy=de.kiedy(+) and de.tpr_xx(+)=1
         and (vtytul is null or vtytul=g.tytul or instr(grb.get_tytmakorder(g.xx),vtytul)>0 or (instr(vtytul,'%')>0 and (decode(substr(g.tytul,1,1),'Y','Z','O','S','C','S',substr(g.tytul,1,1))=substr(vtytul,1,1) or (vtytul='S%' and (grb.get_tytmakorder(g.xx) like '%-S%' or grb.get_tytmakorder(g.xx) like '%-O%' or grb.get_tytmakorder(g.xx) like '%-C%')))))
         and (vmut_xx=0 or 
             (g.obszar is null and exists (select 1 from drzewo_mutacji dm where dm.mutacja=g.mutgrb and dm.xx in (select mut_xx from space_reservation.drzewo_mutacji_wariant start with xx=vmut_xx connect by prior root_xx=xx union all select mut_xx from space_reservation.drzewo_mutacji_wariant start with xx=vmut_xx connect by root_xx=prior xx))) or 
             (g.obszar='RPP ' or exists (select 1 from grzbiet_tpr gt, drzewo_mutacji_wariant dmw where dmw.xx=vmut_xx and gt.grb_xx=g.xx and mod(nvl(instr(gt.obszar,dmw.sym),0),4)=1)))
         and nvl(g.depeszowy,0)=decode(vdepesz,-1,nvl(g.depeszowy,0),vdepesz)
       order by tytul;

     zasiegi_dnia(vkiedy,vzasCur);
  end zamowienie_na_druk;

/********************* OPEN_DRW ************************/
  procedure open_drw (
    vkiedy in char,
    vgrb_xx in grzbiet.xx%type,
    vdrwCur out sr.refCur
  ) as
    /* Otwiera liste tytulow ze zgodna data, ktore  *
     * moga byc zainsertowane do biezacego grzbietu */
    vzuz_xx drzewo.zuz_xx%type;
  begin
      select decode(zuz_xx,5,1,zuz_xx) into vzuz_xx from 
          (select d.zuz_xx from drzewo d, makieta m, table(select strony from grzbiet where xx=vgrb_xx) gs where vgrb_xx>0 and d.xx=m.drw_xx and m.xx=gs.mak_xx and gs.dst_nr_porz=0 
     union select d.zuz_xx from drzewo d, makieta m where vgrb_xx<0 and m.drw_xx=d.xx and m.xx=-vgrb_xx);

      open vdrwCur for
          select d.xx,decode(d.mutacja,' ',d.tytul,d.tytul||' '||d.mutacja)||' - '||rejkod.get_opis_drzewa(d.xx,m.kiedy)
            from drzewo d, makieta m
           where m.kiedy=to_date(vkiedy,sr.vfShortDate)
             and m.drw_xx=d.xx
             and decode(zuz_xx,5,1,zuz_xx)=vzuz_xx
             and sql_check_access(d.xx,'R')>0

      /* elasob 25/05/2006
        open vdrwCur for
          select d.xx,decode(d.mutacja,' ',d.tytul,d.tytul||' '||d.mutacja)||' - '||rejkod.get_opis_drzewa(d.xx,m.kiedy)
            from drzewo d, makieta m,
                 (select mutgrb from grzbiet
                   where vgrb_xx>0 and xx=vgrb_xx
            union select d.mutacja mutgrb from drzewo d, makieta m
                   where vgrb_xx<0 and m.drw_xx=d.xx and m.xx=-vgrb_xx) g
           where m.kiedy=to_date(vkiedy,sr.vfShortDate)
             and m.drw_xx=d.xx
             and decode(zuz_xx,5,1,zuz_xx)=vzuz_xx
             and sql_check_access(d.xx,'R')>0
             and d.mutacja in (
                select mutgrb from grzbiet where xx=vgrb_xx
          union all
                select mutacja
                  from drzewo_mutacji
                start with mutacja=g.mutgrb
                connect by prior xx = root_xx
          union select mutacja
                  from drzewo_mutacji
                start with mutacja=g.mutgrb
                connect by xx = prior root_xx
             ) 
       union select d.xx,decode(d.mutacja,' ',d.tytul,d.tytul||' '||d.mutacja)||' - '||d.opis
            from drzewo d, makieta m,
                 (select mutgrb from grzbiet
                   where vgrb_xx>0 and xx=vgrb_xx
            union select d.mutacja mutgrb from drzewo d, makieta m
                   where vgrb_xx<0 and m.drw_xx=d.xx and m.xx=-vgrb_xx) g
           where m.kiedy=to_date(vkiedy,sr.vfShortDate)
             and m.drw_xx=d.xx
             and decode(zuz_xx,5,1,zuz_xx)=vzuz_xx
             and sql_check_access(d.xx,'R')>0
             and g.mutgrb in (
                select substr(s.mutred,2*p.x+1,2) from spacer_strona s, pivot p
                 where s.mutred is not null and x<length(s.mutred)/2 and s.mak_xx=m.xx
             )*/
       union select d.xx,decode(d.mutacja,' ',d.tytul,d.tytul||' '||d.mutacja)||' - '||d.opis
               from drzewo d, grzbiet g, split_grzbietu s
              where g.xx=vgrb_xx and g.tytul=s.okladka and d.tytul=s.skladka1 
                and d.mutacja=g.mutgrb and nvl(s.do_kiedy,sysdate)>=sysdate
           order by 2;
  end open_drw;

/******************** OPEN_KRA_STR ***********************/
  procedure open_kra_str(
    vgrb_xx in grzbiet.xx%type,
    vstr_xx in number,
    vretCur in out sr.refCur
  ) as
    vmodulyTab spacer_mod_mak := spacer_mod_mak();
    vmoduly spacer_mod_str := spacer_mod_str(0,'','','');
    vi sr.srint;
    vs sr.srint;
    voff sr.srint;
    vmakdate sr.srint;
    vunit number;
    vred_unit number;
    vlocked_unit number;
  begin
    /*
    ** (c) Marcin Buchwald              13/04/2000
    **              Oracle              05/10/2000
    **              Grzbiet             09/05/2002
    **
    **  W wersji dla grzbietu na vstr_xx przekazywany
    **  jest dst_nr_porz. Opis w procedurze glownej
    */

  for kratka in (
    select k.xx, k.ile_mod, g.mak_xx, s.str_xx
      from spacer_str_krat sk, spacer_kratka k,
           spacer_strona s, table(select strony from grzbiet where xx=vgrb_xx) g
     where vgrb_xx>0
       and g.mak_xx=s.mak_xx
       and g.base_nr_porz=s.nr_porz
       and g.dst_nr_porz=vstr_xx
       and sk.mak_xx=g.mak_xx
       and sk.str_xx=s.str_xx
       and sk.kra_xx=k.xx
 union
    select k.xx, k.ile_mod, s.mak_xx, s.str_xx
      from spacer_str_krat sk, spacer_kratka k,
           spacer_strona s
     where vgrb_xx<0
       and s.mak_xx=-vgrb_xx
       and sk.mak_xx=s.mak_xx
       and sk.str_xx=s.str_xx
       and sk.kra_xx=k.xx
  ) loop
    vmodulyTab.extend;
    vmoduly.str_xx := kratka.xx;
    vmoduly.space := '';
    vmoduly.space_red := '';
    vmoduly.space_locked := '';
    vmakdate := makdate(kratka.mak_xx);

    voff := mod(kratka.ile_mod, sr.modUnit);
    if voff > 0 then
      vunit := 0.0;
      vred_unit := 0.0;
      vlocked_unit := 0.0;
    else
      vunit := null;
      vred_unit := null;
      vlocked_unit := null;
    end if;

    vi := 0;
    while vi < kratka.ile_mod loop
      if mod(vi,sr.modUnit) = voff then
        vmoduly.space := vmoduly.space||to_char(vunit,'FM0XXXXXXX');
        vmoduly.space_red := vmoduly.space_red||to_char(vred_unit,'FM0XXXXXXX');
        vmoduly.space_locked := vmoduly.space_locked||to_char(vlocked_unit,'FM0XXXXXXX');
        vunit := 0.0;
        vred_unit := 0.0;
        vlocked_unit := 0.0;
      end if;

      vs := select_modul_status_krat(kratka.mak_xx, kratka.str_xx, kratka.xx, vi, vmakdate);

      vunit := vunit * 2.0;
      vred_unit := vred_unit * 2.0;
      vlocked_unit := vlocked_unit * 2.0;

      if vs = sr.ogloszenie then
        vunit := vunit + 1.0;
      elsif vs = sr.redakcyjny then
        vred_unit := vred_unit + 1.0;
      elsif vs = sr.zablokowany then
        vlocked_unit := vlocked_unit + 1.0;
      end if;

      vi := vi + 1;
    end loop;

    vmoduly.space := vmoduly.space||to_char(vunit,'FM0XXXXXXX');
    vmoduly.space_red := vmoduly.space_red||to_char(vred_unit,'FM0XXXXXXX');
    vmoduly.space_locked := vmoduly.space_locked||to_char(vlocked_unit,'FM0XXXXXXX');

    vmodulyTab(vmodulyTab.last) := vmoduly;
  end loop;

  open vretCur for
    select k.szpalt_x,
           k.szpalt_y,
           m.space,
           m.space_red,
           m.space_locked
      from spacer_kratka k, table(cast(vmodulyTab as spacer_mod_mak)) m
     where k.xx=m.str_xx;

  end open_kra_str;

/******************** OPEN_SPOQ ***************************/
  procedure open_spoq (
    	vgrb_xx in grzbiet.xx%type,
    	vstrCur out sr.refCur,
    	vpubCur out sr.refCur,
    	vopiCur out sr.refCur,
    	vqueCur out sr.refCur
  ) is
    vmodulyTab spacer_mod_mak := spacer_mod_mak();
    vmoduly spacer_mod_str := spacer_mod_str(0,'','','');
    vmsg varchar2(255);
    vi sr.srint;
    vs sr.srint;
    voff sr.srint;
    vmakdate sr.srint;
    vunit number;
    vred_unit number;
    vlocked_unit number;
  begin
    --nie ma grzbietu - otworz makiete
    if vgrb_xx < 0 then
        space_reservation.open_spoq(-vgrb_xx,vstrCur,vpubCur,vopiCur,vqueCur);
        return;
    end if;
    --zmienila sie objetosc grzbietu
    select dirty into vi
      from grzbiet where xx=vgrb_xx;
    if vi is not null then
        begin
          select tytul into vmsg from mak where mak_xx=vi;
        exception
          when no_data_found then
              vmsg := 'zostala usunieta lub';
        end;
        refresh_grzbiet(vgrb_xx,vi);
        raise_application_error(-20001,'Grzbiet jest uszkodzony. Makieta '||vmsg||' zmienila objetosc. Przy ponownym otwarciu grzbiet zostanie naprawiony automatycznie');
    end if;
    --w grzbiecie zamiast str_xx bedzie podawany dst_nr_porz
    for strona in (
	   select /*+ cardinality(gs 60) */ s.str_xx, k.ile_mod, gs.mak_xx, gs.dst_nr_porz
	     from grzbiet g, table(g.strony) gs, spacer_strona s, spacer_kratka k
	    where g.xx=vgrb_xx and s.mak_xx=gs.mak_xx and s.nr_porz=gs.base_nr_porz and s.kra_xx=k.xx
    ) loop
      vmodulyTab.extend;
      vmoduly.str_xx := strona.dst_nr_porz;
      vmoduly.space := '';
      vmoduly.space_red := '';
      vmoduly.space_locked := '';
      vmakdate := makdate(strona.mak_xx);

      voff := mod(strona.ile_mod, sr.modUnit);
      if voff > 0 then
        vunit := 0.0;
        vred_unit := 0.0;
        vlocked_unit := 0.0;
      else
        vunit := null;
        vred_unit := null;
        vlocked_unit := null;
      end if;

      vi := 0;
      while vi < strona.ile_mod loop
        if mod(vi,sr.modUnit) = voff then
          vmoduly.space := vmoduly.space||to_char(vunit,'FM0XXXXXXX');
          vmoduly.space_red := vmoduly.space_red||to_char(vred_unit,'FM0XXXXXXX');
          vmoduly.space_locked := vmoduly.space_locked||to_char(vlocked_unit,'FM0XXXXXXX');
          vunit := 0.0;
          vred_unit := 0.0;
          vlocked_unit := 0.0;
        end if;

        vs := select_modul_status(strona.mak_xx, strona.str_xx, vi, vmakdate);

        vunit := vunit * 2.0;
        vred_unit := vred_unit * 2.0;
        vlocked_unit := vlocked_unit * 2.0;

        if vs = sr.ogloszenie then
          vunit := vunit + 1.0;
        elsif vs = sr.redakcyjny then
          vred_unit := vred_unit + 1.0;
        elsif vs = sr.zablokowany then
          vlocked_unit := vlocked_unit + 1.0;
        end if;

        vi := vi + 1;
      end loop;

      vmoduly.space := vmoduly.space||to_char(vunit,'FM0XXXXXXX');
      vmoduly.space_red := vmoduly.space_red||to_char(vred_unit,'FM0XXXXXXX');
      vmoduly.space_locked := vmoduly.space_locked||to_char(vlocked_unit,'FM0XXXXXXX');

      vmodulyTab(vmodulyTab.last) := vmoduly;
    end loop;

  open vstrCur for
    select /*+ cardinality(g 60) */ t.str_xx, --0
           g.dst_nr_porz,
           s.nr,
           s.num_xx*nvl(b.base,1),
           s.ile_kol,
           nvl(s.spot,0), --5
           s.sciezka,
           s.str_log,
           s.naglowek,
           k.szpalt_x,
           k.szpalt_y, --10
           t.space,
           t.space_red,
           t.space_locked,
           s.prn_mak_xx,
           s.drukarnie, --15
           to_char(nvl(s.deadline,sysdate),sr.vfLongDate),
           nvl(s.dervlvl,sr.derv_none),
           decode(dd.mutacja,' ',dd.tytul,dd.tytul||' '||dd.mutacja)||' '||g.base_nr_porz dervinfo,
           s.mutred,
           nvl(s.pap_xx,0) pap_xx,--20
           nvl(s.rozkladowka,0) is_rozkl,
           nvl(s.wyd_xx,dd.wyd_xx) wyd_xx,
           g.mutczas --23
      from spacer_strona s,
           table(select strony from grzbiet where xx=vgrb_xx) g,
           table(cast(vmodulyTab as spacer_mod_mak)) t,
           spacer_kratka k,
           (select mak_xx, str_xx, max(-1) base
              from spacer_str_krat group by mak_xx, str_xx) b,
           makieta dm,
           drzewo dd -- po przejsciu na oracle 9i trzeba to zmienic
     where s.mak_xx=g.mak_xx
       and s.nr_porz=g.base_nr_porz
       and g.dst_nr_porz=t.str_xx
       and s.kra_xx=k.xx
       and s.mak_xx=b.mak_xx(+)
       and s.str_xx=b.str_xx(+)
       and s.mak_xx=dm.xx
       and dm.drw_xx=dd.xx
     order by t.str_xx;

  -- ogloszenie nie stoi w grzbiecie na str_xx ale na dst_nr_porz
  open vpubCur for
    select /*+ cardinality(gs 60) */ pub.xx, --0
           nvl(pub.add_xx,-1),
           makieta.xx,
           g.dst_nr_porz,
           szpalt_x,
           szpalt_y, --5
           x,
           y,
           blokada,
           flaga_rezerw,
           pub.sizex, --10
           pub.sizey,
           pub.nazwa,
           nvl(adno,-1),
           wersja,
           uwagi, --15
           pub.ile_kol,
           nvl(pub.spo_xx,0),
           op_zew,
           sekcja,
           op_sekcji, --20
           nvl(nr_w_sekcji,-1),
           p_l,
           op_pl,
           nvl(nr_pl,-1),
           poz_na_str, --25
           txtposx,
           txtposy,
           decode(kratowe,0,typ_xx,0),
           decode(czas_obow,null,
              '#z',
              decode(sign(vmakdate-czas_obow),-1,'czas minal',
                  0,'zaraz spadnie!',
                  to_char(makieta.kiedy-czas_obow/sr.timeUnit,sr.vfLongDate)))||
           ' ['||spacer_users.imie||' '||spacer_users.nazwisko||' '||spacer_users.tel||']',
           decode(sign(vmakdate-nvl(czas_obow,spad_offset*sr.timeUnit)+spad_offset*sr.timeUnit), --30
              -1,decode(adno,null,-1,decode(bit_zapory,1,-3,-2)),
                 decode(adno,null,1,decode(bit_zapory,1,3,2))),
           nvl(eps_present,1), --31
           nvl(powtorka,0),
           nvl(old_adno,-1),
           nvl(studio,0),
           derived, --35
           uwagi_atex,
           nvl(spad,0),
           rawtohex(precel_flag),
           to_char(adno), --39
           nvl(nag_xx,1), --40
           nvl(podpis_reklama,0),
           nvl(is_digital,0) --42
   from makieta,drzewo,
        table(select strony from grzbiet where xx=vgrb_xx) g,
        (select /*+ cardinality(g2 5) */ xx,add_xx,p.mak_xx,str_xx,typ_xx,adno,x,y,sizex,sizey,txtposx,txtposy,blokada,bit_zapory,flaga_rezerw,nazwa,uwagi,ile_kol,spo_xx,op_zew,sekcja,op_sekcji,nr_w_sekcji,p_l,op_pl,nr_pl,poz_na_str,x_na_str,y_na_str,wersja,czas_obow,eps_present,powtorka,old_adno,studio,eps_date,0 derived,uwagi_atex,spad,nag_xx,is_digital
           from spacer_pub p,(select distinct mak_xx from table(select strony from grzbiet where xx=vgrb_xx)) g2
          where p.mak_xx=g2.mak_xx and p.x>0
      union all select /*+ CLUSTER (s), cardinality(g2 5) */ p.xx,p.add_xx,s.mak_xx,s.str_xx,p.typ_xx,p.adno,s.x,s.y,p.sizex,p.sizey,p.txtposx,p.txtposy,s.blokada,p.bit_zapory,p.flaga_rezerw,p.nazwa,p.uwagi,p.ile_kol,p.spo_xx,p.op_zew,p.sekcja,p.op_sekcji,p.nr_w_sekcji,p.p_l,p.op_pl,p.nr_pl,p.poz_na_str,p.x_na_str,p.y_na_str,p.wersja,p.czas_obow,p.eps_present,p.powtorka,p.old_adno,p.studio,p.eps_date,1 derived,p.uwagi_atex,p.spad,p.nag_xx,is_digital
           from spacer_pubstub s, spacer_pub p, (select distinct mak_xx from table(select strony from grzbiet where xx=vgrb_xx)) g2
          where s.xx=p.xx and s.mak_xx=g2.mak_xx) pub,
        spacer_strona, typ_ogloszenia, spacer_kratka, spacer_add, spacer_users
  where makieta.xx=g.mak_xx
    and makieta.xx=pub.mak_xx
    and spacer_strona.mak_xx=g.mak_xx
    and spacer_strona.nr_porz=g.base_nr_porz
    and spacer_strona.str_xx=pub.str_xx
    and drw_xx=drzewo.xx
    and typ_xx=typ_ogloszenia.xx
    and typ_ogloszenia.kra_xx=spacer_kratka.xx
    and (flaga_rezerw=1 or nvl(czas_obow,0)<=vmakdate)
    and pub.add_xx=spacer_add.xx(+)
    and spacer_add.sprzedal=spacer_users.xx(+)
  order by g.dst_nr_porz;

  open vopiCur for -- empty
    select xx, --0
           rect_top,
           rect_left,
           rect_bottom,
           rect_right,
           tekst --5
      from spacer_opis
     where mak_xx=-1;

  open vqueCur for -- empty
    select p.xx, --0
           p.add_xx,
           decode(t.kratowe,0,t.xx,0),
           k.szpalt_x,
           k.szpalt_y,
           p.sizex, --5
           p.sizey,
           p.nazwa,
           p.uwagi,
           p.wersja,
           p.ile_kol, --10
           nvl(p.spo_xx,0)
      from spacer_pub_que p, typ_ogloszenia t, spacer_kratka k
     where mak_xx=-1
       and p.typ_xx=t.xx
       and t.kra_xx=k.xx;
  end open_spoq;
end grb;