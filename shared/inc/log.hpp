#pragma once

#include <string>

bool log(const std::wstring_view& message, const bool error = false, const bool pause = false);

int success(const std::wstring_view& message);
int error(const std::wstring_view& message);