
#pragma once

constexpr int CLIENT_SCALE = 20;
constexpr int PRINT_VOFFSET = -10;
constexpr int MIN_COLSPERROW = 10;
constexpr int pszpalt_x = 5;               // liczba kolumn w kracie domyœlnej
constexpr int pszpalt_y = 6;               // liczba wierszy w kracie domyœlnej
constexpr int pmodcnt = 30;                // pszpalt_x * pszpalt_y
constexpr int DLGMSG_MAX_LEN = 3000;       // timeout na uzyskanie po³¹czenia z puli wyra¿ony w milisekundach
constexpr int MIN_VALID_ADNO = 10'000'000; // minimalny prawid³owy numer ADNO
constexpr int MANAM_DEFAULT_PORT = 2501;   // domyœlny numer portu u¿ywany przez aplikacjê
constexpr auto MAX_STUDIO_PATH = (_MAX_PATH - 12);
constexpr auto c_formatDaty = _T("%02d/%02d/%04d");
constexpr auto c_formatCzasu = _T("%02d/%02d/%04d %02d:%02d");
constexpr auto c_ctimeDataWs = _T("%Y%m%d"); // domyœlny format dla parametrów web serwisów
constexpr auto c_ctimeData = _T("%d/%m/%Y");
constexpr auto c_ctimeCzas = _T("%d/%m/%Y %H:%M");
constexpr __time64_t ONEDAY = 86400;         // liczba sekund w dobie
constexpr __time64_t POWTSEED_0 = 946594800; // 31/12/1999 00:00 data, w stosunku do której s¹ obliczane powtórki
constexpr __time64_t POWTSEED_1 = 946598400; // 31/12/1999 01:00 data, w stosunku do której s¹ obliczane powtórki plus godzina
                                             // kolory
constexpr int c_brak = 1;
constexpr int c_spot = 2;
constexpr int c_full = 4;
constexpr auto BRAK = _T("(brak)");
constexpr auto FULL = _T("(full)");
constexpr COLORREF BIALY = RGB(255, 255, 255);
// numeracja - odpowiada PK s³ownika TYP_NUMERACJI z bazy
constexpr int c_normal = 1;
constexpr int c_rzym = 2;
// typy obiektow
constexpr int c_page = 0;
constexpr int c_add = 1;
constexpr int c_opis = 2;
constexpr int c_addque = 3;
constexpr int c_page_lib = 4;
constexpr int c_opis_lib = 5;
// grupy
constexpr int R_DEA = 1;
constexpr int R_RED = 2;
constexpr int R_STU = 4;
constexpr int R_KIE = 8;
constexpr int R_ADM = 16;
constexpr int R_MAS = 32;
// status studia
constexpr int STUDIO_BRAK = 0;
constexpr int STUDIO_JEST = 1;
constexpr int STUDIO_NOWY = 2;
constexpr int STUDIO_ACC = 3;
constexpr int STUDIO_OK = 4;
constexpr int STUDIO_SEND = 5;
constexpr int STUDIO_FILTR = 6;
constexpr int STUDIO_MSG = 7;
// gen eps
constexpr float mm2pkt = 0.2835f; // 1mm = 2.835pt
constexpr float pkt2mm = 0.3527f; // 1pt = 0.3527mm
constexpr float pkt_10m = 2.835f;
constexpr int podpisH = 6; // 2.1162 [mm]
constexpr int preview_offset = 30;
// format materialu
constexpr int F_EPS = 0;
constexpr int F_PS = 1;
constexpr int F_PDF = 2;
// dziedziczenie
constexpr int DERV_NONE = 0;
constexpr int DERV_ADDS = 1;
constexpr int DERV_TMPL = 2;
constexpr int DERV_FIXD = 3;
constexpr int DERV_PROH = 4;
constexpr int DERV_DRUK = 5;
constexpr int DERV_COLO = 6;
constexpr auto DERV_TMPL_WER = "$c";
constexpr auto OPI_TAG = "%%MANAM-OPI ";
constexpr auto APP_NAME = _T("Manam");

constexpr int bigSize = 0x8000;   // 32kB
constexpr size_t n_size = 0x7FFF; // (bigSize-1)

enum class DrawShape : uint8_t
{
    add = ID_DRAW_ADD,
    red = ID_DRAW_RED,
    lock = ID_DRAW_LOCK,
    opis = ID_DRAW_OPIS,
    color = ID_DRAW_KOLOR,
    select = ID_DRAW_SELECT,
    caption = ID_DRAW_CAPTION,
    deadline = ID_DRAW_DEADLINE,
    space = ID_DRAW_SPACELOCKED,
};

enum class SelectMode : uint8_t
{
    none,
    size,
    move,
    dontmove, // bo zablokowane - ale nie zmieniaj selekcji
    dontsize, // bo niekratowe lub zablokowane
    netSelect,
};

enum class ToolbarMode : uint8_t
{
    normal = 0,
    czas_obow = 1,
    tryb_studia = 2,
};

enum class DocType : uint8_t // typ wyliczeniowy rodzajów dokumentów
{
    makieta,
    makieta_lib,
    grzbiet_drukowany
};

enum class SpaceMode : uint8_t
{
    avail,
    redlock,
    spacelock,
};
