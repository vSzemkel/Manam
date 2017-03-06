create or replace procedure "UPDATE_SPACER_PUB" (              
    vpub_xx in out number, -- 0 ; vpub_xx<0 <=> flaga_rezerw=1
    vadd_xx in number,
    vmak_xx in number,  -- do obliczenia czasu; nie przenosi sie pomiedzy makietami
    vstr_xx in number,
    vszpalt_x in number,
    vszpalt_y in number, -- 5
    vx in number,
    vy in number,
    vblokada in number,
    vsizex in number, 
    vsizey in number, -- 10
    vnazwa in varchar2,
    vadno in number,
    vwersja in varchar2,              
    vuwagi in varchar2,                
    vile_kolorow in number, -- 15
    vspo_xx in number,
    vop_zew in varchar2,              
    vsekcja in varchar2,              
    vop_sekcji in varchar2,              
    vnr_w_sekcji in number, -- 20
    vpl in varchar2,
    vop_pl in varchar2,              
    vnr_pl in number,
    vpoz_na_str in varchar2,                
    vtxtposx in number, -- 25
    vtxtposy in number,
    vtyp_xx in number,
    vczaskto in varchar2,
    vpowtorka in number,
    vold_adno in number, -- 30
    vstudio in number, -- not used
    vuwagi_atex in varchar2,
    vaccept_flag in number,
    vspad in number,
    vnag_xx in number,
    vis_digital in number default 0 
) as
    vprecel_flag raw(128) := null;
    vflaga_rezerw sr.srtinyint;
    vstatus sr.srtinyint;
    vczas sr.srint;
    vtyp number;
begin
  if vpub_xx < 0.0 then
    vflaga_rezerw := 1; 
    vpub_xx := -vpub_xx;
  else
    vflaga_rezerw := 0;
  end if;

  if substr(vczaskto,14,1) = ':' then
    vczas := pubdate(vmak_xx, to_date(substr(vczaskto,1,16),sr.vfLongDate));
  else
    vczas := null;
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
    select precel_flag into vprecel_flag from typ_ogloszenia where xx=vtyp_xx;
  end if;
  
  if vstudio = 7 then -- skasuj material
    insert into eps_present_log (ope_xx,adno,kiedy,contentid)
         select 14,p.adno,m.kiedy,p.cid_xx
           from makieta m,spacer_pub p,spacer_users u
          where m.xx=p.mak_xx and p.xx=vpub_xx and p.adno>0 
            and p.powtorka is null and u.xx=uid and bitand(u.gru_xx,sr.studio)>0;
  end if;

  update cid_info set accepted=vaccept_flag
   where xx in (select cid_xx from spacer_pub where xx=vpub_xx);
  
  update spacer_pub              
     set add_xx=decode(vadd_xx,-1,null,vadd_xx),     
         str_xx=vstr_xx,       
         typ_xx=vtyp,           
         x=vx,           
         y=vy,           
         czas_obow=decode(bit_zapory,1,null,vczas),
         blokada=vblokada,           
         sizex=vsizex,           
         sizey=vsizey,           
         nazwa=vnazwa,           
         adno=decode(vadno,-1,null,vadno),
         wersja=vwersja,           
         uwagi=vuwagi,           
         ile_kol=vile_kolorow,           
         spo_xx=decode(vspo_xx,0,null,vspo_xx),           
         op_zew=vop_zew,           
         sekcja=substr(vsekcja,1,5),
         op_sekcji=vop_sekcji,           
         nr_w_sekcji=decode(vnr_w_sekcji,0,null,vnr_w_sekcji),           
         p_l=vpl,           
         op_pl=vop_pl,           
         nr_pl=decode(vnr_pl,0,null,vnr_pl),           
         poz_na_str=vpoz_na_str,           
         txtposx=vtxtposx,           
         txtposy=vtxtposy,     
      	 flaga_rezerw=vflaga_rezerw,
      	 eps_present=decode(vaccept_flag,2,null,vaccept_flag),
      	 powtorka=decode(vpowtorka,0,null,vpowtorka),
      	 old_adno=decode(vold_adno,-1,null,vold_adno),
      	 studio=decode(vstudio,0,null,7,null,vstudio),
      	 uwagi_atex=vuwagi_atex,
      	 spad=decode(vspad,0,null,vspad),
         nag_xx=decode(vnag_xx,1,null,vnag_xx),
         is_digital=decode(vis_digital,0,null,1)
   where xx=vpub_xx;

  -- ogloszenia na stronach i nieusuniete przez spacer
  if sql%rowcount = 1 and vx > 0.0 then
    if vflaga_rezerw = 1 then
      vstatus := sr.flaga_rezerw ;
    else
      vstatus := sr.ogloszenie ;
    end if;
    
    select count(1) into vflaga_rezerw from str 
     where mak_xx=vmak_xx and str_xx=vstr_xx 
       and szpalt_x=vszpalt_x and szpalt_y=vszpalt_y;
    if vflaga_rezerw = 0 then -- nonbase
      declare
        vkra_xx number;
      begin
        select xx into vkra_xx from spacer_kratka where szpalt_x=vszpalt_x and szpalt_y=vszpalt_y;
        spacer_set_kratsp(vmak_xx,vstr_xx,vkra_xx,vx,vy,vsizex,vsizey,vstatus,vszpalt_x,nvl(vczas,0),vprecel_flag);
      exception
        when no_data_found then
          raise_application_error(-20001, 'UPDATE_SPACER_PUB: Wyspecyfikowano bledna krate strony');        
      end;
    else -- base
      spacer_set_space2(vmak_xx,vstr_xx,vx,vy,vsizex,vsizey,vstatus,vszpalt_x,nvl(vczas,0),vprecel_flag);
    end if;       
  end if;
exception
  when no_data_found then
    raise_application_error(-20001, 'UPDATE_SPACER_PUB: Wyspecyfikowano bledny typ ogloszenia');        
end update_spacer_pub;