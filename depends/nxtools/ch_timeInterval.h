#pragma once
#include <iostream>
#include <memory>
#include <string>
#include "ch_tools.h"
#ifdef GCC
#include <sys/time.h>
#else
#include <ctime>
#endif // GCC

class CTimeInterval
{
public:
	CTimeInterval(const std::string& d) : detail(d)
	{
		init();
	}

	CTimeInterval()
	{
		init();
	}

	~CTimeInterval()
	{
		end = getCurrentTime();
		std::cout << detail << " "
			<< (int64_t)(end - start) << " ms" << std::endl;
	}

protected:
	void init() {
		start = getCurrentTime();
	}
private:
	std::string detail;
    int64_t start,end;
};

#define TIME_INTERVAL_SCOPE(d)   std::shared_ptr<CTimeInterval> time_interval_scope_begin = std::make_shared<CTimeInterval>(d)