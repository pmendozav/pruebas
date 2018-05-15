#pragma once
#include <iostream>

#ifndef UTILS
#define UTILS


static void error(const std::string &str)
{
	std::cout << str.c_str() << std::endl;
	exit(-1);
}

static void pause()
{
	system("pause");
}
#endif // !UTILS