#ifndef _HCT_SCOPE_GUARD_H_
#define _HCT_SCOPE_GUARD_H_

#include <exception>

namespace hct
{

	class New_Exception_Detector
	{
	private:

		const int count;

	public:

		New_Exception_Detector() : count(std::uncaught_exceptions()) {}

		operator bool() const
		{
			return count < std::uncaught_exceptions();
		}

	};

	template <typename Func, bool on_success, bool on_failure>
	class Scope_Guard
	{
	private:

		Func func;
		New_Exception_Detector detector;

		Scope_Guard() = delete;
		Scope_Guard(const Scope_Guard&) = delete;
		Scope_Guard(Scope_Guard&& obj) = delete;
		Scope_Guard& operator=(const Scope_Guard&) = delete;
		Scope_Guard& operator=(Scope_Guard&&) = delete;

	public:

		Scope_Guard(const Func& arg_func) : func(arg_func) {}
		Scope_Guard(Func&& arg_func) : func(arg_func) {}

		~Scope_Guard()
		{
			if ((on_success && !detector) ||
				(on_failure && detector))
				func();
		}

	};

	template <typename Func>
	inline Scope_Guard<Func, true, true> make_guard_exit(Func&& func)
	{
		return Scope_Guard<Func, true, true>(func);
	}

	template <typename Func>
	inline Scope_Guard<Func, true, false> make_guard_success(Func&& func)
	{
		return Scope_Guard<Func, true, false>(func);
	}

	template <typename Func>
	inline Scope_Guard<Func, false, true> make_guard_failure(Func&& func)
	{
		return Scope_Guard<Func, false, true>(func);
	}

}

#endif // _HCT_SCOPE_GUARD_H_