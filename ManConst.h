
#pragma once

#pragma region constants
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
constexpr auto BRAK = _T("(brak)");
constexpr auto FULL = _T("(full)");
constexpr COLORREF BIALY = RGB(255, 255, 255);
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

constexpr auto DERV_TMPL_WER = "$c";
constexpr auto OPI_TAG = "%%MANAM-OPI ";
constexpr auto APP_NAME = _T("Manam");
constexpr int bigSize = 0x8000;   // 32kB
constexpr size_t n_size = 0x7FFF; // (bigSize-1)
#pragma endregion constants

#pragma region flag_enums
namespace UserRole // grupy
{ 
    constexpr uint8_t dea = 1;
    constexpr uint8_t red = 2;
    constexpr uint8_t stu = 4;
    constexpr uint8_t kie = 8;
    constexpr uint8_t adm = 16;
    constexpr uint8_t mas = 32;
}

namespace ColorId
{
    constexpr uint8_t brak = 1;
    constexpr uint8_t spot = 2;
    constexpr uint8_t full = 4;
}

namespace PaginaType // numeracja - odpowiada PK s³ownika TYP_NUMERACJI z bazy
{
    constexpr uint16_t arabic = 1;
    constexpr uint16_t roman = 2;
}
#pragma endregion flag_enums

#pragma region enums
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

enum class DervType : uint8_t
{
    none = 0,
    adds = 1,
    tmpl = 2,
    fixd = 3,
    proh = 4,
    druk = 5,
    colo = 6
};

enum class EntityType : uint8_t // typy obiektow
{
    page = 0,
    add = 1,
    opis = 2,
    addque = 3,
    page_lib = 4,
    opis_lib = 5
};

enum class StudioStatus : uint8_t // status studia
{
    brak = 0,
    jest = 1,
    nowy = 2,
    acc = 3,
    ok = 4,
    send = 5,
    filtr = 6,
    msg = 7
};
#pragma endregion enums

