create or replace package body pagina is
  pragma serially_reusable;

/************************** PODPIS ***************************/
  function podpis (vpub_xx in spacer_pub.xx%type) return varchar2
  as 
    vadno varchar2(12);
    vpodpis varchar2(64);
    vzaj binary_integer;
    --vderv binary_integer;
    vtytul drzewo.tytul%type;
    vmutacja drzewo.mutacja%type;
  begin
    /*select max(tytul),max(mutacja),max(to_char(adno)||wersja),max(decode(wersja,'.z',1,0)),max(derv) into vtytul,vmutacja,vadno,vzaj,vderv
      from (select xx,mak_xx,adno,wersja,0 derv from spacer_pub p union all select p.xx,s.mak_xx,p.adno,p.wersja,1 derv from spacer_pubstub s,spacer_pub p where p.xx=s.xx) p,makieta m,drzewo d
     where p.xx=vpub_xx and p.mak_xx=m.xx and m.drw_xx=d.xx;*/
    select max(tytul),max(mutacja),max(to_char(adno)||wersja),max(decode(wersja,'.z',1,0)) into vtytul,vmutacja,vadno,vzaj
      from spacer_pub p,makieta m,drzewo d
     where p.xx=vpub_xx and p.mak_xx=m.xx and m.drw_xx=d.xx;
   
    if vtytul='TBP' then
      if vzaj = 1 then
        vpodpis := '';
      /*elsif vderv = 1 then
        vpodpis := 'oferta krajowa/'||vadno;*/
      else
        select 'oferta '||decode(vmutacja,'RP','krajowa','BI','bia³ostocka','BY','bydgosko-toruñska','CZ','czêstochowska','GD','gdañska','KA','katowicka','KI','kielecka','KR','krakowska','LO','³ódzka','LU','lubelska','OL','olsztyñska','OP','opolska','PO','poznañska','PL','p³ocka','RA','radomska','RZ','rzeszowska','RP','krajowa','SZ','szczeciñska','WA','warszawska','WR','wroc³awska','ZI','lubuska','lokalna')||'/'||vadno into vpodpis from dual;
      end if;
    else
      vpodpis := vadno;
    end if;
    
    return vpodpis;
  end podpis;

/*********************** PRZEDLUZ_KRESKI **************************/
  procedure przedluz_kreski
  as begin
     update spacer_prn_fun_arg
        set arg=substr(arg,1,instr(arg,' - '))||vckreska||substr(arg,instr(arg,' - ')+2)
      where arg like '% - %';
  end przedluz_kreski;

/*********************** COMPUTE_DEFLAG **************************/
  procedure compute_deflag (vxx in spacer_prn_makieta.xx%type)
  as
    vflag number := 1;
    vcc pls_integer := 0;
  begin
    for c in (select deflag from spacer_prn_mak_fun where mak_xx=vxx order by lp desc) loop
      vcc := vcc + 1;
      vflag := vflag * 2;
      if c.deflag = 1 then
        vflag := vflag + 1;
      end if;
    end loop;

    update spacer_prn_makieta set is_def=mod(vflag,power(2,vcc)) where xx=vxx;
  end compute_deflag;

/******************** GET_BOUNDINGBOX ************************/
  procedure get_boundingbox (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    vbb out varchar2
  ) as begin
    select nvl(min(t.boundingbox),'0 0 751 1092') into vbb
      from spacer_strona s, spacer_prn_makieta m, typ_paginy t
     where s.mak_xx=vmak_xx and s.str_xx=vstr_xx and s.prn_mak_xx=m.xx and m.typ_xx=t.xx;
  end get_boundingbox;

/******************** GET_PPOLE_TRANS ************************/
  procedure get_ppole_trans (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    vtrans out varchar2,
    vrekl out varchar2,
    vmdg out number
  ) as begin
    select decode((select count(1) from spacer_prn_mak_fun mf,spacer_prn_fun f 
                    where s.prn_mak_xx=m.xx and bitand(s.prn_flag,power(2,mf.lp-1))>0
                      and m.xx=mf.mak_xx and mf.fun_xx=f.xx and f.opis not like 'Wer%'),0,nvl(t.pelne_pole_trans,'0 2'),null),
           v.label,nvl(t.margines_do_grzbietu,45) 
      into vtrans,vrekl,vmdg
      from spacer_strona s, spacer_prn_makieta m, typ_paginy t, spacer_prn_vert_label v
     where s.mak_xx=vmak_xx and s.str_xx=vstr_xx and s.prn_mak_xx=m.xx and m.typ_xx=t.xx and m.vrt_xx=v.xx(+);
  exception
    when no_data_found then
    begin
       select t.pelne_pole_trans,v.label,nvl(t.margines_do_grzbietu,45)
         into vtrans,vrekl,vmdg
         from makieta m,spacer_strona s,spacer_prn_makieta mp,typ_paginy t,spacer_prn_vert_label v
        where m.xx=vmak_xx and s.mak_xx=m.xx and s.str_xx=vstr_xx and s.prn_mak_xx is null and mp.drw_xx=m.drw_xx 
        and mp.parity=bitand(s.nr_porz,1) and mp.is_def>0 and mp.typ_xx=t.xx and mp.vrt_xx=v.xx(+);
    exception 
       when no_data_found then
         raise_application_error(-20001,'Nie okreslono paginy dla strony');
       end;
  end get_ppole_trans;

/******************** GET_DROBNEH ************************/
  procedure get_drobneh (
    vmak_xx in makieta.xx%type,
    vdrobneh out number
  ) as begin
    select nvl(min(drobne_height),1034) into vdrobneh
      from spacer_strona s, spacer_prn_makieta mp, typ_paginy t
     where s.mak_xx=vmak_xx and s.prn_mak_xx=mp.xx and mp.typ_xx=t.xx;
  exception
    when no_data_found then
      raise_application_error(-20001,'Nie okreslono papieru');
  end get_drobneh;

/*********************** TTU2TAM2 **************************/
  procedure ttu2tam2(
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type,
    vnazwa in spacer_prn_makieta.nazwa%type
  ) as
  begin
    for c in (
      select m.xx from spacer_prn_makieta m, drzewo d
       where m.drw_xx=d.xx and d.tytul=vtytul and d.mutacja=vmutacja
         and substr(m.nazwa,1,instr(m.nazwa,' ')-1)=vnazwa
    ) loop
      ttu2tam(c.xx);
    end loop;
  end ttu2tam2;

/*********************** TTU2TAM **************************/
  procedure ttu2tam(vxx in spacer_prn_makieta.xx%type) as
    vparity spacer_prn_makieta.parity%type;
  begin
    select parity into vparity
      from spacer_prn_makieta m, drzewo d
     where m.xx=vxx and m.drw_xx=d.xx and d.tytul='TAM';
    --prostokat
    update spacer_prn_mak_fun set fun_xx=decode(vparity,0,48,148)
     where mak_xx=vxx and lp=1;
    --logo
    update spacer_prn_mak_fun set fun_xx=decode(vparity,0,100,200)
     where mak_xx=vxx and lp=13;
    --nr
    update spacer_prn_mak_fun set fun_xx=decode(vparity,0,50,150)
     where mak_xx=vxx and lp=14;
    --data
    update spacer_prn_mak_fun set fun_xx=decode(vparity,0,49,149)
     where mak_xx=vxx and lp=15;
  exception
    when no_data_found then
      raise_application_error(-20001,'To nie jest makieta TAM ('||vxx||')');
  end ttu2tam;

/*********************** GET_DEST_NAME **************************/
  procedure get_dest_name (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    vname out varchar2
  ) as
    vwer varchar2(1);
    vprn_mak_xx spacer_prn_makieta.xx%type;
    vprn_flag spacer_strona.prn_flag%type;
    vdrukarnie spacer_strona.drukarnie%type;
  begin
    begin --wersja
      select prn_mak_xx,prn_flag,decode(drukarnie,null,17,0,17,drukarnie) into vprn_mak_xx,vprn_flag,vdrukarnie
        from spacer_strona where mak_xx=vmak_xx and str_xx=vstr_xx;
      for c in (
        select opis from spacer_prn_mak_fun mf, spacer_prn_fun f
         where f.xx=mf.fun_xx and mf.mak_xx=vprn_mak_xx order by lp
      ) loop
        if bitand(vprn_flag,1)=1 then
          vprn_flag := vprn_flag - 1;
          if instr(c.opis,'Wersja ')=1 then
            vwer := substr(c.opis,8,1);
            for g in (select grb_xx from (select grb_xx from makingrb where mak_xx=vmak_xx union select grb_okl_xx from makingrb where mak_xx=vmak_xx) where grb_xx is not null) loop
              update table(select strony from grzbiet where xx=g.grb_xx)
                 set mutczas=to_number(vwer)
               where mak_xx=vmak_xx and base_nr_porz=(select nr_porz from spacer_strona where mak_xx=vmak_xx and str_xx=vstr_xx);
            end loop;
            goto foundwer;
          end if;
        end if;
        vprn_flag := vprn_flag / 2;
      end loop;
      vwer := '1';
    exception
      when no_data_found then
          vwer := '1';
    end;
<<foundwer>>
    select count(1) into vprn_flag 
      from spacer_strona s, spacer_prn_makieta m, typ_paginy t
     where s.mak_xx=vmak_xx and s.str_xx=vstr_xx and s.prn_mak_xx=m.xx 
       and m.typ_xx=t.xx and length(t.page_device_size)<10;

    select to_char(m.kiedy,'mmdd')||d.tytul||d.mutacja||to_char(decode(s.nr_porz,0,m.objetosc,s.nr_porz),'fm000')||'-'||vwer||decode(s.ile_kol,1,'b','a')||
           case when vdrukarnie<=17 then vsymdruk(vdrukarnie) else decode(vdrukarnie,64,'s',512,'h','-') end||decode(vprn_flag,0,'p','4')
      into vname from drzewo d, makieta m, spacer_strona s
     where m.xx=vmak_xx and d.xx=m.drw_xx and m.xx=s.mak_xx and s.str_xx=vstr_xx;
  end get_dest_name;

/********************** DESC_MAKIETA *************************/
  procedure desc_makieta (
    vxx in spacer_prn_makieta.xx%type,
    vmakCur in out sr.refCur,
    vfunCur in out sr.refCur,
    vfunArgCur in out sr.refCur
  ) as begin
    open vmakCur for select d.tytul||' '||d.mutacja tytul,m.nazwa,m.parity
      from spacer_prn_makieta m, drzewo d where m.xx=vxx and m.drw_xx=d.xx;

    open vfunCur for select mf.lp,f.xx,mf.deflag,f.nazwa,f.opis
      from spacer_prn_makieta m, spacer_prn_mak_fun mf, spacer_prn_fun f
     where m.xx=vxx and m.xx=mf.mak_xx and mf.fun_xx=f.xx
     order by mf.lp;

    open vfunArgCur for select a.fun_xx,a.arg_no,a.arg_type,a.arg
      from spacer_prn_makieta m, spacer_prn_mak_fun mf, spacer_prn_fun f, spacer_prn_fun_arg a
     where m.xx=vxx and m.xx=mf.mak_xx and mf.fun_xx=f.xx and f.xx=a.fun_xx
     order by a.fun_xx,a.arg_no;
  end desc_makieta;

/************************ COPY_MAK ***************************/
  procedure copy_mak (
    vnazwa in spacer_prn_makieta.nazwa%type,
    vsrc_tytul in drzewo.tytul%type,
    vsrc_mutacja in drzewo.tytul%type,
    vdst_tytul in drzewo.tytul%type,
    vdst_mutacja in drzewo.tytul%type
  ) as 
    vdst_drw_xx drzewo.xx%type;
    vsrc_xx_even spacer_prn_makieta.xx%type;
    vsrc_xx_odd spacer_prn_makieta.xx%type;
    vdst_xx_even spacer_prn_makieta.xx%type;
    vdst_xx_odd spacer_prn_makieta.xx%type;
  begin -- definicja dla strony nieparzystej
    select d.xx,me.xx,mo.xx into vdst_drw_xx,vsrc_xx_even,vsrc_xx_odd 
      from drzewo d, spacer_prn_makieta me, spacer_prn_makieta mo, drzewo src_d 
     where d.tytul=upper(vdst_tytul) and d.mutacja=upper(vdst_mutacja)
       and src_d.tytul=upper(vsrc_tytul) and src_d.mutacja=upper(vsrc_mutacja)
       and me.drw_xx=src_d.xx and me.parity=0 and mo.drw_xx=src_d.xx and mo.parity=1
       and me.nazwa=vnazwa||vceven and mo.nazwa=vnazwa||vcodd;
    -- mutex for update
    select xx,xx+1 into vdst_xx_even,vdst_xx_odd
      from spacer_prn_makieta where nazwa='NEXT_XX' for update;
    update spacer_prn_makieta set xx=xx+2 where nazwa='NEXT_XX';
    -- nowe makiety
    insert into spacer_prn_makieta (xx,drw_xx,is_def,parity,nazwa,is_wzorzec,naglowek,typ_xx,vrt_xx)
    select vdst_xx_even,vdst_drw_xx,is_def,parity,nazwa,is_wzorzec,naglowek,typ_xx,vrt_xx
      from spacer_prn_makieta where xx=vsrc_xx_even;
    insert into spacer_prn_makieta (xx,drw_xx,is_def,parity,nazwa,is_wzorzec,naglowek,typ_xx,vrt_xx)
    select vdst_xx_odd,vdst_drw_xx,is_def,parity,nazwa,is_wzorzec,naglowek,typ_xx,vrt_xx
      from spacer_prn_makieta where xx=vsrc_xx_odd;

    insert into spacer_prn_mak_fun (mak_xx,fun_xx,lp,deflag)
         select vdst_xx_even,fun_xx,lp,deflag 
           from spacer_prn_mak_fun where mak_xx=vsrc_xx_even;
    insert into spacer_prn_mak_fun (mak_xx,fun_xx,lp,deflag)
         select vdst_xx_odd,fun_xx,lp,deflag 
           from spacer_prn_mak_fun where mak_xx=vsrc_xx_odd;
           
  exception
    when no_data_found then
      raise_application_error(-20001,'Niepoprawne parametry');
  end copy_mak;
  
/************************ COPY_FUN ***************************/
  procedure copy_fun (
    vmak_dst_xx in spacer_prn_makieta.xx%type,
    vmak_src_xx in spacer_prn_makieta.xx%type,
    vdst_xx in spacer_prn_fun.xx%type,
    vsrc_xx in spacer_prn_fun.xx%type
  ) as begin
    insert into spacer_prn_fun (xx,parity,nazwa,opis,admopis)
    select vdst_xx,parity,nazwa,opis,admopis
      from spacer_prn_fun where xx=vsrc_xx;
    insert into spacer_prn_fun_arg (fun_xx,arg_no,arg_type,arg)
    select vdst_xx,arg_no,arg_type,arg
      from spacer_prn_fun_arg where fun_xx=vsrc_xx;
    insert into spacer_prn_mak_fun (mak_xx,fun_xx,lp,deflag)
    select vmak_dst_xx,vdst_xx,lp,deflag
      from spacer_prn_mak_fun
     where mak_xx=vmak_src_xx and fun_xx=vsrc_xx;
  end copy_fun;

/************************ CLONE_FUN ***************************/
  procedure clone_fun (
    vmak_xx in spacer_prn_makieta.xx%type,
    vlp in spacer_prn_fun.xx%type
  ) as
    vfun_xx spacer_prn_makieta.xx%type;
  begin
    select nvl(max(xx)+1,1) into vfun_xx from spacer_prn_fun;

    insert into spacer_prn_fun (xx,parity,nazwa,opis,admopis)
    select vfun_xx,f.parity,f.nazwa,f.opis,f.admopis
      from spacer_prn_fun f,spacer_prn_mak_fun mf
     where mf.mak_xx=vmak_xx and mf.lp=vlp and mf.fun_xx=f.xx;
    insert into spacer_prn_fun_arg (fun_xx,arg_no,arg_type,arg)
    select vfun_xx,arg_no,arg_type,arg from spacer_prn_fun_arg 
     where fun_xx=(select fun_xx from spacer_prn_mak_fun where mak_xx=vmak_xx and lp=vlp);

    update spacer_prn_mak_fun set fun_xx=vfun_xx where mak_xx=vmak_xx and lp=vlp;
  end clone_fun;

/************************** NR_STR ***************************/
  procedure nr_str (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as
  begin
    select '('||decode(num_xx,2,rzym.liczba(abs(nr)),ltrim(to_char(abs(nr))))||')'||
      case when abs(nr)>99 then vcNrScl else null end
      into voutText from spacer_strona
     where mak_xx=vmak_xx and str_xx=vstr_xx;
  exception
    when no_data_found then
        raise_application_error(-20001,'PRN_NR_STR: Nie ma takiej strony');
  end nr_str;

/************************** HEADER_LEN ************************/
  procedure header_len (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as
  begin
    select nvl(length(n.text),0) into voutText
      from spacer_strona s, spacer_naglowki_prn n
     where s.mak_xx=vmak_xx
        and s.str_xx=vstr_xx
        and s.prn_mak_xx=n.xx(+);
  end header_len;

/************************** GET_HEADER ************************/
  procedure get_header (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as
  begin
    select '(' || n.text || ')' into voutText
      from spacer_strona s, spacer_naglowki_prn n
     where s.mak_xx=vmak_xx
        and s.str_xx=vstr_xx
        and s.prn_mak_xx=n.xx(+);
  end get_header;

/************************** GET_WERSJA ************************/
  procedure get_wersja (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    varg in varchar2,
    voutText out varchar2
  ) as
  begin
    select '(' || decode(mutacja,'RP',varg,decode(bitand(nr_porz,1),1,varg||' '||mutacja||substr(tytul,2,1),mutacja||substr(tytul,2,1)||' '||varg)) || ')' into voutText
      from spacer_strona s, makieta m, drzewo d
     where m.xx=vmak_xx and m.drw_xx=d.xx
       and s.mak_xx=m.xx and s.str_xx=vstr_xx;
  end get_wersja;

/************************** GET_WERSJA_TTU ************************/
  procedure get_wersja_ttu (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    varg in varchar2,
    voutText out varchar2
  ) as
  begin
    select '(' || decode(mutacja,'RP',varg,decode(bitand(nr_porz,1),0,varg||' '||mutacja||substr(tytul,2,1),mutacja||substr(tytul,2,1)||' '||varg)) || ')' into voutText
      from spacer_strona s, makieta m, drzewo d
     where m.xx=vmak_xx and m.drw_xx=d.xx
       and s.mak_xx=m.xx and s.str_xx=vstr_xx;
  end get_wersja_ttu;

/**************************** GET_WWW *****************************/
  procedure get_www (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as begin
    select case d.tytul 
              when 'DLO' then '(wyborcza.pl)'
              else '(wyborcza.pl)'
           end into voutText 
      from makieta m, drzewo d where m.xx=vmak_xx and d.xx=m.drw_xx;
  end get_www;

/*************************** GET_IMIENINY *************************/
  procedure get_imieniny (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    vouttext out varchar2
  ) as 
    vCur sys_refcursor;
  begin -- top 5
    open vcur for
       select nazwa from (
          select i.nazwa from makieta m,drzewo d,imieniny i where m.xx=vmak_xx and d.xx=m.drw_xx and i.data_im like '%d'||to_char(m.kiedy,'fmdd')||' m'||to_char(m.kiedy,'fmmm')||'%' order by length(data_im) desc
       ) where rownum<6 order by 1;
       
    select '(' || vformat.concat_list(vcur,', ') || ')' into vouttext from dual;
  end get_imieniny;

/*********************** ILE_DNI_DATY ***************************/
  function ile_dni_daty (
     vmak_xx makieta.xx%type,
     vstr_xx spacer_strona.str_xx%type
  ) return binary_integer as
     vtt char;
     vile binary_integer;
  begin     
     select substr(tytul,1,1) into vtt from drzewo d,makieta m where m.drw_xx=d.xx and m.xx=vmak_xx;
     if vtt <> 'D' then
        return 0;
     end if;     
  
     with nostd as (
        select min(d.ile_dni) d
          from spacer_strona s,makieta m,spacer_prn_makieta p,spacer_prn_data_swiat d
         where s.mak_xx=vmak_xx and s.str_xx=vstr_xx and s.mak_xx=m.xx and s.prn_mak_xx=p.xx and p.typ_xx=d.typ_xx and m.kiedy=d.kiedy
     ) select nvl(nostd.d,decode(to_char(kiedy,'d'),'6',1,0)) into vile from makieta,nostd where xx=vmak_xx;
     
     return vile;
  end ile_dni_daty;

/*************************** GET_DAY *************************/
  procedure get_day (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as
    vile binary_integer := ile_dni_daty(vmak_xx,vstr_xx); 
  begin    
    select '('||decode(vile,0,to_char(kiedy,'fmday'),to_char(kiedy,'fmday')||vckreska||to_char(kiedy+vile,'fmday'))||')' 
      into voutText from makieta where xx=vmak_xx;
  end get_day;

/*************************** GET_DATE_DGW **********************/
  procedure get_date_dgw (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as
    vkiedy  date;
    vmonth1 number;
    vmonth2 number;
    vile binary_integer := ile_dni_daty(vmak_xx,vstr_xx); 
  begin
    select kiedy,to_number(to_char(kiedy, 'mm')),to_number(to_char(kiedy+vile, 'mm')) into vkiedy,vmonth1,vmonth2 
      from makieta where xx=vmak_xx;
    if vile = 0 then
      voutText := '('||to_char(vkiedy,'fmDD')||' '||vmiesiaca(vmonth1)||' '||to_char(vkiedy,'yyyy')||')';
    elsif vmonth1 = vmonth2 then
      voutText := '('||to_char(vkiedy,'fmDD')||vckreska||to_char(vkiedy+vile,'fmDD')||' '||vmiesiaca(vmonth1)||' '||to_char(vkiedy,'yyyy')||')';
    elsif vmonth1 < vmonth2 then
      voutText := '('||to_char(vkiedy,'fmDD')||' '||vmiesiaca(vmonth1)||vckreska||
                  to_char(vkiedy+vile,'fmDD')||' '||vmiesiaca(vmonth2)||' '||to_char(vkiedy,'yyyy')||')';
    else
      voutText := '('||to_char(vkiedy,'fmDD')||' '||vmiesiaca(vmonth1)||' '||to_char(vkiedy,'yyyy')||vckreska||
                  to_char(vkiedy+vile,'fmDD')||' '||vmiesiaca(vmonth2)||' '||to_char(vkiedy+vile,'yyyy')||')';
    end if;
  end get_date_dgw;

/*************************** GET_DATE *************************/
  procedure get_date ( /* uzywane w TQC */
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as
    vkiedy date;
    vcrossday sr.srtinyint;
    vcrossmonth sr.srtinyint;
    vckl_xx drzewo.ckl_xx%type;
  begin
    select count(1) into vcrossday from makieta m, drzewo d 
     where m.xx=vmak_xx AND m.drw_xx=d.xx 
       and d.tytul in ('DLO','DGW') and m.kiedy=to_date('19/06/2019',sr.vfshortdate);  
    if vcrossday>0 then
      select '(Œroda' || vckreska || 'czwartek, 19' || vckreska || '20 czerwca 2019)' into voutText from dual;
      return;
    end if;
    
    -- (c) Marcin Buchwald                      2000
    -- format: Sobota-niedziela 30 wrzesnia-1 pazdziernika 2000

    select kiedy,nvl(ckl_xx,0) into vkiedy,vckl_xx from makieta m,drzewo d where m.xx=vmak_xx and m.drw_xx=d.xx;

    select to_number(to_char(vkiedy,'D')), decode(to_char(vkiedy,'MM'),to_char(vkiedy+1,'MM'),0,1)
      into vcrossday, vcrossmonth from dual;
    if vcrossday = 6 then
        if vckl_xx = 1 then
           vcrossday := 1;
        else
           vcrossday := 0;
           vcrossmonth := 0;
        end if;
    else
        vcrossday := 0;
        vcrossmonth := 0;
    end if;

    select '(' ||
    decode(to_number(to_char(vkiedy,'D')),
        1,'Poniedzia³ek, ',
        2,'Wtorek, ',
        3,'Œroda, ',
        4,'Czwartek, ',
        5,'Pi¹tek, ',
        6,decode(vckl_xx,1,'Sobota'||vckreska||'niedziela, ','Sobota, '),
        7,'Niedziela, ') ||
    to_char(vkiedy,'fmDD') ||
    decode(vcrossmonth,
        0,null,
        1,' '||vmiesiaca(to_number(to_char(vkiedy,'MM'))))||
    decode(vcrossday,
        0,null,
        1,vckreska||to_char(vkiedy+1,'fmDD')) ||
    ' '||vmiesiaca(to_number(to_char(vkiedy+vcrossmonth,'MM')))||' '||
    to_char(vkiedy,'RRRR') || ')' into voutText from dual;
  end get_date;

  /************************** GET_DATE_UPPER ************************/
  procedure get_date_upper ( /* zwraca date w postaci SOBOTA?NIEDZIELA 10?11 CZERWCA 2006 */
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as
    vText varchar2(256);
  begin
    get_date(vmak_xx, vstr_xx, vText); 
    select upper(vText) into voutText from dual;
  end get_date_upper;
  
/************************** GET_DATE_UPPER_MTH ************************/
  procedure get_date_upper_mth ( /* zwraca date w postaci SOBOTA?NIEDZIELA 10?11 CZERWCA 2006 */
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as begin
    get_date_dgw(vmak_xx, vstr_xx, voutText); 
    voutText := upper(voutText);
  end get_date_upper_mth;
  
/*************************** GET_DATE_INITLOWER *************************/
  procedure get_date_initlower ( /* zmienia pierwsza litere na mala */
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as 
  begin
    get_date(vmak_xx, vstr_xx, voutText);  
    select '('||lower(substr(voutText,2,1))||substr(voutText,3) into voutText from dual;
  end get_date_initlower;
  
/************************** GET_DATE_TCG ************************/
  procedure get_date_tcg (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as
    vkiedy date;
    vdrw_xx drzewo.xx%type;
    vcrossday sr.srtinyint;
    vcrossmonth sr.srtinyint;
  begin
    select kiedy,drw_xx into vkiedy,vdrw_xx from makieta where xx=vmak_xx;
    if to_char(vkiedy,sr.vfshortdate)='30/04/2015' then
      voutText := '(Czwartek'||vckreska||'czwartek  30 kwietnia '||vckreska||' 7 maja 2015)';
      return;
    end if;
 
    select to_number(to_char(vkiedy,'D')), decode(to_char(vkiedy,'MM'),to_char(vkiedy+6,'MM'),0,1)
      into vcrossday, vcrossmonth from dual;
    if vcrossday = 5 then
        vcrossday := 1;
    else
        voutText := '(TCG, TMC i TTR ukazuje sie tylko w piatki)';
        return;
    end if;

    select decode(tytul,'TMC','(','(Pi¹tek'||vckreska||'czwartek ')||to_char(vkiedy,'fmDD')||
           decode(vcrossmonth,1,' '||vmiesiaca(to_number(to_char(vkiedy,'MM'))),null)||vckreska||to_char(vkiedy+6,'fmDD')||' '||
           vmiesiaca(to_number(to_char(vkiedy+6,'MM')))||' '||to_char(vkiedy+6,'RRRR')||')' into voutText from drzewo where xx=vdrw_xx;
  end get_date_tcg;

/************************** GET_DATE_CTA ************************/
  procedure get_date_cta (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as
  begin
    select '('||initcap(trim(to_char(kiedy+15,'month'))||' '||to_char(kiedy+15,'rrrr'))||')'
      into voutText
      from makieta m, spacer_strona s
     where m.xx=vmak_xx and s.mak_xx=m.xx and s.str_xx=vstr_xx;
  end get_date_cta;

/************************** GET_DATE_TPR_2011 ************************/
  procedure get_date_tpr_2011 (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as
    vkiedy date;
    vparity pls_integer;
  begin
    select kiedy into vkiedy from makieta where xx=vmak_xx;

    if to_char(vkiedy,'dd/mm/rrrr')='10/11/2005' then
      select '('||decode(vparity,1,'| ',null)||'10'||vckreska||'11 LISTOPADA'||decode(vparity,0,' |',null)||')'
        into voutText from dual;
      return;
    end if;

    select '('||to_char(vkiedy,'fmDD')||' '||vmiesiaca(to_number(to_char(vkiedy,'MM')))||' '||to_char(vkiedy,'RRRR') || ')' 
      into voutText from dual;
  end get_date_tpr_2011;

/*************************** GET_DATE_MDN *************************/
  procedure get_date_mdn ( /* Format: 5 wrzesnia 2006 */
  	vmak_xx in makieta.xx%type,
  	vstr_xx in spacer_strona.str_xx%type,
  	voutText out varchar2
  ) as 
    vkiedy date;
  begin
    select kiedy into vkiedy from makieta where xx=vmak_xx;
    select '('||to_char(vkiedy,'fmDD')||' '||vmiesiaca(to_number(to_char(vkiedy,'mm'))) ||' '||to_char(vkiedy,'rrrr') ||')'
		  into voutText from dual;
  end get_date_mdn;

  /*************************** GET_DATE_TRD *************************/
  procedure get_date_trd (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as
    vkiedy date;
  begin
    -- format: 6 lutego 2012 r.
    select kiedy into vkiedy from makieta where xx=vmak_xx;

    select '(' || to_char(vkiedy,'fmDD') ||  
       ' '|| vmiesiaca(to_number(to_char(vkiedy,'MM'))) ||' '||
       to_char(vkiedy,'RRRR') || ' r.)' into voutText from dual;
  end get_date_trd;  
  
/*************************** GET_DATE_TQG *************************/
  procedure get_date_tqg (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as 
    vkiedy date;
  begin 
    select kiedy into vkiedy from makieta where xx=vmak_xx;
    voutText := '('||to_char(vkiedy,'fmDay')||', '||to_char(vkiedy,'fmDD')||' '||vmiesiaca(to_number(to_char(vkiedy,'MM')))||' '||to_char(vkiedy,'yyyy')||')';
  end get_date_tqg;

  /************************ GET_DAY_INITCAP **********************/
  procedure get_day_initcap (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voutText out varchar2
  ) as
    vText varchar2(256);
  begin
    pagina.get_day(vmak_xx, vstr_xx, vText); 
    select initcap(vText) into voutText from dual;
  end get_day_initcap;

/************************* SET_CONFIG *************************/
  procedure set_config (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    vprn_mak_xx in spacer_strona.prn_mak_xx%type,
    vprn_flag in spacer_strona.prn_flag%type default -1,
    vtopage in number default 1
  ) is
    vpc number;
    vnr_porz number;
    pragma autonomous_transaction;
  begin
    update spacer_strona
       set prn_mak_xx=decode(vprn_mak_xx,0,null,vprn_mak_xx),
           prn_flag=decode(vprn_flag,0,null,-1,(select sum(deflag * power(2,lp-1)) from spacer_prn_mak_fun where mak_xx=vprn_mak_xx),vprn_flag)
     where mak_xx=vmak_xx
       and str_xx=vstr_xx
 returning nr_porz into vnr_porz;

    if vtopage > 1 then
      select count(1) into vpc from spacer_strona where mak_xx=vmak_xx;
      for i in 2..vtopage loop
        update spacer_strona
           set prn_mak_xx=decode(vprn_mak_xx,0,null,vprn_mak_xx) + bitand(nr_porz,1) - bitand(vnr_porz,1),
               prn_flag=decode(vprn_flag,0,null,vprn_flag)
         where mak_xx=vmak_xx and nvl(dervlvl,derv.none) not in (derv.fixd,derv.proh)
           and nr_porz=mod(vnr_porz + i - 1,vpc);
      end loop;
    end if;

    commit;
end set_config;

/*************************** GEN_PS ***************************/
  procedure gen_ps (
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    vretCur in out sr.refCur
  ) is
    subtype textLine is varchar2(4000);
    vcmdLine textLine;
    vretArg textLine;
    vpartialCmd textLine;
    voutText spacer_mod_mak := spacer_mod_mak();
    vi sr.srtinyint;
    vparity sr.srtinyint;
    vfc sr.srtinyint;
    vflag sr.srint;
    vprn_mak_xx number;
    visPion boolean := false;
    vargcnt pls_integer := 0;
    vposNrScl pls_integer;
    vtyp_xx typ_paginy.xx%type;
    vpagesize format_papieru.pagesize_pt%type;
  begin
    /*
    **
    **  (c) Marcin Buchwald                     marzec-czerwiec 2000
    **               Oracle                            listopad 2000
    **    up to 31 elements
    */

    select s.prn_mak_xx, s.prn_flag, bitand(s.nr_porz,1), tp.page_device_size
      into vprn_mak_xx, vflag, vparity, vpagesize
      from spacer_strona s, spacer_prn_makieta pm, typ_paginy tp
     where s.mak_xx=vmak_xx and s.str_xx=vstr_xx and s.prn_mak_xx=pm.xx(+) and pm.typ_xx=tp.xx(+);
    if vprn_mak_xx is null then
    begin -- domyslna pusta
   		select pm.xx, 0, bitand(s.nr_porz,1), tp.page_device_size
   		  into vprn_mak_xx, vflag, vparity, vpagesize
        from spacer_prn_makieta pm, makieta m, spacer_strona s, typ_paginy tp
       where pm.drw_xx=m.drw_xx and m.xx=s.mak_xx and str_xx=vstr_xx and m.xx=vmak_xx
         and pm.parity=bitand(nr_porz,1) and pm.is_wzorzec>1 and pm.typ_xx=tp.xx(+);
    exception
      when no_data_found then
        raise_application_error(-20001,'GEN_PS: Nie zdefiniowano zadnej makiety PRN dla tego tytulu.');
    end;
    end if;

    select sign(parity-vparity) into vi from spacer_prn_makieta where xx=vprn_mak_xx;
    if vi <> 0 then
        raise_application_error(-20002,'GEN_PS: Wykryto niezgodnosc parzystosci w przypisaniu pagin.');
    end if;
    
    -- pagesize
    if vpagesize is not null then
      voutText.extend;
      voutText(voutText.last) := spacer_mod_str(0,'<< /PageSize ['||vpagesize||'] >> setpagedevice','','');
    end if;
    voutText.extend;
    voutText(voutText.last) := spacer_mod_str(0,'%%EndPageSetup'||chr(13)||chr(10)||'/STAN_STR save def','','');

    -- poczatkowa translacja
    select t.xx, decode(parity,1,init_trans_odd,init_trans_even)||' translate' into vtyp_xx, vcmdLine
      from spacer_prn_makieta m, typ_paginy t
     where m.xx=vprn_mak_xx and m.typ_xx=t.xx;
    voutText.extend;
    voutText(voutText.last) := spacer_mod_str(0,vcmdLine,'','');

    -- ustal ilosc zdefiniowanych funkcji (numeracja lp jest ciagla!)
    select nvl(max(lp+1),0) into vfc
      from spacer_prn_mak_fun where mak_xx=vprn_mak_xx;

    -- elementy prn
    vi := 1;
    while vi <= vfc loop
      if mod(vflag,2.0) = 1.0 then
        vposNrScl := 0;
        vcmdLine := '';
        for argCur in (
            select arg, arg_type
              from genprn
             where mak_xx=vprn_mak_xx
               and lp=vi
             order by arg_no
        ) loop
          if argCur.arg_type = 0.0 then
            vcmdLine := vcmdLine || ' ' || argCur.arg;
          elsif argCur.arg_type = 1.0 then -- use dynamic sql with single parameter
            if vargcnt = 0 then
              vpartialCmd :=  'call pagina.' || argCur.arg || '(:1,:2,:3,:4)';
              vargcnt := vargcnt + 1;
            else
              vargcnt := 0;
              execute immediate vpartialCmd using in vmak_xx, in vstr_xx, in argCur.arg, out vretArg;
              vcmdLine := vcmdLine || ' ' || vretArg;
            end if;
          elsif argCur.arg_type = 2.0 then -- use dynamic sql
            execute immediate 'call pagina.' || argCur.arg || '(:1,:2,:3)' using in vmak_xx, in vstr_xx, out vretArg ;
            vcmdLine := vcmdLine || ' ' || vretArg;
            vposNrScl := instr(vcmdLine,vcNrScl);
          else
            raise_application_error(-20003,'Niepoprawny identyfikator typu argumentu: '||argCur.arg_type);
          end if;
        end loop;

     begin
        select nazwa into vretArg
          from spacer_prn_fun f, spacer_prn_mak_fun mf
         where f.xx=mf.fun_xx and mf.lp=vi and mf.mak_xx=vprn_mak_xx;
        if vposNrScl>0 then
          if vtyp_xx=7 and vretArg='textKCcmyk' then
            vcmdLine := '-1'||substr(vcmdLine,3,vposNrScl-3)||' .8 1'||substr(vcmdLine,vposNrScl+length(vcNrScl));
            vretArg := 'textKCcmykScl';
          else -- nie obslugujemy inaczej nr>99, znacznik wycinamy
            vcmdLine := substr(vcmdLine,1,vposNrScl-1)||substr(vcmdLine,vposNrScl+length(vcNrScl));
          end if;
        end if;
        visPion := visPion or substr(vretArg,1,4) = 'pion';

        voutText.extend;
        voutText(voutText.last) := spacer_mod_str(vi,ltrim(vcmdLine || ' ' || vretArg),'','');
     exception
        when no_data_found then
            vflag := 2*vflag + 1;
            null; --nie okreslono elementu na paginie
     end;

        vflag := vflag - 1;
      end if;

      vi := vi + 1;
      vflag := vflag / 2;
    end loop;

    -- koncowa translacja
    select decode(parity,1,final_trans_odd,final_trans_even)||' translate' into vcmdLine
      from spacer_prn_makieta m, typ_paginy t
     where m.xx=vprn_mak_xx and m.typ_xx=t.xx;
    voutText.extend;
    voutText(voutText.last) := spacer_mod_str(99999,vcmdLine,'','');

    -- przesuniecie obrazu przy belce pionowej
    if visPion then
      select decode(vparity,1,'11','-11')||' 0 translate' into vcmdLine from dual;
      voutText.extend;
      voutText(voutText.last) := spacer_mod_str(1.5,vcmdLine,'','');
    end if;

    open vretCur for
      select space from table(cast(voutText as spacer_mod_mak)) order by str_xx;

  exception
    when no_data_found then
      select tytul into vcmdLine from mak where mak_xx=vmak_xx;
      raise_application_error(-20004,'GEN_PS: Nie odnaleziono strony.');
  end gen_ps;

/************************** USUN2 ***************************/
  procedure usun2 (
    vxx in spacer_prn_makieta.xx%type
  ) as
    vcc pls_integer;
    vnazwa spacer_prn_makieta.nazwa%type;
    vtytul drzewo.tytul%type;
    vmutacja drzewo.mutacja%type;
  begin
    select count(1), min(tytul), min(mutacja), min(substr(nazwa,1,instr(nazwa,' ')-1))
      into vcc, vtytul, vmutacja, vnazwa
      from spacer_prn_makieta m, drzewo d
     where m.xx=vxx and d.xx=m.drw_xx and (uid=(select user_id from all_users where username='SPACE_RESERVATION') or sql_check_access(d.xx,'W')>=0);
    if vcc < 1 then
       return;--raise_application_error(-20001,'Nie mozesz usunac tej paginy, gdyz nie masz prawa zapisu do tytulu '||vtytul||' '||vmutacja);
    end if;

    usun(vnazwa,vtytul,vmutacja);
  end usun2;

/************************** USUN ***************************/
  procedure usun (
    vnazwa in varchar2,
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type
  )as
    vdrw_xx drzewo.xx%type;
  begin
    begin
      select xx into vdrw_xx from drzewo
       where tytul=vtytul and mutacja=vmutacja ;--and do_kiedy>sysdate;
    exception
      when no_data_found then
          raise_application_error(-20001,'Nie zarejestrowano tytulu '||vtytul||' '||vmutacja||' lub tytul jest nieaktualny');
    end;

    update spacer_strona set prn_mak_xx=null,prn_flag=null
     where prn_mak_xx in
      (select xx from spacer_prn_makieta
        where is_wzorzec<>1 and drw_xx=vdrw_xx
          and nazwa in (vnazwa||vceven,vnazwa||vcodd))
       and mak_xx in (select xx from makieta where drw_xx=vdrw_xx);
    delete from spacer_prn_mak_fun where mak_xx in
      (select xx from spacer_prn_makieta
        where is_wzorzec<>1 and drw_xx=vdrw_xx
          and nazwa in (vnazwa||vceven,vnazwa||vcodd));
    delete from spacer_prn_makieta
        where is_wzorzec<>1 and drw_xx=vdrw_xx
          and nazwa in (vnazwa||vceven,vnazwa||vcodd);
    delete from spacer_prn_fun_arg
     where fun_xx not in (select fun_xx from spacer_prn_mak_fun)
       and fun_xx not in (select xx from spacer_prn_fun_org);
    delete from spacer_prn_fun
     where xx not in (select fun_xx from spacer_prn_mak_fun)
       and xx not in (select xx from spacer_prn_fun_org);

    insert into spacer_log (opis) values ('Usuniecie makiety prn '||vnazwa||' do '||vtytul||' '||vmutacja);
    commit;
  end usun;

/************************** USUN_TYP ***************************/
  procedure usun_typ (
    vtyp_xx in typ_paginy.xx%type
  ) as begin
    if uid<>100 then
      raise_application_error(-20001,'Brak uprawnien');
    end if;
    
    update spacer_prn_makieta set is_wzorzec=0 where is_wzorzec=1 and typ_xx=vtyp_xx;
    for c in (select xx from spacer_prn_makieta where typ_xx=vtyp_xx and parity=0) loop
      usun2(c.xx);
    end loop;
    delete from typ_paginy where xx=vtyp_xx;
    update spacer_prn_makieta set xx=1+(select max(xx) from spacer_prn_makieta where is_wzorzec=0)
     where nazwa='NEXT_XX';
    commit;    
  end usun_typ;

/*********************** COLOURIZE_TEXTKLR **************************/
  procedure colourizeTextKLR(
    vfun_xx in spacer_prn_fun.xx%type,
    vc in number,
    vm in number,
    vy in number,
    vk in number
  ) as 
    vcc pls_integer;
  begin
    select count(1) into vcc from spacer_prn_fun where xx=vfun_xx and nazwa='textKLR';
    if vcc<>1 then
      raise_application_error(-20001,'colourizeTextKLR wywo³ano dla funkcji, która nie jest typu textKLR');
    end if;
    
    update spacer_prn_fun set nazwa='textKLRcmyk',opis='Reklama kolor'
     where xx=vfun_xx;
    update spacer_prn_fun_arg set arg_no=arg_no+4
     where arg_no>1 and fun_xx=vfun_xx;
    insert into spacer_prn_fun_arg (fun_xx,arg_no,arg_type,arg)
         values (vfun_xx,2,0,to_char(vc,'fm9D99','NLS_NUMERIC_CHARACTERS=''.,'''));
    insert into spacer_prn_fun_arg (fun_xx,arg_no,arg_type,arg)
         values (vfun_xx,3,0,to_char(vm,'fm9D99','NLS_NUMERIC_CHARACTERS=''.,'''));
    insert into spacer_prn_fun_arg (fun_xx,arg_no,arg_type,arg)
         values (vfun_xx,4,0,to_char(vy,'fm9D99','NLS_NUMERIC_CHARACTERS=''.,'''));
    insert into spacer_prn_fun_arg (fun_xx,arg_no,arg_type,arg)
         values (vfun_xx,5,0,to_char(vk,'fm9D99','NLS_NUMERIC_CHARACTERS=''.,'''));
  end colourizetextklr;
end pagina;