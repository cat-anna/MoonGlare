#pragma once

namespace modc {

	template< class dummy>
	struct error_t {
		static void Error() {
			++_Errors;
		}

		static void Warning() {
			++_Warnings;
		}

		static unsigned GetErrorCount() { return _Errors; }
		static unsigned GetWarningCount() { return _Warnings; }
	protected:
		static unsigned _Errors;
		static unsigned _Warnings;
	};

	template<class dummy> unsigned error_t<dummy>::_Errors = 0;
	template<class dummy> unsigned error_t<dummy>::_Warnings = 0;

	using Error = error_t < int > ;

} // namespace modc
