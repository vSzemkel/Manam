create or replace PROCEDURE "INSERT_STRONA" (
    vmak_xx in number,
    vstr_xx in out number,
    vszpalt_x in number,
    vszpalt_y in number,
    vnr_porz in number,
    vnr in number,
    vtyp_num in number,
    vile_kolorow in number,
    vnr_spotu in number,
    vsciezka in varchar2,
    vstr_log in varchar2,
    vO in raw, -- ignore
    vB in raw,
    vR in raw,
    vdrukarnie in number,
    vdeadline in char,
    vmutred in varchar2,
    vpap_xx in number,
    vis_rozk in number,
    vwyd_xx in number default null
) as
    vstrlog varchar2(5 char);
    vi sr.srint;
    vj sr.srint;
    vk sr.srint;
    vpadding sr.srint;
    vwyd wydawca.xx%type;
    vile_mod sr.srint;
    vile_unitow sr.srint;
    vlockUnit number(10);
    vredUnit number(10);
begin
    if vdrukarnie<0 or vpap_xx<1 then
     select count(1) into vi from grzbiet
      where (acc_print_org is not null or acc_print_cdrz is not null or drukarnie<1)
        and xx in (
        select grb_xx from makingrb where mak_xx=vmak_xx
         union all select grb_okl_xx from makingrb where mak_xx=vmak_xx and grb_okl_xx is not null
         union all select g.xx from grzbiet g, split_grzbietu s, drzewo d, makieta m
                   where m.xx=vmak_xx and m.kiedy=g.kiedy and m.drw_xx=d.xx
                     and d.tytul=s.okladka and s.skladka1=g.tytul and d.mutacja=g.mutgrb);
     if vi>0 then
       raise_application_error(-20001,'Strona '||vnr_porz||' wstawiana do zaakceptowanego przez ORG lub CDRZ grzbietu musi mie? przypisany papier i drukarni?');
     end if;
    end if;

    select nvl(max(str_xx)+1.0,vnr_porz) into vstr_xx
      from spacer_strona where mak_xx=vmak_xx and str_xx>=vnr_porz;

    if vsciezka is null then
      vstrlog := null;
    else
      vstrlog := substr(vstr_log,1,5);
    end if;

    if vwyd_xx <= 0 then
       select d.wyd_xx into vwyd from drzewo d,makieta m where m.xx=vmak_xx and d.xx=m.drw_xx;
    else
       vwyd := vwyd_xx;
    end if;
    
    vile_mod := vszpalt_x*vszpalt_y;
    vile_unitow := ceil((vile_mod-0.5)/sr.modUnit);
    vpadding := 1 + length(vB) - sr.modUnitB * vile_unitow; -- jeden bajt to dwa znaki w RAW, jeden unit to 8 znakow
    if vile_mod > sr.maxMod then
      vile_mod := sr.maxMod;
    end if;

    insert into spacer_strona (mak_xx,str_xx,nr_porz,nr,kra_xx,num_xx,ile_kol,spot,sciezka,str_log,naglowek,moduly,drukarnie,deadline,mutred,pap_xx,rozkladowka,wyd_xx)
        select vmak_xx,vstr_xx,vnr_porz,vnr,xx,vtyp_num,vile_kolorow,decode(vnr_spotu,0,null,vnr_spotu),vsciezka,vstrlog,vstr_log,lpad('0',vile_mod*sr.modLen,'0'),vdrukarnie,case when to_date(vdeadline,sr.vfLongDate)<=sysdate+1/24 then null else to_date(vdeadline,sr.vfLongDate) end,vmutred,decode(vpap_xx,0,(select d.pap_xx from drzewo d,makieta m where m.xx=vmak_xx and d.xx=m.drw_xx),vpap_xx),decode(vis_rozk,0,null,1),vwyd
        from spacer_kratka where szpalt_x=vszpalt_x and szpalt_y=vszpalt_y;

    -- geste kraty
    vile_mod := vszpalt_x*vszpalt_y;
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

    vi := 0;
    while vi < vile_unitow loop
      vj := vpadding + (vile_unitow-vi-1)*sr.modUnitB;
      vlockUnit := to_number(substr(vB,vj,sr.modUnitB),'XXXXXXXX');
      vredUnit := to_number(substr(vR,vj,sr.modUnitB),'XXXXXXXX');
      vk := vile_mod-vi*sr.modUnit;
      if vk > sr.modUnit then
        vk := sr.modUnit;
      end if;

      vj := 0;
      while vj < vk loop
        if mod(vlockUnit,2.0) = 1.0 then -- 'OB'
          vlockUnit := vlockUnit - 1.0;
          update_modul(vmak_xx,vstr_xx,vile_mod-sr.modUnit*vi-vj-1,sr.zablokowany);
        elsif mod(vredUnit,2.0) = 1.0 then -- 'R'
          vredUnit := vredUnit - 1.0;
          update_modul(vmak_xx,vstr_xx,vile_mod-sr.modUnit*vi-vj-1,sr.redakcyjny);
        end if;
        vlockUnit := vlockUnit / 2.0;
        vredUnit := vredUnit / 2.0;
        vj := vj + 1;
      end loop;
      vi := vi + 1;
    end loop;
END insert_strona;