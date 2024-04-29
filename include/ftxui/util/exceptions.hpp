// ==============================================================================================================================
// Copyright (C) 2024 by the FTXUI open source library
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.
// ==============================================================================================================================
#pragma once

#include <string>
#include <exception>

namespace ftxui
{

namespace detail
{

class ExceptionImpl
:	public std::exception
{
public:
	ExceptionImpl(
		std::string const& message,
		std::string const& file,
		int line_number,
		std::string const& function_name);

	virtual const char* what() const noexcept override;

protected:
	std::string m_exception_message;

private:
	std::string m_message;
	std::string m_file;
	int m_line_number;
	std::string m_function_name;
};

class ErrorExceptionImpl
:	public ExceptionImpl
{
public:
	ErrorExceptionImpl(
		std::string const& message,
		std::string const& file,
		int line_number,
		std::string const& function_name);

private:
	int m_errno;
	std::string m_error_message;
};

} // End of namespace

#define Exception(message) \
	detail::ExceptionImpl(message, __FILE__, __LINE__, __func__)

#define ErrorException(message) \
	detail::ErrorExceptionImpl(message, __FILE__, __LINE__, __func__)

} // End of namespaces
