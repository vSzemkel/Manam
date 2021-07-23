create or replace package body zajawki is

   -- funkcja zwraca liste krat
   function get_kraty return sr.refCur
   is
      c_daneout sr.refCur;
   begin
      open c_daneout for
         select t.xx, t.szpalt_x || 'x' || t.szpalt_y nazwa
         from spacer_kratka t
         order by t.szpalt_x, t.szpalt_y;
      return c_daneout;
   end get_kraty;

   -- funkcja zwraca informacje o wskazanej zajawce
   function get_zajawka(vxx in zajawka.xx%type) return sr.refCur
   is
      c_daneout sr.refCur;
   begin
      open c_daneout for
         select z.xx, z.cli_xx klient, z.cid_xx cid, z.cntid, z.powtorka, z.tytul, z.mutacja, z.nazwa, z.data_from od, z.data_exp do, u.loginname kto_mod, t.szpalt_x || 'x' || t.szpalt_y kratka, tog.sym typ, zajawki.get_status(z.xx) status, z.adno_seed adno, z.maxinmak, z.tpr_xx tytpras
         from zajawka z,spacer_users u,typ_ogloszenia tog,spacer_kratka t
         where z.xx=vxx and u.xx=z.kto_mod and z.typ_xx=tog.xx 
           and tog.kra_xx=t.xx and (tog.kratowe=1 or tog.zajawkowy=1);
      return c_daneout;
   end get_zajawka;
   
   function get_zajawka_2(vxx in zajawka.xx%type) return sr.refCur
   is
      c_daneout sr.refCur;
   begin
      open c_daneout for          
        select z.xx, z.cli_xx klient, z.cid_xx cid, z.cntid, z.powtorka, z.tytul, z.mutacja, z.nazwa, z.data_from, z.data_exp, tog.kra_xx, tog.xx typ_xx, z.adno_seed adno, z.maxinmak, z.tpr_xx
          from zajawka z,spacer_users u,typ_ogloszenia tog
         where z.xx=vxx and u.xx=z.kto_mod and z.typ_xx=tog.xx 
           and (tog.kratowe=1 or tog.zajawkowy=1);
      return c_daneout;
   end get_zajawka_2;

	 -- funkcja zwraca wszystkie zajawki na jednej stronie przefiltrowane
   function get_zajawki_page(vpageno in number, vnazwa in varchar2, vtytul in varchar2, vmut in varchar2, vpocz in date, vkon in date, vkrata in number, vtyp in number, vklient in number, vtpr_xx in number, vbraki in number) return sr.refCur
   is
      crec_per_page constant binary_integer := 20;
      c_daneout sr.refCur;
   begin
      open c_daneout for
        select rownum lp, null xx, null klient, null cid, null cntid, null data_exp, null powtorka, null tytul, null mutacja, null nazwa, null kto_mod, null kratka, null typ, null status, null adno
          from pivot where x<crec_per_page*vpageno
      union all
         select * from (
           select rownum lp,t.* from (
             select z.xx, z.cli_xx klient, z.cid_xx cid, z.cntid, to_char(z.data_exp,sr.vfShortDate) data_exp, z.powtorka, z.tytul, z.mutacja, z.nazwa, u.loginname kto_mod, t.szpalt_x || 'x' || t.szpalt_y kratka, tog.sym typ, zajawki.get_status(z.xx) status, z.adno_seed adno
               from zajawka z,spacer_users u,typ_ogloszenia tog,spacer_kratka t
              where u.xx=z.kto_mod and z.typ_xx=tog.xx and tog.kra_xx=t.xx and (tog.kratowe=1 or tog.zajawkowy=1)
                and (vpocz between z.data_from and z.data_exp or z.data_from between vpocz and vkon)
                and (vnazwa is null or lower(z.nazwa) like lower(vnazwa)) 
                and (vtytul is null or z.tytul like vtytul) 
                and (vmut is null or z.mutacja like vmut) 
                and (vkrata=-1 or tog.kra_xx=vkrata) 
                and (vtyp=-1 or z.typ_xx=vtyp) 
                and (vklient=-1 or z.cli_xx=vklient)
                and (vtpr_xx=-1 or (vtpr_xx=3 and z.tpr_xx>3) or z.tpr_xx=vtpr_xx)
                and (vbraki=0 or z.cid_xx is null or z.powtorka is null or (select nvl(status,-1) from cid_info where xx=z.cid_xx)<>1)
              order by z.kiedy_mod desc) t)
         where lp between crec_per_page*vpageno+1 and crec_per_page*(vpageno+1)
      union all
        select -1 lp, null xx, null klient, null cid, null cntid, null data_exp, null powtorka, null tytul, null mutacja, null nazwa, null kto_mod, null kratka, null typ, null status, null adno
          from pivot where x<1;
      return c_daneout;
   end get_zajawki_page;
   
   -- funkcja zwraca zajawki przefiltrowane (pierwsze 500 rekordów)
   function get_zajawki_2(vnazwa in varchar2, vtytul in varchar2, vmut in varchar2, vpocz in date, vkon in date, vkrata in number, vtyp in number, vklient in number, vtpr_xx in number, vbraki in number) return sr.refCur
   is
      c_daneout sr.refCur;
   begin
      open c_daneout for
         select rownum lp,t.* from (
           select z.xx, z.cli_xx klient, z.cid_xx cid, z.cntid, z.data_from, z.data_exp, z.powtorka, z.tytul, z.mutacja, z.nazwa, u.loginname kto_mod, t.szpalt_x || 'x' || t.szpalt_y kratka, tog.sym typ, zajawki.get_status(z.xx) status, z.adno_seed adno, z.maxinmak, z.tpr_xx, z.kiedy_mod
             from zajawka z,spacer_users u,typ_ogloszenia tog,spacer_kratka t
            where u.xx=z.kto_mod and z.typ_xx=tog.xx and tog.kra_xx=t.xx and (tog.kratowe=1 or tog.zajawkowy=1)
              and (vpocz between z.data_from and z.data_exp or z.data_from between vpocz and vkon)
              and (vnazwa is null or lower(z.nazwa) like lower(vnazwa)) 
              and (vtytul is null or z.tytul like vtytul) 
              and (vmut is null or z.mutacja like vmut) 
              and (vkrata=-1 or tog.kra_xx=vkrata) 
              and (vtyp=-1 or z.typ_xx=vtyp) 
              and (vklient=-1 or z.cli_xx=vklient)
              and (vtpr_xx=-1 or (vtpr_xx=3 and z.tpr_xx>3) or z.tpr_xx=vtpr_xx)
              and (vbraki=0 or z.cid_xx is null or z.powtorka is null or (select nvl(status,-1) from cid_info where xx=z.cid_xx)<>1)
            order by z.kiedy_mod desc) t where rownum<501;
      return c_daneout;
   end get_zajawki_2;
   
   function get_status(vxx zajawka.xx%type) return varchar2
   is
      vcntid zajawka.cntid%type;
      vcid_xx cid_info.xx%type;
      vstatus cid_info.status%type;
      vpath cid_info.preview_path%type;
      vpowtorka zajawka.powtorka%type;
   begin
      select z.cntid,z.cid_xx,ci.status,z.powtorka,
             nvl2(max(ez.adno),mu.uri||'name=ZAJAW&date='||to_char(max(ez.data_atex),'rrrrmmdd')||'&nreps='||max(ez.adno),ci.preview_path) 
        into vcntid,vcid_xx,vstatus,vpowtorka,vpath
        from zajawka z,emisja_zajawki ez,cid_info ci,manam_uri mu
       where z.xx=vxx and mu.xx=10 and z.xx=ez.zaj_xx(+) and z.cid_xx=ci.xx(+)
       group by z.cntid,z.cid_xx,ci.status,z.powtorka,mu.uri,ci.preview_path;

      if vcntid is null then
        return 'Oczekiwanie na CONTENTID';
      elsif vcntid is not null and vcid_xx is null then
        return 'Oczekiwanie na plik';
      elsif vcid_xx is not null and vstatus is null then
        return nvl(vpath,'EpsTest sprawdza material');
      elsif vstatus=1 and vpowtorka is not null then
        return '<a href="'||vpath||'" target="_blank">Gotowy do uzycia</a>';
      elsif vstatus=0 then
        return '<a href="'||vpath||'" target="_blank">Material odrzucony</a>';
      else
        return 'Error';
      end if;
   end get_status;

   -- pobiera slownik kratek przynalezacych do danego typu ogloszenia
   function get_typy_oglosz(v_kra_xx in number) return sr.refCur
   is
      c_daneout sr.refCur;
   begin
      open c_daneout for
         select xx, nazwa from (
         select tog.xx, tog.sym nazwa
           from typ_ogloszenia tog, spacer_kratka spkr
          where tog.kratowe=1 and tog.kra_xx=spkr.xx and spkr.xx=v_kra_xx
          order by to_number(substr(tog.sym, 1, instr(tog.sym, 'x')-1)), to_number(substr(tog.sym, instr(tog.sym, 'x')+1)))
          union all
         select xx, nazwa from (
         select tog.xx, tog.modelid nazwa
           from typ_ogloszenia tog, spacer_kratka spkr
          where tog.zajawkowy=1 and tog.kra_xx=spkr.xx and spkr.xx=v_kra_xx
          order by tog.modelid);
      return c_daneout;
   end get_typy_oglosz;

   -- funkcja zwraca liste tytulow prasowych
   function get_tytuly_prasowe(v_mask in varchar2) return sr.refCur
   is
      c_daneout sr.refCur;
   begin
      open c_daneout for
         select xx, nazwa
         from tytul_prasowy
         where usuniety is null
               and do_kiedy>sysdate
               and (v_mask is null or lower(nazwa) like lower(v_mask))
         order by nazwa;
      return c_daneout;
   end get_tytuly_prasowe;

   -- procedura zapisuje zajawke
   function zapisz_zajawke(v_xx in number, v_typ_xx in number, v_cli_xx in number, v_tpr_xx in number, v_tytul in varchar2, v_mutacja in varchar2, v_nazwa in varchar2, v_ilosc in number, v_od in date, v_do in date, v_ktomod in varchar2) return number
   is
     v_c number;
     v_kto_xx number;
     v_newxx number;
     vactual_tpr_xx tytul_prasowy.xx%type := v_tpr_xx;
   begin
     select max(xx) into v_kto_xx from spacer_users where lower(loginname_ds)=v_ktomod;
     -- tytul musi istniec w bazie
     select count(1) into v_c from drzewo d
        where d.tytul=nvl(v_tytul, d.tytul) and d.mutacja=nvl(v_mutacja, d.mutacja);
     if v_c = 0 then
        raise_application_error(-20000, 'Produkt: ' || v_tytul || ' ' || v_mutacja || ' nie istnieje.');
     end if;
     -- sprawdzonym materialom nie mozna zmieniac formatu
     select count(1) into v_c from zajawka z,cid_info ci
      where z.xx=v_xx and z.cid_xx=ci.xx and ci.status=1 and z.typ_xx<>v_typ_xx;
     if v_c > 0 then
        raise_application_error(-20001, 'Nie mozna modyfikowac formatu zajawki, do ktorej sprawdzono material');
     end if;
     -- produkty inne niz GW i Metro
     if v_tpr_xx > 2 then
        if v_tytul is null then
           raise_application_error(-20002, 'Prosze okreslic tytul produktu');
        end if;
        select nvl(min(tpr_xx),v_tpr_xx) into vactual_tpr_xx from drzewo where tytul=v_tytul;
     end if;
     
     update zajawka set nazwa=v_nazwa,tytul=v_tytul,mutacja=v_mutacja,typ_xx=v_typ_xx,cli_xx=v_cli_xx,tpr_xx=vactual_tpr_xx, 
            maxinmak=v_ilosc,data_from=v_od,data_exp=v_do,kto_mod=v_kto_xx,kiedy_mod=current_timestamp
      where xx=v_xx;         
     if sql%rowcount = 0 then
        insert into zajawka(typ_xx,cli_xx,tpr_xx,tytul,mutacja,nazwa,maxinmak,data_from,data_exp,kto_mod)
             values (v_typ_xx,v_cli_xx,vactual_tpr_xx,v_tytul,v_mutacja,v_nazwa,v_ilosc,v_od,v_do,v_kto_xx)
          returning xx into v_newxx;
        return v_newxx;
     end if;
     return -1;
   end zapisz_zajawke;

   -- usuwamy zajawke
   procedure usun_zajawke(v_xx in number)
   is
      vstatus cid_info.status%type;
   begin
      select nvl(ci.status,0) into vstatus from zajawka z,cid_info ci where z.xx=v_xx and z.cid_xx=ci.xx(+);
      if vstatus = 0 then          
         delete from zajawka where xx=v_xx;
      else
         update zajawka set data_from=sysdate-1, data_exp=sysdate-1 where xx=v_xx;
      end if;
   end usun_zajawke;

   --zmienia date waznosci grupy zajawek
   procedure waznosc_grupy(vaxx in sr.numlist, vpocz in date, vkon in date, vkto_mod in varchar2) is
      vkto zajawka.kto_mod%type;
   begin
      select xx into vkto from spacer_users where lower(loginname_ds)=vkto_mod;
      
      forall i in vaxx.first..vaxx.last
        update zajawka set data_from=vpocz,data_exp=vkon,kto_mod=vkto,kiedy_mod=current_timestamp 
         where xx=vaxx(i);
   end waznosc_grupy;

   -- nadaje adno dla konkretnego ogloszenia na makiecie - wywolywana przez Manam
   procedure nadaj_adno(vzaj_xx in zajawka.xx%type, vmak_xx in makieta.xx%type, vrefCur out sr.refCur)
   is
     vadno spacer_pub.adno%type;
     vbez_powtorki number;
     vostatnie_adno number := null;
     vostatnia_powtorka number := null;
   begin
      select count(1) into vbez_powtorki from zajawka z, makieta m
       where z.xx=vzaj_xx and m.xx=vmak_xx and m.kiedy=sr.powtseed+z.powtorka;
      if vbez_powtorki = 0 then
         select max(kiedy) - sr.powtseed,max(p.adno) into vostatnia_powtorka,vostatnie_adno from makieta m,spacer_pub p 
          where m.xx=p.mak_xx and p.adno=(select max(adno) from emisja_zajawki where zaj_xx=vzaj_xx and nvl(data_atex,sysdate)<sysdate-31);
         if vostatnie_adno is null then
            select min(kiedy - sr.powtseed),min(adno) into vostatnia_powtorka,vostatnie_adno from (
               select m.kiedy,p.adno,rownum r
                 from makieta m,spacer_pub p,emisja_zajawki ez 
                where m.xx=p.mak_xx and p.adno=ez.adno and ez.zaj_xx=vzaj_xx and ez.data_atex is not null
                order by p.adno) where r=1;
         end if;
      end if;
      
      atex.tb_atex_utilities.get_adno(vadno);
      if vadno is not null then
        insert into emisja_zajawki (adno,zaj_xx)
          values (vadno,vzaj_xx);
      end if;

      commit;

      open vrefcur for
         select to_char(vadno),
                decode(powtorka,vostatnia_powtorka,adno_seed,nvl(vostatnie_adno,adno_seed)),
                decode(vbez_powtorki,1,0,nvl(vostatnia_powtorka,powtorka))
           from zajawka where xx=vzaj_xx;
   end nadaj_adno;

   -- zapisuje dla zajawki adno i cntidid
   procedure zapisz_cntid_adno(v_xx in number, v_cntid in number, v_adno in number)
   is
   begin
      if v_cntid > 0 then
         update zajawka set cntid=v_cntid,adno_seed=v_adno where xx=v_xx;
      else
         update zajawka set (adno_seed,cntid,cid_xx)=(
             select z2.adno_seed,z2.cntid,z2.cid_xx from zajawka z2 where z2.xx=-v_cntid) where xx=v_xx;
      end if;
      
      if sql%rowcount = 0 then
         raise_application_error(-20000, 'Nie znaleziono zajawki dla ktorej probowano dodac wartosc cntid.', false);
      end if;
   end zapisz_cntid_adno;

   -- zapisuje potwierdzenie wyslania zadania sprzedazy w ATEXie do TIBCO
   procedure zapisz_daty_sprzedazy(vadnos in sr.numlist, vdates in datelist) as
   begin
      forall i in vadnos.first..vadnos.last
        update emisja_zajawki set data_atex=vdates(i)
         where vdates(i)>sr.powtseed and adno=vadnos(i);
         
      delete from emisja_zajawki ez where data_atex is null 
         and not exists (select 1 from spacer_pub p where p.adno=ez.adno);
   end;

   -- kursor powinien byc pusty, jesli nie jest, trzeba uruchomic automat dla wymienionych dat
   procedure check_sprzedaz(vrefCur out sr.refCur) 
   as begin
     open vrefcur for
        select to_char(m.kiedy,sr.vfShortDate),d.tytul||' '||d.mutacja
          from emisja_zajawki ez,spacer_pub p,makieta m,drzewo d
         where m.xx=p.mak_xx and p.adno=ez.adno and ez.data_atex is null and m.kiedy<sysdate-1 
           and m.drw_xx=d.xx and d.wyd_xx<>36 -- bez Trader'a
         order by kiedy;
   end check_sprzedaz;

   -- lista niesprzedanych zajawek, ktore ukazaly sie danego dnia
   procedure co_sprzedac(vkiedy in makieta.kiedy%type, vrefCur out sr.refCur) 
   as begin
     open vrefcur for
       select z.xx,z.cli_xx,d.tytul,d.mutacja,p.adno,z.cntid,upper(t.sym) format,
              decode(ci.kolor,'B','BRAK','FULL') kolor,decode(instr(s.sciezka,'RED'),0,'OG','IR') page,
              decode(s.nr_porz,0,m.objetosc,s.nr_porz) nr_porz,decode(length(substr(str_log,1,3)),3,decode(substr(str_log,1,3),'DRO','OGL','IOG','OGL','OST','OGL',substr(str_log,1,3)),decode(instr(s.sciezka,'RED'),0,'OGL','RED')) rubryka
         from makieta m,drzewo d,spacer_strona s,spacer_pub p,typ_ogloszenia t,zajawka z,emisja_zajawki ez,cid_info ci
        where m.kiedy=vkiedy and d.xx=m.drw_xx and s.mak_xx=m.xx and p.str_xx=s.str_xx and p.mak_xx=m.xx
          and p.typ_xx=t.xx and p.adno=ez.adno and ez.zaj_xx=z.xx and ez.data_atex is null and z.cid_xx=ci.xx
          and d.wyd_xx<>36; -- bez Trader'a
   end co_sprzedac;

   -- raport uzycia zajawek
   procedure uzycie_zajawek (
     vnazwa in zajawka.nazwa%type, 
     vseed in zajawka.adno_seed%type, 
     vrefCur out sr.refCur
   ) as begin
     open vrefcur for
        select rownum lp,t.* from (
        select z.nazwa,z.adno_seed,z.cli_xx,ez.adno,k.sym,upper(t.sym) rozmiar,d.tytul||' '||d.mutacja,to_char(m.kiedy,sr.vfShortDate),s.sciezka,s.nr_porz
          from zajawka z,emisja_zajawki ez,spacer_pub p,typ_ogloszenia t,spacer_kratka k,spacer_strona s,makieta m,drzewo d
         where z.xx=ez.zaj_xx and ez.adno=p.adno and p.typ_xx=t.xx and t.kra_xx=k.xx 
           and p.str_xx=s.str_xx and p.mak_xx=s.mak_xx and s.mak_xx=m.xx and m.drw_xx=d.xx
           and (vnazwa is null or z.nazwa like vnazwa)
           and (vseed<=0 or z.adno_seed=vseed)
         order by z.nazwa,z.adno_seed,m.kiedy) t;
   end uzycie_zajawek;
   
   -- raport uzycia zajawek
   procedure uzycie_zajawek_2 (
      vnazwa in zajawka.nazwa%type, 
      vseed in zajawka.adno_seed%type, 
      vrefCur out sr.refCur
   ) as begin
     open vrefcur for
        select z.nazwa,z.adno_seed,z.cli_xx,ez.adno,k.sym,upper(t.sym) rozmiar,d.tytul||' '||d.mutacja produkt,to_char(m.kiedy,sr.vfShortDate) edycja,s.sciezka,s.nr_porz
          from zajawka z,emisja_zajawki ez,spacer_pub p,typ_ogloszenia t,spacer_kratka k,spacer_strona s,makieta m,drzewo d
         where z.xx=ez.zaj_xx and ez.adno=p.adno and p.typ_xx=t.xx and t.kra_xx=k.xx 
           and p.str_xx=s.str_xx and p.mak_xx=s.mak_xx and s.mak_xx=m.xx and m.drw_xx=d.xx
           and (vnazwa is null or lower(z.nazwa) like lower(vnazwa))
           and (vseed<=0 or z.adno_seed=vseed)
         order by z.nazwa,z.adno_seed,m.kiedy;
   end uzycie_zajawek_2;

   -- przekazuje do Manamu liste dostepnych zajawek
   procedure zajawki4manam (
     vmak_xx makieta.xx%type,
     vsym_kraty in spacer_kratka.sym%type,
     vsizex in typ_ogloszenia.sizex%type,
     vsizey in typ_ogloszenia.sizey%type,
     vrefCur out sr.refCur
   ) is begin
     open vrefcur for
       select z.nazwa,z.xx from makieta m,drzewo d,zajawka z,typ_ogloszenia t,spacer_kratka k,cid_info ci
        where m.xx=vmak_xx and z.typ_xx=t.xx and t.kra_xx=k.xx and z.cid_xx=ci.xx and ci.status=1 
          and m.drw_xx=d.xx and m.kiedy between z.data_from and z.data_exp
          and nvl(z.tytul,d.tytul)=d.tytul and nvl(z.mutacja,d.mutacja)=d.mutacja 
          and k.sym=vsym_kraty and (d.tpr_xx=z.tpr_xx or (d.zuz_xx=1 and z.tpr_xx=1))
          and ((t.kratowe=1 and t.sym=to_char(vsizex)||'x'||to_char(vsizey))
            or (t.zajawkowy=1 and t.sizex=vsizex and t.sizey=vsizey))
        order by z.nazwa;
   end zajawki4manam;
end;