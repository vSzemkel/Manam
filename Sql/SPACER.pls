create or replace PACKAGE BODY                     "SPACER" is 
/******************** PRIVATE ************************/
  vvszpalt_x spacer_kratka.szpalt_x%type;
  vvszpalt_y spacer_kratka.szpalt_y%type;
  vvsciezka spacer_strona.sciezka%type;
  vvparity sr.srtinyint;
  vvwsekcji sr.srtinyint;
  vvposx sr.srtinyint;
  vvposy sr.srtinyint;
  vvmakdate sr.srint;
  vvfirstrun boolean;

/******************* CHECK MAK EXISTS ****************/
  function check_mak_exists (
    vmak_xx in number,
    vkiedy in varchar2,
    vmutacja in varchar2
  ) return number
  as 
    vxx number;
  begin
    select nvl(min(m2.xx),-1) into vxx
      from makieta m, makieta m2, drzewo d, drzewo d2
     where m.xx=vmak_xx and m.drw_xx=d.xx and d.tytul=d2.tytul
       and d2.mutacja=vmutacja and d2.xx=m2.drw_xx and m2.kiedy=to_date(vkiedy,sr.vfShortDate);
    return vxx;
  end check_mak_exists;
  
/******************* CHECK BASE KRAT ****************/
  function check_base_krat (
    vpx in spacer_pub.x%type, 
    vpy in spacer_pub.x%type, 
    vsizex in spacer_pub.sizex%type, 
    vsizey in spacer_pub.sizey%type, 
    vogl_kra_xx in spacer_kratka.xx%type,
    vmak_xx in makieta.xx%type,
    vstr_xx in spacer_strona.str_xx%type
  ) return number
  as 
    vret number;
    vogl_left number;
    vogl_right number;
    vogl_top number;
    vogl_bottom number;
    vmod_baz_cnt number;
    vbsx spacer_kratka.szpalt_x%type;
    vmakdate number := makdate(vmak_xx);
  begin
    select (vpx-1)*kb.szpalt_x/ko.szpalt_x,(vpx-1+vsizex)*kb.szpalt_x/ko.szpalt_x,
           (vpy-1)*kb.szpalt_y/ko.szpalt_y,(vpy-1+vsizey)*kb.szpalt_y/ko.szpalt_y,
           kb.ile_mod-1,kb.szpalt_x
      into vogl_left,vogl_right,vogl_top,vogl_bottom,vmod_baz_cnt,vbsx
      from spacer_strona s,spacer_kratka kb,spacer_kratka ko
     where s.mak_xx=vmak_xx and s.str_xx=vstr_xx and s.kra_xx=kb.xx and ko.xx=vogl_kra_xx;
    
    for vi in 0..vmod_baz_cnt loop
       if ((mod(vi,vbsx)<=vogl_left and vogl_right<=mod(vi,vbsx)+1) 
        or (vogl_left<=mod(vi,vbsx) and mod(vi,vbsx)+1<=vogl_right) 
        or (vogl_left<mod(vi,vbsx) and mod(vi,vbsx)<vogl_right) 
        or (vogl_left<mod(vi,vbsx)+1 and mod(vi,vbsx)+1<vogl_right))
       and ((floor(vi/vbsx)<=vogl_top and vogl_bottom<=floor(vi/vbsx)+1) 
        or (vogl_top<=floor(vi/vbsx) and floor(vi/vbsx)+1<=vogl_bottom) 
        or (vogl_top<floor(vi/vbsx) and floor(vi/vbsx)<vogl_bottom) 
        or (vogl_top<floor(vi/vbsx)+1 and floor(vi/vbsx)+1<vogl_bottom))
       then
         vret := select_modul_status(vmak_xx,vstr_xx,vi,vmakdate);
         if vret<>sr.wolny then
            return vret;
         end if;
       end if;
    end loop;
    
    return sr.wolny;
  end check_base_krat;

/******************** CHECK OWNED *********************/  
  procedure check_mak_owned ( -- package private
    vmak_xx in number
  ) is
    vmsg varchar2(255);
  begin
     select max('Makieta jest otwarta w trybie zmian przez uzytkownika: '||loginname||' tel. '||tel) into vmsg 
       from spacer_users where xx=(select decode(nvl(system_lock,uid),uid,-1,system_lock) from makieta where xx=vmak_xx);
     if vmsg is not null then
        raise_application_error(-20102,vmsg); 
     end if;
  end check_mak_owned;

/******************** BEGIN DEAL *********************/
  procedure begin_deal (
    vmak_xx in number,
    vstr_xx in number,
    vadd_cnt in number  
  ) is
    pragma autonomous_transaction;
    vcc number;
    vmsg varchar2(1000);
  begin
    -- sekcja krytyczna
    lock table spacer_token in exclusive mode;
    
    select count(m.xx) into vcc 
    from makieta m, spacer_strona s
    where m.xx=vmak_xx
      and s.str_xx=vstr_xx
      and m.xx=s.mak_xx
      and vadd_cnt=(select count(xx) from spacer_pub where mak_xx=m.xx and str_xx=s.str_xx and x>0 and (flaga_rezerw=1 or nvl(czas_obow,0)<=makdate(m.xx)))
      and (nvl(m.system_lock,uid)=uid or not exists (select 1 from v$session where user#=m.system_lock and upper(program)='MANAM.EXE' and status<>'KILLED'))
      and (nvl(s.system_lock,uid)=uid or s.lock_deadline<sysdate or not exists (select 1 from v$session where user#=s.system_lock and upper(program)='MANAM.EXE' and status<>'KILLED'))
      and m.data_zm>sysdate;
    
    if vcc = 0 then
      rollback;
    
      select count(1) into vcc
        from (select 1 from spacer_pub 
               where mak_xx=vmak_xx and str_xx=vstr_xx and x>0
                 and (flaga_rezerw=1 or nvl(czas_obow,0)<=makdate(mak_xx))
               union all select 2 from spacer_pubstub 
               where mak_xx=vmak_xx and str_xx=vstr_xx);
      if vcc <> vadd_cnt then
        raise_application_error(-20100,'Stan makiety jest nieaktualny'); 
      end if;

      begin
        select xx into vcc
        from makieta where xx=vmak_xx and data_zm<sysdate;
        raise_application_error(-20101,'Zakonczono juz sprzedaz do tej makiety'); 
      exception
        when no_data_found then
          null;
      end;
    
      check_mak_owned(vmak_xx);

      select decode(nvl(system_lock,uid),uid,-1,system_lock) into vcc
        from spacer_strona where mak_xx=vmak_xx and str_xx=vstr_xx;
      if vcc > 0 then
        select 'Na tej stronie wlasnie stara sie sprzedac ogloszenie uzytkownik: '||loginname||' tel. '||tel
          into vmsg
          from spacer_users where xx=vcc;
        raise_application_error(-20103,vmsg); 
      end if;
    end if;

    -- set lock_deadline to now plus 10 min.
    update spacer_strona
       set system_lock=uid,
           lock_deadline=sysdate+1/144
     where mak_xx=vmak_xx
       and str_xx=vstr_xx;  
     
    commit;
  end begin_deal;

/******************* CANCEL DEAL ********************/
  procedure cancel_deal (
    vmak_xx in number,
    vstr_xx in number
  ) is
  begin
    update spacer_strona
       set system_lock=null,
           lock_deadline=null
     where mak_xx=vmak_xx
       and str_xx=vstr_xx;  
     
    commit;
  end cancel_deal;
  
/******************* DEL_QUE ********************/
  procedure del_que (
    vpub_xx in number
  ) is 
    vpub2_xx number;
    vdummy sr.refCur;
  begin
    update spacer_pub p set p.adno=(select adno from spacer_pub_que where xx=vpub_xx),p.czas_obow=null,bit_zapory=1
     where exists (select 1 from spacer_pub_que q 
                    where q.xx=vpub_xx and p.mak_xx=q.mak_xx and p.add_xx=q.add_xx 
                      and p.nazwa=q.nazwa and q.adno is not null) and p.adno is null and rownum=1
           returning p.xx into vpub2_xx;
    if sql%rowcount=1 then
      spacer_set_space(vpub2_xx,sr.ogloszenie);
      epstest.get_prod_info(vpub2_xx,vdummy); -- inicjalizacja cid_xx
    end if;
    delete from spacer_pub_que where xx=vpub_xx;
    commit;
  end del_que;
  
/******************* DEL_PUB ********************/
  procedure del_pub (
    vpub_xx in number -- <0 to qued
  ) is 
    vadd_xx number;
    vadno number;
  begin
    -- usunac moze kierownik, master lub dealer, ktory sprzedal
    select a.xx,p.adno into vadd_xx,vadno
    from (select xx, add_xx, adno, bit_zapory, mak_xx from spacer_pub where xx=vpub_xx union 
          select -1*xx, add_xx, adno, 0, mak_xx from spacer_pub_que where xx=-1*vpub_xx) p, 
          spacer_add a, spacer_users u, spacer_role r
    where p.add_xx=a.xx
      and r.nazwa='kierownik'
      and u.xx=uid
      and (exists (select 1 from makieta m where m.xx=p.mak_xx and m.kiedy>sysdate) or u.gru_xx>40)
      and ((a.sprzedal=uid and exists (select 1 from makieta m where m.xx=p.mak_xx and m.data_zm>sysdate) and not exists (select 1 from spacer_pub where xx=vpub_xx and adno>0)) 
          or bitand(u.gru_xx,r.xx)=r.xx);

    if vpub_xx > 0 then
      spacer_set_space(vpub_xx,sr.wolny);
      notify_pub_delete(vpub_xx);
      delete_spacer_object(0,1,vpub_xx); --stuby tez
      powtorki(vadd_xx);
    else
      delete from spacer_pub_que where xx=(-1)*vpub_xx;
    end if;
    
    delete from spacer_add where xx=vadd_xx and 
        not exists (select 1 from spacer_pub where add_xx=vadd_xx)
    and not exists (select 1 from spacer_pub_que where add_xx=vadd_xx);

    commit;
  
  exception
     when no_data_found then
        raise_application_error(-20106,'Nie masz prawa usunac emisji ogloszenia');
  end del_pub;

/******************* DEL_PUB2 ********************/
  procedure del_pub2 (
    vadd_xx in number,
    vmak_xx in number
  ) is 
  begin
    -- usunac moze kierownik, master lub dealer, ktory sprzedal
    for cur in (
      select p.xx pub_xx, p.adno
        from (select xx, add_xx, mak_xx, adno, bit_zapory 
                from spacer_pub where mak_xx=vmak_xx
        union select -1*xx, add_xx, mak_xx, adno, 0 
                from spacer_pub_que where mak_xx=vmak_xx) p, 
             spacer_add a, spacer_users u, spacer_role r
       where p.add_xx=vadd_xx
         and p.add_xx=a.xx
         and r.nazwa='kierownik'
         and u.xx=uid
         and (exists (select 1 from makieta m where m.xx=p.mak_xx and m.kiedy>sysdate) or u.gru_xx>40)
         and ((a.sprzedal=uid and exists (select 1 from makieta m where m.xx=p.mak_xx and m.data_zm>sysdate) and not exists (select 1 from spacer_pub where mak_xx=vmak_xx and add_xx=vadd_xx and adno>0)) 
             or bitand(u.gru_xx,r.xx)=r.xx)
    ) loop
      if cur.pub_xx > 0 then
        spacer_set_space(cur.pub_xx,sr.wolny);
        notify_pub_delete(cur.pub_xx);
        delete_spacer_object(0,1,cur.pub_xx); --stuby tez
        powtorki(vadd_xx);
      else
        delete from spacer_pub_que where xx=(-1)*cur.pub_xx;
      end if;
    end loop;
    
    delete from spacer_add where xx=vadd_xx and 
       not exists (select 1 from spacer_pub where add_xx=vadd_xx)
   and not exists (select 1 from spacer_pub_que where add_xx=vadd_xx);
    
    commit;
  
  exception
     when no_data_found then
        raise_application_error(-20106,'Nie masz prawa usunac emisji ogloszenia');
  end del_pub2;

/******************* DEL_ALL ********************/
  procedure del_all (
    vadd_xx in number
  ) is 
    vx number;
  begin
    -- usunac moze kierownik, master lub dealer, ktory sprzedal
    select 1 into vx
      from spacer_add a, spacer_users u, spacer_role r
     where r.nazwa='kierownik'
       and u.xx=uid
       and a.xx=vadd_xx
       and ((a.sprzedal=uid and not exists (select 1 from spacer_pub where add_xx=vadd_xx and adno>0)) or bitand(u.gru_xx,r.xx)=r.xx);

    for vpub in (
      select p.xx from spacer_pub p, makieta m 
       where p.add_xx=vadd_xx and p.mak_xx=m.xx 
         and m.data_zm>sysdate and p.bit_zapory=0
    ) loop
      spacer_set_space(vpub.xx,sr.wolny);
      notify_pub_delete(vpub.xx);
      delete_spacer_object(0,1,vpub.xx); --stuby tez
    end loop;
    delete from spacer_pub_que 
     where add_xx=vadd_xx and exists (select 1 from makieta where xx=spacer_pub_que.mak_xx and data_zm>sysdate);
    delete from spacer_add 
     where xx=vadd_xx and not exists (select 1 from spacer_pub where add_xx=vadd_xx)
       and not exists (select 1 from spacer_pub_que where add_xx=vadd_xx);
    commit;
  
  exception
     when no_data_found then
        raise_application_error(-20106,'Rezerwacja '||vadd_xx||' nie istnieje lub nie masz prawa jej usunac');     
  end del_all;
  
/******************* NOTIFY_PUB_DELETE ********************/
  procedure notify_pub_delete (
    vpub_xx in number
  ) as 
    vholder spacer_users.xx%type;
    vmsg varchar2(256);
  begin
    select nvl(system_lock,uid) into vholder
      from makieta m, spacer_pub p
     where p.xx=vpub_xx and m.xx=p.mak_xx;
    if vholder<>uid then
      select 'uzytkownik '||user||' usunal ogloszenie z numerem spacera '||p.add_xx||' ze strony '||abs(s.nr)||'. Usun recznie to ogloszenie lub odswiez makiete bez zapisywania zmian.'
        into vmsg from spacer_pub p, spacer_strona s
       where p.xx=vpub_xx and p.mak_xx=s.mak_xx and p.str_xx=s.str_xx;
      manam_msg.send(vholder,vmsg);
    end if;
  end notify_pub_delete;

/******************* LIST EMISJE ********************/
  procedure list_emisje (
    vmak_xx in number,
    vkrok in number,
    vkon in char,
    vretCur out sr.refCur
  ) is
    vkiedy date;
    vkoniec date;
    vkrokdays number;
    vemisje spacer_emisje := spacer_emisje();  
    vemisja spacer_emisja := spacer_emisja(0,'');
  begin
    select xx, kiedy, kiedy, to_date(vkon, sr.vfShortDate), decode(vkrok,0,1,1,2,2,3,3,7,4,14,5,28) 
      into vemisja.xx, vemisja.kiedy, vkiedy, vkoniec, vkrokdays
      from makieta where xx=vmak_xx;
    
    vemisje.extend;
    vemisje(vemisje.last) := vemisja;
    vkiedy := vkiedy + vkrokdays;
    
    while vkiedy <= vkoniec loop      
      begin
        select m.xx, m.kiedy into vemisja.xx, vemisja.kiedy
        from makieta m, makieta m2
        where m.drw_xx=m2.drw_xx and m.kiedy=vkiedy and m2.xx=vmak_xx;

        vemisje.extend;
        vemisje(vemisje.last) := vemisja;

      exception
        when no_data_found then
          null;
      end;
      vkiedy := vkiedy + vkrokdays;
    end loop;
       
    open vretCur for
      select t.xx, to_char(t.kiedy, sr.vfShortDate)
        from table(cast(vemisje as spacer_emisje)) t;
  
  end list_emisje;  
  
/******************** MUTACJE_MAKIETY *********************/
  procedure mutacje_makiety (
    vmak_xx in number,
    vretCur out sr.refCur
  ) as begin 
    open vretCur for
      select m.xx,d.mutacja from makieta m, makieta m2, drzewo d, drzewo d2
       where m2.xx=vmak_xx and m2.drw_xx=d2.xx
         and m.kiedy=m2.kiedy and m.drw_xx=d.xx
         and d.tytul=d2.tytul and d.mutacja<>d2.mutacja
       order by 2;
  end;
  
/********************** DATA_EMISJI **********************/
  procedure data_emisji (vmak_xx in number,vkiedy in varchar2,vxx out makieta.xx%type)
  as begin
      select m.xx into vxx
        from makieta m, makieta m2
       where m.drw_xx=m2.drw_xx and m2.xx=vmak_xx and m.kiedy=to_date(vkiedy,sr.vfShortDate);
  exception
      when no_data_found then
          raise_application_error(-20001, 'Ten tytul nie zostanie wydany w dniu: '||vkiedy);
  end data_emisji;

/********************** ZAPORA **********************/
  procedure zapora (vpub_xxArr sr.numlist)
  as begin
      for vi in 1..vpub_xxArr.last loop
          update spacer_pub set bit_zapory=1, czas_obow=null
           where xx=vpub_xxArr(vi);
          spacer_set_space(vpub_xxArr(vi),sr.ogloszenie);
      end loop;
      commit;
  end zapora;
  
/*********************** POWTORKI *************************/
  procedure powtorki (
    vadd_xx in spacer_add.xx%type
  ) as begin
    update spacer_pub p
       set (p.powtorka,p.old_adno)=
           (select decode(p.wersja,null,t.powt,p.powtorka),
                   decode(t.old_eps,p.adno,null,decode(p.wersja,null,t.old_eps,p.old_adno))
              from (select p2.xx,
           decode(m.drw_xx,lag(m.drw_xx) over (order by m.drw_xx,m.kiedy),lag(m.kiedy) over (order by m.drw_xx,m.kiedy) - sr.powtseed,null) powt,
           decode(m.drw_xx,lag(m.drw_xx) over (order by m.drw_xx,m.kiedy),lag(p2.adno) over (order by m.drw_xx,m.kiedy),null) old_eps
      from spacer_pub p2, makieta m
     where p2.mak_xx=m.xx and p2.add_xx=vadd_xx) t where t.xx=p.xx)
    where p.add_xx=vadd_xx
      and exists (select 1 from makieta m where m.xx=p.mak_xx and m.kiedy>sysdate);
  end powtorki;
  
/*********************** ATEX *************************/
  procedure atex (
    vadno in number,
    vxx in number,
    vzaporuj in pls_integer default 0,
    vnowe_mat in pls_integer default 0,
    vuwagi in varchar2 default null
  ) as 
    vdummy sr.refCur;
    vpowt spacer_pub.powtorka%type;
  begin
    if vxx > 0 then
      for cur in (select xx,mak_xx,str_xx,x,y,sizex,sizey from spacer_pub p
           where p.add_xx is null and p.adno=vadno and p.mak_xx in (
          select p2.mak_xx from spacer_pub p2, makieta m 
           where p2.xx=vxx and p2.mak_xx=m.xx and m.kiedy>=trunc(sysdate)))
      loop
        update spacer_pubstub set xx=vxx where xx=cur.xx;
        delete from spacer_pub where xx=cur.xx;
        if cur.x > 0 then
          spacer_set_space2(cur.mak_xx,cur.str_xx,cur.x,cur.y,cur.sizex,cur.sizey,sr.wolny);
        end if;
      end loop;
      update spacer_pub set adno=vadno,wersja=decode(vnowe_mat,0,wersja,decode(wersja,null,'.n',wersja||'n')),sekcja=null,op_sekcji=null,nr_w_sekcji=null,p_l=null,op_pl=null,nr_pl=null,poz_na_str=null,
             uwagi=uwagi||'$'||sekcja||' '||op_sekcji||' '||nr_w_sekcji||' '||p_l||' '||op_pl||' '||nr_pl||' '||poz_na_str,uwagi_atex=nvl(vuwagi,uwagi_atex),
             powtorka=decode(vnowe_mat,0,powtorka,null)
       where xx=vxx
         and exists (select 1 from makieta where xx=spacer_pub.mak_xx and kiedy>=trunc(sysdate))
   returning powtorka into vpowt;
      if vpowt is null then
          epstest.get_prod_info(vxx,vdummy); -- inicjalizacja cid_xx
      end if;
      if vzaporuj > 0 then
            update spacer_pub set bit_zapory=1, czas_obow=null
             where xx=vxx;
            spacer_set_space(vxx,sr.ogloszenie);
      end if;
    else
      update spacer_pub_que set adno=vadno,wersja=decode(vnowe_mat,0,wersja,decode(wersja,null,'.n',wersja||'n')) where xx=-vxx;
    end if;
  end atex;

/****************** CHANGE_BASE_KRAT *******************/
  procedure change_base_krat (
    vmak_xx in spacer_strona.mak_xx%type,
    vstr_xx in spacer_strona.str_xx%type,
    voldkra_xx in spacer_kratka.xx%type,
    vnewkra_xx in spacer_kratka.xx%type    
  ) as 
    vcc pls_integer;vi pls_integer;vj pls_integer;
    vile_mod spacer_kratka.ile_mod%type;
  begin
    /*select count(1) into vcc from spacer_kratka where xx=voldkra_xx and sym in ('5x36','7x36');
    if vcc>0 then
      raise_application_error(-20001,'CHANGE_BASE_KRAT: Spacer nie pozwala na zmiane tej kraty ('||voldkra_xx||')');
    end if;*/
    
    begin
      select ile_mod into vile_mod from spacer_kratka where xx=vnewkra_xx;
    exception
      when no_data_found then
         raise_application_error(-20002,'CHANGE_BASE_KRAT: Brak kraty '||vnewkra_xx);
    end;
    -- przenies krate bazowa do spacer_str_krat
    begin
      insert into spacer_str_krat(mak_xx,str_xx,kra_xx,lp,moduly)
         select mak_xx,str_xx,kra_xx,0,moduly
           from spacer_strona 
          where mak_xx=vmak_xx and str_xx=vstr_xx and kra_xx=voldkra_xx
            and translate(moduly,'X0','X') is not null;
    exception
      when dup_val_on_index then
         update spacer_str_krat 
            set moduly=(select moduly from spacer_strona where mak_xx=vmak_xx and str_xx=vstr_xx and kra_xx=voldkra_xx)
          where mak_xx=vmak_xx and str_xx=vstr_xx and kra_xx=voldkra_xx;
    end;
    if sql%rowcount > 0 then
    insert into spacer_str_krat(mak_xx,str_xx,kra_xx,lp,moduly)
         select mak_xx,str_xx,voldkra_xx,lp,moduly
           from spacer_pow 
          where mak_xx=vmak_xx and str_xx=vstr_xx;
    else
         select count(1) into vcc from spacer_strona where mak_xx=vmak_xx and str_xx=vstr_xx and kra_xx=voldkra_xx;
         if vcc = 0 then
            rollback;
            raise_application_error(-20002,'CHANGE_BASE_KRAT: Krata bazowa jest inna niz zadano ('||voldkra_xx||')');
         end if;
    end if;
    -- ustalenie bazowej
    select count(1) into vcc from spacer_str_krat where mak_xx=vmak_xx and str_xx=vstr_xx and kra_xx=vnewkra_xx;
    delete from spacer_pow where mak_xx=vmak_xx and str_xx=vstr_xx;
    if vcc = 0 then
      update spacer_strona
         set moduly=lpad('0',vile_mod*sr.modLen,'0'), kra_xx=vnewkra_xx
       where mak_xx=vmak_xx and str_xx=vstr_xx;         
      -- uzupelnij spacer_pow
      if vile_mod > sr.maxMod then
         vi := 1;
         while vi*sr.maxMod < vile_mod loop
            vj := vile_mod-sr.maxMod*vi;
            if vj > sr.maxMod then
              vj := sr.maxMod;
            end if;
            insert into spacer_pow (mak_xx,str_xx,lp,moduly) 
                 values (vmak_xx,vstr_xx,vi,lpad('0',vj*sr.modLen,'0'));
            vi := vi + 1;
          end loop;
      end if;
    else -- przepisz 
      update spacer_strona
         set moduly=(select moduly from spacer_str_krat 
                     where mak_xx=vmak_xx and str_xx=vstr_xx and kra_xx=vnewkra_xx and lp=0), 
             kra_xx=vnewkra_xx
       where mak_xx=vmak_xx and str_xx=vstr_xx;         
      -- uzupelnij spacer_pow       
      insert into spacer_pow (mak_xx,str_xx,lp,moduly) 
      select mak_xx,str_xx,lp,moduly
        from spacer_str_krat
       where mak_xx=vmak_xx and str_xx=vstr_xx and kra_xx=vnewkra_xx and lp>0;
      -- usun z niebazowych
      delete from spacer_str_krat where mak_xx=vmak_xx and str_xx=vstr_xx and kra_xx=vnewkra_xx;
    end if;
  end change_base_krat;

/******************* CHECK_SPACE ********************/
  procedure check_space (
    vmak_xx in number,
    vstr_xx in number,
    vexactplace boolean,
    vsizex in sr.srtinyint,
    vsizey in sr.srtinyint,
    vposx in out sr.srtinyint,
    vposy in out sr.srtinyint
  ) is
    vok boolean := true;
    vx sr.srtinyint; 
    vy sr.srtinyint; 
    vdx sr.srtinyint; 
    vdy sr.srtinyint; 
    vdrx sr.srtinyint; 
    vdry sr.srtinyint; 
  begin
    vx := vposx - 1;
    vy := vposy - 1;
    vdx := vx + vsizex - 1;
    vdy := vy + vsizey - 1;
    for i in vx..vdx loop
        for j in vy..vdy loop
          exit when not vok;
          vok := select_modul_status(vmak_xx,vstr_xx,i+vvszpalt_x*j,vvmakdate) = sr.wolny;
        end loop;
    end loop;

    if vok then
      return;
    end if;

    if vexactplace then
      vposx := null;
      return;
    end if;
    
    vdrx := vvszpalt_x - vsizex + 1;
    vdry := vvszpalt_y - vsizey + 1;
    for rx in 1..vdrx loop
      for ry in 1..vdry loop
        vok := true;
        vx := rx - 1;
        vy := ry - 1;
        vdx := vx + vsizex - 1;
        vdy := vy + vsizey - 1;
        for i in vx..vdx loop
            for j in vy..vdy loop
              exit when not vok;
              vok := select_modul_status(vmak_xx,vstr_xx,i+vvszpalt_x*j,vvmakdate) = sr.wolny;
            end loop;
        end loop;
        if vok then
          vposx := rx;
          vposy := ry;
          return;
        end if;
      end loop;
    end loop;

    vposx := null;
  end check_space;

/********************* UPADATE_RESERVATION **********************/
  procedure update_reservation (
    vpub_xx in spacer_pub.xx%type,
    vile_kol in spacer_pub.ile_kol%type,
    vspo_xx in spacer_pub.spo_xx%type,
    vpowtorka in spacer_pub.powtorka%type,
    vold_adno in spacer_pub.old_adno%type,
    vuwagi in spacer_pub.uwagi%type,
    vwersja in spacer_pub.wersja%type default null,
    vczaskto in varchar2 default null,
    vaccept_flag in number default 2
  )as 
    vcc number;
    vdczas date;
    vczas number;
    vmsg varchar2(256);
    vmak_xx makieta.xx%type;
    vdrw_xx makieta.drw_xx%type;
    vholder spacer_users.xx%type;
    vdata_zm makieta.data_zm%type;
  begin
    /*select count(1) into vcc
      from spacer_pub p, spacer_add a
     where p.xx=vpub_xx and p.add_xx=a.xx and a.sprzedal=uid;
    if vcc = 0 then
      select count(1) into vcc
        from spacer_pub p, spacer_add a, spacer_users k, spacer_users d
       where p.xx=vpub_xx and p.add_xx=a.xx and a.sprzedal=d.xx
         and k.xx=uid and decode(k.lok_xx,2,11,k.lok_xx)=d.lok_xx and bitand(k.gru_xx,sr.kierownik)>0;
      if vcc = 0 then
        raise_application_error(-20001,'Mozesz modyfikowac tylko rezerwacje swoje lub sprzedawcow z twojego oddzialu. Jedyna dopuszczalna operacja na cudzych rezerwacjach jest powiazanie z numerem ATEX');
      end if;
    end if;*/

    select drw_xx,mak_xx,data_zm into vdrw_xx,vmak_xx,vdata_zm 
      from spacer_pub p, makieta m where p.xx=vpub_xx and m.xx=p.mak_xx;
    if substr(vczaskto,14,1) = ':' then
      vdczas := to_date(substr(vczaskto,1,16),sr.vfLongDate);
      if vdczas<=sysdate+1/sr.timeUnit then
        raise_application_error(-20098,'Proponowany czas obowiazywania jest zbyt wczesny');
      elsif vdata_zm<vdczas then
        raise_application_error(-20099,'Proponowany czas obowiazywania jest pozniejszy od daty zamkniecia makiety');
      end if;    
      vczas := pubdate(vmak_xx, least(vdata_zm,vdczas));
    else
      vczas := null;
    end if;
    
    select count(1) into vcc from spacer_users u
     where u.xx=uid and u.gru_xx in (1,5,8,12,44);
    if vcc=0 then
      raise_application_error(-20100,'Tylko sprzedawcy i kierownicy makiet z prawem do spacerowania maja prawo do korzystania z tej funkcji');
    end if;

    if not check_access(vdrw_xx,'S') then
      raise_application_error(-20101,'Brak prawa do modyfikowania rezerwacji w tym produkcie');
    end if;
    
    select count(1) into vcc
      from spacer_pub p2
     where p2.xx=vpub_xx and vpowtorka>0 and vwersja is null
       and not exists (
         select 1 from makieta m,makieta m2,spacer_pub p
          where p.add_xx=p2.add_xx and m.xx=p.mak_xx and m2.xx=p2.mak_xx and m.kiedy<m2.kiedy);
    if vcc>0 then
      raise_application_error(-20102,'Pierwsza emisja rezerwacji nie mo¿e byæ powtórk¹. Dodaj emisje do istniej¹cego spaceru.');
    end if;
      
    update cid_info set accepted=vaccept_flag
     where xx in (select cid_xx from spacer_pub where xx=vpub_xx);
  
    update spacer_pub
       set ile_kol=vile_kol,spo_xx=decode(vspo_xx,0,null,vspo_xx),uwagi=vuwagi,wersja=vwersja,
        	 powtorka=decode(vpowtorka,0,null,vpowtorka),old_adno=decode(vold_adno,-1,null,vold_adno),
        	 czas_obow=decode(czas_obow,null,null,nvl(vczas,czas_obow)),eps_present=decode(vaccept_flag,2,null,vaccept_flag)
     where xx=vpub_xx and exists (select 1 from makieta where xx=spacer_pub.mak_xx and data_zm>sysdate);
    if sql%rowcount=0 then
      raise_application_error(-20103,'Nie mozna modyfikowac rezerwacji w makiecie po dacie zamkniecia');
    end if;
    
    select nvl(m.system_lock,uid), abs(s.nr) into vholder, vcc
      from makieta m, spacer_pub p, spacer_strona s
     where p.xx=vpub_xx and m.xx=p.mak_xx and p.str_xx=s.str_xx and p.mak_xx=s.mak_xx;
    if vholder<>uid then
      select 'SYS1^'||p.xx||'^'||p.ile_kol||'^'||vspo_xx||'^'||nvl(p.uwagi,' ')||'^'||nvl(p.wersja,' ')||'^'||vpowtorka||'^'||vold_adno||'^SYS1Uzytkownik '||user||' ('||u.tel||') zmodyfikowal rezerwacje.'||chr(10)||'Numer spacera '||p.add_xx||' ze strony '||abs(vcc)||' '||m.tytul||' na '||m.skiedy
        into vmsg from spacer_pub p, spacer_strona s, spacer_users u, mak m
       where p.xx=vpub_xx and p.mak_xx=s.mak_xx and p.str_xx=s.str_xx and m.mak_xx=p.mak_xx and u.xx=uid;
      manam_msg.send(vholder,vmsg);
    end if;

  exception
    when no_data_found then
      raise_application_error(-20104,'Rezerwacja zostala usunieta. Odswiez makiete');
  end update_reservation;
  
/********************* QUE **********************/
  procedure que (
    vmak_xx in number,
    vszpalt_x in number,
    vszpalt_y in number,
    vsizex in sr.srtinyint, 
    vsizey in sr.srtinyint,
    vnazwa in varchar2, --5
    vwersja in varchar2,              
    vuwagi in varchar2,                
    vile_kolorow in number, 
    vspo_xx in number,
    vtyp_xx in number, --10 
    vadd_xx in out number,
    vpub_xx out number --12
  ) is
    vcc number;
    vtyp number;
  begin 
if vmak_xx < 0 then
  raise_application_error(-20000,'Blad kolejkowania rezerwacji. Makieta jest uszkodzona zadzwon, prosze po dnumer 56158');
end if;
    -- kolejkowanie tego, co stoi juz na makiecie
    select count(xx) into vcc from spacer_pub where mak_xx=vmak_xx and add_xx=vadd_xx and add_xx is not null;
    if vcc > 0 then
       vpub_xx := -1;
       return;
    end if;
    
    -- czy numer spacera jest prawidlowy
    select count(xx) into vcc from spacer_add where xx=nvl(vadd_xx,-1);
    if vcc = 0.0 and vadd_xx>0 then
        raise_application_error(-20104,'Proba dopisania nowej emisji do nie istniejacego ogloszenia '||vadd_xx);
    end if;

    select min(xx) into vcc
      from makieta where xx=vmak_xx and data_zm<sysdate;
    if vcc is not null then
       raise_application_error(-20105,'Zakonczono juz sprzedaz do tej makiety'); 
    end if;

    if nvl(vadd_xx,0)=0 then -- narysowane z boku makiety
      insert into spacer_add (query_flag) values (0) 
        returning xx into vadd_xx;
    end if;
  
    if vtyp_xx = 0.0 then -- ogloszenie wymiarowe
      select typ_ogloszenia.xx into vtyp
        from typ_ogloszenia, spacer_kratka
       where kratowe=1
         and typ_ogloszenia.sym=vsizex||'x'||vsizey
         and kra_xx=spacer_kratka.xx
         and szpalt_x=vszpalt_x
         and szpalt_y=vszpalt_y;
    else
      vtyp := vtyp_xx;
    end if;
    
    insert into spacer_pub_que (mak_xx,add_xx,typ_xx,sizex,sizey,nazwa,wersja,uwagi,ile_kol,spo_xx)
         values (vmak_xx,vadd_xx,vtyp,vsizex,vsizey,vnazwa,vwersja,vuwagi,vile_kolorow,decode(vspo_xx,0,null,vspo_xx))
      returning xx into vpub_xx;
      
    commit;
  end que;
  
/****************** SAVE_QUERY *******************/
  procedure save_query (
    vpub_xx in number,
    vquery_flag in sr.srtinyint
  ) as begin
    if bitand(vquery_flag,req_exactplace)>0 then
      update spacer_pub p
        set (sekcja,op_sekcji,nr_w_sekcji,x_na_str,y_na_str)=
            (select 'ALL','=',s.nr_porz,p.x,p.y from spacer_strona s where s.mak_xx=p.mak_xx and s.str_xx=p.str_xx) 
       where p.xx=vpub_xx;
      return;
    end if;
    
    if bitand(vquery_flag,req_sekcja)>0 then
      update spacer_pub p
        set sekcja=(select s.str_log from spacer_strona s where s.mak_xx=p.mak_xx and s.str_xx=p.str_xx)
       where p.xx=vpub_xx;
    end if;
    if bitand(vquery_flag,req_wsekcji)>0 then
      update spacer_pub p
        set op_sekcji='=',nr_w_sekcji=vvwsekcji
       where p.xx=vpub_xx;
    end if;
    if bitand(vquery_flag,req_pageparity)>0 then
      update spacer_pub p
        set p_l=decode(vvparity,1,'P','L')
       where p.xx=vpub_xx;
    end if;
    if bitand(vquery_flag,req_pagelayout)>0 then
      update spacer_pub p
        set poz_na_str='XY',x_na_str=x,y_na_str=y
       where p.xx=vpub_xx;
    end if;
  end save_query;
  
/******************* RESERVE ********************/
  procedure reserve (
    vmak_xx in number,
    vstr_xx in number,
    vszpalt_x in number,
    vszpalt_y in number,
    vposx in sr.srtinyint,
    vposy in sr.srtinyint, --5
    vsizex in sr.srtinyint,
    vsizey in sr.srtinyint,
    vquery_flag in sr.srtinyint,
    vblokada in number,
    vnazwa in varchar2, --10
    vwersja in varchar2,              
    vuwagi in varchar2,                
    vile_kolorow in number,
    vspo_xx in number,
    vtyp_xx in number, --15
    vadd_xx in out number,
    vpub_xx in out number,
    vczaskto out varchar2
  ) is
    vtyp number;
    vx sr.srtinyint;
    vy sr.srtinyint;
    vdx sr.srtinyint;
    vdy sr.srtinyint;
    vid_mod sr.srint;
    voccnt sr.srsmallint := 0;
    vpubdate sr.srint;
    voglkra_xx number;
    vstrkra_xx number;
    vadno number := null;
  begin
    select drw_xx into vtyp from makieta where xx=vmak_xx;
    if not check_access(vtyp,'S') then
      raise_application_error(-20105,'Brak prawa do dokonywania rezerwacji w tym produkcie');
    end if;
    
    -- nowa emisja istniejacego spacera
    if nvl(vadd_xx,0) > 0 then 
        select min(sprzedal), (select gru_xx from spacer_users where xx=uid) gru_xx into vx, vy
          from spacer_add a where a.xx=nvl(vadd_xx,-1) group by 2;
        if vx is null then
          raise_application_error(-20106,'Proba dopisania nowej emisji do nie istniejacej rezerwacji '||vadd_xx);
        end if;
        if vx<>uid and bitand(vy,sr.kierownik)<>sr.kierownik then
          raise_application_error(-20107,'Proba dopisania nowej emisji do cudzej rezerwacji '||vadd_xx);
        end if;
        select count(1) into vx from spacer_pub
         where add_xx=vadd_xx 
           and (sizex<>vsizex or sizey<>vsizey 
             or ile_kol<>vile_kolorow or nvl(vspo_xx,-1)<>nvl(vspo_xx,-1));
        if vx > 0.0 then
          raise_application_error(-20108,'Proba dopisania emisji nie zgodnej z istniejacym ogloszeniem '||vadd_xx);
        end if;
    end if;
    
    if vtyp_xx = 0.0 then -- og1oszenie wymiarowe
      select typ_ogloszenia.xx into vtyp
        from typ_ogloszenia, spacer_kratka
       where kratowe=1
         and typ_ogloszenia.sym=vsizex||'x'||vsizey
         and kra_xx=spacer_kratka.xx
         and szpalt_x=vszpalt_x
         and szpalt_y=vszpalt_y;
    else
      vtyp := vtyp_xx;
    end if;

    select count(1) into vx
      from spacer_pub where add_xx=vadd_xx and typ_xx<>vtyp;
    if vx > 0 then
      raise_application_error(-20109,'Proba dopisania nowego ogloszenia o typie nie zgodnym z ogloszeniami z rezerwacji '||vadd_xx);
    end if;
  
    select k.xx, s.kra_xx, s.ile_kol 
      into voglkra_xx, vstrkra_xx, vx
      from spacer_kratka k, spacer_strona s
     where k.szpalt_x=vszpalt_x and k.szpalt_y=vszpalt_y
       and s.mak_xx=vmak_xx and s.str_xx=vstr_xx;
    if vx < vile_kolorow then
      raise_application_error(-20110,'Kolorystyka ogloszenia i strony nie sa zgodne');
    end if;

    select count(1) into vx from typ_ogloszenia t 
     where t.xx=vtyp and t.unikalne_na_stronie=1
       and exists (select 1 from spacer_pub p where p.mak_xx=vmak_xx and p.str_xx=vstr_xx and p.typ_xx=t.xx);
    if vx > 0 then
      raise_application_error(-20111,'Na stronie moze stac tylko jedno ogloszenie wybranego typu');
    end if;

    check_mak_owned(vmak_xx);

    vvmakdate := makdate(vmak_xx);
    vx := vposx - 1;
    vy := vposy - 1;
    vdx := vx + vsizex - 1;
    vdy := vy + vsizey - 1;

    -- sekcja krytyczna
    lock table spacer_token in exclusive mode;

    -- czy na pewno wolne
    if vstrkra_xx <> voglkra_xx then
        if check_base_krat(vposx,vposy,vsizex,vsizey,voglkra_xx,vmak_xx,vstr_xx)<>sr.wolny then
            raise_application_error(-20112,'Status powierzchni w kracie bazowej nie pozwala na wykonanie rezerwacji');
        end if;
        change_base_krat(vmak_xx,vstr_xx,vstrkra_xx,voglkra_xx);
    end if;

    for vvx in vx..vdx loop
      for vvy in vy..vdy loop
        voccnt := voccnt + select_modul_status(vmak_xx,vstr_xx,vvx+vszpalt_x*vvy,vvmakdate);
      end loop;
    end loop;

    if voccnt > 0 then
      rollback;
      raise_application_error(-20113,'Rezerwacja nie moze byc zrealizowana. Miejsce jest juz zajete.');
    end if;

    select floor(sr.timeUnit*(kiedy-data_zm)),
           to_char(data_zm,sr.vfLongDate)||' ['||user||']'
      into vpubdate, vczaskto
      from makieta where xx=vmak_xx;

    if nvl(vadd_xx,0) = 0 then
      insert into spacer_add (query_flag) values (vquery_flag)
        returning xx into vadd_xx;
    end if;

    if vpub_xx<-1 then --przenoszenie z kolejki
      select adno into vadno from spacer_pub_que where xx=-vpub_xx;
      
      if vadno is not null then
         vpubdate := null;
         vczaskto := '#z';
      end if;
    end if;
    
    insert into spacer_pub (mak_xx,add_xx,str_xx,typ_xx,adno,x,y,blokada,bit_zapory,sizex,sizey,nazwa,wersja,uwagi,ile_kol,spo_xx,czas_obow)
         values (vmak_xx,vadd_xx,vstr_xx,vtyp,vadno,vposx,vposy,vblokada,0,vsizex,vsizey,vnazwa,vwersja,vuwagi,vile_kolorow,decode(vspo_xx,0,null,vspo_xx),vpubdate)
      returning xx into vpub_xx;

    -- rezerwuj powierzchnie
    spacer_set_space(vpub_xx,sr.ogloszenie,vpubdate);
    
    if vadno>0 then
      powtorki(vadd_xx);
    end if;
    
    if is_pub_covered(vpub_xx)>0 then
      rollback;
      raise_application_error(-20114,'Rezerwacja nie moze byc zrealizowana. Miejsce zajmuje ogloszenie z innej kraty. '||vpub_xx);
    end if;
    
    if vadno is null then
      save_query(vpub_xx,vquery_flag);
    end if;
    
    cancel_deal(vmak_xx,vstr_xx); -- koniec sekcji krytycznej
   exception
     when others then
        rollback; -- koniec sekcji krytycznej
        if sqlcode<-20999 or -20000<sqlcode then
           raise_application_error(-20115, sqlerrm);
        else
           raise_application_error(-20115, substr(sqlerrm,11));
        end if;
   end reserve;

/******************* RESERVE MUL ********************/
  procedure reserve_mul (
    vmak_xxArr in sr.numlist,
    vstr_xx in number,
    vszpalt_x in number,
    vszpalt_y in number, 
    vposx in sr.srtinyint,
    vposy in sr.srtinyint, --5
    vsizex in sr.srtinyint,
    vsizey in sr.srtinyint,
    vquery_flag in sr.srtinyint,
    vblokada in number,
    vnazwa in varchar2, --10 
    vwersja in varchar2,              
    vuwagi in varchar2,                
    vile_kolorow in number,
    vspo_xx in number,
    vtyp_xx in number, --15
    vadd_xx in out number,
    vpub_xx out number,
    vczaskto out varchar2, --sekcja
    vmsgArr out msglist
  ) is
    vxx number;
    vx sr.srtinyint;
    vy sr.srtinyint;
    vi sr.srtinyint;
    vmak_cnt number;
    vadd_cnt number;
    vfc sr.srtinyint;
    strCur sr.refCur;
    voglkra_xx number;
    vstrkra_xx number;
    vstr_xxmul number;
    vmsg varchar2(1000);
    vtempStr spacer_found_mak := spacer_found_mak();
  begin
    -- czy numer spacera jest prawidlowy
    select count(xx) into vmak_cnt from spacer_add where xx=nvl(vadd_xx,0);
    if vmak_cnt = 0.0 and nvl(vadd_xx,0) > 0 then
        raise_application_error(-20000,'Proba dopisania nowej emisji do nie istniejacego ogloszenia '||vadd_xx);
    end if;
    
    vmak_cnt := vmak_xxArr.last;
    if vmak_cnt > 320 then
       raise_application_error(-20001,'System nie realizuje zlecen wiekszych niz 320 emisji. Zazadano rezerwacji '||vmak_cnt||' emisji');
    end if;
    
    -- odczytaj dane strony z pierwsza emisja na zmienne pakietowe
    begin
      if nvl(vadd_xx,0) = 0 then
        select nr_porz, sciezka, bitand(nr_porz,1)
          into vxx, vvsciezka, vvparity
          from spacer_strona 
         where mak_xx=vmak_xxArr(1) and str_xx=vstr_xx;
      else
        select nr_porz, sciezka, bitand(nr_porz,1)
          into vxx, vvsciezka, vvparity
          from spacer_strona s, spacer_pub p, makieta m 
         where p.add_xx=vadd_xx and s.mak_xx=p.mak_xx and s.str_xx=p.str_xx and m.xx=s.mak_xx
           and rownum=1 and not exists (select 1 from makieta m2, spacer_pub p2 
               where m2.xx=p2.mak_xx and p2.add_xx=p.add_xx and m2.kiedy<m.kiedy);
      end if;
    exception
      when no_data_found then
        raise_application_error(-20002,'Strona zostala usunieta');
    end;
    select count(1) into vvwsekcji
      from spacer_strona 
     where mak_xx=vmak_xxArr(1) and nr_porz<=vxx 
       and instr(sciezka,vvsciezka)=1;
    if nvl(vadd_xx,0) = 0 then 
      vvposx := vposx; 
      vvposy := vposy; 
      vvfirstrun := true; 
    else
      vvfirstrun := false; 
    end if;

    -- dokladnie w zadanym miejscu
    if bitand(vquery_flag,req_exactplace)>0 then
      for i in reverse 1..vmak_cnt loop 
    begin
        vpub_xx := 0;
        -- ustal str_xx dla testowanej strony
        if vmak_cnt > 1.0 then
          select s2.str_xx into vstr_xxmul 
            from spacer_strona s, spacer_strona s2
          where s2.mak_xx=vmak_xxArr(i)
            and s2.nr_porz=s.nr_porz
            and nvl(s2.dervlvl,-1) not in (sr.derv_fixd,sr.derv_proh)
            and s.mak_xx=vmak_xxArr(1)
            and s.str_xx=vstr_xx;
        else
          -- kolejne wywolanie
          select s2.str_xx into vstr_xxmul 
            from spacer_strona s, spacer_strona s2, makieta m, spacer_pub p
          where s2.mak_xx=vmak_xxArr(i)
            and s2.nr_porz=s.nr_porz
            and nvl(s2.dervlvl,-1) not in (sr.derv_fixd,sr.derv_proh)
            and s.mak_xx=m.xx
            and s.str_xx=vstr_xx
            and p.mak_xx=m.xx
            and p.add_xx=vadd_xx
            and not exists (select 1 from makieta m2, spacer_pub p2
                             where p2.mak_xx=m2.xx and p2.add_xx=vadd_xx 
                               and m2.drw_xx=m.drw_xx and m2.kiedy<m.kiedy);
        end if;
        -- ustal ilosc ogloszen ktore juz stoja
        select count(xx) into vadd_cnt
          from spacer_pub 
         where mak_xx=vmak_xxArr(i)
           and str_xx=vstr_xxmul and x>0
           and (flaga_rezerw=1 or nvl(czas_obow,0)<=makdate(vmak_xxArr(i)));
        -- zablokuj
        begin_deal(vmak_xxArr(i),vstr_xxmul,vadd_cnt);
        -- rezerwuj
        reserve(vmak_xxArr(i),vstr_xxmul,vszpalt_x,vszpalt_y,
                vposx,vposy,vsizex,vsizey,vquery_flag,vblokada,
                vnazwa,vwersja,vuwagi,vile_kolorow,vspo_xx,
                vtyp_xx,vadd_xx,vpub_xx,vczaskto);
        vmsgArr(i) := '';
    exception
      when no_data_found then
        vmsgArr(i) := 'brak strony';
      when others then
        vadd_cnt := sqlcode;
        vmsg := sqlerrm;
        select decode(vadd_cnt,
                      -20104,'brak miejsca',
                      vmsg)
          into vmsgArr(i) from dual;
    end;
      end loop;
      return;
    end if; --exactplace

    vvszpalt_x := vszpalt_x;
    vvszpalt_y := vszpalt_y;
    for i in reverse 1..vmak_cnt loop 
      vpub_xx := 0;
      vvmakdate := makdate(vmak_xxArr(i));
      vtempStr.delete;
      -- dla kazdej spelniajacej warunki strony w makiecie
      open strCur for
       'select s1.str_xx,'||
             ' (select count(1) from spacer_pub where mak_xx=s1.mak_xx and str_xx=s1.str_xx and x>0 and (flaga_rezerw=1 or nvl(czas_obow,0)<=makdate(mak_xx))) cc'||
        ' from spacer_strona s1'||
       ' where nvl(s1.dervlvl,-1) not in (3,4)'||
         ' and s1.mak_xx=:1'||
         ' and s1.ile_kol>=:2'||
         ' and (bitand('||vquery_flag||','||req_pageparity||')=0 or bitand(s1.nr_porz,1)='||vvparity||')'||
         ' and (bitand('||vquery_flag||','||req_sekcja||')=0 or instr(s1.sciezka,:3)=1)'||
         ' and (bitand('||vquery_flag||','||req_wsekcji||')=0'||
              ' or (select count(1) from spacer_strona s2'|| 
                   ' where s2.mak_xx=s1.mak_xx'||
                     ' and s2.nr_porz<=s1.nr_porz'||
                     ' and instr(s2.sciezka,:4)=1)=:5)'
         using in vmak_xxArr(i), in vile_kolorow, in vvsciezka, in vvsciezka, in vvwsekcji;
      fetch strCur into vxx, vadd_cnt;
      if strCur%notfound then
        vmsgArr(i) := 'nie odnaleziono odpowiedniej strony';
      end if;
      while strCur%found loop
      begin
        -- zablokuj
        begin_deal(vmak_xxArr(i),vxx,vadd_cnt);
        -- zgodnosc krat
        select k.xx,s.kra_xx into voglkra_xx, vstrkra_xx
          from spacer_kratka k, spacer_strona s
         where k.szpalt_x=vszpalt_x and k.szpalt_y=vszpalt_y
           and s.mak_xx=vmak_xxArr(i) and s.str_xx=vxx;
        if vstrkra_xx <> voglkra_xx then
            if vvfirstrun and i = 1 and vxx = vstr_xx then -- tam, gdzie narysowano
              change_base_krat(vmak_xxArr(1),vxx,vstrkra_xx,voglkra_xx);
            elsif bitand(vquery_flag,9) = 0 then -- w sekcji lub exact
              cancel_deal(vmak_xxArr(i),vxx);
              raise_application_error(-20005,'Bazowa krata strony nie jest zgodna z ogloszeniem. Wybierz warunek "polozenie w sekcji" lub "w tym samym miejscu".');
            else
              if check_base_krat(vposx,vposy,vsizex,vsizey,voglkra_xx,vmak_xxArr(i),vxx)<>sr.wolny then
                raise_application_error(-20006,'Status powierzchni w kracie bazowej nie pozwala na wykonanie rezerwacji');
              end if;
              change_base_krat(vmak_xxArr(i),vxx,vstrkra_xx,voglkra_xx);
            end if;
        end if;
        
        -- ile jest wolnych modulow
        select count(1) into vfc from pivot dx,pivot dy
         where dx.x<vszpalt_x and dy.x<vszpalt_y 
           and select_modul_status(vmak_xxArr(i),vxx,dx.x+vszpalt_x*dy.x,vvmakdate) = sr.wolny;
        
        if vfc < vsizex*vsizey then
          vmsgArr(i) := 'brak miejsca';
          vx := null;
        else -- zbadaj strone
          -- sprawdz czy jest miejsce
          vx := vvposx;
          vy := vvposy;
          check_space(vmak_xxArr(i),vxx,bitand(vquery_flag,req_pagelayout)>0,vsizex,vsizey,vx,vy);
        end if;
        
        if vx is null then
          vmsgArr(i) := 'brak miejsca';
          cancel_deal(vmak_xxArr(i),vxx);
        else
          if vvfirstrun and i = 1 and vxx = vstr_xx then -- postaw gdzie narysowano
            vfc := -1;
          end if;
          vtempStr.extend();
          vtempStr(vtempStr.last) := spacer_found_str(vxx,vfc,vx,vy);
          vmsgArr(i) := '';
        end if;
      exception
        when others then
          vadd_cnt := sqlcode;
          vmsg := sqlerrm;
          select decode(vadd_cnt,
                        -20104,'brak miejsca',
                        vmsg)
            into vmsgArr(i) from dual;      
      end;
        fetch strCur into vxx, vadd_cnt;
      end loop;
      close strCur;
      -- ustal na ktora rezerwujemy
      if vtempStr.count > 0 then
        select t.str_xx,t.x,t.y
          into vxx,vx,vy
          from table(cast(vtempStr as spacer_found_mak)) t
         where t.fc=(select min(t2.fc) from table(cast(vtempStr as spacer_found_mak)) t2)
           and rownum=1;
        -- rezerwuj
        begin
        reserve(vmak_xxArr(i),vxx,vszpalt_x,vszpalt_y,
                vx,vy,vsizex,vsizey,vquery_flag,vblokada,
                vnazwa,vwersja,vuwagi,vile_kolorow,vspo_xx,
                vtyp_xx,vadd_xx,vpub_xx,vczaskto);
        vmsgArr(i) := '';
        exception
          when others then
            vadd_cnt := sqlcode;
            vmsg := sqlerrm;
            select decode(vadd_cnt,
                          -20104,'brak miejsca',
                          vmsg)
              into vmsgArr(i) from dual;      
        end;
      elsif vmsgArr(i) is null then
        vmsgArr(i) := 'zadane miejsce niedostepne';
      end if;
      -- odblokuj strony
      update spacer_strona s
         set s.system_lock=null,
             s.lock_deadline=null
       where s.mak_xx=vmak_xxArr(i)
         and exists (select t.str_xx 
                       from table(cast(vtempStr as spacer_found_mak)) t 
                      where t.str_xx=s.str_xx);
    end loop;
    
    commit;
  end reserve_mul;
end spacer;