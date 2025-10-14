#pragma once

#include <string>

bool log(const std::wstring_view& message, const bool pause = false, const bool error = false);
int error(const std::wstring_view& message);