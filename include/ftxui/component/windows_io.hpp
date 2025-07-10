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

class WindowsIO
:	public BaseIO
{
public:
	WindowsIO();

	virtual ~WindowsIO();

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
	bool m_quiting;
};

} // End of namespaces
