create or replace PACKAGE BODY                     "SPACER_ADM" as
  pragma serially_reusable;
/******************** AFTER_LOAD_DMP ******************/
  procedure after_load_dmp
  as begin 

    execute immediate 'alter session set constraints=deferred';
    execute immediate 'alter table spacer_uprawnienia drop constraint uidx_oso_drw cascade';

    -- dodaj nowych userow do bazy
    for currow in (
      select loginname,nazwa as grupa,imie,nazwisko,tel,opis
        from spacer_users, spacer_role 
       where not exists (select 1 from all_users where username=upper(loginname))
         and gru_xx=spacer_role.xx
         and loginname<>'sa'
    ) loop
      add_spacer_user(currow.loginname,null,currow.loginname||'123',currow.grupa,currow.imie,currow.nazwisko,currow.tel,currow.opis);
    end loop;
  
    -- przepisz klucze z all_users
    for currow in ( 
      select xx, user_id 
        from spacer_users, all_users
       where upper(loginname)=username
       order by 2 desc
    ) loop
      update makieta set id_kierownik=currow.user_id where id_kierownik=currow.xx;
      update makieta_lib set id_kierownik=currow.user_id where id_kierownik=currow.xx;
      update spacer_add set sprzedal=currow.user_id where sprzedal=currow.xx;
      update spacer_uprawnienia set oso_xx=currow.user_id where oso_xx=currow.xx;
    end loop;
  
    update makieta set system_lock=null where system_lock is not null;
    update makieta_lib set system_lock=null where system_lock is not null;
    update spacer_strona set system_lock=null where system_lock is not null;

    update spacer_users set xx=-xx;
    update spacer_users
       set xx=(select user_id from all_users where username=upper(loginname));
  
    execute immediate 'alter session set constraints=immediate';
    execute immediate 'alter table spacer_uprawnienia add constraint uidx_oso_drw unique (OSO_XX,DRW_XX) using index tablespace indx';

    commit;
  end after_load_dmp;
  
/******************** RECREATE_USERS *******************/
  procedure recreate_users (
    vseed in number default 0
  ) as -- zamienic spac_users na spac_users@spac92
    vcc number ;  
  begin  
    select count(1) into vcc  
      from (select max(user_id) xx from all_users) e,  
           (select min(xx) xx from spac_users where xx>vseed) a  
     where e.xx<a.xx;  
    if vcc=0 then  
      raise_application_error(-20001,'Konflikt kluczy');  
    end if;  
  
    for cur in  
    (select xx,loginname,password from spac_users where xx>vseed order by xx) loop  
<<jeszcze_raz>>    
      execute immediate 'create user '||cur.loginname||' identified by values '''||cur.password||''' default tablespace users temporary tablespace temp';
      select user_id into vcc from all_users where username=cur.loginname;
      if vcc<cur.xx then
        execute immediate 'drop user '||cur.loginname;
        goto jeszcze_raz;
      end if;      
      execute immediate 'grant connect,unlimited tablespace to '||cur.loginname;
    end loop;
  end recreate_users;
  
/******************** RECREATE_SPACER_GRANTS ******************/
  procedure recreate_spacer_grants
  as 
    vrole varchar2(32);
  begin
    for cur in (select gru_xx,loginname from spacer_users) loop
      select db_role into vrole from spacer_role where xx=cur.gru_xx;
      execute immediate 'grant '||vrole||' to '||cur.loginname;
    end loop;
  end recreate_spacer_grants;

  procedure update_spacer_user (
    vimie in spacer_users.imie%type,  
    vnazwisko in spacer_users.nazwisko%type,  
    vtel in spacer_users.tel%type,  
    vlok_xx in spacer_users.lok_xx%type,
    vloginname in spacer_users.loginname%type,
    vloginname_ds in spacer_users.loginname_ds%type,
    vpass in varchar2,  
    vemail in varchar2,  
    vuwagi in varchar2,
    vwydaw_role_xx in spacer_users.wydaw_role_xx%type,
    vgru_xx in spacer_users.gru_xx%type,
    vrejkod_role_xx in spacer_users.rejkod_role_xx%type,
    vvinsert_role_xx in spacer_users.vinsert_role_xx%type    
  ) as 
    vcNowy constant pls_integer := -100;
    vcc pls_integer;
    vmanam_acc_org spacer_users.gru_xx%type;
  begin
    select nvl(min(nvl(gru_xx,-1)),vcNowy) into vmanam_acc_org from spacer_users where loginname=vloginname;
    if vmanam_acc_org >= 16 then
      raise_application_error(-20001,'Brak uprawnien do modyfikacji tego uzytkownika');
    elsif vmanam_acc_org = vcNowy then -- nowe konto
      execute immediate 'create user '||vloginname||' identified by '||vpass||' default tablespace users temporary tablespace temp' ;
    elsif vpass is not null then
      execute immediate 'alter user '||vloginname||' identified by '||vpass;
      spacer_sms.send_sms(user||' zresetowal haslo dla konta '||vloginname);
    end if;

    if bitand(vgru_xx,1)>0 then
      execute immediate 'grant dealer to '||vloginname;
    else
      select count(1) into vcc from dba_role_privs where granted_role='DEALER' and grantee=upper(vloginname);
      if vcc>0 then
        execute immediate 'revoke dealer from '||vloginname;
      end if;
    end if;
    if bitand(vgru_xx,2)>0 then
      execute immediate 'grant redaktor to '||vloginname;
    else
      select count(1) into vcc from dba_role_privs where granted_role='REDAKTOR' and grantee=upper(vloginname);
      if vcc>0 then
        execute immediate 'revoke redaktor from '||vloginname;
      end if;
    end if;
    if bitand(vgru_xx,4)>0 then
      execute immediate 'grant studio to '||vloginname;
    else
      select count(1) into vcc from dba_role_privs where granted_role='STUDIO' and grantee=upper(vloginname);
      if vcc>0 then
        execute immediate 'revoke studio from '||vloginname;
      end if;
    end if;
    if bitand(vgru_xx,8)>0 then
      execute immediate 'grant kierownik to '||vloginname;
    else
      select count(1) into vcc from dba_role_privs where granted_role='KIEROWNIK' and grantee=upper(vloginname);
      if vcc>0 then
        execute immediate 'revoke kierownik from '||vloginname;
      end if;
    end if;
    
    if vmanam_acc_org = vcNowy then
      insert into spacer_users (loginname,loginname_ds,tel,imie,nazwisko,lok_xx,opis,wydaw_role_xx,gru_xx,rejkod_role_xx,vinsert_role_xx) 
      values (lower(vloginname),vloginname_ds,vtel,initcap(vimie),vnazwisko,vlok_xx,vuwagi,decode(vwydaw_role_xx,-1,null,vwydaw_role_xx),decode(vgru_xx,-1,null,vgru_xx),decode(vrejkod_role_xx,-1,null,vrejkod_role_xx),decode(vvinsert_role_xx,-1,null,vvinsert_role_xx));
    else
      update spacer_users
         set imie=vimie,nazwisko=vnazwisko,tel=vtel,loginname_ds=vloginname_ds,opis=vuwagi,lok_xx=vlok_xx,
             wydaw_role_xx=decode(vwydaw_role_xx,-1,null,vwydaw_role_xx),gru_xx=decode(vgru_xx,-1,null,vgru_xx),email=vemail,
             rejkod_role_xx=decode(vrejkod_role_xx,-2,null,vrejkod_role_xx),vinsert_role_xx=decode(vvinsert_role_xx,-1,null,vvinsert_role_xx)
       where loginname=vloginname;
    end if;
                  
    if vvinsert_role_xx > 0 then
      insert into vinsert.vinsert_usr (usr_xx,acccnt,lastlogintime)
      select xx,0,to_date('01/01/2000',sr.vfShortDate) from spacer_users u
       where loginname=vloginname and not exists (select 1 from vinsert.vinsert_usr where usr_xx=u.xx);
      execute immediate 'grant vinsert_user to '||vloginname;
    else
      select count(1) into vcc from dba_role_privs where granted_role='VINSERT_USER' and grantee=upper(vloginname);
      if vcc>0 then
        execute immediate 'revoke vinsert_user from '||vloginname;
      end if;
    end if;

    if vrejkod_role_xx > 0 then
      execute immediate 'grant rejkod_role to '||vloginname;
    else
      select count(1) into vcc from dba_role_privs where granted_role='REJKOD_ROLE' and grantee=upper(vloginname);
      if vcc>0 then
        execute immediate 'revoke rejkod_role from '||vloginname;
      end if;
    end if;
    
    if uid<>515 and uid<>30 then
      spacer_sms.send_sms(user||' wywolal UPDATE_SPACER_USER dla konta '||vloginname||' '||vimie||' '||vnazwisko||' '||vtel||' uprawnienia (K,M,R,V)=('||vwydaw_role_xx||','||vgru_xx||','||vrejkod_role_xx||','||vvinsert_role_xx||')');
    end if;
  end update_spacer_user;
  
/******************** ADD_SPACER_USER ******************/
  procedure add_spacer_user (
    vloginname in varchar2,
    vlogin_nds in varchar2,
    vpass in varchar2,  
    vgrupa in varchar2, 
    vimie in varchar2,  
    vnazwisko in varchar2,  
    vtel in varchar2,  
    vuwagi in varchar2
  ) as begin
    -- konto w bazie
    execute immediate 'create user '||vloginname||' identified by '||vpass||' default tablespace users temporary tablespace temp' ;

    -- uprawnienia w bazie
    if vgrupa<>'dealer' then
      execute immediate 'grant unlimited tablespace to '||vloginname;
    end if;

    -- przypisz role
    if vgrupa = 'admin' then
      execute immediate 'grant itspec to '||vloginname;
    elsif vgrupa = 'studio' then
      execute immediate 'grant studio to '||vloginname;
    elsif vgrupa = 'redaktor' then
      execute immediate 'grant redaktor to '||vloginname;
    elsif vgrupa = 'ogladacz' then
      null;
    elsif vgrupa = 'kierownik' then
      execute immediate 'grant kierownik to '||vloginname;
    elsif vgrupa = 'dealer' then
      execute immediate 'grant dealer to '||vloginname;
    elsif vgrupa = 'public' then
      execute immediate 'grant srmaster to '||vloginname;
    end if;
  
    -- rejestracja w spacer_users
    insert into spacer_users (loginname,loginname_ds,tel,imie,nazwisko,gru_xx,opis) 
    select lower(vloginname),vlogin_nds,vtel,initcap(vimie),vnazwisko,xx,vuwagi
    from spacer_role where nazwa=vgrupa;
    
    if sql%rowcount = 0 then
      raise_application_error(-20000,'Nie zarejestrowano roli: '||vgrupa);
    end if;
  
    -- lokalizacja
    update spacer_users set lok_xx=(
      select lok_xx from (select lok_xx,count(1) cnt from spacer_users
        where tel like substr(vtel,1,2)||'%' group by lok_xx order by cnt desc) where rownum=1)
     where xx=(select user_id from all_users where username=UPPER(vloginname));
     
    if uid<>515 and uid<>30 then
      spacer_sms.send_sms(user||' wywolal ADD_SPACER_USER tworzac konto '||vloginname);
    end if;

    commit;
  exception
    when dup_val_on_index then
      null;  
  end add_spacer_user;

/*********************** ANALYZE_ALL ******************/
  procedure analyze_all
  is
    vuowner    VARCHAR2(30);
  begin
  -- (c) Marcin Buchwald  1997
  -- ANALYZE_ALL analizuje
  -- obiekty swego wlasciciela

    -- select owner name
    select owner into vuowner
      from all_objects
     where object_type='PACKAGE' 
       and object_name='SPACER_ADM';
     
    -- clusters
    for currow in ( 
      select object_name
        from all_objects
       where object_type='CLUSTER' 
         and owner=vuowner
    ) loop
      execute immediate 'analyze cluster '||currow.object_name||' validate structure cascade';
      execute immediate 'analyze cluster '||currow.object_name||' compute statistics';
    end loop;
  
    -- nonclustered tables 
    for currow in ( 
      select table_name from all_tables
       where cluster_name is null
         and owner=vuowner
    ) loop
      execute immediate 'analyze table '||currow.table_name||' validate structure cascade';
      execute immediate 'analyze table '||currow.table_name||' compute statistics';
    end loop ;
  end analyze_all;
  
/******************** DEFINE_ROZM_KRATY ***************/
  procedure define_rozm_kraty (
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type,
    vszpalt_x in spacer_kratka.szpalt_x%type,
    vszpalt_y in spacer_kratka.szpalt_x%type,
    vpagex in pls_integer default 2500, 
    vpagey in pls_integer default 3623,
    vspacx in pls_integer default 40,
    vspacy in pls_integer default 35
  ) as 
    vkra_xx spacer_kratka.xx%type;
    vdrw_xx drzewo.xx%type;
  begin 
  begin
    select xx into vdrw_xx 
    from drzewo where tytul=vtytul and mutacja=vmutacja;
  exception
    when no_data_found then
        raise_application_error(-20000,'Nie zarejestrowano tytulu '||vtytul||' '||vmutacja);
  end;
  begin
    if vszpalt_x=0 then --ustalenie dla wszystkich istniejacych krat
      for c in (select szpalt_x,szpalt_y from spacer_kratka k where exists (select 1 from spacer_rozm_kraty r where r.drw_xx=vdrw_xx and r.kra_xx=k.xx)) loop
        define_rozm_kraty(vtytul,vmutacja,c.szpalt_x,c.szpalt_y,vpagex,vpagey,vspacx,vspacy);
      end loop;
      return;
    end if;
    select xx into vkra_xx 
    from spacer_kratka where szpalt_x=vszpalt_x and szpalt_y=vszpalt_y;
  exception
    when no_data_found then
        raise_application_error(-20000,'Nie zarejestrowano kratki '||vszpalt_x||'x'||vszpalt_y);
  end;
  begin
    insert into spacer_rozm_kraty (drw_xx,kra_xx,width,height,swidth,sheight)
    values (vdrw_xx,vkra_xx,round((vpagex - (vszpalt_x - 1)*vspacx)/vszpalt_x,2),round((vpagey - (vszpalt_y - 1)*vspacy)/vszpalt_y,2),vspacx,vspacy);
  exception
    when dup_val_on_index then
        update spacer_rozm_kraty 
           set width=round((vpagex - (vszpalt_x - 1)*vspacx)/vszpalt_x,2),
               height=round((vpagey - (vszpalt_y - 1)*vspacy)/vszpalt_y,2),
               swidth=vspacx,sheight=vspacy,
               kto_mod=uid,kiedy_mod=current_timestamp
        where drw_xx=vdrw_xx and kra_xx=vkra_xx;
  end;
    commit;
  end define_rozm_kraty;

/******************** DEFINE_ROZM_NOSTD ***************/
  procedure define_rozm_nostd (
    vmodelid in typ_ogloszenia.modelid%type,
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type,
    vsizex in spacer_pub.sizex%type,
    vsizey in spacer_pub.sizey%type,
    vsizex_mm in pls_integer,
    vsizey_mm in pls_integer,
    vspacx in pls_integer default 40,
    vspacy in pls_integer default 35
  ) as 
    vtyp_xx typ_ogloszenia.xx%type;
    vdrw_xx drzewo.xx%type;
  begin 
  begin
    select d.xx,t.xx into vdrw_xx,vtyp_xx
      from drzewo d,typ_ogloszenia t
     where d.tytul=vtytul and d.mutacja=vmutacja and t.modelid=vmodelid;
  exception
    when no_data_found then
        raise_application_error(-20000,'Nie zarejestrowano tytulu '||vtytul||' '||vmutacja||' lub typu ogloszenia '||vmodelid);
  end;
  begin
    insert into spacer_rozm_nostd (drw_xx,typ_xx,width,height,swidth,sheight,scaleit)
    values (vdrw_xx,vtyp_xx,round((vsizex_mm+vspacx)/vsizex-vspacx,1),round((vsizey_mm+vspacy)/vsizey-vspacy,1),vspacx,vspacy,0);
  exception
    when dup_val_on_index then
        update spacer_rozm_nostd 
           set width=round((vsizex_mm+vspacx)/vsizex-vspacx,1),
               height=round((vsizey_mm+vspacy)/vsizey-vspacy,1),
               swidth=vspacx,sheight=vspacy
        where drw_xx=vdrw_xx and typ_xx=vtyp_xx;
  end;
    commit;
  end define_rozm_nostd;

/********************* DEFINE_MODELID *******************/
  procedure define_modelid (
    vsizex in typ_ogloszenia.sizex%type,
    vsizey in typ_ogloszenia.sizey%type,
    vsym in spacer_kratka.sym%type,
    vmodelid in typ_ogloszenia.modelid%type,
    vnazwa in typ_ogloszenia.nazwa%type default null
  ) as
    vkra_xx spacer_kratka.xx%type;
  begin
    select count(1) into vkra_xx from typ_ogloszenia where modelid=vmodelid;
    if vkra_xx>0 then
      raise_application_error(-20002,'Podany kod jest juz wykorzystywany');
    end if;
    
    select xx into vkra_xx from spacer_kratka
     where sym=vsym and vsizex between 1 and szpalt_x and vsizey between 1 and szpalt_y;

    insert into typ_ogloszenia (kra_xx,kratowe,legalne,sym,nazwa,sizex,sizey,modelid,uzyj_atexkrat)
         values (vkra_xx,0,1,vmodelid,nvl(vnazwa,vmodelid),vsizex,vsizey,vmodelid,null);
           
    insert into spacer_modelid@oraent sm 
    select vmodelid from dual where not exists (select 1 from spacer_modelid@oraent sm2 where  modelid=vmodelid);
  exception
    when no_data_found then
      raise_application_error(-20002,'Brak kraty lub zly rozmiar w szpaltach');
  end define_modelid;
  
/********************* DROP_ALL *******************/
  procedure drop_all (
    vcode in number
  ) as begin
    if vcode<>6427691 then
      raise_application_error(-20000, 'Ty podly zboju!');
    end if;

    stop_triggers();
    
    /*for us in (
      select username from all_users
      where user_id>uid
        and username<>'GWSPACER' -- repository owner
    ) loop
      execute immediate 'drop user '||us.username||' cascade';
    end loop ;*/
  
    delete from spacer_log;
    delete from spacer_publog;
    commit;
    delete from spacer_opis;
    delete from spacer_opis_lib;
    commit;
    delete from spacer_pub;
    delete from spacer_pub_que;
    commit;
    delete from spacer_str_krat;
    delete from spacer_strona;
    delete from spacer_strona_lib;
    commit;
    delete from spot_makiety;
    delete from spot_makiety_lib;
    delete from makieta;
    delete from makieta_lib;
    commit;
  
    --delete from spacer_uprawnienia;
    --delete from drzewo;
    --delete from spacer_users;
    commit;
  
    -- reset sequences
    execute immediate 'drop sequence makieta_lib_xx';
    execute immediate 'drop sequence makieta_xx';
    execute immediate 'drop sequence spacer_add_xx';
    execute immediate 'drop sequence spacer_opis_lib_xx';
    execute immediate 'drop sequence spacer_opis_xx';
    execute immediate 'drop sequence spacer_pub_que_xx';
    execute immediate 'drop sequence spacer_pub_xx';
    --execute immediate 'drop sequence spacer_uprawnienia_xx';

    execute immediate 'create sequence makieta_lib_xx';
    execute immediate 'create sequence makieta_xx';
    execute immediate 'create sequence spacer_add_xx';
    execute immediate 'create sequence spacer_opis_lib_xx';
    execute immediate 'create sequence spacer_opis_xx';
    execute immediate 'create sequence spacer_pub_que_xx';
    execute immediate 'create sequence spacer_pub_xx';
    --execute immediate 'create sequence spacer_uprawnienia_xx';

    start_triggers();
end;

/******************** INIT_TYP_OGLOSZENIA *************/
  procedure init_typ_ogloszenia (
  	vszpalt_x in number,
  	vszpalt_y in number
  ) as 
    vkra_xx number;
  begin
  	select xx into vkra_xx
     	from spacer_kratka 
  	 where szpalt_x=vszpalt_x and szpalt_y=vszpalt_y;

  	for i in 1..vszpalt_x loop
  		for j in 1..vszpalt_y loop
  			insert into typ_ogloszenia (kra_xx, kratowe, legalne, sym, nazwa)
             values (vkra_xx, 1, 1, i||'x'||j, i||'x'||j||' na '||vszpalt_x||'x'||vszpalt_y);
  		end loop;
  	end loop;
  exception
    when no_data_found then
      raise_application_error(-20001,'Nie zarejestrowano jeszcze kraty '||vszpalt_x||'x'||vszpalt_y);
  end init_typ_ogloszenia;

/******************** NIGHT_TOUCH ******************/
  procedure night_touch
  as 
    vcc number;
    vinstance varchar2(12);
  begin
    -- invoked once a night

    -- nullowe kolumny nie zajmuja miejsca w bazie  
    update spacer_pub p set eps_present=null,eps_date=null
     where eps_present is not null
       and exists (select 1 from makieta m where m.xx=p.mak_xx and m.kiedy between sysdate-32 and sysdate-30);
    update /*+ index (s) */ spacer_pubstub s set eps_date=null
     where eps_date is not null and xx in (select p.xx from makieta m,spacer_pub p where m.xx=p.mak_xx and m.kiedy between sysdate-32 and sysdate-30);

    -- stare logi nie sa istotne
    delete from spacer_log where kiedy<sysdate-90;
    delete from spacer_publog where kiedy<sysdate-90;
    delete from spacer_pubstublog where kiedy<sysdate-90;
    delete from blacha where not exists (select 1 from grzbiet where xx=grb_xx and kiedy>sysdate-14);
    delete from emisja_zajawki where data_atex<sysdate-90;
    delete from eps_present_log where kiedy<sysdate-10;
    delete from cid_present where kiedy<sysdate-90;
    delete from zajawka_log where kiedy_mod<sysdate-90;
    
    -- wygas stare produkty
    update drzewo set do_kiedy=sysdate where od_kiedy<sysdate-365 and do_kiedy>sysdate 
       and not exists (select 1 from makieta where kiedy>sysdate-365 and drw_xx=drzewo.xx);
       
    -- odblokuj zwieszonych
    update makieta set system_lock=null where system_lock is not null;
    update makieta_lib set system_lock=null where system_lock is not null;
    update spacer_strona set system_lock=null, lock_deadline=null where system_lock is not null;
   
    -- bledy rekompilacji
    delete from ei_errors where sqlerrmsg like '%ORA-04068%';
    commit;

    -- aktualizacja statystyk
    dbms_stats.gather_schema_stats (ownname => 'SPACE_RESERVATION', cascade => true, estimate_percent => dbms_stats.auto_sample_size);
    -- dbms_stats.gather_schema_stats ignoruje indexy IOT - TOP
    for c in (select index_name from user_indexes where last_analyzed<sysdate-1/24) loop
       execute immediate 'analyze index '||c.index_name||' compute statistics';
    end loop;
    -- compute statistics dla CID_INFO
    dbms_stats.gather_table_stats(ownname => 'SPACE_RESERVATION', tabname => 'CID_INFO', partname => NULL);
    
    select count(1) into vcc from overlappub;
    if vcc>0 then
      select instance_name into vinstance from v$instance;
      spacer_sms.send_sms('Wykryto '||vcc||' bledy OVERLAPPUB',spacer_sms.recipients(1),3);
      for cu in (select tytul||' na '||to_char(kiedy,sr.vfShortDate)||' str. '||nr_porz||'. Nakladaja sie ogloszenia: '||xx||' ('||vinstance||')' text from overlappub) loop
         spacer_sms.send_sms(cu.text,'velvet@agora.pl',3);
         spacer_sms.send_sms(cu.text,'tup-staff@agora.pl',3);
         spacer_sms.send_sms(cu.text,'makietowanieodd@agora.pl',3);
      end loop;
    end if;
    select count(1) into vcc from makobjerr;
    if vcc>0 then
      spacer_sms.send_sms('Wykryto '||vcc||' bledy MAKOBJERR',spacer_sms.recipients(1),3);
    end if;
    select count(1) into vcc from duppub;
    if vcc>0 then
      spacer_sms.send_sms('Wykryto '||vcc||' bledy DUPPUB',spacer_sms.recipients(1),3);
    end if;
    select count(1) into vcc from grbmakobjerr where substr(tytul,1,1) not in ('U','Y');
    if vcc>0 then
      spacer_sms.send_sms('Wykryto '||vcc||' bledy GRBMAKOBJERR',spacer_sms.recipients(1),3);
    end if;
  end night_touch;

/******************** RECOMPILE_ALL *******************/
  procedure recompile_all
    (result out number)
  is
    vuowner    VARCHAR2(30);
  begin
  -- (c) Marcin Buchwald  1997
  -- RECOMPILE_ALL probuje rekompilowac
  -- obiekty swego wlasciciela, ktore sa invalid

  -- procedure nalezy wywolywac w petli
  -- declare 
  --	 res number;
  --	 oldres number;
  -- begin
  --   res := 0; oldres := 99999999;
  --   while res<oldres loop
  --     oldres := res;
  --     recompile_all(res);
  --     dbms_output.put_line('zostalo: '||res);
  --   end loop;
  -- end;

    -- select owner name
    SELECT owner INTO vuowner
      FROM all_objects
     WHERE object_type='PACKAGE' 
       AND object_name='SPACER_ADM';

    -- procedures
    for currow in (
      select object_name
        from all_objects
       where object_type='PROCEDURE' 
  	     --and object_name<>'RECOMPILE_ALL'
         and owner=vuowner
         and status='INVALID'
    ) loop
      execute immediate 'alter procedure '||currow.object_name||' compile';
    end loop;

    -- functions
    for currow in (
      select object_name
        from all_objects
       where object_type='FUNCTION' 
         and owner=vuowner
         and status='INVALID'
    ) loop
      execute immediate 'alter function '||currow.object_name||' compile';
    end loop;

    -- triggers
    for currow in (
      select object_name
        from all_objects
       where object_type='TRIGGER' 
         and owner=vuowner
         and status='INVALID'
    ) loop
      execute immediate 'alter trigger '||currow.object_name||' compile';
    end loop;
  
    -- packages
    for currow in (
      select object_name
        from all_objects
       where object_type in ('PACKAGE','PACKAGE BODY')
         and owner=vuowner
         and status='INVALID'
    ) loop
      execute immediate 'alter package '||currow.object_name||' compile';
      execute immediate 'alter package '||currow.object_name||' compile body';
    end loop;
  
    -- views
    for currow in (
      select object_name
        from all_objects
       where object_type='VIEW' 
         and owner=vuowner
         and status='INVALID'
    ) loop
      execute immediate 'alter view '||currow.object_name||' compile';
    end loop;
 
    -- statistics
    select count(1) into result
      from all_objects
     where object_type in ('FUNCTION', 'PACKAGE', 'PACKAGE BODY', 'PROCEDURE', 'TRIGGER', 'VIEW')
       and owner=vuowner
       and object_name<>'RECOMPILE_ALL'
       and status='INVALID';
  end recompile_all;

/******************** REGISTER_TITLE ******************/
  procedure register_title (
    vtytul in varchar2,
    vmutacja in varchar2,
    vopis in varchar2,
    vdo_kiedy in varchar2,
    vtytul_upraw in varchar2,
    vmutacja_upraw in varchar2
  ) as 
    vdrw_xx drzewo.xx%type;
  begin

    insert into spacer_log (opis)
    values ('Wywolanie REGISTER_TITLE('||vtytul||','||vmutacja||','||vopis||','||vdo_kiedy||')');

    if length(vtytul)<>3 or length(vmutacja)<>2 then
       raise_application_error(-20001,'Tytul ma miec 3 znaki, mutacja 2');
    elsif length(vopis)>length(translate(vopis,' ()''"&<>',' ')) then
       raise_application_error(-20002,'W nazwie produktu nie moga wystepowac znaki specjalne: ( ) < > '' " &');
    end if;
    
    if vmutacja<>' ' then
      select min(xx) into vdrw_xx from drzewo_mutacji where mutacja=vmutacja;
      if vdrw_xx is null then
        raise_application_error(-20003,'Mutacja '||vmutacja||' nie zostala okreslona w slowniku drzewo_mutacji');        
      end if;
    end if;
    
    /*if uid<>515 and uid<>30 then
      spacer_sms.send_sms(user||' wywolal REGISTER_TITLE '||vtytul||','||vmutacja);
    end if;*/
         
    update drzewo set do_kiedy=decode(do_kiedy,sr.dozywocie,do_kiedy,decode(vdo_kiedy,null,sr.dozywocie,greatest(to_date(vdo_kiedy,sr.vfShortDate),do_kiedy)))
     where tytul=vtytul and mutacja=vmutacja returning xx into vdrw_xx;

    if sql%rowcount=0 then -- nowy tytul
      insert into drzewo (xx,tytul,mutacja,do_kiedy,opis,ppp)  
           select max(nvl(xx,0))+1,upper(vtytul),upper(vmutacja),to_date(vdo_kiedy,sr.vfShortDate),vopis,2
             from drzewo;
      select xx into vdrw_xx
        from drzewo where tytul=vtytul and mutacja=vmutacja;
      insert into spacer_uprawnienia (oso_xx,drw_xx,dostep)
           select u.xx,d.xx,'R'
             from spacer_users u, drzewo d, 
                  (select l2.xx, l2.strefa from
                    lokalizacja l2,
                   (select mutacja,level lvl from drzewo_mutacji start with mutacja=vmutacja connect by xx = prior root_xx) m2
                   where l2.strefa=m2.mutacja
                     and m2.lvl=(select max(lvl) from lokalizacja l3,
                                (select mutacja,level lvl from drzewo_mutacji start with mutacja=vmutacja connect by xx = prior root_xx) m3
                                  where l3.strefa=m3.mutacja)) l
            where u.lok_xx=l.xx 
              and u.lastlogintime>sysdate-21 and d.xx=vdrw_xx
              and not exists (select 1 from spacer_uprawnienia where oso_xx=u.xx and drw_xx=d.xx);
    elsif vopis is not null then
      rejkod.rename_drzewo(vtytul,vmutacja,vopis,vdo_kiedy);
    end if;

    if vtytul_upraw is not null then
      for a in (
        select u.loginname,a.dostep from drzewo d, spacer_uprawnienia a, spacer_users u 
         where d.tytul=vtytul_upraw and d.mutacja=vmutacja_upraw and d.xx=a.drw_xx and a.oso_xx=u.xx
      ) loop
        spacer_grant2(a.loginname,a.dostep,vdrw_xx);
      end loop;
    end if;
  
    commit;
  end register_title;

/******************** START_TRIGGERS ******************/
  procedure start_triggers
  as begin
    for t_name in (
      select trigger_name 
        from all_triggers 
       where owner=user
    ) loop
      execute immediate 'alter trigger '||t_name.trigger_name||' enable';
    end loop;
  end start_triggers;

/******************** STOP_TRIGGERS *******************/
  procedure stop_triggers
  as begin
    for t_name in (
      select trigger_name 
        from all_triggers 
       where owner=user
    ) loop
      execute immediate 'alter trigger '||t_name.trigger_name||' disable';
    end loop;
  end stop_triggers;

/******************** REGISTER_SZKOL *******************/
  procedure register_szkol (
    vcnt in pls_integer,
    vacc in varchar2,
    vrole in varchar2
  ) as begin
      for vi in 1..vcnt loop
          add_spacer_user('szkol'||vi,null,'szkol123',vrole,'Imie'||vi,'Nazwisko'||vi,'Tel'||vi,'test user');
          spacer_grant('szkol'||vi,vacc,'%','%');
      end loop;
  end register_szkol;

/********************** DROP_SZKOL *********************/
  procedure drop_szkol (
    vcnt in pls_integer
  ) as
    vxx spacer_users.xx%type;
  begin
      for vi in 1..vcnt loop
          select xx into vxx from spacer_users where loginname='szkol'||vi;
          for p in (
              select xx from spacer_pub where add_xx in (select xx from spacer_add where sprzedal=vxx)
          ) loop
              delete_spacer_object(0,1,p.xx);
          end loop;
          delete from spacer_pub_que where add_xx in (select xx from spacer_add where sprzedal=vxx);
          delete from spacer_add where sprzedal=vxx;
          for m in ( -- bardzo wolno - brak indeksu
              select xx from makieta
               where kiedy>sysdate-14 and id_kierownik=vxx
          ) loop
              delete_makieta(m.xx);
          end loop;
          delete from spacer_uprawnienia where oso_xx=vxx;
          delete from spacer_users where xx=vxx;
          execute immediate 'drop user szkol'||vi||' cascade';
      end loop;
  end drop_szkol;

/********************** SPACER_DATY *********************/
  procedure spacer_daty (
    vod_kiedy in varchar2,
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type,
    voglHH in number,
    vredHH in number,
    vdow in number default null
  ) as begin
    if vdow is null and voglHH>0 then
        update drzewo set wykup_offset=-voglHH, zamkniecie_offset=-voglHH
         where tytul=vtytul and mutacja=vmutacja;
    end if;

    update makieta set 
       data_wykupu=decode(voglHH,0,data_wykupu,kiedy-voglHH/24), 
       data_zm=decode(voglHH,0,data_zm,kiedy-voglHH/24), 
       data_do_red=decode(vredHH,0,data_do_red,kiedy-vredHH/24)
    where drw_xx in (select xx from drzewo where tytul=vtytul and mutacja like vmutacja)
      and kiedy>to_date(vod_kiedy,sr.vfShortDate) and (vdow is null or to_char(kiedy,'d')=to_char(vdow));
    dbms_output.put_line('Zmodyfikowano '||sql%rowcount||' makiet.');

    update spacer_pub set czas_obow=(select sr.timeUnit*(m.kiedy-m.data_zm) from makieta m where m.xx=mak_xx)
     where mak_xx in (select m2.xx from makieta m2 
                       where m2.drw_xx in (select xx from drzewo where tytul=vtytul and mutacja like vmutacja)
                         and m2.kiedy>to_date(vod_kiedy,sr.vfShortDate) and (vdow is null or to_char(kiedy,'d')=to_char(vdow)))
       and add_xx is not null and czas_obow is not null
       and (flaga_rezerw=1 or czas_obow<=makdate(mak_xx));
    dbms_output.put_line('Zmodyfikowano '||sql%rowcount||' ogloszen.');

  end spacer_daty;

/********************** DELETE_DRZEWO ************************/
  procedure delete_drzewo (vdrw_xx in number) as
  begin
    delete from spacer_uprawnienia where drw_xx=vdrw_xx;
    delete from spacer_rozm_kraty where drw_xx=vdrw_xx;
    delete from strukt_drzewa where drw_xx=vdrw_xx;
    delete from schemat_derv where base_drw_xx=vdrw_xx or base_drw_xx=vdrw_xx;
    delete from drzewo where xx=vdrw_xx;
  end;

/********************** VINSERT_PROMOTE ************************/
  procedure vinsert_promote (
    vloginname spacer_users.loginname%type,
    vvinsert_role number
  ) as
    vusr_xx spacer_users.xx%type;
  begin
    update spacer_users set vinsert_role_xx=vvinsert_role 
     where loginname=vloginname returning xx into vusr_xx;
    execute immediate 'grant vinsert_user to '||vloginname;
    insert into vinsert.vinsert_usr values (vusr_xx,0,'01/01/2000',null);
    commit;
  exception
    when no_data_found then
      raise_application_error(-20001,'Nie znalezion konta '||vloginname);
  end vinsert_promote;

/********************** KILL_SESSION ************************/
  procedure kill_session(
     vloginname in varchar2, 
     vclient_info in varchar2 default '%Manam%'
  ) as begin
    for c in (select sid||','||serial# s from v$session where upper(username)=upper(vloginname) and program='Manam.exe') loop
      execute immediate 'alter system kill session '''||c.s||'''';
    end loop;

    update makieta set system_lock=null
     where kiedy between sysdate and sysdate+40 and system_lock=(select user_id from dba_users where username=upper(vloginname));

    spacer_sms.send_sms(user||' wywolal KILL_SESSION dla sesji '||vloginname);
  end kill_session;
  
/********************** OBLICZ_NRED ************************/
  procedure oblicz_nred (vrok number)
  as begin
    update makieta m set m.numerrok=(select count(1) from makieta m2 where m2.drw_xx=664 and m2.kiedy>=to_date('01/01/'||vrok,'dd/mm/rrrr') and m2.kiedy<=m.kiedy)
     where m.drw_xx=664 and m.kiedy>=to_date('01/01/'||vrok,'dd/mm/rrrr') and m.kiedy<to_date('01/01/'||to_char(vrok+1),'dd/mm/rrrr');
  end oblicz_nred;

/********************** GRANT_APPEND ************************/
  procedure grant_append (
    vloginname in spacer_users.loginname%type,
    vco in char,
    vtytul in drzewo.tytul%type,
    vmutacja in drzewo.mutacja%type
  ) as 
    vdostep spacer_uprawnienia.dostep%type;
    voso_xx spacer_users.xx%type;
    vdrw_xx drzewo.xx%type;
  begin
    select min(dostep),min(oso_xx),min(drw_xx) into vdostep,voso_xx,vdrw_xx
      from spacer_users u, spacer_uprawnienia a, drzewo d
     where u.loginname=vloginname and u.xx=a.oso_xx and a.drw_xx=d.xx
       and d.tytul=vtytul and d.mutacja=vmutacja;
    if vdostep is null then
      spacer_grant(vloginname,vco,vtytul,vmutacja);
      return;
    end if;
    
    if instr(vdostep,vco)=0 then
      update spacer_uprawnienia set dostep=dostep||vco
       where oso_xx=voso_xx and drw_xx=vdrw_xx;
    end if;
  end;

/********************** PRZEPISZ_REZERWACJE ************************/
  function przepisz_rezerwacje (
    vkomu in spacer_users.loginname%type,
    vczyje in spacer_users.loginname%type,
    vspacer in spacer_add.xx%type default 0
  ) return number as 
    vile number;
  begin
    if vspacer<>0 then
       select count(1) into vile from spacer_add where xx=vspacer;
       if vile=0 then
          raise_application_error(-20000, 'W systemie nie istnieje rezerwacja o numerze: '||vspacer);
       end if;
    end if;
    
    update spacer_add ad set sprzedal=(select xx from spacer_users where loginname=vkomu)
     where (vspacer=0 or ad.xx=vspacer) and ad.sprzedal=(select xx from spacer_users where loginname=vczyje) 
       and (exists (select 1 from spacer_pub p,makieta m where p.add_xx=ad.xx and p.mak_xx=m.xx and m.kiedy>sysdate)
            or exists (select 1 from spacer_pub_que p,makieta m where p.add_xx=ad.xx and p.mak_xx=m.xx and m.kiedy>sysdate));
    vile:=sql%rowcount;   
    dbms_output.put_line('Zmodyfikowano '||vile||' rezerwacji.');
    commit;
    return vile;
  end przepisz_rezerwacje;

/********************** BLOKUJ NIEAKTYWNYCH ************************/
  procedure blokuj_nieaktywnych (
    vile_dni in number
  ) as begin
    if vile_dni < 200 then
       raise_application_error(20000, 'Przesadziles');
    end if;
    
    for cur in (select u.loginname from spacer_users u, dba_users d where upper(u.loginname)=d.username and d.account_status='OPEN' and u.lastlogintime<sysdate-vile_dni) loop
       execute immediate 'alter user '||cur.loginname||' account lock';
    end loop;
  end blokuj_nieaktywnych;
  
  /********************** PASSWORD CHANGE WARNING ************************/
  procedure pass_change_warn
  as begin
    for c in (select 'velvet@agora.pl' ln from dual union all select distinct substr(sqlerrmsg,1,instr(sqlerrmsg,':')-1) ln from ei_errors where kiedy>trunc(sysdate)-7 and sqlerrmsg like '%has_a %') loop
       spacer_sms.send_sms('Prosze o zmiane hasla w programie Manam (Makieta->Administracja->Zmiana hasla). Nowe haslo ma miec co najmniejmniej 4 znaki i nie byc oparte o login. Automat zablokuje konta uzywajace zbyt prostych hasel.',c.ln);
    end loop;
  end pass_change_warn;

end spacer_adm;