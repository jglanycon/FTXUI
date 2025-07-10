// ==============================================================================================================================
// Copyright (C) 2025 by the FTXUI open source library
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.
// ==============================================================================================================================
// For opening a file to handle stdin and stdout look at the following file in sudo plugio.
// ./src/get_pty.c

#include "ftxui/component/linux_io.hpp"

#include <iostream>
#include <array>
#include <utility>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <termios.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/eventfd.h>

#include <string>
#include <sstream>

#include "ftxui/util/exceptions.hpp"
#include "ftxui/component/terminal_input_parser.hpp"

#include <stdio.h>
#include <string.h>

namespace ftxui
{

constexpr int const DEFAULT_WIDTH = 80;
constexpr int const DEFAULT_HEIGHT = 24;

LinuxIO::LinuxIO(
	FILE* input_stream,
	FILE* output_stream)
:	BaseIO(
		{DEFAULT_WIDTH, DEFAULT_HEIGHT}),
	m_input_stream(input_stream),
	m_output_stream(output_stream),
	m_epoll_descriptor(0),
	m_quit_event(0),
	m_original_terminal({0})
{
	m_quit_event =
		eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);

	if (m_quit_event == -1)
		throw ErrorException("eventfd() error");

	start();
}

LinuxIO::~LinuxIO()
{
}

void LinuxIO::install()
{
	if (tcgetattr(m_input_stream->_fileno, &m_original_terminal) == -1)
		throw ErrorException("Unable to get current tcgetattr()");

	// New terminal settings
	termios terminal =
		m_original_terminal;

	// Enabling raw terminal input mode
	terminal.c_iflag &= ~IGNBRK; // Disable ignoring break condition
	terminal.c_iflag &= ~BRKINT; // Disable break causing input and output to be flushed
	terminal.c_iflag &= ~PARMRK; // Disable marking parity errors.
	terminal.c_iflag &= ~ISTRIP; // Disable striping 8th bit off characters.
	terminal.c_iflag &= ~INLCR; // Disable mapping NL to CR.
	terminal.c_iflag &= ~IGNCR; // Disable ignoring CR.
	terminal.c_iflag &= ~ICRNL; // Disable mapping CR to NL.
	terminal.c_iflag &= ~IXON; // Disable XON/XOFF flow control on output

	terminal.c_lflag &= ~ECHO; // Disable echoing input characters.
	terminal.c_lflag &= ~ECHONL; // Disable echoing new line characters.
	terminal.c_lflag &= ~ICANON; // Disable Canonical mode.
	terminal.c_lflag &= ~ISIG; // Disable sending signal when hitting:
								// -     => DSUSP
								// - C-Z => SUSP
								// - C-C => INTR
								// - C-d => QUIT
	terminal.c_lflag &= ~IEXTEN; // Disable extended input processing
	terminal.c_cflag |= (CS8);	 // 8 bits per byte

	terminal.c_cc[VMIN] = 0;	// Minimum number of characters for non-canonical read.
	terminal.c_cc[VTIME] = 0;   // Timeout in deciseconds for non-canonical read.

	if (tcsetattr(m_input_stream->_fileno, TCSANOW, &terminal) == -1)
		throw ErrorException("Unable to set tcsetattr");
}

void LinuxIO::uninstall()
{
	std::array<char, 100> buffer;
	auto byte_read = read(buffer.data(), buffer.size()); // NOLINT

	// Restore the terminal settings
	if (tcsetattr(m_input_stream->_fileno, TCSANOW, &m_original_terminal) == -1)
		throw ErrorException("Unable to set terminal");

	quit();
}

void LinuxIO::event_listener(
	Sender<Task> out)
{
	auto parser = TerminalInputParser(
		std::move(out));

	struct epoll_event events[2];

	do
	{
		memset(events, 0x00, sizeof(events));

		int event_count =
			epoll_wait(
				m_epoll_descriptor,
				events,
				sizeof(events) / sizeof(epoll_event),
				-1);

		// EINTR will be ignored, due GDB debugger.
		if ( (event_count == -1) && (errno != EINTR) )
			throw ErrorException("Error epoll_wait");

		if ( (events[0].data.fd == m_input_stream->_fileno) ||
			 (events[1].data.fd == m_input_stream->_fileno) )
		{
			std::array<char, 100> buffer;

			auto byte_read = read(buffer.data(), buffer.size()); // NOLINT

			for (size_t byte_count = 0; byte_count < byte_read; ++byte_count)
			{
				parser.Add(buffer[byte_count]); // NOLINT
			}
		}
	}
	while ( (events[0].data.fd != m_quit_event) &&
			(events[1].data.fd != m_quit_event) );

	stop();
}

void LinuxIO::quit()
{
	// Write the value as an whole 64 bits data (data length must be 8 bytes)
	uint64_t event_value = 1;
	::write(m_quit_event, &event_value, sizeof(event_value));
}

void LinuxIO::start()
{
	if ( (m_epoll_descriptor = epoll_create1(0)) == -1)
		throw ErrorException("epoll_create1()");

	epoll_event event = {0};
	event.events = EPOLLIN;
	event.data.fd = m_input_stream->_fileno;

	if (epoll_ctl(m_epoll_descriptor, EPOLL_CTL_ADD, m_input_stream->_fileno, &event) == -1)
		throw ErrorException("epoll_ctl() error");

	event.events = EPOLLIN;
    event.data.fd = m_quit_event;

	if (epoll_ctl(m_epoll_descriptor, EPOLL_CTL_ADD, m_quit_event, &event) == -1)
		throw ErrorException("epoll_ctl() error");
}

void LinuxIO::stop()
{
	epoll_ctl(m_epoll_descriptor, EPOLL_CTL_DEL, m_input_stream->_fileno, nullptr);
	epoll_ctl(m_epoll_descriptor, EPOLL_CTL_DEL, m_quit_event, nullptr);

	close(m_quit_event);
	close(m_epoll_descriptor);
}

std::size_t LinuxIO::read(
	void* buffer,
	std::size_t size)
{
	return ::read(
		m_input_stream->_fileno,
		buffer,
		size);
}

std::size_t LinuxIO::write(
	void const* data,
	std::size_t size)
{
	// std::cout.write(reinterpret_cast<char const*>(data), size);
	// std::cout.flush();
	// return 0;

	auto byte_read = fwrite(
		reinterpret_cast<char const*>(data),
		sizeof(char),
		size,
		m_output_stream);

	fflush(m_output_stream);

	return byte_read;
}

Dimensions LinuxIO::dimensions() const
{
	winsize terminal_size = {0};

	auto const status =
		ioctl(m_output_stream->_fileno , TIOCGWINSZ, &terminal_size);

	// The ioctl return value result should be checked. Some operating systems
	// don't support TIOCGWINSZ.
	if (terminal_size.ws_col == 0 || terminal_size.ws_row == 0 || status < 0)
		return fallback_dimensions();

	return Dimensions{terminal_size.ws_col, terminal_size.ws_row};
}

} // End of namespaces
