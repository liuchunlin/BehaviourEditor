#pragma once

#include <locale>
#include <iostream>
#include <algorithm>
using namespace std;

// templated version of my_equal so it could work with both char and wchar_t
template<typename charT>
struct my_equal {
	my_equal(const std::locale& loc) : _loc(loc) {}
	bool operator()(charT ch1, charT ch2) {
		return std::toupper(ch1, _loc) == std::toupper(ch2, _loc);
	}
private:
	const std::locale& _loc;
};

// find substring (case insensitive)
template<typename T>
int ci_find_substr(const T& str1, const T& str2, const std::locale& loc = std::locale())
{
	typename T::const_iterator it = std::search(str1.begin(), str1.end(),
		str2.begin(), str2.end(), my_equal<typename T::value_type>(loc));
	if (it != str1.end()) return it - str1.begin();
	else return -1; // not found
}