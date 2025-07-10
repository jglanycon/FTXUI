// ==============================================================================================================================
// Copyright (C) 2025 by the FTXUI open source library
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
	BaseIO(
		Dimensions const& fallback_dimensions);

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

	virtual void flush();

	void write(
		char data);

	void write(
		std::string const& data);

	Dimensions fallback_dimensions() const;

	void set_fallback_dimensions(
		Dimensions const& fallback_dimensions);

private:
	bool m_quitting;
	Dimensions m_fallback_dimensions;


};

} // End of namespaces
