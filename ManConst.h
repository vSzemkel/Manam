
#pragma once

#pragma region constants
constexpr int CLIENT_SCALE = 20;
constexpr int PRINT_VOFFSET = -10;
constexpr int MIN_COLSPERROW = 10;
constexpr int pszpalt_x = 5;               // liczba kolumn w kracie domy�lnej
constexpr int pszpalt_y = 6;               // liczba wierszy w kracie domy�lnej
constexpr int pmodcnt = 30;                // pszpalt_x * pszpalt_y
constexpr int DLGMSG_MAX_LEN = 3000;       // timeout na uzyskanie po��czenia z puli wyra�ony w milisekundach
constexpr int MIN_VALID_ADNO = 10'000'000; // minimalny prawid�owy numer ADNO
constexpr int MANAM_DEFAULT_PORT = 2501;   // domy�lny numer portu u�ywany przez aplikacj�
constexpr auto MAX_STUDIO_PATH = (_MAX_PATH - 12);
constexpr auto c_formatDaty = _T("%02d/%02d/%04d");
constexpr auto c_formatCzasu = _T("%02d/%02d/%04d %02d:%02d");
constexpr auto c_ctimeDataWs = _T("%Y%m%d"); // domy�lny format dla parametr�w web serwis�w
constexpr auto c_ctimeData = _T("%d/%m/%Y");
constexpr auto c_ctimeCzas = _T("%d/%m/%Y %R");
constexpr __time64_t ONEDAY = 86400;         // liczba sekund w dobie
constexpr __time64_t POWTSEED_0 = 946594800; // 31/12/1999 00:00 data, w stosunku do kt�rej s� obliczane powt�rki
constexpr __time64_t POWTSEED_1 = 946598400; // 31/12/1999 01:00 data, w stosunku do kt�rej s� obliczane powt�rki plus godzina
                                             // kolory
constexpr auto BRAK = _T("(brak)");
constexpr auto FULL = _T("(full)");
// gen eps
constexpr float mm2pkt = 0.2835F; // 1mm = 2.835pt
constexpr float pkt2mm = 0.3527F; // 1pt = 0.3527mm
constexpr float pkt_10m = 2.835F;
constexpr int podpisH = 6; // 2.1162 [mm]
constexpr int preview_offset = 32;

constexpr auto DERV_TMPL_WER = "$c";
constexpr auto OPI_TAG = "%%MANAM-OPI ";
constexpr auto APP_NAME = _T("Manam");
constexpr int bigSize = 0x10000;   // 64kB
constexpr size_t n_size = 0xFFFF; // (bigSize-1)
#pragma endregion constants

#pragma region namespacedconstants
namespace CManDbType // ODP.NET datatypes for unmanaged code
{
    constinit const uint8_t DbTypeByte      = 103; // OracleDbType::Byte
    constinit const uint8_t DbTypeInt32     = 112; // OracleDbType::Int32
    constinit const uint8_t DbTypeDouble    = 108; // OracleDbType::Double
    constinit const uint8_t DbTypeVarchar2  = 126; // OracleDbType::Varchar2
    constinit const uint8_t DbTypeRefCursor = 121; // OracleDbType::RefCursor
};

namespace CManDbDir // ODP.NET parameter directions for unmanaged code
{
    constinit const uint8_t ParameterIn     = 1;   // ParameterDirection::Input
    constinit const uint8_t ParameterOut    = 2;   // ParameterDirection::Output
    constinit const uint8_t ParameterInOut  = 3;   // ParameterDirection::InputOutput
    constinit const uint8_t ReturnValue     = 6;   // ParameterDirection::ReturnValue
};

namespace ManColor // Predefined colors
{
    constinit const COLORREF White = RGB(255, 255, 255);
    constinit const COLORREF Cyan = RGB(120, 255, 255);
    constinit const COLORREF Magenta = RGB(255, 100, 255);
    constinit const COLORREF Yellow = RGB(255, 255, 150);
    constinit const COLORREF RomanPage = RGB(135, 135, 135);
    constinit const COLORREF ReadOnlyDoc = RGB(200, 200, 220);
    constinit const COLORREF Black = RGB(0, 0, 0);
};
#pragma endregion namespacedconstants

#pragma region flag_enums
namespace UserRole // grupy
{ 
    constinit const uint8_t dea = 1;
    constinit const uint8_t red = 2;
    constinit const uint8_t stu = 4;
    constinit const uint8_t kie = 8;
    constinit const uint8_t adm = 16;
    constinit const uint8_t mas = 32;
}

namespace ColorId
{
    constinit const uint8_t brak = 1;
    constinit const uint8_t spot = 2;
    constinit const uint8_t full = 4;
}

namespace PaginaType // numeracja - odpowiada PK s�ownika TYP_NUMERACJI z bazy
{
    constinit const uint16_t arabic = 1;
    constinit const uint16_t roman = 2;
}
#pragma endregion flag_enums

#pragma region enums
enum class CManFormat : uint8_t // format materialu
{
    EPS = 0,
    PS  = 1,
    PDF = 2,
};

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

enum class SelectUpdateMode : uint8_t
{
    replace,
    add,
    add_range
};

enum class ToolbarMode : uint8_t
{
    normal = 0,
    czas_obow = 1,
    tryb_studia = 2,
};

enum class DocType : uint8_t // typ wyliczeniowy rodzaj�w dokument�w
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

enum class EntityType : uint8_t // typ obiektu
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

enum class PrintFormat : uint8_t // typ wydruku
{
    page = 0,
    two_pages = 1,
    doc = 2,
    all = 3,
    null = 4 // nie wybrano co si� drukuje
};
#pragma endregion enums

