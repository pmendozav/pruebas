#pragma once
#include <iostream>

#ifdef WIN32
#include <windows.h>
#else
#include <Winsock2.h>
#endif

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


template <class T>
class _Stopwatch
{
public:
	_Stopwatch();
	~_Stopwatch() {};

	void tic();	// time in clock
	double toc(const char* msg = NULL); // time out clock
private:
	T inT, f;
};

template <class T>
_Stopwatch<T>::_Stopwatch()
{
#ifdef WIN32
	QueryPerformanceFrequency(&f);
#endif
	tic();
}

#ifdef WIN32
typedef _Stopwatch<LARGE_INTEGER> Stopwatch; // only CTimer makes sense
#else
typedef _Stopwatch<struct timeval> Stopwatch;
#endif

template <class T>
void _Stopwatch<T>::tic()
{
#ifdef WIN32
	QueryPerformanceCounter(&inT);
#else
	gettimeofday(&inT, NULL);
#endif
}

template <class T>
double _Stopwatch<T>::toc(const char* msg)
{
#ifdef WIN32
	LARGE_INTEGER outT;
	QueryPerformanceCounter(&outT);

	double dt = (double)(outT.QuadPart - inT.QuadPart) / f.QuadPart;
#else
	struct timeval outT;
	gettimeofday(&outT, NULL);
	double dt = 1000000 * (outT.tv_sec - inT.tv_sec) + outT.tv_usec - inT.tv_usec;
	dt /= 1000000;
#endif
	if (msg) {
		printf("%s %f [s]\n", msg, dt);
	}

	tic();
	return dt;
}






#endif // !UTILS