create or replace PACKAGE BODY "EPSTEST" as
  function decode_oddzial(vmutacja drzewo.mutacja%type) return drzewo.mutacja%type is
     vret drzewo.mutacja%type;
  begin
     select decode(vmutacja,'SZ','BY','LU','BY','BI','BY','ZI','BY','PL','BY','OL','BY','RA','KA','OP','KA','RZ','KA','KI','KA','CZ','KA','KR','KA','GD','BY','WR','KA','PO','BY','LO','KA','BS','BY','TO','BY',vmutacja) into vret from dual;
     return vret;
  end decode_oddzial;

  function ycntid (vcntid cid_info.xx%type) return cid_info.xx%type
  as
    vvnoflag varchar2(1);
    vactual_cntid cid_info.xx%type;
  begin
    select min(p.contentid) into vactual_cntid 
      from pub@oraent p, pub@oraent p2
     where p2.contentid=vcntid and p.vnoflag='Y'
       and p.adno=p2.adno and p.pubno=p2.pubno;
    return vactual_cntid;
  end ycntid;

  function get_cid (vpub_xx spacer_pub.xx%type) return number is 
    vroot_xx spacer_pub.xx%type := vpub_xx;
    vcntid cid_info.xx%type;
    vparent_xx spacer_pub.xx%type;
    vdatawyliczona date;
  begin 
    loop
      select decode(p.powtorka,null,p.cid_xx,null),
             (select min(p2.xx) from spacer_pub p2,makieta m 
             where p.powtorka is not null and p2.mak_xx=m.xx and m.kiedy=sr.powtseed+p.powtorka and p2.adno=nvl(p.old_adno,p.adno))
        into vcntid,vparent_xx
        from spacer_pub p
       where p.xx=vroot_xx;

      exit when vcntid is null and vparent_xx is null;

      if vcntid is not null then
         return vcntid;
      else
         vroot_xx := vparent_xx;         
      end if;
    end loop;
   
    select min(p.cid_xx) into vcntid from spacer_pub p,emisja_zajawki ez
     where p.xx=vpub_xx and p.adno=ez.adno;
     
    return vcntid;
  end get_cid;

  function powt_path (
    vdaydir varchar2, 
    vadno spacer_pub.adno%type
  ) return varchar2 as
    vret varchar2(32);
  begin
    select min(nvl2(p.powtorka,to_char(sr.powtseed+p.powtorka,vfdaydir)||'\'||nvl(p.old_adno,p.adno),to_char(m.kiedy,vfdaydir)||'\'||p.adno)||'.eps') into vret
      from makieta m,spacer_pub p
     where m.xx=p.mak_xx and m.kiedy=to_date(vdaydir,vfdaydir) and p.adno=vadno and instr(nvl(p.wersja,'-'),'.z')=0 and rownum=1;
     
    -- to rozwi¹zanie preferuje powtórki nad nowymi treœciami przy niejednoznacznoœci <adno,kiedy> w ró¿nych makietach
    /*select path into vret from (
       select first_value(nvl2(p.powtorka,to_char(sr.powtseed+p.powtorka,vfdaydir)||'\'||nvl(p.old_adno,p.adno),to_char(m.kiedy,vfdaydir)||'\'||p.adno)||'.eps') over (order by p.powtorka desc nulls last) path
         from makieta m,spacer_pub p
        where m.xx=p.mak_xx and m.kiedy=to_date(vdaydir,vfdaydir) and p.adno=vadno and instr(nvl(p.wersja,'-'),'.z')=0
    ) where rownum=1;*/

    if vret is null then -- pierwsza emisja zajawki
       select min(to_char(m.kiedy, 'rrrrmmdd')||'\'||z.adno_seed||'.eps') into vret
         from makieta m,spacer_pub p,emisja_zajawki e,zajawka z
        where m.xx=p.mak_xx and m.kiedy=to_date(vdaydir,vfdaydir) and p.adno=vadno 
          and p.powtorka is null and instr(p.wersja,'.z')>0 and e.adno=p.adno and e.zaj_xx=z.xx;
       
       if vret is null then -- pierwsza powtórka zajawki
          select min(to_char(sr.powtseed+p.powtorka,vfdaydir)||'\'||z.adno_seed||'.eps') into vret
            from makieta m,spacer_pub p,makieta mold,spacer_pub pold,emisja_zajawki e,zajawka z
           where m.xx=p.mak_xx and m.kiedy=to_date(vdaydir,vfdaydir) and p.adno=vadno 
             and mold.kiedy=to_char(sr.powtseed+p.powtorka) and mold.xx=pold.mak_xx and pold.adno=nvl(p.old_adno,p.adno)
             and pold.powtorka is null and instr(p.wersja,'.z')>0 and e.adno=pold.adno and e.zaj_xx=z.xx;
    
          if vret is null then -- kolejna emisja zajawki
             select min(nvl2(p.powtorka,to_char(sr.powtseed+p.powtorka,vfdaydir)||'\'||nvl(p.old_adno,p.adno),to_char(m.kiedy,vfdaydir)||'\'||p.adno)||'.eps') into vret
               from makieta m,spacer_pub p
              where m.xx=p.mak_xx and m.kiedy=to_date(vdaydir,vfdaydir) and p.adno=vadno and rownum=1;
          end if;
       end if;
    end if;
    
    return vret;
  end powt_path;
  
  procedure gen_cid (
    vkiedy in varchar2,
    vadno in spacer_pub.adno%type,
    vol in varchar2,
    vfileid out cid_info.xx%type
  ) as
    vatex_cntid cid_info.atex_cntid%type;
  begin
    select max(get_cid(p.xx)) into vfileid from drzewo d,makieta m,spacer_pub p
     where m.drw_xx=d.xx and m.kiedy=to_date(vkiedy,sr.vfShortDate) and p.mak_xx=m.xx and p.adno=vadno and d.mutacja=vol;
    if vfileid is not null then
       return;
    end if;
    
    select max(get_cid(p.xx)) into vfileid from makieta m,spacer_pub p
     where m.kiedy=to_date(vkiedy,sr.vfShortDate) and p.mak_xx=m.xx and p.adno=vadno;
    if vfileid is not null then
       return;
    end if;
    
    select max(ci.xx),nvl(max(ap.contentid),-1) into vfileid,vatex_cntid from cid_info ci,pub@oraent ap
     where ap.pdate=to_date(vkiedy,sr.vfShortDate) and ap.adno=vadno and ap.edition=vol and ap.vnoflag='Y' and ap.contentid=ci.atex_cntid(+);
    if vfileid is not null then
       return;
    end if;

    select max(ci.xx),nvl(max(ap.contentid),-1) into vfileid,vatex_cntid from cid_info ci,pub@oraent ap
     where ap.pdate=to_date(vkiedy,sr.vfShortDate) and ap.adno=vadno and ap.vnoflag='Y' and ap.contentid=ci.atex_cntid(+);
    if vfileid is not null then
       return;
    end if;

    check_file(vatex_cntid,'archiwum','ARCH',vfileid);
    
    update cid_info set status=1,storage_date=least(storage_date,to_date(vkiedy,sr.vfShortDate)-1),preview_date=sysdate,preview_path='Plik wprowadzony z archiwum dla '||vadno||' na '||vkiedy||' ('||vol||')'
     where xx=vfileid;
    update spacer_pub set cid_xx=vfileid,powtorka=null 
     where adno=vadno and mak_xx in (select xx from makieta where kiedy<sysdate and kiedy=to_date(vkiedy,sr.vfShortDate));
  end gen_cid;

  procedure transmisja (
    vmutacja varchar2,
    vcntid number, 
    vadno number, 
    vkiedy varchar2
  ) as 
    vwyd_xx wydawca.xx%type;
  begin
    select min(xx) into vwyd_xx from wydawca where sym=vmutacja;
    if vwyd_xx is null then
       raise_application_error(-20001,'Nieznany oddzial '||vmutacja);
    end if;
    
    insert into cid_present (cid,zsy_xx,kiedy,adno)
         values (vcntid,vwyd_xx,to_date(vkiedy,sr.vfShortDate),vadno);
         
  end transmisja;
  
  procedure get_paczka_xml (
    vcntid in out cid_info.xx%type,
    vapp_source out cid_info.app_source%type,
    vrefCur out sr.refCur
  ) as begin
    select app_source into vapp_source from cid_info where xx=vcntid;
    if vapp_source in (cDual,cPrft) then 
      return;
    end if;
    
    open vrefCur for
      select '<ogloszenie><adno>'||adno||'</adno><vno>'||vno||'</vno><pubno>'||pubno||'</pubno></ogloszenie>'
        from pub@oraent ap, cid_info ci where ci.xx=vcntid and nvl(ci.atex_cntid,ci.xx)=ap.contentid;
       
    select atex_cntid into vcntid from cid_info where xx=vcntid;
  end get_paczka_xml;
  
  procedure get_paczka_xml2 (
    vadno in spacer_pub.adno%type,
    vkiedy in makieta.kiedy%type,
    vrefCur out sr.refCur
  ) as 
    vcc binary_integer;
    vmsg varchar2(6);
  begin
    select count(1) into vcc from pub@oraent ap where ap.adno=vadno and ap.pdate=vkiedy and ap.vnoflag='Y' and ap.ppage<>'CM';
    if vcc = 0 then
       raise_application_error(-20001,'Dla podanego adno='||vadno||' na '||to_char(vkiedy,sr.vfShortDate)||' material nie powinien byc sprawdzany');
    end if;

    select min(d.tytul||' '||d.mutacja) into vmsg from drzewo d,makieta m,spacer_pub p
     where m.drw_xx=d.xx and p.mak_xx=m.xx and p.powtorka is not null and p.adno=vadno and m.kiedy=vkiedy and (p.flaga_rezerw=1 or nvl(czas_obow,0)<=makdate(m.xx));
    if vmsg is not null then
       raise_application_error(-20002,'Dla podanego adno='||vadno||' w makiecie '||vmsg||' na '||to_char(vkiedy,sr.vfShortDate)||' jest zaznaczona powtorka i nowy material nie moze byc sprawdzany');
    end if;
     
    open vrefCur for
      select '<ogloszenie><adno>'||adno||'</adno><vno>'||vno||'</vno><pubno>'||pubno||'</pubno></ogloszenie>'
        from pub@oraent ap where adno=vadno and pdate=vkiedy and vnoflag='Y';
  end get_paczka_xml2;

  procedure check_makieta_prod (
     vmak_xx in makieta.xx%type,
     vrefCur out sr.refCur
  ) as begin
   open vrefcur for select info,null from (
      select ', '||p.adno||' str. '||decode(s.nr_porz,0,m.objetosc,s.nr_porz) info
        from makieta m,spacer_strona s,spacer_pub p
       where m.xx=vmak_xx and s.mak_xx=m.xx and p.mak_xx=s.mak_xx and p.str_xx=s.str_xx and p.x>0 and p.powtorka is null and nvl(p.adno,-1)>0
         and p.cid_xx is not null and p.cid_xx<>nvl((select max(cp.cid) from cid_present cp 
            where cp.kiedy=m.kiedy and cp.zsy_xx=m.wyd_xx and nvl(cp.adno,-1)=nvl(p.adno,-2)),p.cid_xx)
       order by decode(s.nr_porz,0,m.objetosc,s.nr_porz),p.adno)
     union all
       select null,nvl(w.opi_server_url,'Okresl zsylajacego makiete') from makieta m,wydawca w where m.xx=vmak_xx and m.wyd_xx=w.xx(+);
  end check_makieta_prod;
  
  procedure check_makieta_prod2 (
     vmak_xx in makieta.xx%type,
     vopi_url out varchar2,
     verr_text out varchar2
  ) as begin
    select nvl(w.opi_server_url,'Okresl zsylajacego makiete'),vformat.concat_list(cursor(
      select ', '||p.adno||' str. '||decode(s.nr_porz,0,m.objetosc,s.nr_porz) info
        from makieta m,spacer_strona s,spacer_pub p
       where m.xx=vmak_xx and s.mak_xx=m.xx and p.mak_xx=s.mak_xx and p.str_xx=s.str_xx and p.x>0 and p.powtorka is null and nvl(p.adno,-1)>0
         and p.cid_xx is not null and p.cid_xx<>nvl((select max(cp.cid) from cid_present cp 
            where cp.kiedy=m.kiedy and cp.zsy_xx=m.wyd_xx and nvl(cp.adno,-1)=nvl(p.adno,-2)),p.cid_xx)
       order by decode(s.nr_porz,0,m.objetosc,s.nr_porz),p.adno))
      into vopi_url,verr_text
      from makieta m,wydawca w where m.xx=vmak_xx and m.wyd_xx=w.xx(+);
  end check_makieta_prod2;
  
  procedure modemix2 (  
    vdkiedy in makieta.kiedy%type,
    vileDni in binary_integer,
    vmutacja in wydawca.sym%type,
    vlokalne in binary_integer,
    vprobne in binary_integer,
    vwszystko in binary_integer,
    vrefCur out sr.refCur
  ) as
    vfirstEd date;
    vlastEd date;
  begin
    select min(kiedy),max(kiedy) into vfirstEd,vlastEd from makieta m
     where m.kiedy>=vdkiedy and m.kiedy<vdkiedy+21
       and vdkiedy<=trunc(m.data_zm) and trunc(m.data_zm)<vdkiedy+vileDni;
       
   insert into modemix# (contentid,paper,edition,pdate,adno,pubno,kod_oddzialu,adtypno,resstate,cus2name,modelid)
        select ap.contentid,ap.paper,ap.edition,ap.pdate,ap.adno,ap.pubno,
               ad.kod_oddzialu,ap.adtypno,ap.resstate,ad.cus2name,ap.modelid
          from pub@oraent ap, ad@oraent ad
         where ap.pdate between vfirstEd and vlastEd
           and decode_oddzial(ad.kod_oddzialu)=vmutacja 
           and ap.adno=ad.adno and ap.vno=ad.vno and ap.vnoflag='Y' and ad.vnoflag='Y' 
           and ap.state<>'PER' and ad.prodtype='STUDIO';
           
    if vwszystko = 0 then
    open vrefCur for
      select rownum lp,t.* from (
      select d.tytul||' '||d.mutacja tytul,to_char(m.kiedy,sr.vfShortDate) kiedy,
             p.nazwa,t.sym,p.adno,w.sym wyd,
             decode(ci.preview_path,null,'BRAK',decode(ci.status,0,'ZLY MATERIAL','<a href="'||ci.preview_path||chr(38)||'nreps='||p.adno||chr(38)||'date='||to_char(m.kiedy,sr.vfShortDate)||chr(38)||'name='||d.tytul||' '||d.mutacja||'">JEST</a>')) status,
             nvl2((select min(cp.cid) from cid_present cp where cp.cid=ci.xx and cp.zsy_xx=m.wyd_xx and cp.kiedy=m.kiedy) ,'TAK','NIE') dotarl
        from drzewo d, makieta m, spacer_pub p, typ_ogloszenia t, wydawca w,
             modemix# a, cid_info ci
       where m.kiedy>=vdkiedy and m.kiedy<vdkiedy+21
         and vdkiedy<=trunc(m.data_zm) and trunc(m.data_zm)<vdkiedy+vileDni and p.powtorka is null 
         and d.xx=m.drw_xx and m.xx=p.mak_xx and m.wyd_xx=w.xx and p.typ_xx=t.xx
         and (vprobne>0 or a.resstate in ('2','3','4')) 
         and a.paper=d.tytul and a.edition=d.mutacja and a.adno=p.adno and a.pdate=m.kiedy
         and (vlokalne>0 or decode_oddzial(a.kod_oddzialu)<>w.sym)
         and a.adtypno not in (2,3,9) and a.contentid=ci.atex_cntid(+)
         and not exists (select 1 from cid_info ci2 where ci2.atex_cntid=ci.atex_cntid and ci2.xx>ci.xx)
       union all
      select d.tytul||' '||d.mutacja tytul,to_char(m.kiedy,sr.vfShortDate) kiedy,
             a.cus2name,'CLF',a.adno,w.sym wyd,
             decode(ci.preview_path,null,'BRAK',decode(ci.status,0,'ZLY MATERIAL','<a href="'||ci.preview_path||chr(38)||'nreps='||a.adno||chr(38)||'date='||to_char(m.kiedy,sr.vfShortDate)||chr(38)||'name='||d.tytul||' '||d.mutacja||'">JEST</a>')) status,
             nvl2((select min(cp.cid) from cid_present cp where cp.cid=ci.xx and cp.zsy_xx=m.wyd_xx and cp.kiedy=m.kiedy) ,'TAK','NIE') dotarl
        from drzewo d, makieta m, wydawca w,
             modemix# a, cid_info ci
       where m.kiedy>=vdkiedy and m.kiedy<vdkiedy+21
         and vdkiedy<=trunc(m.data_zm) and trunc(m.data_zm)<vdkiedy+vileDni
         and d.xx=m.drw_xx and m.wyd_xx=w.xx
         and (vprobne>0 or a.resstate in ('2','3','4'))
         and a.paper=d.tytul and a.edition=d.mutacja and a.pdate=m.kiedy
         and (vlokalne>0 or decode_oddzial(a.kod_oddzialu)<>w.sym)
         and a.adtypno in (2,3,9) and a.pubno=1 and a.contentid=ci.atex_cntid(+)
         and not exists (select 1 from cid_info ci2 where ci2.atex_cntid=ci.atex_cntid and ci2.xx>ci.xx)
       order by wyd,kiedy) t;
    else --vwszysko>0
    open vrefCur for
      select rownum lp,t.* from (
      select d.tytul||' '||d.mutacja tytul,to_char(m.kiedy,sr.vfShortDate) kiedy,
             a.cus2name nazwa,nvl(a.modelid,'CLF') sym,a.adno,w.sym wyd,
             decode(ci.preview_path,null,'BRAK','<a href="'||ci.preview_path||'">JEST</a>') status,
             nvl2((select min(cp.cid) from cid_present cp where cp.cid=ci.xx and cp.zsy_xx=m.wyd_xx and cp.kiedy=m.kiedy) ,'TAK','NIE') dotarl
        from drzewo d, makieta m, wydawca w,
             modemix# a, cid_info ci
       where m.kiedy>=vdkiedy and m.kiedy<vdkiedy+21
         and vdkiedy<=trunc(m.data_zm) and trunc(m.data_zm)<vdkiedy+vileDni
         and d.xx=m.drw_xx and m.wyd_xx=w.xx 
         and (vprobne>0 or a.resstate in ('2','3','4'))
         and a.paper=d.tytul and a.edition=d.mutacja and a.pdate=m.kiedy
         and (vlokalne>0 or decode_oddzial(a.kod_oddzialu)<>w.sym)
         and a.contentid=ci.atex_cntid(+)
         and not exists (select 1 from cid_info ci2 where ci2.atex_cntid=ci.atex_cntid and ci2.xx>ci.xx)
       order by wyd,kiedy) t;
     end if;     
  end modemix2;

  procedure get_httpsource (
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type,
    vkiedy in varchar2,
    vadno in spacer_pub.adno%type,
    vrefCur out sr.refCur
  ) as 
    vpowtkiedy date;
    vlink varchar2(256);
    vcntid cid_info.xx%type;
    vdkiedy date := to_date(vkiedy,sr.vfshortdate);
    vlinknew varchar2(256) := 'http://epsilon.agora.pl/PdfRepository/GetPdfWithEmbedData.aspx?name='||vtytul||vmutacja||'&date='||to_char(vdkiedy,'rrrrmmdd')||'&nreps='||vadno;
  begin
    if vdkiedy < sysdate - 15 then
      open vrefcur for
        select -1,vlinknew from dual;
    else
      select count(1) into vcntid
        from drzewo d, makieta m, spacer_pub p
       where d.tytul=vtytul and d.mutacja=vmutacja and d.xx=m.drw_xx and m.kiedy=vdkiedy 
         and m.xx=p.mak_xx and p.adno=vadno and p.powtorka is null and p.cid_xx is null;
      if vcntid>0 then
         select max(xx) into vcntid from cid_info ci 
          where status>0 and epstest_adno=vadno and atex_cntid in (select max(contentid) from pub@oraent where edition=vmutacja and pdate=vdkiedy and adno=vadno and vnoflag='Y');
         if vcntid is not null then
            store_cntid(vcntid);
         elsif vadno < 20000000 then
            select max(ci.xx) into vcntid from drzewo d,makieta m,cid_info ci
             where d.tytul=vtytul and d.mutacja=vmutacja and m.kiedy=vdkiedy
               and d.xx=m.drw_xx and ci.atex_cntid=vadno
               and ci.epstest_kiedy=m.kiedy and ci.app_source=cPrft;
            if vcntid is not null then
               update spacer_pub set cid_xx=vcntid where adno=vadno 
                  and mak_xx in (select m.xx from drzewo d,makieta m where d.tytul=vtytul and d.mutacja=vmutacja and m.kiedy=vdkiedy and d.xx=m.drw_xx);
            end if;
         else
            select max(z.cid_xx) into vcntid from emisja_zajawki ez,zajawka z where ez.adno=vadno and ez.zaj_xx=z.xx;
            if vcntid is not null then --zajawka
              update spacer_pub p set cid_xx=vcntid where adno=vadno;
            end if;
         end if;
      end if;
                                                                
      select min(mold.kiedy),min('http://epsilon.agora.pl/PdfRepository/GetPdfWithEmbedData.aspx?name=&date='||to_char(mold.kiedy,'rrrrmmdd')||'&nreps='||pold.adno)
        into vpowtkiedy,vlink
        from drzewo d, makieta m, spacer_pub p, spacer_pub pold, makieta mold
       where d.tytul=vtytul and d.mutacja=vmutacja and m.kiedy=vdkiedy and p.adno=vadno and p.powtorka is not null
         and d.xx=m.drw_xx and m.xx=p.mak_xx and pold.cid_xx=get_cid(p.xx) and pold.mak_xx=mold.xx;
      if vpowtkiedy is not null and vpowtkiedy < sysdate - 15 then
        open vrefCur for
          select -1,vlink from dual;
      else
        open vrefcur for
          select c.status-2,vlinknew
            from drzewo d, makieta m, spacer_pub p, cid_info c
           where d.tytul=vtytul and d.mutacja=vmutacja and m.kiedy=vdkiedy and p.adno=vadno
             and d.xx=m.drw_xx and m.xx=p.mak_xx and get_cid(p.xx)=c.xx and c.preview_path is not null
             and (p.powtorka is not null or exists (select 1 from cid_present cp where cp.cid=c.xx and cp.zsy_xx=m.wyd_xx))
           union all
          select c.status-2,vlinknew
            from drzewo d, makieta m, spacer_pub p, spacer_pubstub ps, cid_info c
           where d.tytul=vtytul and d.mutacja=vmutacja and m.kiedy=vdkiedy and p.adno=vadno and p.xx=ps.xx
             and d.xx=m.drw_xx and m.xx=ps.mak_xx and get_cid(p.xx)=c.xx and c.preview_path is not null
             and (p.powtorka is not null or exists (select 1 from cid_present cp where cp.cid=c.xx and cp.zsy_xx=m.wyd_xx));
      end if;             
    end if; 
  end get_httpsource;
  
  procedure cntid_emisje (
    vcntid in cid_info.xx%type,
    vrefCur out sr.refCur
  ) as 
    vold_cntid cid_info.atex_cntid%type;
    vatex_cntid cid_info.atex_cntid%type;
    vapp_source cid_info.app_source%type;
  begin
    select app_source into vapp_source from cid_info where xx=vcntid;
    if vapp_source = cDual then 
      raise_application_error(-20200,'Material: '||vcntid||' byl sprawdzany sciezka DUAL');
    elsif vapp_source = cZajw then 
      open vrefCur for
         select distinct to_char(m.kiedy,sr.vfShortDate),ez.adno,'00' zsyla,5 isCLF
           from zajawka z,emisja_zajawki ez,spacer_pub p,makieta m
          where z.cid_xx=vcntid and ez.zaj_xx=z.xx and ez.adno=p.adno and p.mak_xx=m.xx and m.kiedy>sysdate
          union all
         select to_char(sr.powtseed+z.powtorka,sr.vfShortDate),z.adno_seed,'00' zsyla,5 isCLF
           from zajawka z where z.cid_xx=vcntid and sr.powtseed+z.powtorka>sysdate;          
      return;
    elsif vapp_source = cPrft then 
      open vrefCur for
         select to_char(ci.epstest_kiedy,sr.vfShortDate),ci.epstest_adno,'00' zsyla,5 isCLF
           from cid_info ci where xx=vcntid;
      return;
    elsif vapp_source in (cIbok,cXmls) then 
       update cid_info ci set ci.epstest_kiedy=(select min(ap.pdate) from pub@oraent ap 
              where ap.contentid=ci.atex_cntid and ap.pdate>sysdate and ap.vnoflag='Y' and ap.ppage<>'CM')
        where ci.xx=vcntid;
    end if;

    select atex_cntid,ycntid(atex_cntid) into vold_cntid,vatex_cntid from cid_info where xx=vcntid;
    if vatex_cntid is null then
       update cid_info set preview_path='W ATEXie nie odnaleziono aktualnego contentid dla fileid: '||vcntid where preview_path is null and xx=vcntid;
       commit;
       raise_application_error(-20001,'W ATEXie nie odnaleziono aktualnego contentid dla fileid: '||vcntid);
    elsif vold_cntid is not null and vold_cntid<>vatex_cntid then
       update cid_info set atex_cntid=vatex_cntid where atex_cntid=vold_cntid;
    end if;

    select count(1) into vold_cntid from pub@oraent ap 
     where ap.contentid=vatex_cntid and ap.paper='WWW' and ap.edition is null
       and not exists (select 1 from pub@oraent ap2 where ap2.paper<>'WWW' and ap2.contentid=ap.contentid);
    if vold_cntid>0 then
       open vrefCur for 
          select to_char(ap.pdate,sr.vfShortDate),ap.adno,'00' zsyla,decode(ap.adtypno,2,1,3,2,9,3,0) isCLF
            from pub@oraent ap where 0=1;
       return;
    end if;
    
    store_cntid(vcntid);
    
    select count(1) into vold_cntid
      from pub@oraent ap, drzewo d, makieta m
     where ap.contentid=vatex_cntid and ap.paper=d.tytul and ap.edition=d.mutacja 
       and ap.pdate=m.kiedy and m.drw_xx=d.xx and ap.vnoflag='Y' and ap.ppage<>'CM';
    if vold_cntid=0 then
       select count(1) into vold_cntid from pub@oraent ap 
        where ap.contentid=vcntid and ap.vnoflag='Y' and ap.ppage<>'CM' and ap.edition='RP';
       if vold_cntid=0 then
          select count(1) into vold_cntid from pub@oraent ap 
           where ap.contentid=vatex_cntid and ap.vnoflag='Y' and ap.ppage<>'CM' and ap.edition is null and ap.paper<>'WWW';
          if vold_cntid>0 then
            update cid_info set preview_path='W ATEXie nie okreslono atrybutu EDITION dla contentid='||vatex_cntid where preview_path is null and xx=vcntid;
            commit;
            raise_application_error(-20002,'W ATEXie nie okreslono atrybutu EDITION dla contentid='||vatex_cntid);
          else
            select count(1) into vold_cntid from pub@oraent ap, drzewo d, makieta m 
             where ap.contentid=vatex_cntid and ap.vnoflag='Y' and ap.ppage<>'CM' and decode(ap.paper,'TCG','DLO',ap.paper)=d.tytul 
               and (d.mutacja=ap.edition or ap.adtypno in (2,3,9)) and ap.pdate=m.kiedy and d.xx=m.drw_xx;
            if vold_cntid=0 then
              update cid_info set preview_path='W Manamie nie zalozono makiety dla contentid='||vatex_cntid||' lub emisja jest wycmentarzowana' where preview_path is null and xx=vcntid;
              commit;
              raise_application_error(-20003,'W Manamie nie zalozono makiety dla contentid='||vatex_cntid||' lub emisja jest wycmentarzowana');
            end if;
          end if;
          
          open vrefCur for
          select to_char(ap.pdate,sr.vfShortDate),ap.adno,nvl(w.sym,decode_oddzial(ap.edition)) zsyla,decode(ap.adtypno,2,1,3,2,9,3,0) isCLF
            from pub@oraent ap, drzewo d, makieta m, wydawca w
           where ap.contentid=vatex_cntid and ap.vnoflag='Y' and ap.ppage<>'CM' and d.tytul='DLO'
             and (d.mutacja=ap.edition or ap.adtypno in (2,3,9)) and d.xx=m.drw_xx and ap.pdate=m.kiedy 
             and m.kiedy>sysdate and m.wyd_xx=w.xx(+)
           order by 1,2;
       else
          open vrefCur for
             select to_char(ap.pdate,sr.vfShortDate),ap.adno,'00' zsyla,decode(ap.adtypno,2,1,3,2,9,3,0) isCLF
               from pub@oraent ap where ap.contentid=vatex_cntid and ap.vnoflag='Y' and ap.pdate>sysdate and ap.ppage<>'CM';
       end if;
       return;
    end if;
    
    open vrefCur for
    select distinct to_char(ap.pdate,sr.vfShortDate),ap.adno,case when ap.paper in ('TAM','TCG','TDN','TPR','TTU') and ap.edition='RP' 
      then '00' else nvl(w.sym,decode_oddzial(ap.edition)) end zsyla,decode(ap.adtypno,2,1,3,2,9,3,0) isCLF
      from cid_info ci, pub@oraent ap, drzewo d, makieta m, wydawca w
     where ci.xx=vcntid and ap.contentid in (select ci2.atex_cntid from cid_info ci2 where ci2.xx=vcntid) and ap.vnoflag='Y' and ap.ppage<>'CM'
       and d.tytul=ap.paper and d.mutacja=ap.edition and d.xx=m.drw_xx and ap.pdate=m.kiedy 
       and nvl(ci.epstest_kiedy,m.kiedy)<=m.kiedy and m.kiedy>sysdate and m.wyd_xx=w.xx(+)
     union 
    select distinct to_char(m.kiedy,sr.vfShortDate),p.adno,decode(m.ope_xx,10,'WA',11,'BY',12,'KA','00') zsyla,0 isCLF
      from spacer_pub p,(
         select m.xx,m.kiedy,epl.ope_xx
           from cid_info ci, makieta m, eps_present_log epl
          where ci.xx=vcntid and nvl(m.zsy_red,0)<>nvl(m.wyd_xx,0)
            and epl.kiedy=m.kiedy and epl.contentid=ci.xx and epl.kiedy>sysdate and epl.ope_xx in (10,11,12)) m
     where p.mak_xx=m.xx and get_cid(p.xx)=vcntid
     order by 1,2;
  exception
     when no_data_found then
        raise_application_error(-20006, 'Material graficzny oznaczony fileid='||vcntid||' nie byl sprawdzany przez EpsTest');
  end cntid_emisje;
  
  procedure cntid_info (
    vcntid in cid_info.xx%type,
    vrefCur out sr.refCur
  ) as 
    vtytul drzewo.tytul%type;
    vmutacja drzewo.mutacja%type;
    vkiedy date;
    vcolour varchar2(20);
    vvnoflag varchar2(3);
    vfilepath varchar2(21);
    vatexVerified pls_integer;
    vold_cntid cid_info.atex_cntid%type;
    vapp_source cid_info.app_source%type;
    vatex_cntid cid_info.atex_cntid%type;
  begin
    select app_source into vapp_source from cid_info where xx=vcntid;
    if vapp_source = cDual then 
      raise_application_error(-20200,'Material: '||vcntid||' byl sprawdzany sciezka DUAL');
    elsif vapp_source = cZajw then -- podstaw wymiary 
      begin
        select /*+ FIRST_ROWS(1) */ p.xx into vold_cntid
          from zajawka z,spacer_pub p,makieta m,drzewo d
         where rownum=1 and z.cid_xx=vcntid and z.typ_xx=p.typ_xx and p.mak_xx=m.xx 
           and nvl(z.tytul,d.tytul)=d.tytul and nvl(z.mutacja,d.mutacja)=d.mutacja
           and m.drw_xx=d.xx and m.kiedy between sysdate-30 and sysdate+30;
      exception
        when no_data_found then
          update cid_info set preview_path='Przed sprawdzeniem tego formatu zajawki narysuj ja w Manamie i zapisz makiete' where preview_path is null and xx=vcntid;
          commit;
          raise_application_error(-20201, 'Przed sprawdzeniem tego formatu zajawki narysuj ja w Manamie i zapisz makiete');
      end;
      open vrefCur for 
        select distinct r.width szerokosc,r.heigth wysokosc,'FULL' kolor,ci.material_path,ci.preview_path,0 konwertujDoXml,
               'TAK' ispresent,'NIE WYMAGA' akceptacja,to_char(sr.powtseed+z.powtorka,sr.vflongdate) zamkniecie,
               es.app_code,'A' setup,to_char(sr.powtseed+z.powtorka,'rrrrmmdd')||'\'||z.adno_seed||'.eps' path
          from cid_info ci, zajawka z, spacer_rozm r, epstest_app_source es
         where ci.xx=vcntid and ci.xx=z.cid_xx and ci.app_source=es.xx and r.pub_xx=vold_cntid and z.tpr_xx<3
         union all -- zajawka profitowa
        select distinct z.typ_xx szerokosc,0 wysokosc,'FULL' kolor,ci.material_path,ci.preview_path,0 konwertujDoXml,
               'TAK' ispresent,z.tytul akceptacja,to_char(ci.epstest_kiedy,sr.vflongdate) zamkniecie,
               'PRFT','A' setup,to_char(sr.powtseed+z.powtorka,'rrrrmmdd')||'\'||z.adno_seed||'.eps' path
          from cid_info ci, zajawka z
         where ci.xx=vcntid and ci.xx=z.cid_xx and z.tpr_xx>=3;
      return;
    elsif vapp_source = cPrft then -- podstaw wymiary 
      open vrefCur for 
        select distinct ci.epstest_adno szerokosc,0 wysokosc,'FULL' kolor,ci.material_path,ci.preview_path,0 konwertujDoXml,
             'TAK' ispresent,'NIE WYMAGA' akceptacja,to_char(ci.epstest_kiedy,sr.vfLongDate) zamkniecie,
             es.app_code,'A' setup
        from cid_info ci, epstest_app_source es
       where ci.xx=vcntid and ci.app_source=es.xx;
      return;
    end if;

    select atex_cntid,ycntid(atex_cntid) into vold_cntid,vatex_cntid from cid_info where xx=vcntid;
    if vatex_cntid is null then
        update cid_info set preview_path='W ATEXie nie odnaleziono aktualnego contentid' where preview_path is null and xx=vcntid;
        commit;
        raise_application_error(-20001,'W ATEXie nie odnaleziono aktualnego contentid: '||vcntid);
    elsif vold_cntid is not null and vold_cntid<>vatex_cntid then
       update cid_info set atex_cntid=vatex_cntid where atex_cntid=vold_cntid;
    end if;

    select nvl(max(ci2.xx),0) into vold_cntid from cid_info ci2 where ci2.atex_cntid=vatex_cntid;
    if vold_cntid > vcntid then
        update cid_info set preview_path='Istnieje nowszy plik do cntid: '||vatex_cntid||' o fileid: '||vold_cntid where preview_path is null and xx=vcntid;
        commit;
        raise_application_error(-20003,'Istnieje nowszy plik do cntid: '||vatex_cntid||' o fileid: '||vold_cntid);
    end if;
     
    select paper,edition,pdate,vnoflag,col,path,atexok
      into vtytul,vmutacja,vkiedy,vvnoflag,vcolour,vfilepath,vatexverified
      from (select paper,edition,pdate,vnoflag,decode(colourcnt,0,'BRAK',1,'SPOT','FULL') col,to_char(pdate,'rrrrmmdd')||'\'||adno||'.eps' path,
              (select count(1) from drzewo d,makieta m,spacer_pub p,typ_ogloszenia t
                where d.tytul=ap.paper and d.mutacja=ap.edition and d.xx=m.drw_xx 
                  and m.kiedy=ap.pdate and m.xx=p.mak_xx and p.adno=ap.adno and p.typ_xx=t.xx
                  and (instr(ap.modelid,'#')>0 or instr(ap.modelid,'^')>0 or instr(ap.modelid,'@')>0 or upper(ap.modelid)=upper(t.modelid)) 
                  and decode(ap.colourcnt,0,1,1,2,4)=p.ile_kol) atexOK
                 from cid_info ci, pub@oraent ap
            where ci.xx=vcntid and ap.contentid=vatex_cntid and ap.vnoflag='Y'
              and not exists (select 1 from cid_info ci2 where ci2.xx>ci.xx and ci2.atex_cntid=ci.atex_cntid)
            order by atexOK desc
    ) where rownum=1;
    if vvnoflag is null then
       select min(ap.vnoflag) into vvnoflag from pub@oraent ap where ap.contentid=vatex_cntid; 
       if vvnoflag is null then
          update cid_info set preview_path='W ATEXie nie odnaleziono contentid: '||vatex_cntid where preview_path is null and xx=vcntid;
          commit;
          raise_application_error(-20004,'W ATEXie nie odnaleziono contentid: '||vatex_cntid);
       else
          update cid_info set preview_path='Dla contentid: '||vatex_cntid||' nie istnieje emisja z vnoflag=''Y''' where preview_path is null and xx=vcntid;
          commit;
          raise_application_error(-20005,'Dla contentid: '||vatex_cntid||' nie istnieje emisja z vnoflag=''Y''');
       end if;
    end if;
      
    store_cntid(vcntid);

    open vrefCur for -- jest na makiecie w manamie i zgadza sie z atex'em
      select * from (
      select r.width szerokosc,
             r.heigth wysokosc,
             nvl(vcolour,'BRAK') kolor,
             ci.material_path,
             ci.preview_path,
             0 konwertujDoXml,
             decode(p.powtorka,null,decode((select count(1) from cid_present cp where cp.cid=p.cid_xx and cp.zsy_xx=m.wyd_xx),0,'NIE','TAK'),'REP') ispresent,
             decode(p.eps_present,0,'NIE',1,'TAK',decode((select count(1) from atex.track@oraent t where t.adno=p.adno and t.prodstep='Akceptacja Ogloszenia'),0,'NIE WYMAGA','NIE WIADOMO')) akceptacja,
             to_char(m.data_zm + cWyprzedzenieWysylania,sr.vfLongDate) zamkniecie,
             es.app_code,
             case when bitand(s.drukarnie,7)=s.drukarnie then 'A' when bitand(s.drukarnie,7)=0 then 'W' else 'AW' end setup,
             vfilepath path
        from cid_info ci, drzewo d, makieta m, spacer_pub p, spacer_strona s, spacer_rozm r, epstest_app_source es
       where vatexVerified>0 and ci.xx=vcntid and ci.app_source=es.xx(+)
         and d.tytul=vtytul and d.mutacja=vmutacja and m.kiedy=vkiedy and m.xx=s.mak_xx and p.str_xx=s.str_xx
         and d.xx=m.drw_xx and p.mak_xx=m.xx and ci.xx=p.cid_xx and p.xx=r.pub_xx and rownum=1
         and not exists (select 1 from cid_info ci2 where ci2.xx>ci.xx and ci2.atex_cntid=ci.atex_cntid)
   union all -- nie ma na makiecie w manamie lub rozni sie od atex'a, ale jest modelid i makieta
      select nvl(nvl((select min(r.width) from spacer_rozm_modelid r where r.modelid=ap.modelid),(select r.width  from makieta m2,spacer_pub p,typ_ogloszenia t,spacer_rozm r where rownum=1 and m2.drw_xx=d.xx and upper(nvl(t.modelid,t.sym))=upper(ap.modelid) and m2.xx=p.mak_xx and m2.kiedy between sysdate-60 and sysdate and r.pub_xx=p.xx and p.typ_xx=t.xx)),0) szerokosc,
             nvl(nvl((select min(r.heigth) from spacer_rozm_modelid r where r.modelid=ap.modelid),(select r.heigth from makieta m2,spacer_pub p,typ_ogloszenia t,spacer_rozm r where rownum=1 and m2.drw_xx=d.xx and upper(nvl(t.modelid,t.sym))=upper(ap.modelid) and m2.xx=p.mak_xx and m2.kiedy between sysdate-60 and sysdate and r.pub_xx=p.xx and p.typ_xx=t.xx)),0) wysokosc,
             nvl(vcolour,'BRAK') kolor,
             ci.material_path,
             ci.preview_path,
             1 konwertujDoXml,
             'NIE' ispresent,
             decode((select count(1) from atex.track@oraent t where t.adno=ap.adno and t.vno=ap.vno and t.prodstep='Akceptacja Ogloszenia'),0,'NIE WYMAGA','NIE WIADOMO') akceptacja,
             to_char(m.data_zm + cWyprzedzenieWysylania,sr.vfLongDate) zamkniecie,
             es.app_code,
             (select case when bitand(s.drukarnie,7)=s.drukarnie then 'A' when bitand(s.drukarnie,7)=0 then 'W' else 'AW' end from spacer_strona s where s.mak_xx=m.xx and s.nr_porz=1) setup,
             vfilepath path
        from pub@oraent ap, cid_info ci, drzewo d, makieta m, epstest_app_source es
       where ci.xx=vcntid and ap.contentid=vatex_cntid and ci.app_source=es.xx(+)
         and d.tytul=ap.paper and d.mutacja=ap.edition and m.kiedy=ap.pdate
         and d.xx=m.drw_xx and ap.vnoflag='Y' and ap.modelid is not null and rownum=1
         and not exists (select 1 from cid_info ci2 where ci2.xx>ci.xx and ci2.atex_cntid=ci.atex_cntid)
         and ((vatexVerified=0 and ap.adtypno in (4,5)) or not exists (select 1 from spacer_pub p where p.mak_xx=m.xx and p.adno=ap.adno))
   union all -- nie ma makiety lub modelid
      select ap.xsize*pg.colwidth + (ap.xsize-1)*pg.colspace,
             decode(ap.adtypno,9,decode(ap.ysizemm,ap.ysize,2.4*ap.ysizemm,ap.ysizemm),ap.ysizemm) wysokosc,
             nvl(vcolour,'BRAK') kolor,
             ci.material_path,
             ci.preview_path,
             2 konwertujDoXml,
             'NIE' ispresent,
             decode((select count(1) from atex.track@oraent t where t.adno=ap.adno and t.vno=ap.vno and t.prodstep='Akceptacja Ogloszenia'),0,'NIE WYMAGA','NIE WIADOMO') akceptacja,
             null zamkniecie,
             es.app_code,
             '?' setup,
             vfilepath path
        from cid_info ci, pub@oraent ap, page@oraent pg, epstest_app_source es
       where ci.xx=vcntid and ap.contentid=vatex_cntid and ci.app_source=es.xx(+)
         and ap.paper=pg.paper and ap.ppage=pg.page and ap.vnoflag='Y' and rownum=1
         and not exists (select 1 from cid_info ci2 where ci2.xx>ci.xx and ci2.atex_cntid=ci.atex_cntid)
         and (ap.adtypno in (2,3,9) or not exists (select 1 from drzewo d, makieta m where m.drw_xx=d.xx and d.tytul=ap.paper 
                                    and d.mutacja=ap.edition and m.kiedy=ap.pdate))
     ) where rownum=1;
  exception
     when no_data_found then
        raise_application_error(-20006, 'Material graficzny oznaczony fileid='||vcntid||' nie byl sprawdzany przez EpsTest');
  end cntid_info;

  procedure select_pub (
    vadno in spacer_pub.adno%type,
    vkiedy in varchar2,
    vrefCur out sr.refCur
  ) as 
    vapp_source cid_info.app_source%type;
    vdkiedy date := to_date(vkiedy,sr.vfShortDate);
  begin
    for c in (select max(cid) cid from cid_present where kiedy=vdkiedy and adno=vadno group by zsy_xx) loop
       store_cntid(c.cid);
    end loop;

    open vrefCur for
      select ci.xx,
             ap.contentid,
             decode(p.powtorka,null,decode((select count(1) from cid_present cp where cp.cid=p.cid_xx and cp.zsy_xx=m.wyd_xx),0,'NIE','TAK'),'REP') ispresent,
             decode(p.eps_present,0,'NIE',1,'TAK',decode((select count(1) from atex.track@oraent t where t.adno=ap.adno and t.vno=ap.vno and t.prodstep='Akceptacja Ogloszenia'),0,'NIE WYMAGA','NIE WIADOMO')) akceptacja,
             ap.paper tytul,ap.edition mutacja,
             t.sym symbol,
             r.width szerokosc,
             r.heigth wysokosc,
             decode(ap.colourcnt,0,'BRAK','FULL') kolor,
             case when ap.paper in ('DLO','TAM','TCG','TDN','TPR','TTU') and ap.edition='RP' then '00' else w.sym end zsyla,
             to_char(m.data_zm + cWyprzedzenieWysylania,sr.vfLongDate) zamkniecie,
             ci.material_path,
             ci.preview_path,
             0 wersja
        from pub@oraent ap, spacer_pub p, drzewo d, makieta m, wydawca w, 
             typ_ogloszenia t, spacer_rozm r, cid_info ci
       where ap.pdate=m.kiedy and m.kiedy=vdkiedy
         and ap.vnoflag='Y' and ap.adno=p.adno and p.adno=vadno
         and p.mak_xx=m.xx and m.wyd_xx=w.xx and m.drw_xx=d.xx and p.typ_xx=t.xx and p.xx=r.pub_xx
         and ap.paper=d.tytul and ap.edition=d.mutacja
         and ap.contentid=ci.atex_cntid(+)
         and not exists (select 1 from cid_info ci2 where ci2.xx>ci.xx and ci2.atex_cntid=ci.atex_cntid)
   union all
      select ci.xx,
             ap.contentid,
             decode((select count(1) from cid_present cp where cp.cid=ap.contentid and cp.zsy_xx=m.wyd_xx),0,'NIE','TAK') ispresent,
             decode((select count(1) from atex.track@oraent t where t.adno=ap.adno and t.vno=ap.vno and t.prodstep='Akceptacja Ogloszenia'),0,'NIE WYMAGA','NIE WIADOMO') akceptacja,
             ap.paper tytul,ap.edition mutacja,
             ap.modelid symbol,
             nvl(nvl((select min(r.width) from spacer_rozm_modelid r where r.modelid=ap.modelid),(select r.width  from makieta m2,spacer_pub p,typ_ogloszenia t,spacer_rozm r where rownum=1 and m2.drw_xx=d.xx and upper(t.modelid)=upper(ap.modelid) and m2.xx=p.mak_xx and m2.kiedy between sysdate-60 and sysdate and r.pub_xx=p.xx and p.typ_xx=t.xx)),0) szerokosc,
             nvl(nvl((select min(r.heigth) from spacer_rozm_modelid r where r.modelid=ap.modelid),(select r.heigth from makieta m2,spacer_pub p,typ_ogloszenia t,spacer_rozm r where rownum=1 and m2.drw_xx=d.xx and upper(t.modelid)=upper(ap.modelid) and m2.xx=p.mak_xx and m2.kiedy between sysdate-60 and sysdate and r.pub_xx=p.xx and p.typ_xx=t.xx)),0) wysokosc,
             decode(ap.colourcnt,0,'BRAK','FULL') kolor,
             case when ap.paper in ('DLO','TAM','TCG','TDN','TPR','TTU') and ap.edition='RP' then '00' else w.sym end zsyla,
             to_char(m.data_zm + cWyprzedzenieWysylania,sr.vfLongDate) zamkniecie,
             ci.material_path,
             ci.preview_path,
             1 wersja
        from pub@oraent ap, page@oraent pg, drzewo d, makieta m, wydawca w, cid_info ci
       where ap.pdate=m.kiedy and m.kiedy=vdkiedy
         and ap.paper=pg.paper and ap.ppage=pg.page and ap.adtypno in (4,5) 
         and ap.vnoflag='Y' and ap.adno=vadno 
         and ap.contentid=ci.atex_cntid(+)
         and not exists (select 1 from cid_info ci2 where ci2.xx>ci.xx and ci2.atex_cntid=ci.atex_cntid)
         and m.wyd_xx=w.xx and m.drw_xx=d.xx
         and ap.paper=d.tytul and ap.edition=d.mutacja
         and not exists (select 1 from spacer_pub p where p.mak_xx=m.xx and p.adno=ap.adno)
   union all
      select ci.xx,
             ap.contentid,
             decode((select count(1) from cid_present cp where cp.cid=ap.contentid),0,'NIE','TAK') ispresent,
             decode((select count(1) from atex.track@oraent t where t.adno=ap.adno and t.vno=ap.vno and t.prodstep='Akceptacja Ogloszenia'),0,'NIE WYMAGA','NIE WIADOMO') akceptacja,
             ap.paper tytul,ap.edition mutacja,
             ap.modelid symbol,
             ap.xsize*pg.colwidth + (ap.xsize-1)*pg.colspace,
             decode(ap.adtypno,9,decode(ap.ysizemm,ap.ysize,2.4*ap.ysizemm,ap.ysizemm),ap.ysizemm) wysokosc,
             decode(ap.colourcnt,0,'BRAK','FULL') kolor,
             null zsyla,
             null zamkniecie,
             ci.material_path,
             ci.preview_path,
             2 wersja
        from pub@oraent ap, page@oraent pg, cid_info ci
       where ap.pdate=vdkiedy
         and ap.paper=pg.paper and ap.ppage=pg.page 
         and ap.vnoflag='Y' and ap.adno=vadno 
         and ap.contentid=ci.atex_cntid(+)
         and not exists (select 1 from cid_info ci2 where ci2.xx>ci.xx and ci2.atex_cntid=ci.atex_cntid)
         and (ap.adtypno in (2,3,9) or not exists (select 1 from drzewo d, makieta m where m.drw_xx=d.xx and d.tytul=ap.paper 
                                    and d.mutacja=ap.edition and m.kiedy=ap.pdate))
       order by tytul,mutacja;
  end select_pub;
  
  procedure store_cntid (
    vcntid in cid_info.xx%type
  ) as begin
     update spacer_pub set cid_xx=vcntid where xx in ( /* ma byc zgodny albo tytul albo mutacja */ 
            select p.xx from spacer_pub p, makieta m, drzewo d, pub@oraent ap, cid_info ci
             where ap.contentid=ci.atex_cntid and ci.xx=vcntid 
               and (ap.paper=d.tytul or ap.edition=d.mutacja)
               and ap.vnoflag='Y' and nvl(ci.epstest_kiedy,m.kiedy)=m.kiedy
               and ap.pdate=m.kiedy and p.adno=ap.adno and m.drw_xx=d.xx and m.xx=p.mak_xx)
        and nvl(cid_xx,-1)<>vcntid;
  end store_cntid;
      
  procedure log_change (
    vope_xx in eps_present_log.ope_xx%type,
    vcntid in cid_info.xx%type
  ) as 
    vfileid cid_info.xx%type;
  begin
    select max(xx) into vfileid from cid_info where atex_cntid=vcntid;
    
    insert into eps_present_log (kiedy,ope_xx,adno,contentid)
         select sysdate,vope_xx,max(adno),vfileid
           from pub@oraent where contentid=vcntid;
  end log_change;

  procedure cntid_change_notify (
    vnew_cntid in cid_info.atex_cntid%type,
    vold_cntid in cid_info.atex_cntid%type
  ) as begin
    update cid_info set atex_cntid=vnew_cntid where atex_cntid=vold_cntid;
  end;
  
  procedure check_file (
    vatex_cntid in cid_info.atex_cntid%type,
    vmaterial_path in cid_info.material_path%type,
    vapp_source in epstest_app_source.app_code%type,
    vfileid out cid_info.xx%type
  ) as 
    vcc pls_integer;
  begin
    if vapp_source not in ('DUAL','ZAJW','ARCH','PRFT') then
       select count(1) into vcc from pub@oraent ap where ap.contentid=vatex_cntid and ap.vnoflag='Y' and ap.ppage<>'CM';
       if vcc = 0 then
          raise_application_error(-20001,'Dla podanego contentid='||vatex_cntid||' material nie powinien byc sprawdzany');
       end if;

       select count(1) into vcc from pub@oraent ap
        where ap.contentid=vatex_cntid and ap.modelid is null and ap.adtypno in (4,5);
       if vcc > 0 then
          raise_application_error(-20002,'Blad w ATEX: Dla contentid='||vatex_cntid||' nie mozna ustalic, czy to drobne, czy modulowe');
       end if;
    
       select max(xx) into vfileid from cid_info 
        where atex_cntid=vatex_cntid and material_path=vmaterial_path and storage_date>sysdate-3/86400;
       if vfileid is not null then
          return;
       end if;
    end if;
        
    insert into cid_info (material_path,app_source,storage_date,atex_cntid)
         select vmaterial_path,xx,sysdate,vatex_cntid
           from epstest_app_source where app_code=vapp_source;
    if sql%rowcount < 1 then
       raise_application_error(-20003,'Nieprawidlowy kod aplikacji zrodlowej: '||vapp_source);
    end if;

    select cid_info_xx.currval into vfileid from dual;

    if vapp_source not in ('DUAL','ZAJW','ARCH','PRFT') then
      store_cntid(vfileid);
    end if;
    
    if vapp_source in ('IBOK','XMLS') then
       insert into eps_present_log (adno,kiedy,contentid,ope_xx) 
            select max(adno),sysdate,vfileid,9 from pub@oraent where contentid=vatex_cntid;
    elsif vapp_source = 'EPSTEST' then
       update cid_info set epstest_kiedy=to_date(substr(material_path,1,10),sr.vfShortDate),material_path=substr(material_path,11)
        where xx=vfileid;
    elsif vapp_source = 'ZAJW' then
       update zajawka set cid_xx=vfileid,powtorka = trunc(sysdate) - sr.powtseed + 1
        where cntid=vatex_cntid;
       insert into eps_present_log (adno,kiedy,contentid,ope_xx) 
            select adno_seed,sysdate,vfileid,9 from zajawka where cntid=vatex_cntid;
    elsif vapp_source = 'PRFT' then
       update cid_info set epstest_adno=vatex_cntid,epstest_kiedy=to_date(substr(material_path,1,10),sr.vfShortDate),material_path=substr(material_path,11) where xx=vfileid;
    end if;
  end check_file;

  procedure store_check_result (
    vfileid in cid_info.xx%type,
    vstatus in cid_info.status%type,
    vurl in cid_info.preview_path%type
  ) as 
    voldcid_xx cid_info.xx%type;
  begin
    update cid_info 
       set status=vstatus,preview_path=vurl,preview_date=sysdate
     where xx=vfileid;
     
    if vstatus>0 then
       update spacer_pub set cid_xx=null where cid_xx in (
          select ci.xx from cid_info ci,cid_info ci2
           where ci2.xx=vfileid and ci.atex_cntid=ci2.atex_cntid and ci.epstest_adno=ci2.epstest_adno 
             and ci.epstest_kiedy=ci2.epstest_kiedy and ci.xx<ci2.xx);
    elsif vstatus=0 then
       select max(cid) into voldcid_xx from cid_present cp,makieta m,spacer_pub p
        where cp.kiedy=m.kiedy and cp.adno=p.adno and cp.zsy_xx=m.wyd_xx and p.mak_xx=m.xx and p.cid_xx=vfileid;
       update spacer_pub set cid_xx=voldcid_xx where cid_xx=vfileid;
    end if;
  end store_check_result;

  procedure get_prod_info (
    vpub_xx in spacer_pub.xx%type,
    vrefCur out sr.refCur
  ) as 
    vroot_xx spacer_pub.xx%type;
    vparent_xx spacer_pub.xx%type;
    vcid_xx cid_info.xx%type := null;
  begin 
    vroot_xx := vpub_xx;
    
    while vroot_xx is not null and vcid_xx is null loop
      select decode(p.powtorka,null,p.cid_xx,null),
             (select min(p2.xx) from spacer_pub p2,makieta m 
               where p.powtorka is not null and p2.mak_xx=m.xx and m.kiedy=sr.powtseed+p.powtorka and p2.adno=nvl(p.old_adno,p.adno))
        into vcid_xx,vparent_xx from spacer_pub p where p.xx=vroot_xx;

      vroot_xx := vparent_xx;         
    end loop;

    if vcid_xx is null then
       select max(ci.xx) into vcid_xx from cid_info ci 
        where ci.status>0 and ci.atex_cntid in (
          select max(ap.contentid) from drzewo d,makieta m,spacer_pub p,pub@oraent ap 
           where d.xx=m.drw_xx and m.xx=p.mak_xx and p.xx=vpub_xx
             and ap.paper=d.tytul and ap.edition=d.mutacja and ap.pdate=m.kiedy and ap.adno=p.adno and ap.vnoflag='Y');
       if vcid_xx is not null then
          store_cntid(vcid_xx);
       else
          select max(z.cid_xx) into vcid_xx from spacer_pub p,emisja_zajawki ez,zajawka z
           where p.xx=vpub_xx and p.adno=ez.adno and ez.zaj_xx=z.xx;             
          if vcid_xx is not null then
             update spacer_pub set cid_xx=vcid_xx where xx=vpub_xx;
          else
            select max(p2.cid_xx) into vcid_xx from spacer_pub p,makieta m,spacer_pub p2,makieta m2
             where p.xx=vpub_xx and p.mak_xx=m.xx and p.adno=p2.adno and m.kiedy=m2.kiedy and p2.mak_xx=m2.xx;
            if vcid_xx is not null then
               update spacer_pub set cid_xx=vcid_xx where xx=vpub_xx;
            end if;
          end if;
       end if;
    end if;
   
    -- czy zostalo przetransmitowane
    select case 
      when p.powtorka is not null or nvl(instr(p.wersja,'z'),0)>0 then 2 
      else (select count(1) cc from cid_present cp where cp.cid=p.cid_xx and cp.zsy_xx in (m.wyd_xx,6) and cp.kiedy=m.kiedy) end
      into vroot_xx from spacer_pub p,makieta m
     where p.xx=vpub_xx and p.mak_xx=m.xx;
     
    open vrefcur for
      select bbox,kolor,vroot_xx from cid_info where xx=nvl(vcid_xx,-1) and nvl(status,0)>0 and vroot_xx>0;
  exception
    when others then
      open vrefCur for select 1 from dual where 0=1;
  end get_prod_info;
  
  procedure store_prod_info (
    vfileid in cid_info.xx%type,
    vepstest_adno in spacer_pub.adno%type,
    vbbox in cid_info.bbox%type,
    vkolor in cid_info.kolor%type,
    vfile_write_date in cid_info.file_write_date%type,
    vuserid in cid_info.userid%type
  ) as 
    vdummy sr.refCur;
  begin
    update cid_info
       set epstest_adno=vepstest_adno,bbox=vbbox,kolor=vkolor,file_write_date=vfile_write_date,
           userid=decode(app_source,cZajw,(select min(u.loginname) from spacer_users u,zajawka z where z.cid_xx=vfileid and z.kto_mod=u.xx),vuserid)
     where xx=vfileid;
     
     for cur in (
        select p.xx from spacer_pub p,makieta m,cid_info ci
         where p.adno=vepstest_adno and ci.xx=vfileid and p.mak_xx=m.xx
           and ci.epstest_kiedy=m.kiedy and ci.epstest_adno=p.adno and ci.status>0
     ) loop
        get_prod_info(cur.xx,vdummy);
     end loop;
  end store_prod_info;
  
  procedure request_prod_info(vpub_xx in spacer_pub.xx%type)
  as begin
     insert into eps_present_log (adno,kiedy,contentid,ope_xx) 
          select p.adno,m.kiedy,get_cid(p.xx),13 from spacer_pub p,makieta m where p.xx=vpub_xx and p.mak_xx=m.xx;
          
     delete from eps_present_log e --usuwanie starych dubli
      where (e.adno,e.kiedy,e.contentid,e.ope_xx) in (
            select p.adno,m.kiedy,get_cid(p.xx),13 from spacer_pub p,makieta m where p.xx=vpub_xx and p.mak_xx=m.xx)
        and exists (select 1 from eps_present_log e2 where e2.xx>e.xx and e2.adno=e.adno and e2.kiedy=e.kiedy and e2.contentid=e.contentid and e2.ope_xx=e.ope_xx);
  end request_prod_info;

  procedure log_workflow (
    v_lastchange      eps_present_log.xx%type,
    v_wf_status_date  eps_present_log.wf_status_date%type,
    v_wf_lokalizacja  eps_present_log.wf_lokalizacja%type,
    v_wf_instance_id  eps_present_log.wf_instance_id%type,
    v_wf_status       eps_present_log.wf_status%type,
    v_wf_completed    eps_present_log.wf_completed%type,
    v_wf_error        eps_present_log.wf_error%type
  ) as begin
    update eps_present_log
       set wf_status_date=v_wf_status_date,wf_lokalizacja=v_wf_lokalizacja,wf_instance_id=v_wf_instance_id,
           wf_status=v_wf_status,wf_completed=v_wf_completed,wf_error=v_wf_error
     where xx=v_lastchange;      
  end log_workflow;
end;