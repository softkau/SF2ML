#include <sftypes.hpp>

namespace SF2ML {
	constexpr const char* SF2MLErrorStr[] = {
		"SF2ML_SUCCESS",
		"SF2ML_FAILED",
		"SF2ML_ZSTR_CHECK_FAILED",
		"SF2ML_INVALID_CK_SIZE",
		"SF2ML_BAD_WAV_DATA",
		"SF2ML_UNSUPPORTED_WAV_DATA",
		"SF2ML_INCOMPATIBLE_BIT_DEPTH",
		"SF2ML_NOT_STEREO_CHANNEL",
		"SF2ML_NOT_MONO_CHANNEL",
		"SF2ML_NO_SUCH_SAMPLE",
		"SF2ML_BAD_LINK",
		"SF2ML_NO_SUCH_INSTRUMENT",
		"SF2ML_MISSING_TERMINAL_RECORD",
		"SF2ML_EMPTY_CHUNK",
		"SF2ML_MIXED_BIT_DEPTH",
		"SF2ML_NO_SUCH_MODULATORS",
		"SF2ML_UNIMPLEMENTED",
	}; static_assert(SF2ML_END_OF_ERRCODE == sizeof(SF2MLErrorStr) / sizeof(const char*));

	auto ToCStr(SF2MLError err) -> const char* {
		return SF2MLErrorStr[err];
	}

	auto ToString(SF2MLError err) -> std::string {
		return SF2MLErrorStr[err];
	}

	auto ToStringView(SF2MLError err) -> std::string_view {
		return SF2MLErrorStr[err];
	}
}