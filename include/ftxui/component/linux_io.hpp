// ==============================================================================================================================
// Copyright (C) 2024 by the FTXUI open source library
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.
// ==============================================================================================================================
#pragma once

#include "base_io.hpp"

#include <stdio.h>
#include <cstddef>
#include <termios.h>

#include <ftxui/component/task.hpp>

namespace ftxui
{

class LinuxIO
:	public BaseIO
{
public:
	LinuxIO(
		FILE* input_stream = stdin,
		FILE* output_stream = stdout);

	virtual ~LinuxIO();

	void install() override;

	void uninstall() override;

	Dimensions dimensions() const override;

	void event_listener(
		Sender<Task> out) override;

	void quit() override;

	void start();

	void stop();

	std::size_t write(
		void const* data,
		std::size_t size) override;

	std::size_t read(
		void* data,
		std::size_t size) override;

private:
	FILE* m_input_stream;
	FILE* m_output_stream;

	int m_epoll_descriptor;
	int m_quit_event;

	// Original terminal settings
	termios m_original_terminal;
};

} // End of namespaces
