create or replace PACKAGE BODY                     "VFORMAT" is
  function format_zasiegi (vzasiegi in varchar2) 
  return varchar2 is 
    vret varchar2(512);
  begin 
    for c in (select substr(vzasiegi,3*p.x+1,3) z from space_reservation.pivot p where p.x<ceil(length(vzasiegi)/3)) loop
      vret := vret||' , '||c.z;
    end loop;
    return substr(vret,4); 
  end;

  function concat_list ( 
    vCur sr.refCur, 
    vsepstr varchar2 := ',' 
  ) return varchar2 is
    ret varchar2(32767); 
    tmp varchar2(4000);
  begin
    loop
        fetch vCur into tmp;
        exit when vCur%NOTFOUND;
            ret := ret || vsepstr || tmp;
    end loop;
    return substr(ret,length(vsepstr)+1);
  exception 
    when others then
      if sqlcode=-6502 then
        return 'Wynik operacji skladania tekstu jest za dlugi';
      end if;
  end concat_list;

  function tab2list (vtab in spacer_msglist) return varchar2
  is 
    vmsg varchar2(4000);
  begin
    select substr(sys_connect_by_path(name,','),2) into vmsg
      from (select column_value name,rownum rp,count(1) over () cnt from table(cast(vtab as spacer_msglist)))
     where rp=cnt connect by prior rp=rp-1 start with rp=1;
     
    return vmsg;
  end tab2list;
  
  function list2tab (vlist in varchar2) return spacer_numlist
  is
    vret spacer_numlist;
  begin
    with data as (
       select trim( substr (txt,
          instr (txt, ',', 1, level  ) + 1,
          instr (txt, ',', 1, level+1)
             - instr (txt, ',', 1, level) -1 ) )
           as token
         from (select ','||vlist||',' txt from dual)
      connect by level <= length(vlist)-length(replace(vlist,',',''))+1
    ) select cast (multiset ( select token from data ) as spacer_numlist ) 
      into vret from dual;
    return vret;
  end list2tab;

  function slist2tab (vlist in varchar2) return spacer_msglist
  is
    vret spacer_msglist;
  begin
    with data as (
       select trim( substr (txt,
          instr (txt, ',', 1, level  ) + 1,
          instr (txt, ',', 1, level+1)
             - instr (txt, ',', 1, level) -1 ) )
           as token
         from (select ','||vlist||',' txt from dual)
      connect by level <= length(vlist)-length(replace(vlist,',',''))+1
    ) select cast (multiset ( select token from data ) as spacer_msglist ) 
      into vret from dual;
    return vret;
  end slist2tab;

  function cecha_stron(vrefCur in sr.refCur) return varchar2
  as
    vcecha number(1);
    vret varchar2(256);
    vpocz pls_integer := null;
    vobj grzbiet.objetosc%type := 0;
    vincecha boolean := false;
  begin
    loop
      fetch vrefCur into vcecha;
      exit when vrefCur%notfound;
      vobj := vobj + 1;

      if vcecha>0 and not vincecha then
        vpocz := vobj;
        vincecha := true;
      elsif vcecha=0 and vincecha then
        if vpocz < vobj-1 then
          vret := vret||','||vpocz||'-'||(vobj-1);
        else
          vret := vret||','||vpocz;
        end if;
        vincecha := false;
      end if;
    end loop;

    if vincecha then
      if vpocz < vobj then
        vret := vret ||','||vpocz||'-'||vobj;
      else
        vret := vret ||','||vpocz;
      end if;
    end if;

    return substr(vret,2);
  end cecha_stron;

  function to_datepicker(vdt in date default sysdate) return number
  as
  	vi number;
  begin
  	select round(vdt-to_date('30/12/1899','dd/mm/rrrr'),6)
  	into vi
  	from dual;
  	return vi;
  end to_datepicker;
end;