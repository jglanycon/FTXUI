// ==============================================================================================================================
// Copyright (C) 2024 by the FTXUI open source library
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.
// ==============================================================================================================================
#pragma once

#include <stdio.h>
#include <cstddef>

#include "ftxui/component/task.hpp"  // for Task, Closure, AnimationTask
#include "ftxui/component/receiver.hpp"  // for ReceiverImpl, Sender, MakeReceiver, SenderImpl, Receiver
#include "ftxui/screen/terminal.hpp"

namespace ftxui
{

class BaseIO
{
public:
	BaseIO();

	virtual ~BaseIO() noexcept(false);

	virtual void install() = 0;

	virtual void uninstall() = 0;

	virtual Dimensions dimensions() const = 0;

	virtual void event_listener(
		Sender<Task> out) = 0;

	virtual void quit() = 0;

	virtual std::size_t write(
		void const* data,
		std::size_t size) = 0;

	virtual std::size_t read(
		void* data,
		std::size_t size) = 0;

	void write(
		char data);

	void write(
		std::string const& data);

private:

};

} // End of namespaces
