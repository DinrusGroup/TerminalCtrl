# Terminal Package for Ultimate++

*Copyright © 2019, İsmail  Yılmaz*

E-mail: iylmz.iylmz@gmail.com

![From left to right: Emacs, Lynx, mapscii](https://github.com/ismail-yilmaz/upp-components/blob/master/CtrlLib/Images/Terminal-Screenshot-1.png)
## Introduction

Terminal package is a flexible, easy-to-use yet powerful cross-platform virtual terminal emulation library and widget written in C/C++ for [Ultimate++](https://www.ultimatepp.org/).

It is not based on existing terminal emulation libraries. It is designed from the ground up with modularity and maintainability in mind. In this respect the package consists of several re-usable classes, only one being the Terminal widget. (See the *Classes* section for the brief descriptions of these classes.)

A short video demonstration of Terminal widget:
https://vimeo.com/349761874
	
## Requirements

- Ultimate++ (ver. >= 2019.1)
- POSIX, Windows (and probably MacOS, though not tested.)
- A decent enough C/C++ compiler that supports at least C++11. (GCC/CLANG/MinGW/Msc)
- Snacks & beer.

## Highlights

- **Terminal package completely separates the virtual terminal from the pseudo-terminal process (pty)**.
As a result, Terminal package and its Terminal ctrl are not bound by any platform-specific pty implementation. Instead, they are decoupled, and an optional pty process class, PtyProcess, is provided with tha package as the default option. In this way, using the ctrl on any platform supported by Ultimate++, directly as a front-end for some other terminal based services, such as SSH or TELNET, etc., has become possible.
This point is demonstrated with one of the provided  examples: While the PtyProcess is currently not available on Windows (it's a TODO), Terminal widget can be compiled, run, and used on Windows, or on other supported platforms, as an SSH terminal with no code modification required on Windows and Linux. (See the *Examples* section.)

- **Terminal package is designed with simplicity in mind.**
 A fully-fledged terminal emulation requires less than 50 sLoC. In fact, the first basic example provided with the package is only a single file with 31 sLoC, and it can run complex/heavy applications such as GNU Emacs, vim, Lynx web browser (with mouse tracking), and even [mapscii](https://github.com/rastapasta/mapscii), an OpenStreetMap implementation for  [xterm](https://invisible-island.net/xterm/) compatible virtual terminal emulators, with ease. (See the *Examples* section) 
- **Terminal package combines simplicity with configurability.** 
Although it is easy to use and requires very little coding, Terminal ctrl is by no means restrictive. It it highly configurable.
- **Terminal widget is a regular ctrl.**
It is derived from Upp::Ctrl, and is following the same basic rule: *Everthing belongs somewhere*. It supports most of the generic Ctrl methods where applicable or makes sense. Of course, If you are determined enough, you can even do some “interesting” things, such as adding Terminal instances to a TreeCtrl or ArrayCtrl. ;)
- ***Everything belongs somewhere* rule runs through the heart of Terminal package.**
There are no manual memory allocations/deallocations, no new/delete pairs, and no smart/not-so-smart/shared pointers in the code; only the containers, and extensive application of the [RAII](https://www.wikiwand.com/en/Resource_acquisition_is_initialization) principle.

- **Terminal widget can also run inside a web browser such as Firefox and Chromium, or their derivatives.**
  Thanks to Ultimate++ team, it is possible to run U++ GUI applications from within a web browser that supports HTML-5 canvas and websockets. And Terminal package is no exception. Applications using Terminal ctrl can basically turn into a remote terminal that can be accessed via any decent web browser (even from a smartphone!) if compiled with the TURTLE flag. (See the *Examples* section).
- **Terminal package has a** [BSD 3-Clause](https://en.wikipedia.org/wiki/BSD_licenses?oldformat=true#3-clause_license_%28%22BSD_License_2.0%22,_%22Revised_BSD_License%22,_%22New_BSD_License%22,_or_%22Modified_BSD_License%22%29) **license**.

## Features

*Note that below list is only a summary of the currently supported features. Technical specifications and documentation will be available soon. (I'm a lazy guy.) In the meantime you can examine the supported sequences, modes, and control bytes by looking into the "Terminal/Tables.cpp"* file.  (And don't worry, the file is in a "fairly human-readable" form.)

- Supports whatever platform Ultimate++ supports. (Linux, Windows, MacOS).
- Supports VT52/VT1xx/VT2xx, partial VT4XX/5XX, and xterm emulation modes.
- Supports user configurable device conformance levels (1, 2, 3, 4, and 0 as VT52 emulation).
- Supports both 7-bits and 8-bits I/O.
- Supports Unicode/UTF8.
- Supports user configurable, legacy “g-set” (G0/G1/G2/G3), and related shifting functions (LS0/LS1/LS1R/LS2/LS2R/LS3/LS3R/SS2/SS3).
- Supports ANSI conformance levels.
- Supports various terminal state, device, and mode reports.
- Supports DEC VT52 graphics charset, VT1xx line-drawing charset, VT2xx multinational charset, and VT3xx technical charset.
- Supports VT52/VT1xx/VT2xx keyboard emulation with function keys.
- Supports UDK (DEC’s user-defined function keys feature).
- Supports user configurable blinking text and blink interval.
- Supports ANSI colors (16 colors palette).
- Supports ISO colors (256 colors palette).
- Supports ISO direct/true color mode (16 million colors) via TRUECOLOR compiler flag.
- Supports xterm dynamic colors (dynamic ink/paper/selection colors).
- Supports bright colors.
- Supports background color erase (BCE).
- Supports transparency (i.e. allows background images).
- Supports VT4xx rectangular area operations: copy, invert, fill. erase.
- Supports VT4xx rectangular area checksum calculation and reporting.
- Supports both DEC and ANSI style selective erases.
- Supports alternate screen buffer.
- Supports history/scrollback buffer.
- Has a user switchable scrollbar.
- Supports xterm style alternate scroll.
- Supports resize (and optional lazy resize to reduce flicker on network terminals such as SSH-based ones).
- Supports both immediate display refresh and delayed (buffered) display refresh.
- Supports xterm style mouse tracking: button, wheel, motion, focus in/out events.
- Supports user configurable cursor styles (block, beam, underscore, blinking/steady).
- Supports cursor locking.
- Supports clipboard operations (copy/paste/select all - including history buffer) and basic drag-and-drop ops.
- Supports bracketed paste mode.
- Has a predefined yet programmable context menu (right mouse button menu).
- Supports window titles.
- Supports bell notifications.
- Supports VT1xx LEDs.
- Supports size hint.
- Supports Ultimate++ style data serialization.
- Supports per-widget customization (i.e no global variables or properties are used).
- Includes a Terminal.usc file for TheIDE’s layout editor.


## Classes
This section briefly describes the classes of the Terminal package. Some of them have a stable and/or growing APIs (e.g. PtyProcess, VTInStream, Console, Terminal) and the others’ (VTPage, VTCell) are subject to change..

*Full API documentation is high on my TODO list.*

### VTInStream
A reliable and high-performance  VT500 series "lexical" parser designed specifically for handling
DEC & ANSI escape sequences. It can easily handle CTL, ESC, CSI, DCS, OSC sequences in both
7-bits and 8-bits forms, and allows switching between UTF8 and non-UTF8 modes on-the-fly
(e.g. using Upp::CheckUtf8() heuristic method). It is actually a finite state machine built upon a very
fast input stream object (Upp::MemReadStream), and is based on [the UML state diagram provided
by Paul-Flo Williams](https://vt100.net/emu/dec_ansi_parser).

#### Notes:
- VTInstream is separately available. All you need to do is include the "Terminal/Parser.h" file in
your application

#### Related files:
	Parser.h, Parser.cpp

#### Dependencies:
None other than Upp/Core

---
### VTPage
This class represents the display logic of a virtual terminal. It is responsible for page, line, cell, and cursor manipulation. It supports both absolute and relative positioning, horizontal and vertical margins, horizontal and vertical scroll, various cell, line and page erasure modes, various rectangular area operations such as copy, fill, erase, invert, and checksum reporting, etc. It is also responsible for history (scrollback) buffer management.

#### Notes:
- VTPage is separately available, but its API is currently unstable.


#### Related files:
	Page.h, Page.cpp, Cell.h, Cell.cpp

#### Dependencies:
VTCell and Upp/Core

----
### Console 
This is the actual virtual terminal emulator. It processes the terminal sequences pumped by the VTInStream, by translating those events into page logic, using lookup tables, and finally commanding the VTPage instance to manipulate the terminal display as per request.

#### Notes:
- Console is technically a back-end for, or head-less version of Terminal widget.
- It does everyhing except drawing and input handling.
- It is specifically designed this way so as to be used separately in MT/MP environments,
  and/or over the network, or it can be used for terminal multiplexing.

#### Related files:
	Console.h, Console.cpp, Tables.cpp, Modes.cpp, Charsets.cpp,
	Colors.cpp, Ctl.cpp, Esc.cpp, Csi.cpp, Dcs.cpp, Osc.cpp, Sgr.cpp,

#### Dependencies:
VTInStream, VTCell, VTPage, and Upp/Core

---
### Terminal
This is the virtual terminal emulation ctrl. It is responsible for painting the content of VTPage onto computer screen, and handling the key and mouse events, clipboard and dnd operations. Since it is designed as a regular ctrl, it can be used in any place where a U++ ctrl can be used. It is also responsible for user configuration such as cursor styles, colors, resize behaviour, etc.

#### Notes:
- Terminal is a graphical front-end for Console.

#### Related files:
	Terminal.h, Terminal.cpp, Renderer.cpp, Keys.cpp

#### Dependencies:
VTInStream, VTCell, VTPage, Console, and Upp/Core	

## Examples
As it is already noted above, one of the strengths of the Terminal package, and Ultimate++, is that you can do more with less with these tools. Currently 5 basic examples are provided with the package:

1 TerminalExample

2 SshTerminalExample

3 TerminalInABrowserExample

4 TerminalMultiplexingExample

5 TerminalMultiplexingInAWebBrowserExample

### Terminal Example

This example demonstrates the basic usage of, and interaction between, Terminal and PtyProcess classes. Since PtyProcess class currently runs only on POSIX-compliant systems, this code will be a POSIX-only example for the time being. (This will change in the near future. A Windows power-shell encapsulation is on its way.)

Here is an xterm compatible terminal emulator in 31 sLoC:
	
```C++    	
#include <Terminal/Terminal.h>
#include <Terminal/PtyProcess.h>

using namespace Upp;

const char *nixshell = "/bin/bash";

struct TerminalExample : TopWindow {
	Terminal  term;
	PtyProcess pty;	// This class is completely optional
	
	TerminalExample()
	{
		SetRect(term.GetStdSize()); // 80 x 24 cells (scaled)
		Sizeable().Zoomable().CenterScreen().Add(term.SizePos());

		term.WhenBell   = [=]()		{ BeepExclamation(); };
		term.WhenTitle  = [=](String s)	{ Title(s);  };
		term.WhenResize = [=]()		{ pty.SetSize(term.GetPageSize()); };
		term.WhenOutput = [=](String s)	{ PutGet(s); };

		SetTimeCallback(-1, [=] { PutGet(); });
		pty.Start(nixshell, Environment(), GetHomeDirectory());
	}
	
	void PutGet(String out = Null)
	{
		term.WriteUtf8(pty.Get());
		pty.Write(out);
		if(!pty.IsRunning())
			Break();
	}
};

GUI_APP_MAIN
{
	TerminalExample().Run();
}
```

Yup, that's all.

And here is the result (Lynx web browser running on the Terminal Example (Linux)):

![Lynx web browser running on the Terminal Example (Linux) ](https://github.com/ismail-yilmaz/upp-components/blob/master/CtrlLib/Images/TerminalExample-Screenshot.png)
---	
### Ssh Terminal Example

This example demonstrates the basic usage of, and interaction between, Terminal and SshShell classes. It also exemplifies the first point put forth in the *Highlights* section, since Core/SSH package can run on Windows, Linux and MacOS.

Here is a cross-platform, xterm compatible, basic SSH terminal in only 43 sLoC:
	
	
```C++
#include <Core/Core.h>
#include <Core/SSH/SSH.h>
#include <Terminal/Terminal.h>

using namespace Upp;

String url = "demo:password@test.rebex.net:22";	// A well-known public SSH test server.

struct SshTerminal : Terminal, SshShell {
	SshTerminal(SshSession& session) : SshShell(session)
	{
		SshShell::Timeout(Null);
		SshShell::ChunkSize(65536);
		SshShell::WhenOutput = [=](const void *data, int size) { Terminal::Write(data, size); };
		Terminal::WhenOutput = [=](String data) { SshShell::Send(data); };
		Terminal::WhenResize = [=]() { SshShell::PageSize(Terminal::GetPageSize()); };		
	}

	void Run(const String& termtype)
	{
		SshShell::Run(termtype, Terminal::GetPageSize());
		if(SshShell::IsError())
			ErrorOK(DeQtf(GetErrorDesc()));
	}
};

struct SshTerminalExample : TopWindow {
	void Run()
	{
		if(!EditTextNotNull(url, "SSH server", "Url"))
			return;
		SshSession session;
		session.WhenWait = [=]{ ProcessEvents(); };
		if(!session.Timeout(10000).Connect(url)) {
			ErrorOK(DeQtf(session.GetErrorDesc()));
			return;
		}
		SshTerminal term(session);
		SetRect(term.GetStdSize()); // 80 x 24 cells (scaled)
		Sizeable().Zoomable().CenterScreen().Add(term.SizePos());
		OpenMain();
		term.Run("xterm");
	}
};

GUI_APP_MAIN
{
	SshTerminalExample().Run();
}
```

On the left is PuTTY and on the right is Ssh Terminal Example (on Windows):

![On the left is PuTTY, and on the right is the Ssh Terminal Example (on Windows)](https://github.com/ismail-yilmaz/upp-components/blob/master/CtrlLib/Images/SSTerminalExample-Screenshot.png)
---
### Terminal in a Web Browser Example

Now some cool stuff.
Turtle package allows any U++ GUI app to be accessed via decent web browser. To demonstrate this point we'll use the Terminal Example above.
A few lines of code will turn it into a remote terminal:

```C++
#include <Terminal/Terminal.h>
#include <Terminal/PtyProcess.h>

using namespace Upp;

const char *nixshell = "/bin/bash";

struct TerminalExample : TopWindow {
	Terminal  term;
	PtyProcess pty;			// This class is completely optional

	TerminalExample()
	{
		SetRect(term.GetStdSize());	// 80 x 24 cells (scaled)
		Sizeable().Zoomable().CenterScreen().Add(term.SizePos());

		term.WhenBell   = [=]()			{ BeepExclamation(); };
		term.WhenTitle  = [=](String s)	{ Title(s);	};
		term.WhenResize = [=]()			{ pty.SetSize(term.GetPageSize()); };
		term.WhenOutput = [=](String s)	{ PutGet(s); };

		SetTimeCallback(-1, [=] { PutGet(); });
		pty.Start(nixshell, Environment(), GetHomeDirectory()); // Defaults to TERM=xterm
	}

	void PutGet(String out = Null)
	{
		term.CheckWriteUtf8(pty.Get());
		pty.Write(out);
		if(!pty.IsRunning())
			Break();
	}
};

void Main()
{
	TerminalExample().Run();
}

#ifdef flagTURTLE
CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);

	MemoryLimitKb(100000000);
	Ctrl::host = "localhost";
	Ctrl::port = 8888;
	Ctrl::connection_limit = 15; // Maximum number of concurrent users (preventing DDoS)

#ifdef _DEBUG
	Ctrl::debugmode = true;		 // Only single session in debug (no forking)
#endif
	if(Ctrl::StartSession()) {
		Main();
		Ctrl::EndSession();
	}
	LOG("Session Finished");
}
#else
GUI_APP_MAIN
{
	Main();
}
#endif
```

The result is "browserception": Lynx running on Terminal accessed from within Firefox!

![The result is "browserception": Lynx running on Terminal accessed from within Firefox](https://github.com/ismail-yilmaz/upp-components/blob/master/CtrlLib/Images/TerminalExample-Turtle.png)

### Terminal Multiplexing Example

As it is already mentioned in the *Highlights* section, Terminal widget is a regular ctrl: It follows the ingenious design of the Ultimate++ ctrl library as closely as possible. This example demonstrates how a terminal multiplexing can be achieved simply by using a splitter widget. Splitter is a container ctrl that can be used to split any parent ctrl into resizeable horizontal and/or vertical panes. It also demonstrates the usage of NTL containers with Terminal widget:

```C++
#include <Terminal/Terminal.h>
#include <Terminal/PtyProcess.h>

// This example demonstrates a barebone terminal multiplexer.
// It uses PtyProcess, therefore it is currently POSIX-only.

const char *nixshell = "/bin/bash";
const int  PANECOUNT = 2;				// You can increase the number of panes if you like.

using namespace Upp;

struct TerminalPane : Terminal, PtyProcess {
	TerminalPane()
	{
		WhenBell   = [=]()		{ BeepExclamation(); };
		WhenResize = [=]()		{ PtyProcess::SetSize(GetPageSize()); };
		WhenOutput = [=](String s)	{ Do(s); };
		PtyProcess::Start(nixshell, Environment(), GetHomeDirectory());
	};
	
	bool Do(String out = Null)
	{
		WriteUtf8(PtyProcess::Get());
		PtyProcess::Write(out);
		return PtyProcess::IsRunning();
	}
};

class TerminalMultiplexerExample : public TopWindow {
	Splitter splitter;
	Array<TerminalPane> terminals;	// Let's dynamically create the TerminalPane instances.
public:
	void SetupSplitter()
	{
		Add(splitter.Horz());
		for(int i = 0; i < PANECOUNT; i++)
			splitter.Add(terminals.Add().SizePos());
	}
	
	void RemovePane(int i, Ctrl& c)
	{
		splitter.Remove(c);
		splitter.RefreshLayoutDeep();
		terminals.Remove(i);
	}
	
	void Run()
	{
		Title(t_("Terminal Multiplexing Example"));
		SetRect(0, 0, 1024, 600);
		Sizeable().Zoomable().CenterScreen();
		SetupSplitter();
		OpenMain();
		while(IsOpen() && !terminals.IsEmpty()) {
			ProcessEvents();
			for(int i = 0; i < terminals.GetCount(); i++) {
				TerminalPane& pane = terminals[i];
				if(!pane.Do()) {
					RemovePane(i, pane);
					i--;
				}
			}
			Sleep(1);
		}
	}
};

GUI_APP_MAIN
{
	TerminalMultiplexerExample().Run();
}
```

Here is the result: On the left is htop, and on the right is GNU nano running on Terminal Multiplexing Example (Linux)

![On the left is htop, and on the right is GNU nano running on Terminal Multiplexing Example (Linux)](https://github.com/ismail-yilmaz/upp-components/blob/master/CtrlLib/Images/Terminal-Multiplexer.png)

- *P.s. The two Terminal panes in the above screenshot do not have to share any configuration.
For example one can have a Courier (16) font,and the other can have Liberation Mono (8) at the same time. This is also true for their color settings and other properties.*


### Terminal Multiplexing In a Web Browser Example

Now, let's compile the above multiplexing example with TURTLE flag, and access it via a web browser:

```C++
#include <Terminal/Terminal.h>
#include <Terminal/PtyProcess.h>

// This example demonstrates a barebone terminal multiplexer that can be accessed via a web borwser.
// It uses PtyProcess, therefore it is currently POSIX-only.


const char *nixshell = "/bin/bash";
const int  PANECOUNT = 2;				// You can increase the number of panes if you like.

using namespace Upp;

struct TerminalPane : Terminal, PtyProcess {
	TerminalPane()
	{
		WhenBell   = [=]()			{ BeepExclamation(); };
		WhenResize = [=]()			{ PtyProcess::SetSize(GetPageSize()); };
		WhenOutput = [=](String s)	{ Do(s); };
		PtyProcess::Start(nixshell, Environment(), GetHomeDirectory());
	};
	
	bool Do(String out = Null)
	{
		WriteUtf8(PtyProcess::Get());
		PtyProcess::Write(out);
		return PtyProcess::IsRunning();
	}
};

class TerminalMultiplexerExample : public TopWindow {
	Splitter splitter;
	Array<TerminalPane> terminals;	// Let's dynamically create the TerminalPane instances.
public:
	
	void SetupSplitter()
	{
		Add(splitter.Horz());
		for(int i = 0; i < PANECOUNT; i++)
			splitter.Add(terminals.Add().SizePos());
	}
	
	void RemovePane(int i, Ctrl& c)
	{
		splitter.Remove(c);
		splitter.RefreshLayoutDeep();
		terminals.Remove(i);
	}
	
	void Run()
	{
		Title(t_("Terminal Multiplexing Example"));
		SetRect(0, 0, 1024, 600);
		Sizeable().Zoomable().CenterScreen();
		SetupSplitter();
		OpenMain();
		while(IsOpen() && !terminals.IsEmpty()) {
			ProcessEvents();
			for(int i = 0; i < terminals.GetCount(); i++) {
				TerminalPane& pane = terminals[i];
				if(!pane.Do()) {
					RemovePane(i, pane);
					i--;
				}
			}
			Sleep(1);
		}
	}
};

void Main()
{
	TerminalMultiplexerExample().Run();
}

#ifdef flagTURTLE
CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);

	MemoryLimitKb(100000000);
	Ctrl::host = "localhost";
	Ctrl::port = 8888;
	Ctrl::connection_limit = 15;	// Maximum number of concurrent users (preventing DDoS)

#ifdef _DEBUG
	Ctrl::debugmode = true;			// Only single session in debug (no forking)
#endif
	if(Ctrl::StartSession()) {
		Main();
		Ctrl::EndSession();
	}
	LOG("Session Finished");
}
#else
GUI_APP_MAIN
{
	Main();
}
#endif
```

Extra code added to the source is basically the same as in example 3. 

And here is the result: A basic, xterm compatible terminal multiplexer running htop and GNU nano simultaneously, and accessed via Firefox! (Linux)

![terminal multiplexer running htop and GNU nano simultaneously, and accessed via Firefox](https://github.com/ismail-yilmaz/upp-components/blob/master/CtrlLib/Images/Terminal-Multiplexer-in-web-browser.png)


## To Do
There is always room for improvement and new features.

- Implement the remaining useful DEC, ANSI, and xterm sequences and modes.
- Encapsulate the Windows power-shell process in PtyProcess.
- Sixel graphics.
- True color (24 bit) support.
- Improve modifier keys handling.
- Implement reverse wrap.
- Improve legacy charsets support.
- DEC locator support.
- xterm style rectangular area checksum reports.
- Terminal printer support.
- BIDI support.
- Copy/paste and screen dump as QTF, SVG and HTML.
- Refactor VTPage class.
- More detailed logging.
- Various optimizations.
- Write a terminfo file
- API documentation.

## Known Issues

Nothing is perfect and Terminal package is no exception. Known major issues are listed below.

- Index and back index commands don't obey horizontal margins.
- At the moment VT3xx screen panning feature does not really work (it is imitated using scroll).

## Version

Terminal package is currently at v0.1. (It is considered a beta until v1.0)

## License

	Copyright (c) 2019, İsmail Yılmaz
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this
	  list of conditions and the following disclaimer.

	* Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	* Neither the name of the copyright holder nor the names of its
	  contributors may be used to endorse or promote products derived from
	  this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
