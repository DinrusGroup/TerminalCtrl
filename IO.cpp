#include "Terminal.h"

#define LLOG(x)		// RLOG("Terminal: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

Terminal& Terminal::SetLevel(int level)
{
	SoftReset();

	clevel = level;

	switch(level) {
	case LEVEL_0:
		DECanm(false);
		break;
	case LEVEL_1:
	case LEVEL_2:
	case LEVEL_3:
	case LEVEL_4:
	default:
		break;
	}
	return *this;
}

void Terminal::Reset(bool full)
{
	LLOG("Performing " << (full ? "full" : "soft") << " reset...");
	
	if(full) {
		AlternateScreenBuffer(false);
		DECcolm(false);
		dpage.Reset();
		apage.Reset();
		gsets_backup.Reset();
		cellattrs_backup = Null;
		dpage.WhenScroll();
	}

	gsets.Reset();

	cellattrs.Reset();

	udk.Clear();
	
	modes.Clear();
	modes.Set(SRM);
	modes.Set(DECTCEM);
	modes.Set(DECANM);
	modes.Set(DECAWM);
	modes.Set(DECSDM);

	dpage.SetTabs(8);
	dpage.Displaced(false);
	dpage.AutoWrap(true);
	dpage.ReverseWrap(false);

	apage.SetTabs(8);
	apage.Displaced(false);
	apage.AutoWrap(true);
	apage.ReverseWrap(false);
	
	caret = Caret();
	
	CancelOut();
}

void Terminal::Backup(bool tpage, bool csets, bool attrs)
{
	if(tpage)
		page->Backup();
	if(attrs)
		cellattrs_backup = cellattrs;
	if(csets) {
		gsets_backup  = gsets;
		gsets.Reset();
	}
}

void Terminal::Restore(bool tpage, bool csets, bool attrs)
{
	if(tpage)
		page->Restore();
	if(attrs) {
		cellattrs = cellattrs_backup;
		cellattrs_backup = Null;
	}
	if(csets) {
		gsets = gsets_backup;
		gsets_backup.Reset();
	}
}

void Terminal::PutChar(int c)
{
	VTCell cell = cellattrs;
	cell.chr = LookupChar(c);
	if(modes[IRM])
		page->InsertCell(cell);
	else
		page->AddCell(cell);
}

void Terminal::PreParse()
{
	if(delayedrefresh && (!lazyresize || !resizing)) {
		if(!ExistsTimeCallback(TIMEID_REFRESH)) { // Don't cancel a pending refresh.
			SetTimeCallback(16, THISFN(DoDelayedRefresh), TIMEID_REFRESH);
		}
	}
}

void Terminal::PostParse()
{
	if(!delayedrefresh)
		RefreshDisplay();
}

void Terminal::Write(const void *data, int size, bool utf8)
{
	if(size > 0) {
		PreParse();
		parser.Parse(data, size, utf8);
		PostParse();
	}
}

void Terminal::Flush()
{
	WhenOutput(out);
	if(!modes[SRM]) // Local echo on/off.
		WriteUtf8(out);
	out = Null;
}

void Terminal::PutC(int c)
{
	if(Is8BitMode())
		out.Cat(c);
	else
	if(c >= 0x80 && 0x9F >= c) {
		out.Cat(0x1B);
		out.Cat(c - 0x40);
	}
	else
	if(!IsLevel2() && c >= 0xA0 && 0xFF >= c)
		out.Cat(c & 0x7F);
	else
		out.Cat(c);
	// Don't flush
}

void Terminal::PutC(const String& s)
{
	for(const auto& c: s)
		PutC(c);
}

void Terminal::Put(const String& s, int cnt)
{
	while(cnt-- > 0)
		PutC(s);
	Flush();
}

void Terminal::Put(int c, int cnt)
{
	while(cnt-- > 0)
		PutC(c);
	Flush();
}

void Terminal::PutUtf8(int c, int cnt)
{
	String s;
	
	word code = c;
	if(code < 0x80)
		s.Cat(code);
	else
	if(code < 0x800) {
		s.Cat(0xc0 | (code >> 6));
		s.Cat(0x80 | (code & 0x3f));
	}
	else
	if((code & 0xFF00) == 0xEE00)
		s.Cat(code);
	else {
		s.Cat(0xe0 | (code >> 12));
		s.Cat(0x80 | ((code >> 6) & 0x3f));
		s.Cat(0x80 | (code & 0x3f));
	}
	PutRaw(s, cnt);
}

void Terminal::PutRaw(const String& s, int cnt)
{
	while(cnt-- > 0)
		out.Cat(s);
	Flush();
}

void Terminal::PutESC(const String& s, int cnt)
{
	LLOG("PutESC() -> " << s);
	
	while(cnt-- > 0) {
		PutC(0x1B);
		PutC(s);
	}
	Flush();
}

void Terminal::PutESC(int c, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x1b);
		Put(c);
	}
	Flush();
}

void Terminal::PutCSI(const String& s, int cnt)
{
	LLOG("PutOSC() -> " << s);
	
	while(cnt-- > 0) {
		PutC(0x9B);
		PutC(s);
	}
	Flush();
}

void Terminal::PutCSI(int c, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x9B);
		PutC(c);
	}
	Flush();
}

void Terminal::PutOSC(const String& s, int cnt)
{
	LLOG("PutOSC() -> " << s);
	
	while(cnt-- > 0) {
		PutC(0x9D);
		PutRaw(s);
		PutC(0x9C);
	}
	Flush();
}

void Terminal::PutOSC(int c, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x9D);
		PutC(c);
		PutC(0x9C);
	}
	
	Flush();
}

void Terminal::PutDCS(const String& s, int cnt)
{
	LLOG("PutDCS() -> " << s);
	
	while(cnt-- > 0) {
		PutC(0x90);
		PutRaw(s);
		PutC(0x9C);
	}
	Flush();
}

void Terminal::PutDCS(int c, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x90);
		PutC(c);
		PutC(0x9C);
	}
	Flush();
}

void Terminal::PutSS2(const String& s, int cnt)
{
	LLOG("PutSS2() -> " << s);
	
	while(cnt-- > 0) {
		PutC(0x8E);
		PutC(s);
	}
	Flush();
}

void Terminal::PutSS2(int c, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x8E);
		PutC(c);
	}
	Flush();
}

void Terminal::PutSS3(const String& s, int cnt)
{
	LLOG("PutSS3() -> " << s);
	
	while(cnt-- > 0) {
		PutC(0x8F);
		PutC(s);
	}
	Flush();
}

void Terminal::PutSS3(int c, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x8F);
		PutC(c);
	}
	Flush();
}

void Terminal::PutEncoded(const String& s, bool noctl)
{
	LTIMING("Terminal::PutEncoded");

	String txt, buf = s;

	if(!modes[LNM])
		buf.Replace("\n", "\r");

	for(int c : buf) {
		c = ConvertToCharset(c, gsets.Get(c, IsLevel2()));
		if(!noctl		||
			IsSpace(c)	||
				(c >= 0x20 && c <= 0xFFFF))
					txt.Cat(c == DEFAULTCHAR ? '?' : c);
	}
	PutRaw(txt);
}

void Terminal::PutEncoded(const WString& s, bool noctl)
{
	PutEncoded(ToUtf8(s), noctl);
}

void Terminal::PutEol()
{
	Put(modes[LNM] ? "\r\n" : "\r");
}

void Terminal::Serialize(Stream& s)
{
	GuiLock __;

	int version = 1;
	s / version;
	
	if(version >= 1) {
		s % clevel;
		s % charset;
		s %	eightbit;
		s % font;
		s % caret;
		s % reversewrap;
		s % keynavigation;
		s % legacycharsets;
		s % alternatescroll;
		s % wheelstep;
		s % userdefinedkeys;
		s % userdefinedkeyslocked;
		s % metakeyflags;
		s % windowactions;
		s % windowreports;
		s % sixelimages;
		s % jexerimages;
		s % iterm2images;
		s % hyperlinks;
		s % delayedrefresh;
		s % lazyresize;
		s % sizehint;
		s % nobackground;
		s % intensify;
		s % blinkingtext;
		s % blinkinterval;
		s % dynamiccolors;
		s % adjustcolors;
		s % lightcolors;
		s % gsets;
		s % dpage;
		s % apage;
		for(int i = 0; i < MAX_COLOR_COUNT; i++)
			s % colortable[i];
	}

	if(s.IsLoading()) {
		SetLevel(clevel);
		Layout();
	}
}
}