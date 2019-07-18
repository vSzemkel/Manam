create or replace PACKAGE BODY "WWWUSER" as
  function get_grb_color( vgrb_xx grzbiet.xx%type) return varchar2 as
  begin
    return grb.get_color(vgrb_xx);
  end;
  
  function grbmaklp (
    vgrb_xx in grzbiet.xx%type
  ) return sr.refCur
  as 
    vtyt1ch char;
    vmin_xx number;
    vmax_xx number;
    vbase_nr number;
    vcur sr.refCur;
  begin
    select min(mak_xx),max(mak_xx),min(base_nr_porz) into vmin_xx,vmax_xx,vbase_nr
      from table(select strony from grzbiet where xx=vgrb_xx);
    if vmin_xx=vmax_xx then
       select substr(tytul,1,1) into vtyt1ch from grzbiet where xx=vgrb_xx;
       if vtyt1ch in ('U','Y') then
         open vcur for select rownum, vgrb_xx xx, t.* from (select /*+ cardinality(s 40) */ d.tytul,d.mutacja,rejkod.get_opis_drzewa(d.xx,m.kiedy),m.numerrok,g.objetosc,decode(nvl(m.szycie,0),0,'NIE','TAK') szyj,nvl((select p.zuz_xx from produkt p where p.sta_xx<>3 and m.kiedy=p.kiedy and p.tytul=d.tytul and d.mutacja=p.mutacja),d.zuz_xx) zuz_xx,d.ppp,m.grzbietowanie,m.prowadzacy1
           from drzewo d, makieta m, grzbiet g, table(g.strony) s where g.xx=vgrb_xx and g.tytul=d.tytul and g.mutgrb=d.mutacja and m.xx=vmin_xx and s.base_nr_porz=vbase_nr) t;
       else
         open vcur for select rownum, vgrb_xx xx, t.* from (select /*+ cardinality(s 40) */ d.tytul,d.mutacja,rejkod.get_opis_drzewa(d.xx,m.kiedy),m.numerrok,m.objetosc,decode(nvl(m.szycie,0),0,'NIE','TAK') szyj,nvl((select p.zuz_xx from produkt p where p.sta_xx<>3 and m.kiedy=p.kiedy and p.tytul=d.tytul and d.mutacja=p.mutacja),d.zuz_xx) zuz_xx,d.ppp,m.grzbietowanie,m.prowadzacy1
           from drzewo d, makieta m, table(select strony from grzbiet where xx=vgrb_xx) s 
          where d.xx=m.drw_xx and m.xx=vmin_xx and s.base_nr_porz=vbase_nr) t;
       end if;
    else
      open vcur for select rownum, vgrb_xx xx, t.* from (select tytul,mutacja,rejkod.get_opis_drzewa(d.xx,m.kiedy),numerrok,objetosc,decode(nvl(m.szycie,0),0,'NIE','TAK') szyj,nvl((select p.zuz_xx from produkt p where p.sta_xx<>3 and m.kiedy=p.kiedy and p.tytul=d.tytul and d.mutacja=p.mutacja),d.zuz_xx) zuz_xx,d.ppp,grzbietowanie,prowadzacy1
        from drzewo d, makieta m, (with gs as (select * from table(select strony from grzbiet where xx=vgrb_xx) where dst_nr_porz>0) select gs.mak_xx,min(gs.dst_nr_porz) nr from gs group by gs.mak_xx) g2
       where d.xx=m.drw_xx and m.xx=g2.mak_xx order by g2.nr) t;
    end if;
    return vcur;
  end grbmaklp;
  
  function grbmaklpbl (
    vgrb_xx in grzbiet.xx%type
  ) return sr.refCur
  as 
    vcur sr.refCur;
  begin
   open vcur for select rownum, vgrb_xx, t.* from (select /*+ cardinality(s 5) */ tytul,mutacja,(select decode(min(lp),null,'brak blach',min(lp)||'-'||max(lp)) from blacha b where b.grb_xx=vgrb_xx and grb.platecode_in_mak(b.code,m.xx)>0) blachy
      from drzewo d, makieta m, (select mak_xx,min(dst_nr_porz) nr from table(select strony from grzbiet where xx=vgrb_xx) group by mak_xx) s 
     where d.xx=m.drw_xx and m.xx=s.mak_xx order by nr) t;
   return vcur;
  end grbmaklpbl;

  function mak_ile_mod (vmak_xx in makieta.xx%type) return number 
  as 
    vtotal number := 0;
    vmakdate number := makdate(vmak_xx);
  begin 
    select sum(decode(t.kratowe,1,p.sizex*p.sizey,t.sizex*t.sizey)) into vtotal from spacer_pub p, typ_ogloszenia t
     where p.mak_xx=vmak_xx and p.typ_xx=t.xx and t.kra_xx=1 and modelid not in ('NR','N2')
       and p.x>0 and (p.flaga_rezerw=1 or nvl(p.czas_obow,0)<=vmakdate);

    select nvl(vtotal,0)+nvl(sum(30/k.ile_mod*decode(t.kratowe,1,p.sizex*p.sizey,t.sizex*t.sizey)),0) into vtotal 
      from spacer_pub p, typ_ogloszenia t, spacer_kratka k
     where p.mak_xx=vmak_xx and p.typ_xx=t.xx and t.kra_xx=k.xx and k.xx<>1
       and p.x>0 and (p.flaga_rezerw=1 or nvl(p.czas_obow,0)<=vmakdate);

    return round(nvl(vtotal,0),3); 
  end mak_ile_mod;  
  
  function mak_ile_mod_noadno (vmak_xx in makieta.xx%type) return number 
  as 
    vtotal number := 0;
    vmakdate number := makdate(vmak_xx);
  begin 
    select sum(decode(t.kratowe,1,p.sizex*p.sizey,t.sizex*t.sizey)) into vtotal from spacer_pub p, typ_ogloszenia t
     where p.mak_xx=vmak_xx and p.typ_xx=t.xx and t.kra_xx=1 and modelid not in ('NR','N2')
       and p.x>0 and (p.flaga_rezerw=1 or nvl(p.czas_obow,0)<=vmakdate) and adno is null;

    select nvl(vtotal,0)+nvl(sum(30/k.ile_mod*decode(t.kratowe,1,p.sizex*p.sizey,t.sizex*t.sizey)),0) into vtotal 
      from spacer_pub p, typ_ogloszenia t, spacer_kratka k
     where p.mak_xx=vmak_xx and p.typ_xx=t.xx and t.kra_xx=k.xx and k.xx<>1
       and p.x>0 and (p.flaga_rezerw=1 or nvl(p.czas_obow,0)<=vmakdate) and adno is null;

    return round(nvl(vtotal,0),3); 
  end mak_ile_mod_noadno;  

  function mak_ile_red (vmak_xx in makieta.xx%type) return number 
  as 
    vpartsum number;
    vtotal number := 0;
    vmakdate number := makdate(vmak_xx);
  begin 
    for c in (
      select s.str_xx,k.ile_mod from spacer_strona s,spacer_kratka k 
       where s.mak_xx=vmak_xx and s.kra_xx=k.xx and s.dervlvl<>sr.derv_proh
    ) loop
      vpartsum := 0;
      for vi in 0..c.ile_mod-1 loop
        if select_modul_status(vmak_xx,c.str_xx,vi,vmakdate)=sr.redakcyjny then
          vpartsum := vpartsum + 1;
        end if;
      end loop;
      vtotal := vtotal + vpartsum/c.ile_mod;
    end loop;

    return round(nvl(vtotal,0),4); 
  end mak_ile_red;  
  
  function current_manam_ver (
     vuserid mac_adresy.login_xx%type
  ) return varchar2 is
     vret varchar2(50);
  begin
    select last_ip_addr||' '||last_port||' manam '||version into vret
      from mac_adresy m 
     where m.login_xx=vuserid
       and not exists (select 1 from mac_adresy m2 where m2.login_xx=m.login_xx and m2.ostatnio>m.ostatnio);
    return vret;      
  end current_manam_ver;
  
  function powt_info (
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type,
    vdkiedy in makieta.kiedy%type,
    vadno in spacer_pub.adno%type
  ) return varchar2 is
    vret varchar2(64);
  begin
    select decode(p.powtorka,null,'Nowe',decode(p.old_adno,null,to_char(sr.powtseed+p.powtorka,sr.vfShortDate),to_char(sr.powtseed+p.powtorka,sr.vfShortDate)||' '||p.old_adno))
      into vret
      from drzewo d,makieta m,spacer_pub p
     where d.tytul=vtytul and d.mutacja=vmutacja and d.xx=m.drw_xx 
       and m.kiedy=vdkiedy and m.xx=p.mak_xx and p.adno=vadno;
    return vret;
  end powt_info;

  function mak_zsyla (
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type,
    vdkiedy in makieta.kiedy%type
  ) return varchar2 is
    vret varchar2(3);
  begin
    select nvl(w.sym,'N/A') into vret
      from drzewo d, makieta m, wydawca w
     where d.tytul=vtytul and d.mutacja=vmutacja and m.kiedy=vdkiedy
       and d.xx=m.drw_xx and m.wyd_xx=w.xx(+);
    return vret;
  end mak_zsyla;
  
  function get_opis_drzewa (
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type,
    vkiedy in varchar2
  ) return varchar2 is
    vret varchar2(128);
  begin
    select rejkod.get_opis_drzewa(xx,to_date(vkiedy,sr.vfShortDate)) into vret 
      from drzewo where tytul=vtytul and mutacja=vmutacja;
    return vret;
  end get_opis_drzewa;

  function sprawdz_uprawnienia (
     vuser in spacer_users.loginname_ds%type,
     vtytul in drzewo.tytul%type,
     vmutacja in drzewo.mutacja%type,
     vlevel in number
  ) return char is
     vdostep    number;
  begin
     select count(1) into vdostep from spacer_users u
      where lower(u.loginname_ds)=lower(vuser) and (u.manam_vip=1 or exists (
        select 1 from spacer_uprawnienia su,drzewo d
         where u.xx=su.oso_xx and su.drw_xx=d.xx and d.tytul=upper(vtytul) 
           and d.mutacja=upper(vmutacja) and instr(su.dostep,decode(vlevel,1,'R',2,'W','x'))>0));
      
     if vdostep=0 then
        return '0';
     else
        return '1';   
     end if; 
  end sprawdz_uprawnienia; 
  
  function is_mak_owned (
     vtytul in drzewo.tytul%type,
     vmutacja in drzewo.mutacja%type,
     vkiedy in varchar2,
     vuser in spacer_users.loginname%type,
     vfrom_addr in varchar2
  ) return number is
     vcc number;
  begin
     select count(1) into vcc from mak m, v$session s, spacer_users u, mac_adresy ad
      where tytul=vtytul||' '||vmutacja and kiedy=to_date(vkiedy,sr.vfShortDate) and holder=vuser and ad.last_ip_addr=vfrom_addr
        and u.xx=ad.login_xx and m.holder=u.loginname and upper(m.holder)=s.username and ad.ostatnio>trunc(sysdate) and lower(s.program)='manam.exe';
     return vcc;
  end;

  function get_paginy_produktu (
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type
  ) return sr.refCur 
  is 
    c_paginy  sr.refCur; 
  begin
    open c_paginy for
       select substr(p.nazwa,1,instr(p.nazwa,pagina.vceven)-1) nazwa
       from spacer_prn_makieta p,drzewo d
       where p.drw_xx=d.xx and p.parity=0 and p.is_wzorzec=0
         and d.tytul=vtytul and d.mutacja=vmutacja
       order by p.nazwa; 
    return c_paginy;
  end get_paginy_produktu;  

  function get_httppreview (
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type,
    vdkiedy in makieta.kiedy%type,
    vadno in spacer_pub.adno%type
  ) return varchar2 is
    pragma autonomous_transaction;
    vrefCur epstest.http_preview_cursor;
    vrec epstest.http_previewrec;
  begin    
    epstest.get_httpsource(vtytul,vmutacja,to_char(vdkiedy,sr.vfShortDate),vadno,vrefCur);
    
    fetch vrefCur into vrec;
    close vrefcur;
    commit;
    return vrec.httplink;
  end get_httppreview;
  
  function podpis (vpub_xx in spacer_pub.xx%type) return varchar2 /* TO JEST KOPIA FUNKCJI Z PAKIETU pagina SERIALLY REUSABLE */
  is 
    vadno varchar2(12);
    vpodpis varchar2(64);
    vzaj binary_integer;
    vderv binary_integer;
    vtytul drzewo.tytul%type;
    vmutacja drzewo.mutacja%type;
  begin
    select max(tytul),max(mutacja),max(to_char(adno)||wersja),max(decode(wersja,'.z',1,0)),max(derv) into vtytul,vmutacja,vadno,vzaj,vderv
      from (select xx,mak_xx,adno,wersja,0 derv from spacer_pub p union all select p.xx,s.mak_xx,p.adno,p.wersja,1 derv from spacer_pubstub s,spacer_pub p where p.xx=s.xx) p,makieta m,drzewo d
     where p.xx=vpub_xx and p.mak_xx=m.xx and m.drw_xx=d.xx;
   
    if vtytul='TBP' then
      if vzaj = 1 then
        vpodpis := '';
      elsif vderv = 1 then
        vpodpis := 'oferta krajowa/'||vadno;
      else
        select 'oferta '||decode(vmutacja,'BI','bia³ostocka','BY','bydgosko-toruñska','CZ','czêstochowska','GD','gdañska','KA','katowicka','KI','kielecka','KR','krakowska','LO','³ódzka','LU','lubelska','OL','olsztyñska','OP','opolska','PO','poznañska','PL','p³ocka','RA','radomska','RZ','rzeszowska','RP','krajowa','SZ','szczeciñska','WA','warszawska','WR','wroc³awska','ZI','lubuska','lokalna')||'/'||vadno into vpodpis from dual;
      end if;
    else
      vpodpis := vadno;
    end if;
    
    return vpodpis;
  end podpis;
  
  procedure powtorki (
    vadd_xx in spacer_add.xx%type
  ) as begin
     spacer.powtorki(vadd_xx);
  end powtorki;
  
  procedure dervtmpl_from (
    vfname in varchar2,
    vkiedy in varchar2,
    vretCur out sr.refCur
  ) as begin
    derv.dervtmpl_from(vfname,vkiedy,vretCur);
  end dervtmpl_from;

  procedure tprgrbsep (
    vkiedy in varchar2,
    vretCur out sr.refCur
  ) as begin
    open vretCur for
      select rownum,g.tytul||' '||g.mutgrb,g.objetosc,g.xx,
             grb.get_color(g.xx),to_char(data_zamowienia(g.xx),sr.vfShortDate),
             vformat.concat_list(cursor(select sd.nazwa from space_reservation.spacer_drukarnie sd where bitand(sd.xx,g.drukarnie)=sd.xx),', ') drukarnie,
             grb.get_wydaw(g.xx) wydawca,
             grb.get_zsyla(g.xx) zsylajacy,
             grb.get_wydaw_str(g.xx) wydaw_str,
             decode(g.obszar,null,nvl(dm.mut_kolp,nvl((select vformat.format_zasiegi(gk.zasiegi) from grupy_kolp gk where gk.xx=dm.xx),'BRAK OBSZAROW')),vformat.concat_list(cursor(select substr(g.obszar,4*x+1,4) from pivot where x<length(g.obszar)/4),', ')) zasieg,
             grb.get_paper(g.xx) papier,
             grb.get_ppp(g.xx) ppp,
             nvl((select p.zuz_xx from produkt p where p.kiedy=g.kiedy and p.tytul=d.tytul and p.mutacja=d.mutacja and p.sta_xx<>3),
                  d.zuz_xx) zuz_xx
        from grzbiet g, drzewo_mutacji dm, drzewo d, makieta m
       where g.mutgrb=dm.mutacja(+) and g.kiedy=to_date(vkiedy,sr.vfShortDate) and m.xx=oklgrb(g.xx) and m.drw_xx=d.xx;
  end tprgrbsep;

  procedure grzbietyTPR (
    vkiedy in varchar2,
    vnrwyd out binary_integer,
    vretCur out sr.refCur
  ) as 
    vdkiedy date := to_date(vkiedy,sr.vfShortDate);
  begin
    select nvl(max(nr_wyd),0) into vnrwyd from dane_edycji where kiedy=vdkiedy and tpr_xx=1;
    
    open vretCur for
      select rownum,g.xx,tytul||' '||mutgrb,objetosc,
             (select /*+ cardinality(gs 44) */ count(ile_kol) from spacer_strona s,table(select strony from grzbiet where xx=g.xx) gs where s.ile_kol=4 and s.mak_xx=gs.mak_xx and s.nr_porz=gs.base_nr_porz) full#,
             grb.get_color(g.xx) kol,to_char(data_zamowienia(g.xx),sr.vfShortDate) druk,
             decode(depeszowy,null,'NIE','TAK') depesz,
             vformat.concat_list(cursor(select sd.nazwa from spacer_drukarnie sd where bitand(sd.xx,g.drukarnie)=sd.xx),', ') drukarnie, 
             grb.get_wydaw(g.xx) wydawca,
             grb.get_zsyla(g.xx) zsylajacy,
             grb.get_paper(g.xx) papier,
             grb.get_waga(g.xx) waga,
             decode(acc_print_org,null,0,1) acc
        from grzbiet g, drzewo_mutacji dm 
       where g.mutgrb=dm.mutacja(+) and g.kiedy=vdkiedy;
  end grzbietyTPR;

  procedure grzbiet_find (
    vfname in varchar2,
    vyear in varchar2,
    vxmltext out varchar2
  ) as begin
    grb.find_match(vfname,vyear,vxmltext);
  end grzbiet_find;

  procedure set_mutred (
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type,
    vkiedy in varchar2,
    vnr_porz in spacer_strona.nr_porz%type,
    vmutred in drzewo.mutacja%type,
    vsetval in number
  ) as begin
    derv.set_mutred(vtytul,vmutacja,vkiedy,vnr_porz,vmutred,vsetval);
  end set_mutred;
  
  procedure get_objmodcnt(
    vtytul in drzewo.tytul%type,
    vkiedy in varchar2,
    vretCur out sr.refCur
  ) as begin
    open vretCur for 
     select d.mutacja, m.objetosc, mak_ile_mod(m.xx) ile_modlow, mak_ile_mod_noadno(m.xx) ile_rezerwacji
       from makieta m, drzewo d
      where m.kiedy=to_date(vkiedy,sr.vfShortDate) and m.drw_xx=d.xx and d.tytul=vtytul
      order by d.mutacja;
  end get_objmodcnt;

  procedure sab_export (
    vkiedy in varchar2,
    vretCur out sr.refCur
  ) as begin
    open vretCur for 
      select d.tytul,d.mutacja,m.objetosc,mak_ile_red(m.xx) redakcja,(select count(1) from spacer_strona where mak_xx=m.xx and naglowek='hit') hity,round(mak_ile_mod(m.xx)/30,4) ogloszenia
        from makieta m,drzewo d 
       where m.kiedy=to_date(vkiedy,sr.vfShortDate) and m.drw_xx=d.xx and substr(d.tytul,1,1)<>'Z'
       order by 1,2;
  end sab_export;

  procedure get_stub_adno (
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type,
    vkiedy in varchar2,
    vcc out pls_integer,
    vretCur out sr.refCur
  ) as
    vmak_xx makieta.xx%type; 
  begin
    select m.xx,(select count(1) from spacer_pubstub where mak_xx=m.xx) into vmak_xx,vcc
      from drzewo d,makieta m
     where d.tytul=vtytul and d.mutacja=vmutacja and d.xx=m.drw_xx and m.kiedy=to_date(vkiedy,sr.vfShortDate);
         
    open vretCur for 
      select p.adno from spacer_pub p,spacer_pubstub s 
       where s.mak_xx=vmak_xx and p.xx=s.xx and p.adno>0;
  end get_stub_adno;

  procedure druk_grb (
    vkiedy in varchar2,
    vdrukarnia in number,
    vretCur out sr.refCur
  ) as 
    vdkiedy date := to_date(vkiedy,sr.vfShortDate);
  begin
    open vretCur for
    select to_char(g.kiedy,sr.vfShortDate) edycja,
           g.tytul||' '||g.mutgrb kod,
           rejkod.get_opis_drzewa(d.xx,g.kiedy) opis,
           w.sym wydawca,
           d.zuz_xx zuzycie
      from grzbiet g, makieta m, drzewo d, wydawca w
     where g.kiedy between vdkiedy and vdkiedy+30
       and trunc(g.data_druku)=vdkiedy and bitand(g.drukarnie,vdrukarnia)>0
       and m.xx=grb.oklgrb(g.xx) and m.drw_xx=d.xx and m.wyd_xx=w.xx(+)
     order by 1,2;
  end druk_grb;
  
  procedure ustal_date_druku (
    vtytul in drzewo.tytul%type,
    vmutgrb in drzewo.mutacja%type,
    vkiedy in varchar2,
    vdata_druku in varchar2,
    vdeadline in varchar2
  ) as 
    vgrb_xx grzbiet.xx%type;
    vddeadline date := to_date(vdeadline,sr.vfLongDate);
  begin
    update grzbiet set data_druku=to_date(vdata_druku,sr.vfLongDate),deadline=vddeadline
     where tytul=vtytul and mutgrb=vmutgrb and kiedy=to_date(vkiedy,sr.vfShortDate)
 returning xx into vgrb_xx;
    
    if vtytul<>'DGW' then
       /*for c in (select mak_xx,base_nr_porz from table(select strony from grzbiet where xx=vgrb_xx)) loop
          update spacer_strona s set deadline=vddeadline
           where mak_xx=c.mak_xx and nr_porz=c.base_nr_porz;
       end loop;*/
       
       update spacer_strona s set deadline=vddeadline
        where (mak_xx,nr_porz) in (select /*+ cardinality(gs 40) */ mak_xx,base_nr_porz from table(select strony from grzbiet where xx=vgrb_xx) gs);
        
       /*select cast( multiset (select base_nr_porz,base_nr_porz,mak_xx,mak_xx from table(select strony from grzbiet where xx=vgrb_xx)) as grzbiet_strony)
         into vstrony from dual;
       update spacer_strona s set deadline=vddeadline
        where (mak_xx,nr_porz) in (select mak_xx,base_nr_porz from table(cast(vstrony as grzbiet_strony)));*/
    end if;
  end ustal_date_druku;
  
  procedure update_modelid (
    vmodelid in typ_ogloszenia.modelid%type,
    vnazwa in typ_ogloszenia.nazwa%type,
    vwidth in spacer_rozm_nostd.width%type,
    vheight in spacer_rozm_nostd.height%type,
    vswidth in spacer_rozm_nostd.swidth%type,
    vsheight in spacer_rozm_nostd.sheight%type,
    vprecel_flag in typ_ogloszenia.precel_flag%type
  ) as 
    vtyp_xx number;
  begin
    update typ_ogloszenia set nazwa=vnazwa,precel_flag=vprecel_flag 
     where modelid=vmodelid returning xx into vtyp_xx;
    update spacer_rozm_nostd 
       set width=((vwidth+swidth)/(select sizex from typ_ogloszenia where xx=typ_xx)-swidth),
           height=((vheight+sheight)/(select sizey from typ_ogloszenia where xx=typ_xx)-sheight),
           swidth=vswidth,sheight=vsheight
     where drw_xx=0 and typ_xx=vtyp_xx;
    if sql%rowcount=0 then
      insert into spacer_rozm_nostd (drw_xx,typ_xx,width,height,swidth,sheight)
           values (0,vtyp_xx,((vwidth+vswidth)/(select sizex from typ_ogloszenia where xx=vtyp_xx)-vswidth),((vheight+vsheight)/(select sizey from typ_ogloszenia where xx=vtyp_xx)-vsheight),vswidth,vsheight);
    end if;
  end update_modelid;
  
  procedure utworz_makiete (
     vtytul in drzewo.tytul%type,
     vmutacja in drzewo.mutacja%type,
     vkiedy in varchar2,
     vwersja in makieta_lib.wersja%type
  ) as begin
     wyprzedzeniowe.create_makieta_offline(vkiedy,vtytul,vmutacja,vwersja);
    end utworz_makiete;
    
  procedure makingrb (
     vgrb_xx in grzbiet.xx%type,
     vrefCur out sr.refCur
  ) as begin
     open vrefCur for
        select rownum lp,mig.* from (
        select /*+ cardinality (t 2) */ d.tytul||' '||d.mutacja,rejkod.get_opis_drzewa(d.xx,m.kiedy),m.objetosc,t.ilestr,m.grzbietowanie
          from drzewo d,makieta m,(select min(gs.dst_nr_porz) nr,gs.mak_xx,count(gs.mak_xx) ilestr from table(select strony from grzbiet where xx=vgrb_xx) gs group by gs.mak_xx) t 
         where m.drw_xx=d.xx and m.xx=t.mak_xx order by t.nr) mig;
  end makingrb;

  procedure makksiegingrb (
     vgrb_xx in grzbiet.xx%type,
     vrefCur out sr.refCur
  ) as begin
     open vrefCur for
        select rownum lp,mig.* from (
        select /*+ cardinality (t 2) */ d.tytul||' '||d.mutacja,rejkod.get_opis_drzewa(d.xx,m.kiedy),m.objetosc,t.ilestr,m.grzbietowanie
          from drzewo d,makieta m,(select min(gs.dst_nr_porz) nr,gs.mak_xx,count(gs.mak_xx) ilestr from (select get_root_makieta(mak_xx,base_nr_porz) mak_xx,dst_nr_porz from table(select strony from grzbiet where xx=vgrb_xx)) gs group by gs.mak_xx) t 
         where m.drw_xx=d.xx and m.xx=t.mak_xx order by t.nr) mig;
  end makksiegingrb;

  procedure zrodla_powtorek (
     vtytul in varchar2, 
     vmutacja in varchar2, 
     vkiedy in varchar2, 
     vrefCur out sr.refCur
  ) is
     vdkiedy date := to_date(vkiedy,sr.vfShortDate);
  begin
     open vrefCur for
       select dst_p.adno dest_adno,
              src_d.tytul src_prod,
              src_d.mutacja src_edit,
              to_char(src_m.kiedy,sr.vfShortDate) src_date,
              src_p.adno src_adno,
              nvl(src_p.cid_xx,nvl(epstest.get_cid(src_p.xx),0)) src_id
        from spacer_pub dst_p, makieta dst_m, drzewo dst_d,
             spacer_pub src_p, makieta src_m, drzewo src_d
       where nvl(dst_p.adno,0)>0 and dst_m.xx=dst_p.mak_xx and dst_m.drw_xx=dst_d.xx 
         and dst_d.tytul=vtytul and dst_d.mutacja=vmutacja and dst_m.kiedy=vdkiedy
         and dst_p.powtorka is not null
         and (src_d.mutacja=dst_d.mutacja or not exists (
            select 1 from spacer_pub src_p2, makieta src_m2, drzewo src_d2
             where src_p2.mak_xx=src_m2.xx and src_m2.drw_xx=src_d2.xx 
               and src_p2.adno=src_p.adno and src_m2.kiedy=src_m.kiedy and src_d2.mutacja=vmutacja))
         and src_m.xx=src_p.mak_xx and src_m.drw_xx=src_d.xx 
         and src_p.adno=nvl(dst_p.old_adno,dst_p.adno) 
         and src_m.kiedy=sr.powtseed + dst_p.powtorka
       order by dest_adno;      
  end zrodla_powtorek;
  
  procedure makiety_dla_adno (
     vadno in number,
     vkiedy in varchar2, 
     vrefCur out sr.refCur
  ) as begin
     open vrefCur for
        select d.tytul,d.mutacja,w.sym
          from spacer_pub p,makieta m,drzewo d,wydawca w
         where p.adno=vadno and m.kiedy=to_date(vkiedy,sr.vfShortDate) 
           and p.mak_xx=m.xx and m.drw_xx=d.xx and m.wyd_xx=w.xx
         union all
        select p.paper,p.edition,'--'
          from atex.pub p 
         where p.adno=vadno and p.pdate=to_date(vkiedy,sr.vfShortDate) 
           and p.vnoflag='Y' and p.adtypno not in (2,3,9)
           and not exists (select 1 from spacer_pub sp,makieta m where sp.adno=p.adno and m.kiedy=p.pdate and sp.mak_xx=m.xx)
         union all
        select p.paper,p.edition,'--'
          from atex.pub p 
         where p.adno=vadno and p.pdate=to_date(vkiedy,sr.vfShortDate) 
           and p.vnoflag='Y' and p.adtypno in (2,3,9)
         order by 1,2;
  end makiety_dla_adno;

   procedure papier_grzbietu (
     vgrb_xx in grzbiet.xx%type,
     vrefCur out sr.refCur
   ) as begin
     open vrefCur for
       select fp.sym kod_formatu,fp.sizex,fp.sizey,pa.gramatura,pg.ile_stron
         from papier_ads pa,format_papieru fp,
            ( select /*+ cardinality(gs 40) */ s.pap_xx, count(1) ile_stron from table(select strony from grzbiet where xx=vgrb_xx) gs,spacer_strona s
               where gs.mak_xx=s.mak_xx and gs.base_nr_porz=s.nr_porz group by s.pap_xx) pg
        where pa.xx=pg.pap_xx and pa.for_xx=fp.xx;
  end papier_grzbietu;   

  procedure makieta_info (
     vtytul in varchar2, 
     vmutacja in varchar2, 
     vkiedy in date,
     vrefCur out sr.refCur
  ) as begin
     open vrefCur for
        select rejkod.get_opis_drzewa(d.xx,m.kiedy) "opis",
               to_char(nvl((select min(s.deadline) from spacer_strona s where s.deadline>sr.powtseed and s.mak_xx=m.xx and nvl(s.dervlvl,-1) not in (derv.fixd,derv.proh)),(select min(g.deadline) from makingrb mg,grzbiet g where mg.grb_xx=g.xx and mg.mak_xx=m.xx)),sr.vfLongDate) "deadline",
               nvl(wd.sym,'brak w Rejkodzie') "wydawca", nvl(wo.sym,'brak w Manamie') "zsyla_ogl", nvl(wr.sym,'brak w Manamie') "zsyla_red", nvl(wk.sym,'brak w Manamie') "korekta",
               m.objetosc "objetosc", wp.format "format", 
               nvl(wp.naszosc,0) "kod_zuzycia", m.grzbietowanie "uwagi"
          from drzewo d, makieta m, wydawca wd, wydawca wo, wydawca wr, wydawca wk, plan_wyd.wydanie_produktu wp
          where d.tytul=vtytul and d.mutacja=vmutacja and m.kiedy=vkiedy and d.xx=m.drw_xx
            and d.wyd_xx=wd.xx(+) and m.wyd_xx=wo.xx(+) and nvl(m.zsy_red,m.wyd_xx)=wr.xx(+) and nvl(m.kor_xx,m.wyd_xx)=wk.xx(+)
            and m.kiedy=wp.data_edycji and d.tytul||d.mutacja=wp.kod_produktu and wp.odw_xx is null and wp.del_xx is null;
  end makieta_info;

  procedure makiety_info (
     vzsyla_ogl in varchar2,
     vzsyla_red in varchar2,
     vkiedy in varchar2, 
     vile_dni in number,
     vrefCur out sr.refCur
  ) as 
     vdkiedy date := to_date(vkiedy,sr.vfShortDate);
  begin
     if vile_dni > 40 then
        raise_application_error(-20001,'Za duze okno czasowe. Maxymalnie: 40 dni');
     end if;

     open vrefCur for
        select d.tytul "tytul",d.mutacja "mutacja",to_char(m.kiedy,sr.vfShortDate) "edycja", 
               rejkod.get_opis_drzewa(d.xx,m.kiedy) "opis",
               to_char(nvl(s.deadline,(select min(g.deadline) from makingrb mg,grzbiet g where mg.grb_xx=g.xx and mg.mak_xx=m.xx)),sr.vfLongDate) "deadline",
               wd.sym "wydawca", wo.sym "zsyla_ogl", wr.sym "zsyla_red", 
               m.objetosc "objetosc", wp.format "format", nvl(wp.naszosc,0) "kod_zuzycia",
               m.grzbietowanie "uwagi" 
          from drzewo d, makieta m, spacer_strona s, 
               wydawca wd, wydawca wo, wydawca wr, plan_wyd.wydanie_produktu wp
          where d.xx=m.drw_xx and m.xx=s.mak_xx 
            and m.wyd_xx=wo.xx and nvl(m.zsy_red,m.wyd_xx)=wr.xx           
            and d.wyd_xx=wd.xx and wo.sym=decode(vzsyla_ogl,null,wo.sym,vzsyla_ogl)
            and nvl(wr.sym,wo.sym)=decode(vzsyla_red,null,wr.sym,vzsyla_red)
            and s.nr_porz=1 and m.kiedy=wp.data_edycji and d.tytul||d.mutacja=wp.kod_produktu
            and m.kiedy between vdkiedy and vdkiedy+vile_dni
            and s.deadline between vdkiedy+6/24 and vdkiedy+30/24
            and wp.odw_xx is null and wp.del_xx is null
          order by m.kiedy, d.tytul, d.mutacja;
   end makiety_info;

  procedure ogloszenia_edycji (
     vtytul in varchar2, 
     vmutacja in varchar2, 
     vkiedy in varchar2, 
     vrefCur out sr.refCur
  ) as begin
    open vrefCur for 
      select p.adno,decode(s.nr_porz,0,m.objetosc,s.nr_porz) nr_porz,decode(s.num_xx,2,to_char(abs(nr),'fmRN'),to_char(abs(nr))) nr_pag,p.sekcja,
             p.blokada,decode(p.powtorka,null,0,1) powtorka,p.x,p.y,p.sizex,p.sizey,k.szpalt_x,k.szpalt_y,rozmiar_mod_ogloszenia(p.xx) ile_mod,0 krajowe,
             decode(p.ile_kol,1,'BLACK',2,'SPOT','FULL') kolor,p.is_digital,get_httppreview(d.tytul,d.mutacja,m.kiedy,p.adno) preview,
             d.tytul,d.mutacja,m.kiedy -- dla sabatex
        from drzewo d, makieta m, spacer_strona s, spacer_pub p, typ_ogloszenia t, spacer_kratka k
       where s.mak_xx=p.mak_xx and s.str_xx=p.str_xx and m.xx=p.mak_xx and m.drw_xx=d.xx 
         and (vtytul='*' or d.tytul=vtytul) and (vmutacja='*' or d.mutacja=vmutacja)
         and m.kiedy=to_date(vkiedy,sr.vfshortdate) and p.typ_xx=t.xx and t.kra_xx=k.xx 
         and (p.flaga_rezerw=1 or nvl(p.czas_obow,0)<=makdate(p.mak_xx)) and p.x>0
       union all
      select p.adno,decode(s.nr_porz,0,m.objetosc,s.nr_porz) nr_porz,decode(s.num_xx,2,to_char(abs(nr),'fmRN'),to_char(abs(nr))) nr_pag,p.sekcja,
             p.blokada,decode(p.powtorka,null,0,1) powtorka,ps.x,ps.y,p.sizex,p.sizey,k.szpalt_x,k.szpalt_y,rozmiar_mod_ogloszenia(p.xx) ile_mod,1 krajowe,
             decode(p.ile_kol,1,'BLACK',2,'SPOT','FULL') kolor,p.is_digital,get_httppreview(d.tytul,d.mutacja,m.kiedy,p.adno) preview,
             d.tytul,d.mutacja,m.kiedy -- dla sabatex
        from drzewo d, makieta m, spacer_strona s, spacer_pub p, spacer_pubstub ps, typ_ogloszenia t, spacer_kratka k
       where ps.xx=p.xx and s.mak_xx=ps.mak_xx and s.str_xx=ps.str_xx and m.xx=ps.mak_xx and m.drw_xx=d.xx 
         and (vtytul='*' or d.tytul=vtytul) and (vmutacja='*' or d.mutacja=vmutacja)
         and m.kiedy=to_date(vkiedy,sr.vfshortdate) and p.typ_xx=t.xx and t.kra_xx=k.xx
         and (p.flaga_rezerw=1 or nvl(p.czas_obow,0)<=makdate(p.mak_xx)) and ps.x>0
       order by nr_porz,adno;
  end ogloszenia_edycji;

  procedure ogloszenia_strony (
     vtytul in varchar2, 
     vmutacja in varchar2, 
     vkiedy in date, 
     vnr_porz in number,
     vrefCur out sr.refCur
  ) as 
     vcc number;
     vsym spacer_kratka.sym%type;
  begin
    select min(t.kra_xx) into vcc from drzewo d,makieta m,spacer_strona s,spacer_pub p,typ_ogloszenia t
     where d.tytul=vtytul and d.mutacja=vmutacja and m.kiedy=vkiedy and s.nr_porz=vnr_porz
       and d.xx=m.drw_xx and s.mak_xx=m.xx and p.str_xx=s.str_xx and p.mak_xx=s.mak_xx and p.typ_xx=t.xx
       and not exists (select 1 from spacer_rozm_kraty rk where rk.drw_xx=d.xx and rk.kra_xx=t.kra_xx);
    if vcc is not null then
      select sym into vsym from spacer_kratka where xx=vcc;
      raise_application_error(-20001,'Nie okreslono wymiarow kraty '||vsym||' dla produktu '||vtytul||' '||vmutacja);
    end if;

    open vrefCur for 
        select p.xx,
               p.adno,
               decode(s.nr_porz,0,p.objetosc,s.nr_porz) nr_porz,decode(s.num_xx,2,to_char(abs(nr),'fmRN'),
               to_char(abs(nr))),p.blokada,
               round((p.x-1)*(rk.width+rk.swidth)/10,6) posx,
               round((p.y-1)*(rk.height+rk.sheight)/10 + 
               case when nvl(t.dosuwane_do_gory,0)=0 and t.kratowe=0 then 
                  case when 10*r.heigth/(rk.height+rk.sheight) - trunc(10*r.heigth/(rk.height+rk.sheight)) < 0.5 then 
                     -(rk.sheight + (10*r.heigth/(rk.height+rk.sheight) - trunc(10*r.heigth/(rk.height+rk.sheight)))*(rk.height+rk.sheight))/10
                  else
                     (rk.sheight + (rk.height+rk.sheight)-(10*r.heigth/(rk.height+rk.sheight) - trunc(10*r.heigth/(rk.height+rk.sheight)))*(rk.height+rk.sheight))/10
                  end
               else 0 end,6) posy,
               r.width sizex,
               case when t.makietowane_redakcyjnie is null then r.heigth else (select nvl(max(ap.ysizemm),1) from atex.pub ap where ap.adno=p.adno and ap.pdate=p.kiedy and vnoflag='Y') end sizey,
               get_cid(p.xx),t.na_marginesie,t.dosuwane_do_gory,
               --decode(d.xx,664,null,nvl2(t.podpis_reklama,null,n.tresc)),
               --case when substr(p.tytul,1,1)='D' and p.nag_xx<>17 then null else nvl2(t.podpis_reklama,null,n.tresc) end,
               null, -- naglowek
               t.makietowane_redakcyjnie,nvl2(t.makietowane_redakcyjnie,p.x_na_str,null),nvl2(t.makietowane_redakcyjnie,p.y_na_str,null),
               --case when p.tytul='TBP' then podpis(p.xx) when (substr(p.tytul,1,1)='D' and n.do_podpisu=1) or t.podpis_reklama is not null then n.tresc||' '||p.adno else nvl(atex.nekrologi.geturl(p.adno),case when p.powtorka is null and instr(p.wersja,'z')>0 then check_emisja_zajawki(p.adno) else p.adno end) end podpis,
               case when p.tytul='TBP' then podpis(p.xx) when n.do_podpisu=1 or t.podpis_reklama is not null then n.tresc||' '||p.adno else nvl(atex.nekrologi.geturl(p.adno),case when p.powtorka is null and instr(p.wersja,'z')>0 then check_emisja_zajawki(p.adno) else p.adno end) end podpis,
               p.is_digital
         from (select p.xx,p.mak_xx,m.drw_xx,m.kiedy,d.tytul,m.objetosc,p.str_xx,p.typ_xx,p.adno,p.x,p.y,p.sizex,p.sizey,p.blokada,p.wersja,p.powtorka,p.nag_xx,p.czas_obow,p.flaga_rezerw,p.x_na_str,p.y_na_str,p.is_digital,0 derv 
                 from spacer_pub p,makieta m,drzewo d where p.mak_xx=m.xx and m.drw_xx=d.xx and d.tytul=vtytul and d.mutacja=vmutacja and m.kiedy=vkiedy
               union all select p.xx,s.mak_xx,m.drw_xx,m.kiedy,d.tytul,m.objetosc,s.str_xx,p.typ_xx,p.adno,s.x,s.y,p.sizex,p.sizey,s.blokada,p.wersja,p.powtorka,p.nag_xx,p.czas_obow,p.flaga_rezerw,p.x_na_str,p.y_na_str,p.is_digital,1 derv 
                 from spacer_pubstub s,spacer_pub p,makieta m,drzewo d where s.xx=p.xx and s.mak_xx=m.xx and m.drw_xx=d.xx and d.tytul=vtytul and d.mutacja=vmutacja and m.kiedy=vkiedy) p, 
              spacer_strona s, typ_ogloszenia t, spacer_rozm_kraty rk, spacer_rozm r, naglowek_ogloszenia n
        where s.mak_xx=p.mak_xx and s.str_xx=p.str_xx
          and p.typ_xx=t.xx and t.kra_xx=rk.kra_xx and p.drw_xx=rk.drw_xx and p.xx=r.pub_xx
          and decode(s.nr_porz,0,p.objetosc,s.nr_porz)=vnr_porz and p.x>0 and nvl(p.nag_xx,1)=n.xx
          and (p.flaga_rezerw=1 or nvl(p.czas_obow,0)<=makdate(p.mak_xx))
        order by adno;
   end ogloszenia_strony;
   
  procedure copy_mak (
    vnazwa in spacer_prn_makieta.nazwa%type,
    vsrc_tytul in drzewo.tytul%type,
    vsrc_mutacja in drzewo.tytul%type,
    vdst_tytul in drzewo.tytul%type,
    vdst_mutacja in drzewo.tytul%type
  ) as begin
    pagina.copy_mak(vnazwa,vsrc_tytul,vsrc_mutacja,vdst_tytul,vdst_mutacja);
  end copy_mak;

  procedure pozycjonuj_ogloszenie (
    vpub_xx in spacer_pub.xx%type, 
    vposx in spacer_pub.x_na_str%type, 
    vposy in spacer_pub.y_na_str%type
  ) as begin
    update spacer_pub set x_na_str=vposx,y_na_str=vposy where xx=vpub_xx;
  end pozycjonuj_ogloszenie;

  procedure get_grzbiety_blachy (
     vref_id in blacha.reference_id%type,
     vrefCur out sr.refCur
  ) as begin
     open vrefCur for 
        select g.xx,g.tytul,g.mutgrb,b.lp,to_char(g.kiedy,sr.vfShortDate) kiedy,b.fizpages,
               to_char(g.kiedy,'DDMM')||'_'||to_char(b.lp,'FM00')||g.tytul||'_'||
               (select decode(min(substr(b2.code,8*x+7,2)),max(substr(b2.code,8*x+7,2)),min(substr(b2.code,8*x+7,2)),g.mutgrb)
                  from blacha b2,pivot 
                 where b2.grb_xx=g.xx and b2.reference_id=b.reference_id 
                   and b2.sepcode=b.sepcode and 8*x+7 < length(b2.code) )
               ||'_'||b.sepcode||b.ile_kol||'v01' fname
          from grzbiet g,blacha b              
         where g.xx=b.grb_xx and b.reference_id=vref_id           
         order by 2,3;
  end get_grzbiety_blachy;
end;