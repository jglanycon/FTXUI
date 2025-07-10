// ==============================================================================================================================
// Copyright (C) 2025 by the FTXUI open source library
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.
// ==============================================================================================================================

#include "ftxui/component/base_io.hpp"

namespace ftxui
{

BaseIO::BaseIO(
	Dimensions const& fallback_dimensions)
:	m_quitting(false),
	m_fallback_dimensions(fallback_dimensions)
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

void BaseIO::flush()
{
	write('\0');
}

Dimensions BaseIO::fallback_dimensions() const
{
	return m_fallback_dimensions;
}

void BaseIO::set_fallback_dimensions(
	Dimensions const& fallback_dimensions)
{
	m_fallback_dimensions =
		fallback_dimensions;
}

} // End of namespaces
