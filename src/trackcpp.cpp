// The MIT License (MIT)
//
// Copyright (c) 2015 LNLS Accelerator Division
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <trackcpp/trackcpp.h>
#include <ctime>
#include <cstdlib>
#include <string>


bool verbose_on = true;

bool isfinite(const double& v) {
	return std::isfinite(v);
}

std::string get_timestamp() {

	char buffer[30];
	time_t t = time(NULL);   // get time now
	struct tm* now = localtime(&t);
	sprintf(buffer, "[%04i-%02i-%02i %02i:%02i:%02i]", 1900+now->tm_year, 1+now->tm_mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
	return std::string(buffer);
}
