// ==============================================================================================================================
// Copyright (C) 2024 by the FTXUI open source library
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.
// ==============================================================================================================================

#include "ftxui/component/base_io.hpp"

namespace ftxui
{

BaseIO::BaseIO()
{
}

BaseIO::~BaseIO() noexcept(false)
{
}

void BaseIO::write(
	char data)
{
	write(
		&data,
		1);
}

void BaseIO::write(
	std::string const& data)
{
	if (!data.empty())
	{
		write(
			data.c_str(),
			data.size());
	}
}

} // End of namespaces
