#ifndef SF2ML_SFTYPES_HPP_
#define SF2ML_SFTYPES_HPP_

#include <cstdint>
#include <string>

namespace SF2ML {
	using BYTE  =  uint8_t;
	using WORD  = uint16_t;
	using DWORD = uint32_t;
	using QWORD = uint64_t;
	using CHAR  =   int8_t;
	using SHORT =  int16_t;

	using FOURCC = DWORD;

	enum [[nodiscard]] SF2MLError : int {
		SF2ML_SUCCESS = 0,
		SF2ML_FAILED,
		SF2ML_ZSTR_CHECK_FAILED,
		SF2ML_INVALID_CK_SIZE,
		SF2ML_BAD_WAV_DATA,
		SF2ML_UNSUPPORTED_WAV_DATA,
		SF2ML_INCOMPATIBLE_BIT_DEPTH,
		SF2ML_NOT_STEREO_CHANNEL,
		SF2ML_NOT_MONO_CHANNEL,
		SF2ML_NO_SUCH_SAMPLE,
		SF2ML_BAD_LINK,
		SF2ML_NO_SUCH_INSTRUMENT,
		SF2ML_MISSING_TERMINAL_RECORD,
		SF2ML_EMPTY_CHUNK,
		SF2ML_UNIMPLEMENTED,
		SF2ML_MIXED_BIT_DEPTH,
		SF2ML_END_OF_ERRCODE
	};

	auto ToCStr(SF2MLError err) -> const char*;
	auto ToString(SF2MLError err) -> std::string;
	auto ToStringView(SF2MLError err) -> std::string_view;

	template <class T>
	struct SF2MLResult {
		T value;
		SF2MLError error;
	};

	template <typename T>
	struct Ranges {
		T start;
		T end;
	};

}

#endif