create or replace procedure "OPEN_SPOQ" (
	vmak_xx in number,
	vstrCur out sr.refCur,
	vpubCur out sr.refCur,
	vopiCur out sr.refCur,
	vqueCur out sr.refCur
) as
  vmodulyTab spacer_mod_mak := spacer_mod_mak();
  vmoduly spacer_mod_str := spacer_mod_str(0,'','','');
  vi sr.srint;
  vs sr.srint;
  voff sr.srint;
  vmakdate sr.srint;
  vspad_offset sr.srint;
  vunit number;
  vred_unit number;
  vlocked_unit number;
  vwyd_xx wydawca.xx%type;
begin
  select makdate(m.xx),nvl((select dd.spad_offset from daty_dla_dow dd where dd.tytul=d.tytul and dd.dow=to_char(m.kiedy,'d')),d.spad_offset),d.wyd_xx
    into vmakdate,vspad_offset,vwyd_xx
    from makieta m, drzewo d where m.xx=vmak_xx and m.drw_xx=d.xx;

  for strona in (
	   select str_xx, ile_mod
	     from str
	    where mak_xx=vmak_xx
  ) loop
    vmodulyTab.extend;
    vmoduly.str_xx := strona.str_xx;
    vmoduly.space := '';
    vmoduly.space_red := '';
    vmoduly.space_locked := '';

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

      vs := select_modul_status(vmak_xx, strona.str_xx, vi, vmakdate);

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
    select t.str_xx, --0
           s.nr_porz,
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
           decode(nvl(s.dervlvl,sr.derv_none),sr.derv_none,'',decode(dd.mutacja,' ',dd.tytul,dd.tytul||' '||dd.mutacja)||' '||s.derv_nr_porz||' - '||decode(s.dervlvl,derv.adds,'ogloszenia',derv.tmpl,'wzorzec',derv.colo,'koloru','pelne')) dervinfo,
           s.mutred,
           nvl(s.pap_xx,0) pap_xx, --20
           nvl(s.rozkladowka,0) is_rozkl,
           nvl(s.wyd_xx,vwyd_xx) wyd_xx,
           1 -- mutczas dla niezachowanych grzbietów
      from spacer_strona s,
           table(cast(vmodulyTab as spacer_mod_mak)) t,
           spacer_kratka k,
           (select mak_xx, str_xx, max(-1) base
              from spacer_str_krat group by mak_xx, str_xx) b,
           makieta dm,
           drzewo dd -- po przej?ciu na oracle 9i trzeba to zmieni?
     where s.mak_xx=vmak_xx
       and s.str_xx=t.str_xx
       and s.kra_xx=k.xx
       and s.mak_xx=b.mak_xx(+)
       and s.str_xx=b.str_xx(+)
       and s.derv_mak_xx=dm.xx(+)
       and dm.drw_xx=dd.xx(+)
     order by s.str_xx;

  open vpubCur for
    select pub.xx, --0
           nvl(pub.add_xx,-1),
           makieta.xx,
           str_xx,
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
           ile_kol,
           nvl(spo_xx,0),
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
           decode(sign(vmakdate-nvl(czas_obow,vspad_offset*sr.timeUnit)+vspad_offset*sr.timeUnit), --30
              -1,decode(adno,null,-1,decode(bit_zapory,1,-3,-2)),
                 decode(adno,null,1,decode(bit_zapory,1,3,2))),
           nvl(eps_present,2), --31
           nvl(powtorka,0),
           nvl(old_adno,-1),
           nvl(studio,0),
           derived, --35
           uwagi_atex,
           nvl(spad,0),
           rawtohex(precel_flag), 
           t.sym,
           nvl(nag_xx,1), --40
           nvl(podpis_reklama,0),
           nvl(is_digital,0) --42
   from drzewo, makieta,
        (select  xx,add_xx,mak_xx,str_xx,typ_xx,adno,x,y,sizex,sizey,txtposx,txtposy,blokada,bit_zapory,flaga_rezerw,nazwa,uwagi,ile_kol,spo_xx,op_zew,sekcja,op_sekcji,nr_w_sekcji,p_l,op_pl,nr_pl,poz_na_str,x_na_str,y_na_str,wersja,czas_obow,eps_present,powtorka,old_adno,studio,eps_date,0 derived,uwagi_atex,spad,cid_xx,nag_xx,is_digital
           from spacer_pub where mak_xx=vmak_xx
         union all select p.xx,p.add_xx,s.mak_xx,s.str_xx,p.typ_xx,p.adno,s.x,s.y,p.sizex,p.sizey,p.txtposx,p.txtposy,nvl(s.blokada,0),p.bit_zapory,p.flaga_rezerw,p.nazwa,p.uwagi,p.ile_kol,p.spo_xx,p.op_zew,p.sekcja,p.op_sekcji,p.nr_w_sekcji,p.p_l,p.op_pl,p.nr_pl,p.poz_na_str,p.x_na_str,p.y_na_str,p.wersja,p.czas_obow,p.eps_present,p.powtorka,p.old_adno,p.studio,p.eps_date,1 derived,p.uwagi_atex,spad,cid_xx,nag_xx,is_digital
           from spacer_pubstub s, spacer_pub p
          where s.mak_xx=vmak_xx and s.xx=p.xx) pub,
        typ_ogloszenia t, spacer_kratka, spacer_add, spacer_users, cid_info ci
  where drzewo.xx=makieta.drw_xx
    and makieta.xx=vmak_xx
    and makieta.xx=pub.mak_xx
    and typ_xx=t.xx 
    and t.kra_xx=spacer_kratka.xx
    and (flaga_rezerw=1 or nvl(czas_obow,0)<=vmakdate)
    and pub.add_xx=spacer_add.xx(+)
    and spacer_add.sprzedal=spacer_users.xx(+)
    and pub.cid_xx=ci.xx(+)
  order by str_xx;

  open vopiCur for
    select xx, --0
           rect_top,
           rect_left,
           rect_bottom,
           rect_right,
           tekst, --5
           nvl(skala,1),
           nvl(kolor,1)
      from spacer_opis
     where mak_xx=vmak_xx;

  open vqueCur for
    select /*+ ordered */ p.xx, --0 (hint for oracle10g)
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
           nvl(p.spo_xx,0),
           '# ['||u.imie||' '||u.nazwisko||' '||u.tel||']',
           nvl(adno,-1) --13
      from spacer_pub_que p, typ_ogloszenia t, spacer_kratka k,
           spacer_add a, spacer_users u
     where p.mak_xx=vmak_xx
       and p.typ_xx=t.xx
       and t.kra_xx=k.xx
       and p.add_xx=a.xx
       and a.sprzedal=u.xx
     order by p.add_xx;
end open_spoq;