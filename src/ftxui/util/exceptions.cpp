// ==============================================================================================================================
// Copyright (C) 2024 by the FTXUI open source library
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.
// ==============================================================================================================================

#include "ftxui/util/exceptions.hpp"

#include <sstream>
#include <string.h>

namespace ftxui::detail
{

ExceptionImpl::ExceptionImpl(
	std::string const& message,
	std::string const& file,
	int line_number,
	std::string const& function_name)
:	m_message(message),
	m_file(file),
	m_line_number(line_number),
	m_function_name(function_name)
{
	std::stringstream result;

	result << m_message << " in source file \"" << m_file << " line \"" << m_line_number << "\" function name \"" << m_function_name << "\"";

	m_exception_message =
		result.str();
}

const char* ExceptionImpl::what() const noexcept
{
	return m_exception_message.c_str();
}

ErrorExceptionImpl::ErrorExceptionImpl(
	std::string const& message,
	std::string const& file,
	int line_number,
	std::string const& function_name)
:	ExceptionImpl(
		message,
		file,
		line_number,
		function_name),
	m_errno(errno),
	m_error_message(
		strerror(errno))
{
	std::stringstream result;

	result << " [" << m_errno << "] " << m_error_message;

	m_exception_message +=
		result.str();

}

} // End of namespaces

