create or replace procedure "INSERT_SPACER_PUB" (              
   vpub_xx in out number, -- 0
   vadd_xx in number,
   vmak_xx in number,
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
   veps_present in number,
   vspad in number,
   vnag_xx in number default 1,
   vis_digital in number default 0
) as
   vprecel_flag raw(128) := null;
   vflaga_rezerw sr.srtinyint;
   vstatus sr.srtinyint;
   vdummy sr.refCur;
   vtyp number;
begin      
	if vpub_xx<0 then
		vflaga_rezerw := 1;
	else
		vflaga_rezerw := 0;
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
  
  begin
    insert into spacer_pub (mak_xx,add_xx,str_xx,typ_xx,x,y,blokada,bit_zapory,sizex,sizey,nazwa,adno,wersja,uwagi,ile_kol,spo_xx,op_zew,sekcja,op_sekcji,nr_w_sekcji,p_l,op_pl,nr_pl,poz_na_str,txtposx,txtposy,flaga_rezerw,eps_present,powtorka,old_adno,uwagi_atex,spad,nag_xx,is_digital)
         values (vmak_xx,decode(vadd_xx,-1,null,vadd_xx),vstr_xx,vtyp,vx,vy,vblokada,1,vsizex,vsizey,vnazwa,decode(vadno,-1,null,vadno),vwersja,vuwagi,vile_kolorow,decode(vspo_xx,0,null,vspo_xx),vop_zew,substr(vsekcja,1,5),vop_sekcji,vnr_w_sekcji,vpl,vop_pl,vnr_pl,vpoz_na_str,vtxtposx,vtxtposy,vflaga_rezerw,decode(veps_present,2,null,veps_present),decode(vpowtorka,0,null,vpowtorka),decode(vold_adno,-1,null,vold_adno),vuwagi_atex,decode(vspad,0,null,vspad),decode(vnag_xx,1,null,vnag_xx),decode(vis_digital,0,null,1))
      returning xx into vpub_xx;
  exception
    when others then
      if instr(sqlerrm,'FK_PUB_ADD')>0 then
        insert into spacer_pub (mak_xx,add_xx,str_xx,typ_xx,x,y,blokada,bit_zapory,sizex,sizey,nazwa,adno,wersja,uwagi,ile_kol,spo_xx,op_zew,sekcja,op_sekcji,nr_w_sekcji,p_l,op_pl,nr_pl,poz_na_str,txtposx,txtposy,flaga_rezerw,eps_present,powtorka,old_adno,uwagi_atex,spad,nag_xx,is_digital)
             values (vmak_xx,-1,vstr_xx,vtyp,vx,vy,vblokada,1,vsizex,vsizey,vnazwa,decode(vadno,-1,null,vadno),vwersja,vuwagi,vile_kolorow,decode(vspo_xx,0,null,vspo_xx),vop_zew,substr(vsekcja,1,5),vop_sekcji,vnr_w_sekcji,vpl,vop_pl,vnr_pl,vpoz_na_str,vtxtposx,vtxtposy,vflaga_rezerw,decode(veps_present,2,null,veps_present),decode(vpowtorka,0,null,vpowtorka),decode(vold_adno,-1,null,vold_adno),vuwagi_atex,decode(vspad,0,null,vspad),decode(vnag_xx,1,null,vnag_xx),decode(vis_digital,0,null,1))
          returning xx into vpub_xx;
      end if;
  end;
    
  -- moze jest juz dostepny material do ogloszenia
  if vadno > 0 then
     epstest.get_prod_info(vpub_xx,vdummy); -- inicjalizacja cid_xx
  end if;
  
  -- ogloszenia na stronach
  if vx > 0.0 then
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
        spacer_set_kratsp(vmak_xx,vstr_xx,vkra_xx,vx,vy,vsizex,vsizey,vstatus,vszpalt_x,0,vprecel_flag);
      exception
        when no_data_found then
          raise_application_error(-20001, 'INSERT_SPACER_PUB: Wyspecyfikowano bledna krate strony');        
      end;
    else -- base
      spacer_set_space2(vmak_xx,vstr_xx,vx,vy,vsizex,vsizey,vstatus,vszpalt_x,0,vprecel_flag);
    end if;       
  end if; 
exception
  when no_data_found then
    raise_application_error(-20001, 'INSERT_SPACER_PUB: Wyspecyfikowano bledny typ ogloszenia');
end insert_spacer_pub;