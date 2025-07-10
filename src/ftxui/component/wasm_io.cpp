// ==============================================================================================================================
// Copyright (C) 2024 by the FTXUI open source library
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.
// ==============================================================================================================================
// For opening a file to handle stdin and stdout look at the following file in sudo plugio.
// ./src/get_pty.c

#include "ftxui/component/wasm_io.hpp"

#include <array>
#include <utility>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <termios.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>

#include <string>
#include <sstream>

#include "ftxui/util/exceptions.hpp"
#include "ftxui/component/terminal_input_parser.hpp"

#include <stdio.h>
#include <string.h>

namespace ftxui
{

WasmIO::WasmIO()
:	m_quiting(false)
{
}

WasmIO::~WasmIO()
{
}

void WasmIO::install()
{
}

void WasmIO::uninstall()
{
}

void WasmIO::event_listener(
	Sender<Task> out)
{
}

void WasmIO::quit()
{
}

void WasmIO::start()
{
}

void WasmIO::stop()
{
}

std::size_t WasmIO::read(
	void* buffer,
	std::size_t size)
{
}

std::size_t WasmIO::write(
	void const* data,
	std::size_t size)
{
	return 0;
}

Dimensions WasmIO::dimensions() const
{
	return Dimensions();
}

} // End of namespaces
