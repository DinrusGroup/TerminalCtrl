#include "Console.h"

#define LLOG(x)	// RLOG("Console: " << x)

namespace Upp {

void Console::ParseOperatingSystemCommands(const VTInStream::Sequence& seq)
{
	LLOG(Format("OSC: %s", seq.payload));

	int opcode = seq.GetInt(1, 0);

	switch(opcode) {
	case 0:		// Set window titile
	case 2:		// Set window title (and icon name)
		WhenTitle(seq.GetStr(2));
		break;
	case 4:		// Set ANSI colors
	case 10:	// Set dynamic color (ink)
	case 11:	// Set dynamic color (paper)
	case 17:	// Set dynamic color (selection ink)
	case 19:	// Set dynamic color (selection paper)
	case 104:	// Reset ANSI colors
	case 110:	// Reset dynamic color (ink)
	case 111:	// Reset dynamic color (paper)
	case 117:	// Reset dynamic color (selection ink)
	case 119:	// Reset dynamic color (selection paper)
		ChangeColors(opcode, seq.payload, opcode > 100);
		break;
	case 8:		// Explicit hyperlinks protocol.
		ParseHyperlinks(seq);
		break;
	case 444:	// Parse jexer images (raw, png, jpg)
		ParseJexerGraphics(seq);
		break;
	default:
		LLOG(Format("Unhandled OSC opcode: %d", opcode));
		break;
	}
}

void Console::ParseJexerGraphics(const VTInStream::Sequence& seq)
{
	// For more information on Jexer image protocol, see:
	// https://gitlab.com/klamonte/jexer/wikis/jexer-images

	if(!(consoleflags & FLAG_JEXER))
		return;
	
	int type = seq.GetInt(2, Null);
	if(type > 2 || IsNull(type))	// V1 defines 3 types (0-based).
		return;

	Value data;
	Size  isz = Null;
	bool  scroll = false;

	if(type == 0) {	// Bitmap
		isz.cx = min(seq.GetInt(3, 0), 10000);
		isz.cy = min(seq.GetInt(4, 0), 10000);
		scroll = seq.GetInt(5, 0) > 0;
		data.Add(seq.GetStr(6));
		data.Add(isz);
	}
	else { // Other image formats (jpg, png, etc.)
		scroll = seq.GetInt(3, 0) > 0;
		data.Add(seq.GetStr(4));
		data.Add(isz);
	}

	cellattrs.Hyperlink(false);
	
	RenderImage(data, scroll);
}

void Console::ParseHyperlinks(const VTInStream::Sequence& seq)
{
	// For more information on explicit hyperlinks, see:
	// https://gist.github.com/egmontkob/eb114294efbcd5adb1944c9f3cb5feda

	if(!(consoleflags & FLAG_HYPERLINKS))
		return;

	constexpr const int MAX_URI_LENGTH = 2084;

	String uri = seq.GetStr(3);

	if(IsNull(uri) || uri.GetLength() > MAX_URI_LENGTH) {
		cellattrs.Hyperlink(false);
		cellattrs.data = 0;
	}
	else {
		uri = UrlDecode(uri);
		cellattrs.Image(false);
		cellattrs.Hyperlink(true);
		cellattrs.data = GetHashValue(uri);
		RenderHyperlink(uri);
	}
}
}