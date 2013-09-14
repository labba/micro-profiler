//	Copyright (C) 2011 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#pragma once

#include <memory>

#pragma pack(push, 8)

struct IProfilerFrontend;

namespace wpl
{
	namespace mt
	{
		class thread;
	}
}

namespace micro_profiler
{
	typedef void (*frontend_factory)(IProfilerFrontend **frontend);
	struct calls_collector_i;

	calls_collector_i& get_global_collector_instance() throw();
	void create_local_frontend(IProfilerFrontend **frontend);
	void create_inproc_frontend(IProfilerFrontend **frontend);

	class profiler_frontend
	{
		calls_collector_i &_collector;
		frontend_factory _factory;
		std::auto_ptr<wpl::mt::thread> _frontend_thread;

		void frontend_initialize();
		void frontend_worker();

		profiler_frontend(const profiler_frontend &);
		void operator =(const profiler_frontend &);

	public:
		profiler_frontend(calls_collector_i &collector = get_global_collector_instance(), frontend_factory factory = &create_local_frontend);
		~profiler_frontend();
	};
}

#pragma pack(pop)