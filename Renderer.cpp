#include "Terminal.h"

#define LLOG(x)	   // RLOG("Terminal: " << x)
#define LTIMING(x) // RTIMING(x)

namespace Upp {

// The optimizwd rendering classes below (VT[Rect/Cell]Renderer)
// are adopted from Upp's own code, and modified to some extent.
// Credits should go to the U++ team.
// See: uppsrc/CtrlLib/LineEdit.cpp.

class sVTRectRenderer {
	Draw& w;
	Rect  cr;
	Color color, bkg;
	bool  transparent;

public:
	void DrawRect(const Rect& r, Color c)
	{
		LTIMING("VTRectRencderer::DrawRect");

		if(cr.top == r.top && cr.bottom == r.bottom && cr.right == r.left && c == color)
		{
			cr.right = r.right;
			return;
		}
		Flush();
		cr = r;
		color = c;
	}
	
	void DrawRect(int x, int y, int cx, int cy, Color c)
	{
		DrawRect(RectC(x, y, cx, cy), c);
	}
	
	void Flush()
	{
		if(IsNull(cr) || (!transparent && color == bkg))
			return;

		LTIMING("VTRectRenderer::Flush");

		w.DrawRect(cr, color);
		cr = Null;
	}
	
	sVTRectRenderer(Draw& w, const Color& c, bool b)
	: w(w)
	, bkg(c)
	, transparent(b)
	, cr(Null)
	, color(Null)
	{
	}
	
	~sVTRectRenderer()
	{
		Flush();
	}
};

class sVTCellRenderer {
	Draw&		w;
	const bool&	blink;
	Point		pos;
	Point		p1, p2;
	Point		p3, p4;
	Vector<int>	dx;
	int			dxcount;
	Font		font;
	word		sgr;
	Color		color;
	WString		text;

public:
	void DrawCell(int x, int y, const VTCell& cell, Size fsz, Color c, bool link, bool show)
	{
		LTIMING("VTCellRenderer::DrawCell");
		
		dxcount  = dx.GetCount();
		bool overline = cell.IsOverlined() && show;
		
		if(pos.y != y || pos.x >= x || sgr != cell.sgr || color != c || !dxcount) {
			Flush();
			pos   = Point(x, y);
			color = c;
			sgr   = cell.sgr;
			font.Bold(cell.IsBold());
			font.Italic(cell.IsItalic());
			font.Strikeout(cell.IsStrikeout());
			font.Underline(cell.IsUnderlined());

			if(link)
				p2 = p1 = Point(x, y + fsz.cy);

			if(overline)
				p3 = p4 = Point(x, y);
		}

		if(cell.chr <= 0x20
			|| cell.IsImage()
			|| cell.IsConcealed()
			|| (!show && (cell.IsBlinking() && blink))) {
			if(dxcount)
				dx.Top() += fsz.cx;
		}
		else {
			text.Cat((int) cell.chr);
			dx.Add(fsz.cx);
		}

		if(link)
			p2.x += fsz.cx;

		if(overline)
			p4.x += fsz.cx;
			
	}
	
	void Flush()
	{
		if(text.IsEmpty()) return;
		
		LTIMING("VTCellRenderer::Flush");
		
		w.DrawText(pos.x, pos.y, text, font, color, dx);
		
		if(p1.x < p2.x) // Hyperlink underline
			w.DrawLine(p1, p2, PEN_DOT, color);
		
		if(p3.x < p4.x) // Text overline
			w.DrawLine(p3, p4, PEN_SOLID, color);
		
		dx.Clear();
		p1 = p2 = Null;
		dxcount = 0;
		pos.y = Null;
		text.Clear();
	}
	
	sVTCellRenderer(Draw& dw, const Font& ft, const bool& b)
	: w(dw)
	, blink(b)
	, font(ft)
	, p1(Null)
	, p2(Null)
	, p3(Null)
	, p4(Null)
	, dxcount(0)
	, sgr(VTCell::SGR_NORMAL)
	{
	}
	
	~sVTCellRenderer()
	{
		Flush();
	}
};

void Terminal::Paint0(Draw& w, bool print)
{
	GuiLock __;
	
	int  pos = GetSbPos();
	Size wsz = GetSize();
	Size psz = GetPageSize();
	Size fsz = GetFontSize();
	Font fnt = font;
	ImageParts imageparts;
	Color bc = colortable[COLOR_PAPER];
	
	w.Clip(wsz);

	sVTCellRenderer tr(w, fnt, blinking);
	sVTRectRenderer rr(w, bc, nobackground);

	LTIMING("Terminal::Paint");
	
	if(!nobackground)
		w.DrawRect(wsz, bc);
	for(int i = pos; i < min(pos + psz.cy, page->GetLineCount()); i++) {
		int y = i * fsz.cy - (fsz.cy * pos);
		const VTLine& line = page->FetchLine(i);
		if(!line.IsVoid() && w.IsPainting(0, y, wsz.cx, fsz.cy)) {
			int cellcount = line.GetCount();
			int pass = 0;
			int end  = 2;
			while(pass < end) {
				for(int j = 0; j < psz.cx; j++) {
					const VTCell& cell = j < cellcount ? line[j] : GetAttrs();
					bool lnk = hyperlinks && cell.IsHyperlink();
					Color ink, paper;
					SetInkAndPaperColor(cell, ink, paper);
					int x = j * fsz.cx;
					bool highlight = IsSelected(Point(j, i));
					if(pass == 0) {
						if(!nobackground
							|| !IsNull(cell.paper)
							|| highlight
							|| cell.IsInverted()
							|| lnk && cell.data == activelink
							|| print) {
							int fcx = (j == psz.cx - 1) ? wsz.cx - x : fsz.cx;
							rr.DrawRect(x, y, fcx, fsz.cy, highlight ? colortable[COLOR_PAPER_SELECTED] : paper);
						}
					}
					else
					if(pass == 1) {
						if(cell.IsImage())
							AddImagePart(imageparts, x, y, cell, fsz);
						bool show = highlight || !blinking || print;
						tr.DrawCell(x, y, cell, fsz, highlight ? colortable[COLOR_INK_SELECTED] : ink, lnk, show);
					}
				}
				if(pass == 0) {
					rr.Flush();
				}
				else
				if(pass == 1) {
					tr.Flush();
				}
				pass++;
			}
		}
	}

	// Paint inline images, if any
	if(imageparts.GetCount())
		PaintImages(w, imageparts, fsz);
	
	// Paint a steady (non-blinking) caret, if enabled.
	if(modes[DECTCEM] && HasFocus() && (print || !caret.IsBlinking()))
		w.DrawRect(GetCaretRect(), InvertColor);

	// Hint new size.
	if(sizehint && hinting) {
		auto hint = GetSizeHint(GetView(), psz);
		DrawFrame(w ,hint.b.Inflated(8), LtGray);
		w.DrawRect(hint.b.Inflated(7), SColorText);
		w.DrawText(hint.b.left, hint.b.top, hint.a, StdFont(), SColorPaper);
	}
	w.End();
}


void Terminal::AddImagePart(ImageParts& parts, int x, int y, const VTCell& cell, Size fsz)
{
	// TODO: Move this to cell renderer.
	
	dword id = cell.chr;
	Point coords = Point(x, y);
	Point pt(HIWORD(cell.data), LOWORD(cell.data));
	Rect  ir = RectC(pt.x * fsz.cx, pt.y * fsz.cy, fsz.cx, fsz.cy);
	if(!parts.IsEmpty()) {
		ImagePart& part = parts.Top();
		if(id == part.a && part.b.y == coords.y && part.c.right == ir.left) {
			part.c.right = ir.right;
			return;
		}
	}

	parts.Add(MakeTuple(id, coords, ir));
}

void Terminal::PaintImages(Draw& w, ImageParts& parts, const Size& fsz)
{
	LTIMING("Terminal::PaintImages");
	
	Color ink, paper;
	SetInkAndPaperColor(GetAttrs(), ink, paper);

	for(const ImagePart& part : parts) {
		const dword& id = part.a;
		const Point& pt = part.b;
		const Rect&  rr = part.c;
		Rect r = Rect(pt, rr.GetSize());
		if(w.IsPainting(r)) {
			ImageData imd = GetCachedImageData(id, Null, fsz);
			if(!IsNull(imd.image)) {
				imd.fitsize *= fsz;
				imd.paintrect = rr;
				imgdisplay->Paint(w, r, RawPickToValue(pick(imd)), ink, paper, 0);
			}
		}
	}
	
	parts.Clear();
}

void Terminal::RenderImage(const Value& data, bool scroll)
{
	bool notsixel = IsValueArray(data);
	
	if(WhenImage) {
		if(notsixel)
			WhenImage(Base64Decode(data[0]));
		else
			WhenImage(data);
		return;
	}

	LTIMING("Terminal::RenderImage");

	dword id = GetHashValue(data);
	ImageData imd = GetCachedImageData(id, data, GetFontSize());
	if(!IsNull(imd.image)) {
		page->AddImage(imd.fitsize, id, scroll, notsixel);
		RefreshDisplay();
	}
}

// Shared image data cache support.

static StaticMutex sImageCacheLock;
static LRUCache<Terminal::ImageData> sImageDataCache;
static int sCachedImageMaxSize =  1024 * 1024 * 4 * 128;
static int sCachedImageMaxCount =  256000;

struct sVTImageDataMaker : LRUCache<Terminal::ImageData>::Maker {
	dword	id;
	String	data;
	Size    imgsize;
	Size	fontsize;
	
	String Key() const override
	{
		StringBuffer h;
		RawCat(h, id);
		return h;
	}
	
	int Make(Terminal::ImageData& imagedata) const override
	{
		LTIMING("Terminal::ImageDataMaker");

		Image img = StreamRaster::LoadStringAny(data);
		if(IsNull(img))
			return 0;
		if(IsNull(imgsize))
			imagedata.image = img;
		else {
			Size ssz = imgsize;
			Size isz = img.GetSize();
			if(ssz.cx <= 0) ssz.cx = isz.cx;
			if(ssz.cy <= 0) ssz.cy = isz.cy;
			imagedata.image = Rescale(img, ssz);
		}
		imagedata.fitsize = ToCellSize(imagedata.image.GetSize());
		return imagedata.image.GetLength() * 4;
	}
	
	Size ToCellSize(Sizef isz) const
	{
		isz = isz / Sizef(fontsize);
		return Size(fround(isz.cx), fround(isz.cy));
	}
};

Terminal::ImageData Terminal::GetCachedImageData(dword id, const Value& data, const Size& fsz)
{
	Mutex::Lock __(sImageCacheLock);
	
	LTIMING("Terminal::GetCachedImageData");

	sVTImageDataMaker im;
	im.id = id;
	if(IsValueArray(data)) { // Png, jpg, bmp, etc.
		im.data = Base64Decode(data[0]);
		im.imgsize = (Size) data[1];
	}
	else { // Sixel
		im.data = data;
		im.imgsize.SetNull();
	}
	im.fontsize = fsz;
	sImageDataCache.Shrink(sCachedImageMaxSize, sCachedImageMaxCount);
	return sImageDataCache.Get(im);
}

void Terminal::ClearImageCache()
{
	Mutex::Lock __(sImageCacheLock);
	sImageDataCache.Clear();
}

void Terminal::SetImageCacheMaxSize(int maxsize, int maxcount)
{
	Mutex::Lock __(sImageCacheLock);
	sCachedImageMaxSize  = max(1, maxsize);
	sCachedImageMaxCount = max(1, maxcount);
}

void Terminal::RenderHyperlink(const Value& uri)
{
	GetCachedHyperlink(GetHashValue(uri), uri);
}

// Shared hyperlink cache support.

static StaticMutex sLinkCacheLock;
static LRUCache<String> sLinkCache;
static int sCachedLinkMaxSize = 2084 * 100000;
static int sCachedLinkMaxCount = 100000;

struct sVTHyperlinkDataMaker : LRUCache<String>::Maker {
	dword  id;
	String url;

	String Key() const override
	{
		StringBuffer h;
		RawCat(h, id);
		return h;
	}

	int Make(String& link) const override
	{
		LTIMING("Terminal::HyperlinkDataMaker");
		
		link = url;
		return link.GetLength();
	}
};

String Terminal::GetCachedHyperlink(dword id, const Value& data)
{
	Mutex::Lock __(sLinkCacheLock);

	LTIMING("Terminal::GetCachedHyperlink");

	sVTHyperlinkDataMaker hm;
	hm.id  = id;
	hm.url = data;
	sLinkCache.Shrink(sCachedLinkMaxSize, sCachedLinkMaxCount);
	return sLinkCache.Get(hm);
}

void Terminal::ClearHyperlinkCache()
{
	Mutex::Lock __(sLinkCacheLock);
	sLinkCache.Clear();
}

void Terminal::SetHyperlinkCacheMaxSize(int maxcount)
{
	Mutex::Lock __(sLinkCacheLock);
	sCachedLinkMaxSize  = max(2084, maxcount * 2084);
	sCachedLinkMaxCount = max(1, maxcount);
}

// Image display support.

class NormalImageCellDisplayCls : public Display {
public:
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
	{
		const auto& idata = q.To<Terminal::ImageData>();

		if(!IsNull(idata.image))
			w.DrawImage(r.left, r.top, idata.image, idata.paintrect);
	}
	virtual Size GetStdSize(const Value& q) const
	{
		const auto& idata = q.To<Terminal::ImageData>();
		return idata.image.GetSize();
	}
};

const Display& NormalImageCellDisplay() { return Single<NormalImageCellDisplayCls>(); }

class ScaledImageCellDisplayCls : public Display {
public:
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
	{
		const auto& idata = q.To<Terminal::ImageData>();
		
		if(!IsNull(idata.image))
			w.DrawImage(r, CachedRescale(idata.image, idata.fitsize), idata.paintrect);
	}
	virtual Size GetStdSize(const Value& q) const
	{
		const auto& idata = q.To<Terminal::ImageData>();
		return idata.image.GetSize();
	}
};

const Display& ScaledImageCellDisplay() { return Single<ScaledImageCellDisplayCls>(); }
}