// ==============================================================================================================================
// Copyright (C) 2024 by the FTXUI open source library
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.
// ==============================================================================================================================
// For opening a file to handle stdin and stdout look at the following file in sudo plugio.
// ./src/get_pty.c

#include "ftxui/component/linux_io.hpp"

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

LinuxIO::LinuxIO(
	FILE* input_stream,
	FILE* output_stream)
:	m_input_stream(input_stream),
	m_output_stream(output_stream),
	m_epoll_descriptor(0),
	m_quiting(false),
	m_original_terminal({0})
{
	m_stop_pipe_descriptor[0] = 0;
	m_stop_pipe_descriptor[1] = 0;
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
	// Restore the terminal settings
	if (tcsetattr(m_input_stream->_fileno, TCSANOW, &m_original_terminal) == -1)
		throw ErrorException("Unable to set terminal");
}

void LinuxIO::event_listener(
	Sender<Task> out)
{
	start();

	auto parser = TerminalInputParser(
		std::move(out));

	struct epoll_event events[2];

	// Request terminal id, if possible
	BaseIO::write("\x1bZ");
	BaseIO::write("\x1b[1c");

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

		if (events[0].data.fd == m_input_stream->_fileno)
		{
			std::array<char, 100> buffer;

			auto byte_read = read(buffer.data(), buffer.size()); // NOLINT

			for (size_t byte_count = 0; byte_count < byte_read; ++byte_count)
			{
				parser.Add(buffer[byte_count]); // NOLINT
			}
		}
	}
	while (events[0].data.fd != m_stop_pipe_descriptor[0]);

	stop();
}

void LinuxIO::quit()
{
	uint8_t stop_buffer = 0;
	::write(m_stop_pipe_descriptor[1], &stop_buffer, sizeof(stop_buffer));
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

	if (pipe2(m_stop_pipe_descriptor, O_NONBLOCK) == -1)
		throw ErrorException("Can't initialize stop pipe");

	event.events = EPOLLIN;
    event.data.fd = m_stop_pipe_descriptor[0];

	if (epoll_ctl(m_epoll_descriptor, EPOLL_CTL_ADD, m_stop_pipe_descriptor[0], &event) == -1)
		throw ErrorException("epoll_ctl() error");
}

void LinuxIO::stop()
{
	epoll_ctl(m_epoll_descriptor, EPOLL_CTL_DEL, m_input_stream->_fileno, nullptr);
	epoll_ctl(m_epoll_descriptor, EPOLL_CTL_DEL, m_stop_pipe_descriptor[0], nullptr);

	close(m_stop_pipe_descriptor[0]);
    close(m_stop_pipe_descriptor[1]);
	close(m_epoll_descriptor);
}

std::size_t LinuxIO::read(
	void* buffer,
	std::size_t size)
{
	// HACK / TODO We need to do something here.
	// return fread(
	// 	buffer,
	// 	sizeof(char),
	// 	size,
	// 	stdin);

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

	if (ioctl(m_output_stream->_fileno , TIOCGWINSZ, &terminal_size) == -1)
	  throw ErrorException("Unable get terminal dimensions using ioctl()");

	return Dimensions{terminal_size.ws_col, terminal_size.ws_row};
}

} // End of namespaces
