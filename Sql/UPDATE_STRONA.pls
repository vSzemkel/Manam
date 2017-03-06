create or replace PROCEDURE "UPDATE_STRONA" (
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
    vO in raw,
    vB in raw,
    vR in raw,
    vdrukarnie in number,
    vdeadline in char,
    vmutred in varchar2,
    vpap_xx in number,
    vis_rozk in number,
    vwyd_xx in number default null
) as
    vi sr.srint;
    vj sr.srint;
    vk sr.srint;
    vpadding sr.srint;
    vile_mod sr.srint;
    vile_unitow sr.srint;
    vlockUnit number(10);
    vredUnit number(10);
    voglUnit number(10);
begin
    /*if vpap_xx>0 and vdrukarnie>0 then
      select count(1) into vi from papier_ads where xx=vpap_xx
         and ((bitand(vdrukarnie,grb.cDrukAG)>0 and druk_obcy=1)
          or (bitand(vdrukarnie,grb.cDrukAG)=0 and druk_obcy is null));
      if vi > 0 then
        raise_application_error(-20001,'Rodzaj papieru ('||vpap_xx||') i wybrana drukarnia ('||vdrukarnie||') nie s¹ ze sob¹ zgodne');
      end if;
    end if;*/

    vile_mod := vszpalt_x*vszpalt_y;
    vile_unitow := ceil((vile_mod-0.5)/sr.modUnit);
    vpadding := 1 + length(vO) - sr.modUnitB * vile_unitow; -- jeden bajt to dwa znaki w RAW, jeden unit to 8 znakow

    -- zmiana d?ugo?ci pola moduly
    select ile_mod into vi from str where mak_xx=vmak_xx and str_xx=vstr_xx;
    if vi<>vile_mod then
      if vile_mod > sr.maxMod then
        vile_mod := sr.maxMod;
      end if;

      select xx into vi from spacer_kratka
       where szpalt_x=vszpalt_x and szpalt_y=vszpalt_y;

      update spacer_strona
         set moduly=lpad('0',vile_mod*sr.modLen,'0'), kra_xx=vi
       where mak_xx=vmak_xx and str_xx=vstr_xx;

      delete from spacer_pow where mak_xx=vmak_xx and str_xx=vstr_xx;
      delete from spacer_str_krat where mak_xx=vmak_xx and str_xx=vstr_xx and kra_xx=vi;

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
    end if;

    select count(1) into vi from str where mak_xx=vmak_xx and str_xx=vstr_xx and (szpalt_x<>vszpalt_x or szpalt_y<>vszpalt_y);
    if vi > 0 then --nie zmienia sie dlugosc pola moduly, ale zmienia sie krata
      update spacer_strona set kra_xx=(select xx from spacer_kratka where szpalt_x=vszpalt_x and szpalt_y=vszpalt_y)
       where mak_xx=vmak_xx and str_xx=vstr_xx;
    end if;
    
    vi := 0;
    while vi < vile_unitow loop
      vj := vpadding + (vile_unitow-vi-1)*sr.modUnitB;
      vlockUnit := to_number(substr(vB,vj,sr.modUnitB),'XXXXXXXX');
      vredUnit := to_number(substr(vR,vj,sr.modUnitB),'XXXXXXXX');
      voglUnit := to_number(substr(vO,vj,sr.modUnitB),'XXXXXXXX');
      vk := vile_mod-vi*sr.modUnit;
      if vk > sr.modUnit then
        vk := sr.modUnit;
      end if;

      vj := 0;
      while vj < vk loop
        if mod(voglUnit,2.0) = 0.0 then -- NOT 'O'
          update_modul(vmak_xx,vstr_xx,vile_mod-sr.modUnit*vi-vj-1,sr.wolny);
        else
          if mod(vlockUnit,2.0) = 1.0 then -- 'B'
            vlockUnit := vlockUnit - 1.0;
            update_modul(vmak_xx,vstr_xx,vile_mod-sr.modUnit*vi-vj-1,sr.zablokowany);
          elsif mod(vredUnit,2.0) = 1.0 then -- 'R'
            vredUnit := vredUnit - 1.0;
            update_modul(vmak_xx,vstr_xx,vile_mod-sr.modUnit*vi-vj-1,sr.redakcyjny);
          end if;
          voglUnit := voglUnit - 1.0;
        end if;

        vlockUnit := vlockUnit / 2.0;
        vredUnit := vredUnit / 2.0;
        voglUnit := voglUnit / 2.0;

        vj := vj + 1;
      end loop;
      vi := vi + 1;
    end loop;

    -- poprawienie rekordu strony
    update spacer_strona
       set nr_porz=vnr_porz,
               nr=vnr,
               num_xx=vtyp_num,
               ile_kol=vile_kolorow,
               spot=decode(vnr_spotu,0,null,vnr_spotu),
               sciezka=vsciezka,
               str_log=substr(vstr_log,1,5),
               drukarnie=vdrukarnie,
               deadline=case when to_date(vdeadline,sr.vfLongDate)<=sysdate+1/24 then deadline else to_date(vdeadline,sr.vfLongDate) end,
               mutred=vmutred,
               pap_xx=decode(vpap_xx,0,null,vpap_xx),
               rozkladowka=decode(vis_rozk,0,null,vis_rozk),
               wyd_xx=decode(vwyd_xx,(select d.wyd_xx from drzewo d,makieta m where m.xx=spacer_strona.mak_xx and d.xx=m.drw_xx),null ,-1,wyd_xx,vwyd_xx)
         where mak_xx=vmak_xx and str_xx=vstr_xx;
         
        
    update plan_wyd.strona set wca_xx=vwyd_xx
     where wpr_xx=(select xx from plan_wyd.wydanie_produktu where rownum=1 and del_xx is null and nvl(odw_xx,0)=0 and nvl(do_przelicz,0)<>2 and mak_xx=vmak_xx)
       and numer=decode(vnr_porz,0,(select objetosc from makieta where xx=vmak_xx),vnr_porz);
         
end update_strona;