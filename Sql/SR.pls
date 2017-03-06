create or replace PACKAGE                     "SR" /*************************************
**                                  **
**    (c) Marcin Buchwald 2000      **
**                                  **
*************************************/
is
  --PRAGMA SERIALLY_REUSABLE ;

  -- domyslne formaty daty
  vfShortDate varchar2(10) := 'dd/mm/rrrr';
  vfLongDate varchar2(18) := 'dd/mm/rrrr hh24:mi';
  vfTimestamp varchar2(25) := 'dd/mm/rrrr hh24:mi:ss.ff6';
  -- typy danych dla Space Reservation
  subtype srid       is number(15,0);
  subtype srint      is pls_integer;
  subtype srsmallint is pls_integer;
  subtype srtinyint  is pls_integer;
  type refCur        is ref cursor;
  type numlist       is table of number index by binary_integer;
  type datlist       is table of date index by binary_integer;
  type ridArray      is table of rowid index by binary_integer; 
  type str16list     is table of varchar2(16) index by binary_integer;
  -- mutating table exception handling
  mutRows ridArray; 
  empty   ridArray; 

  -- stale dla Space Reservation
  flagLen   constant srtinyint := 4 ; -- CFlag class unit
  modLen    constant srtinyint := 6 ; -- trzy bajty - szesc znakow
  modUnitB  constant srtinyint := 8 ; -- cztery bajty - osiem znakow
  modShift  constant srtinyint := 8 ; -- 2^modLen
  modUnit   constant srtinyint := 32 ; 
  timeUnit  constant srtinyint := 144 ; 
  maxMod    constant srtinyint := 666 ;
  dozywocie constant date := to_date('01/01/2100', vfShortDate) ;
  powtseed  constant date := to_date('31/12/1999', vfShortDate) ;

  -- kody modulow dla Space Reservation (3 bity)
  wolny            constant srtinyint := 0 ;
  ogloszenie       constant srtinyint := 1 ;
  zablokowany      constant srtinyint := 2 ;
  redakcyjny       constant srtinyint := 3 ;
  flaga_rezerw     constant srtinyint := 4 ;
  --dziedziczony     constant srtinyint := 5 ; nie potrzebny
  nieznany         constant srtinyint := 7 ;

  -- rodzaje dziedziczenia dla wywolan nie serially_reusable
  derv_none        constant srtinyint := 0 ;
  derv_adds        constant srtinyint := 1 ;
  derv_tmpl        constant srtinyint := 2 ;
  derv_fixd        constant srtinyint := 3 ;
  derv_proh        constant sr.srtinyint := 4 ; -- strona niedziedziczona
  derv_druk        constant sr.srtinyint := 5 ; -- uzupelnienie w drukarni
  derv_colo        constant sr.srtinyint := 6 ; -- dziedziczenie separacji barwnych
  
  -- role
  dealer           constant srtinyint := 1 ;
  studio           constant srtinyint := 4 ;
  kierownik        constant srtinyint := 8 ;
end;