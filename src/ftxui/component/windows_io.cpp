// ==============================================================================================================================
// Copyright (C) 2024 by the FTXUI open source library
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.
// ==============================================================================================================================

#include "ftxui/component/windows_io.hpp"

#if defined(_WIN32)
	#define DEFINE_CONSOLEV2_PROPERTIES
	#define WIN32_LEAN_AND_MEAN
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>
	#ifndef UNICODE
		#error Must be compiled in UNICODE mode
	#endif
#endif

#include <array>
#include <utility>

#include <string>
#include <sstream>

#include "ftxui/util/exceptions.hpp"
#include "ftxui/component/terminal_input_parser.hpp"

#include <string.h>

namespace ftxui
{

WindowsIO::WindowsIO()
:	m_quiting(false)
{
}

WindowsIO::~WindowsIO()
{
}

void WindowsIO::install()
{
}

void WindowsIO::uninstall()
{
}

void WindowsIO::event_listener(
	Sender<Task> out)
{
	auto console = GetStdHandle(STD_INPUT_HANDLE);
	auto parser = TerminalInputParser(out->Clone());

	while (!m_quiting)
	{
		// Throttle ReadConsoleInput by waiting 250ms, this wait function will
		// return if there is input in the console.
		auto wait_result = WaitForSingleObject(console, timeout_milliseconds);

		if (wait_result == WAIT_TIMEOUT)
		{
			parser.Timeout(timeout_milliseconds);
			continue;
		}

		DWORD number_of_events = 0;

		if (!GetNumberOfConsoleInputEvents(console, &number_of_events))
			continue;

		if (number_of_events <= 0)
			continue;

		std::vector<INPUT_RECORD> records{number_of_events};
		DWORD number_of_events_read = 0;

		ReadConsoleInput(
			console,
			records.data(),
			(DWORD)records.size(),
			&number_of_events_read);

		records.resize(number_of_events_read);

		for (const auto& r : records)
		{
			switch (r.EventType)
			{
				case KEY_EVENT:
				{
					auto key_event = r.Event.KeyEvent;

					// ignore UP key events
					if (key_event.bKeyDown == FALSE)
						continue;
					std::wstring wstring;
					wstring += key_event.uChar.UnicodeChar;

					for (auto it : to_string(wstring))
					{
						parser.Add(it);
					}

					break;
				}
				case WINDOW_BUFFER_SIZE_EVENT:
				{
					out->Send(Event::Special({0}));
					break;
				}
				case MENU_EVENT:
				case FOCUS_EVENT:
				case MOUSE_EVENT:
				{
					// TODO(mauve): Implement later.
					break;
				}
			}
		}
	}
}

void WindowsIO::quit()
{
}

void WindowsIO::start()
{
}

void WindowsIO::stop()
{
}

std::size_t WindowsIO::read(
	void* buffer,
	std::size_t size)
{
	return 0;
}

std::size_t WindowsIO::write(
	void const* data,
	std::size_t size)
{
	return 0;
}

Dimensions WindowsIO::dimensions() const
{
	return Dimensions();
}

} // End of namespaces
