#ifndef _UConsole_h_
#define _UConsole_h_

#include <Core/Core.h>

#include "Parser.h"
#include "Page.h"
#include "Cell.h"

namespace Upp {

extern byte CHARSET_DEC_VT52;   // DEC VT52 graphics character set.
extern byte CHARSET_DEC_DCS;    // DEC VT100+ line drawing character set.
extern byte CHARSET_DEC_MCS;    // DEC VT200+ multinational character set.
extern byte CHARSET_DEC_TCS;    // DEC VT300+ technical character set.

class Console {
public:
    typedef Console CLASSNAME;

    enum ConformanceLevels : byte {
        LEVEL_0 = 0,
        LEVEL_1 = 1,
        LEVEL_2 = 2,
        LEVEL_3 = 3,
        LEVEL_4 = 4
    };

    enum LEDs : int {
        LED_NUMLOCK  = 0,
        LED_CAPSLOCK = 1,
        LED_SCRLOCK  = 2,
        LED_ALL      = 3
    };

    Console();
    virtual ~Console() {}

    Event<int, bool> WhenLED;
    Event<>          WhenBell;
    Event<String>    WhenTitle;
    Event<String>    WhenOutput;
    Event<bool>      When132Column;

    void            Write(const void *data, int size, bool utf8 = true);
    void            Write(const String& s, bool utf8)       { Write(~s, s.GetLength(), utf8); }
    inline void     WriteUtf8(const String& s)              { Write(s, true);         }
    void            CheckWriteUtf8(const String& s)         { Write(s, CheckUtf8(s)); }

    Console&        Set8BitsMode(bool b = true)             { eightbits = b; return *this;    }
    bool            Is8BitsMode() const                     { return IsLevel2() && eightbits; }
    bool            Is7BitsMode() const                     { return !Is8BitsMode(); }

    Console&        SetLevel(int level);
    bool            IsLevel0() const                        { return !modes[DECANM]; }
    bool            IsLevel1() const                        { return modes[DECANM] && clevel >= LEVEL_1; }
    bool            IsLevel2() const                        { return modes[DECANM] && clevel >= LEVEL_2; }
    bool            IsLevel3() const                        { return modes[DECANM] && clevel >= LEVEL_3; }
    bool            IsLevel4() const                        { return modes[DECANM] && clevel >= LEVEL_4; }

    Console&        EnableUDK(bool b = true)                { udkenabled = b; return *this; }
    Console&        DisableUDK()                            { return EnableUDK(false);      }
    Console&        LockUDK(bool b = true)                  { udklocked = b;  return *this; }
    Console&        UnlockUDK()                             { return LockUDK(false); }
    bool            IsUDKEnabled() const                    { return udkenabled; }
    bool            IsUDKLocked() const                     { return udklocked;  }

    inline void     HardReset()                             { Reset(true);  }
    inline void     SoftReset()                             { Reset(false); }
//  void            SecureReset();

    void            AnswerBackMessage(const String& s)      { answerback = s; }

    virtual void    Serialize(Stream& s);

protected:
    const VTCell&   GetAttrs() const                        { return cellattrs;  }

    virtual void    PreParse()                              {}
    virtual void    PostParse()                             {}

    virtual void    RefreshPage(bool full = false)          {}

    virtual void    SwapPage()                              {}
    void            SetPageSize(Size sz)                    { page->SetSize(sz); }
    VTPage&         GetDefaultPage()                        { return dpage; }
    bool            IsDefaultPage() const                   { return page == &dpage; }
    VTPage&         GetAlternatePage()                      { return apage; }
    bool            IsAlternatePage() const                 { return page == &apage; }

    void            PutC(int c);
    void            PutC(const String& s1);
    void            Put(const char *s, int cnt = 1);
    void            Put(int c, int cnt = 1);
    void            PutUtf8(int c, int cnt = 1);
    void            PutRaw(const char *s, int cnt = 1);
    void            PutESC(const String& s, int cnt = 1);
    void            PutESC(int c, int cnt = 1);
    void            PutCSI(const String& s, int cnt = 1);
    void            PutCSI(int c, int cnt = 1);
    void            PutOSC(const String& s, int cnt = 1);
    void            PutOSC(int c, int cnt = 1);
    void            PutDCS(const String& s, int cnt = 1);
    void            PutDCS(int c, int cnt = 1);
    void            PutSS2(const String& s, int cnt = 1);
    void            PutSS2(int c, int cnt = 1);
    void            PutSS3(const String& s, int cnt = 1);
    void            PutSS3(int c, int cnt = 1);
    void            PutEol();
    
    void            Flush();
    void            CancelOut()                             { out.Clear(); }

    void            DisplayAlignmentTest();

    WString         AsWString(const VTPage::Line& line);

protected:
    VTPage*         page;

private:
    void            PutChar(int c);
    int             LookupChar(int c);

    void            ParseControlChars(byte c);
    void            ParseEscapeSequences(const VTInStream::Sequence& seq);
    void            ParseCommandSequences(const VTInStream::Sequence& seq);
    void            ParseDeviceControlStrings(const VTInStream::Sequence& seq);
    void            ParseOperatingSystemCommands(const VTInStream::Sequence& seq);

    bool            Convert7BitsC1To8BitsC1(const VTInStream::Sequence& seq);

    void            ClearPage(const VTInStream::Sequence& seq);
    void            ClearLine(const VTInStream::Sequence& seq);
    void            ClearTabs(const VTInStream::Sequence& seq);

    void            ReportMode(const VTInStream::Sequence& seq);
    void            ReportDeviceStatus(const VTInStream::Sequence& seq);
    void            ReportDeviceParameters(const VTInStream::Sequence& seq);
    void            ReportDeviceAttributes(const VTInStream::Sequence& seq);
    void            ReportControlFunctionSettings(const VTInStream::Sequence& seq);
    void            ReportRectAreaChecksum(const VTInStream::Sequence &seq);

    void            SelectGraphicsRendition(const VTInStream::Sequence& seq);
    void            SetGraphicsRendition(VTCell& attrs, const Vector<String>& opcodes);
    void            InvertGraphicsRendition(VTCell& attrs, const Vector<String>& opcodes);
    String          GetGraphicsRenditionOpcodes(const VTCell& attrs);

    void            ProtectAttributes(bool protect);

    void            SetCaretStyle(const VTInStream::Sequence& seq);

    void            SetProgrammableLEDs(const VTInStream::Sequence& seq);

    void            SetDeviceConformanceLevel(const VTInStream::Sequence& seq);

    void            SetUserDefinedKeys(const VTInStream::Sequence& seq);

    void            CopyRectArea(const VTInStream::Sequence& seq);
    void            FillRectArea(const VTInStream::Sequence& seq);
    void            ClearRectArea(const VTInStream::Sequence& seq, bool selective = false);
    void            SelectRectAreaAttrsChangeExtent(const VTInStream::Sequence& seq);
    void            ChangeRectAreaAttrs(const VTInStream::Sequence& seq, bool invert);

    dword           GetFillerFlags(const VTInStream::Sequence& seq) const;
    
    void            Backup(bool tpage = true, bool csets = true, bool attrs = true);
    void            Restore(bool tpage = true, bool csets = true, bool attrs = true);

    void            Reset(bool full);

    void            AlternateScreenBuffer(bool b);

private:
    VTInStream      parser;
    VTPage          dpage;
    VTPage          apage;
    VTCell          cellattrs;
    VTCell          cellattrs_backup;
    String          out;
    String          answerback;
    byte            clevel;
    bool            eightbits;
    bool            streamfill;

public:
    // ANSI, dynamic, and ISO colors support
    const int ANSI_COLOR_COUNT = 16;

    enum Colors : int {
        COLOR_BLACK = 0,
        COLOR_RED,
        COLOR_GREEN,
        COLOR_YELLOW,
        COLOR_BLUE,
        COLOR_MAGENTA,
        COLOR_CYAN,
        COLOR_WHITE,
        COLOR_LTBLACK,
        COLOR_LTRED,
        COLOR_LTGREEN,
        COLOR_LTYELLOW,
        COLOR_LTBLUE,
        COLOR_LTMAGENTA,
        COLOR_LTCYAN,
        COLOR_LTWHITE,
        COLOR_INK,
        COLOR_INK_SELECTED,
        COLOR_PAPER,
        COLOR_PAPER_SELECTED
    };

protected:
    void            ReportANSIColor(int opcode, int index, const Color& c);
    void            ReportDynamicColor(int opcode, const Color& c);
    void            ChangeColors(int opcode, const String& oscs, bool reset);
    bool            SetColorTable(int opcode, int index, String colorspec, bool ansicolor, bool reset);
    bool            SetSaveColor(int index, const Color& c);
    bool            ResetLoadColor(int index);
    void            SetISOColor(VTCell& attrs, const Vector<String>& opcodes);
    void            ResetColors();

protected:
    Color           colortable[21];

private:
    VectorMap<int, Color> savedcolors;

public:
    // DEC and xterm style caret (cursor) support.
    class Caret {
        byte        style;
        bool        blinking;
        bool        locked;
    public:
        enum : byte {
            BLOCK,
            BEAM,
            UNDERLINE
        };
        Event<>     WhenAction;
        void        Set(byte style_, bool blink);
        Caret&      Block(bool blink = true)                { Set(BLOCK, blink); return *this; }
        Caret&      Beam(bool blink = true)                 { Set(BEAM, blink);  return *this; }
        Caret&      Underline(bool blink = true)            { Set(UNDERLINE, blink); return *this; }
        Caret&      Lock(bool b = true)                     { locked = b; return *this; }
        Caret&      Unlock(bool b = true)                   { return Lock(false); }
        byte        GetStyle() const                        { return style;    }
        bool        IsBlinking() const                      { return blinking; }
        bool        IsLocked() const                        { return locked;   }
        void        Serialize(Stream& s);
        Caret();
        Caret(byte style, bool blink, bool lock);
    };

protected:
    Caret           caret;

public:
    // DEC user-defined keys (DECUDK) support
    enum UserDefinedKeys : byte {
        UDK_F1      = 11,
        UDK_F2      = 12,
        UDK_F3      = 13,
        UDK_F4      = 14,
        UDK_F5      = 15,
        UDK_F6      = 17,
        UDK_F7      = 18,
        UDK_F8      = 19,
        UDK_F9      = 20,
        UDK_F10     = 21,
        UDK_F11     = 23,
        UDK_F12     = 24,
        UDK_ALT     = 64,
        UDK_SHIFT   = 128
    };

protected:
    bool            GetUDKString(byte key, String& val);

private:
    VectorMap<byte, String> udk;
    bool            udklocked;
    bool            udkenabled;

protected:
    // Currently supported ANSI and private terminal modes.
    enum TerminalModes : byte {
        KAM = 0,
        SRM,
        CRM,
        IRM,
        LNM,
        ERM,
        DECANM,
        DECARM,
        DECAWM,
        DECBKM,
        DECCKM,
        DECCOLM,
        DECKPAM,
        DECLRMM,
        DECOM,
        DECSCLM,
        DECSCNM,
        DECST8C,
        DECTCEM,
        XTASBM,
        XTASCM,
        XTBRPM,
        XTDRAGM,
        XTANYMM,
        XTFOCUSM,
        XTSRCM,
        XTSGRMM,
        XTUTF8MM,
        XTX10MM,
        XTX11MM,
        XTSHOWSB,
        VTMODECOUNT
    };
    Bits            modes;

private:
    void            SetMode(const VTInStream::Sequence& seq, bool enable);
    static int      FindModeId(word modenum, byte modetype, byte level, bool& refresh);

    // ANSI modes.
    void            ANSIkam(bool b);
    void            ANSIcrm(bool b);
    void            ANSIirm(bool b);
    void            ANSIsrm(bool b);
    void            ANSIlnm(bool b);
    void            ANSIerm(bool b);

    // DEC private modes.
    void            DECanm(bool b);
    void            DECarm(bool b);
    void            DECawm(bool b);
    void            DECbkm(bool b);
    void            DECckm(bool b);
    void            DECcolm(bool b);
    void            DECkpam(bool b);
    void            DEClrmm(bool b);
    void            DECom(bool b);
    void            DECsclm(bool b);
    void            DECscnm(bool b);
    void            DECst8c(bool b);
    void            DECtcem(bool b);

    // Private mode extensions.
    void            XTasbm(int mode, bool b);
    void            XTanymm(bool b);
    void            XTascm(bool b);
    void            XTbrpm(bool b);
    void            XTdragm(bool b);
    void            XTfocusm(bool b);
    void            XTsgrmm(bool b);
    void            XTsrcm(bool b);
    void            XTutf8mm(bool b);
    void            XTx10mm(bool b);
    void            XTx11mm(bool b);

private:
    // Currently supported control bytes (C0, C1).
    enum class ControlId : byte {
        // C0
        NUL,
        ENQ,
        BEL,
        BS,
        HT,
        LF,
        VT,
        FF,
        CR,
        LSO,
        LSI,
        XON,
        XOFF,
        DEL,
        // C1
        IND,
        NEL,
        HTS,
        RI,
        SS2,
        SS3,
        SPA,
        EPA,
        DECID,
        ST,
        UNHANDLED
    };
    static ControlId FindControlId(byte ctl, byte level, bool& refresh);

    // Currently supported ESC, CSI, and DCS sequences.
    enum class SequenceId : byte {
        ANSICL1,
        ANSICL2,
        ANSICL3,
        CBT,
        CHA,
        CHT,
        CNL,
        CPL,
        CUB,
        CUD,
        CUF,
        CUP,
        CUU,
        DA1,
        DA2,
        DA3,
        DCH,
        DECALN,
        DECBI,
        DECCARA,
        DECDC,
        DECCRA,
        DECDHL,
        DECDWL,
        DECERA,
        DECFI,
        DECFRA,
        DECIC,
        DECID,
        DECKPAM,
        DECKPNM,
        DECLL,
        DECRARA,
        DECRC,
        DECRQM,
        DECRQSS,
        DECREQTPARM,
        DECRQCRA,
        DECSACE,
        DECSC,
        DECSCA,
        DECSCL,
        DECSCUSR,
        DECSGR,
        DECSED,
        DECSEL,
        DECSLRM,
        DECSERA,
        DECST8C,
        DECSTBM,
        DECSTR,
        DECSWL,
        DECTST,
        DECUDK,
        DECXCPR,
        DL,
        DSR,
        ECH,
        ED,
        EL,
        HOME,
        HPA,
        HPR,
        HVP,
        ICH,
        IL,
        LS1R,
        LS2,
        LS2R,
        LS3,
        LS3R,
        REP,
        RI,
        RIS,
        RM,
        S7C1T,
        S8C1T,
        SCODFK,
        SCORC,
        SCOSC,
        SCS_G0_DEC_DCS,
        SCS_G1_DEC_DCS,
        SCS_G2_DEC_DCS,
        SCS_G3_DEC_DCS,
        SCS_G0_DEC_ACS,
        SCS_G1_DEC_ACS,
        SCS_G0_ASCII,
        SCS_G1_ASCII,
        SCS_G2_ASCII,
        SCS_G3_ASCII,
        SCS_G1_LATIN1,
        SCS_G2_LATIN1,
        SCS_G3_LATIN1,
        SCS_G0_DEC_MCS,
        SCS_G1_DEC_MCS,
        SCS_G2_DEC_MCS,
        SCS_G3_DEC_MCS,
        SCS_G0_DEC_TCS,
        SCS_G1_DEC_TCS,
        SCS_G2_DEC_TCS,
        SCS_G3_DEC_TCS,
        SCS_UTF8,
        SCS_DEFAULT,
        SD,
        SGR,
        SL,
        SM,
        SR,
        SU,
        TBC,
        VPA,
        VPR,
        VT52_CUB,
        VT52_CUD,
        VT52_CUF,
        VT52_CUP,
        VT52_CUU,
        VT52_DA,
        VT52_DCS_ON,
        VT52_DCS_OFF,
        VT52_DECANM,
        VT52_ED,
        VT52_EL,
        VT52_HOME,
        VT52_RI,
        IGNORED,
        UNHANDLED
    };
    static SequenceId FindSequenceId(byte type, byte level, const VTInStream::Sequence& seq, bool& refresh);

public:
    // Terminal legacy character sets ("G-set") support.
    class Charsets {
        byte  g[4], d[4];
        byte  l, r;
        byte  ss;
    public:
        Charsets&   G0toGL()                                { l = 0; return *this; }
        Charsets&   G1toGL()                                { l = 1; return *this; }
        Charsets&   G2toGL()                                { l = 2; return *this; }
        Charsets&   G3toGL()                                { l = 3; return *this; }
        Charsets&   G1toGR()                                { r = 1; return *this; }
        Charsets&   G2toGR()                                { r = 2; return *this; }
        Charsets&   G3toGR()                                { r = 3; return *this; }

        Charsets&   G0(byte c)                              { g[0] = c; return *this; }
        Charsets&   G1(byte c)                              { g[1] = c; return *this; }
        Charsets&   G2(byte c)                              { g[2] = c; return *this; }
        Charsets&   G3(byte c)                              { g[3] = c; return *this; }
        Charsets&   SS(byte c)                              { ss   = c; return *this; }

        byte        Get(int c, bool allowgr = true) const   { return allowgr && c > 0x7F ? g[r] : g[l]; }
        
        byte        GetGL() const                           { return g[l]; }
        byte        GetGR() const                           { return g[r]; }
        byte        GetG0() const                           { return g[0]; }
        byte        GetG1() const                           { return g[1]; }
        byte        GetG2() const                           { return g[2]; }
        byte        GetG3() const                           { return g[3]; }
        byte        GetSS() const                           { return ss;   }

        void        ConformtoANSILevel1();
        void        ConformtoANSILevel2();
        void        ConformtoANSILevel3();

        Charsets&   ResetG0()                               { g[0] = d[0]; return *this; }
        Charsets&   ResetG1()                               { g[1] = d[1]; return *this; }
        Charsets&   ResetG2()                               { g[2] = d[2]; return *this; }
        Charsets&   ResetG3()                               { g[3] = d[3]; return *this; }

        void        Reset();
        void        Serialize(Stream& s);

        Charsets(byte defcset = CHARSET_UNICODE);
        Charsets(byte g0, byte g1, byte g2, byte g3);
    };

    void            SetCharsets(Charsets chrsets)           { charsets = chrsets; }
    const Charsets& GetCharsets() const                     { return charsets;    }

protected:
    int             ConvertToUnicode(int c, byte chrset);
    int             ConvertToCharset(int c, byte chrset);

private:
    Charsets        charsets;
    Charsets        charsets_backup;
};

INITIALIZE(DECCharsets);
}
#endif
