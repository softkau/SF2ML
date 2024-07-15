#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <SF2ML/sf2ml.hpp>

struct SmplData {

};

struct InstData {

};

struct PresetData {

};

struct Sf2Data {
	std::vector<SmplData> smpl_data;
	std::vector<InstData> inst_data;
	std::vector<PresetData> preset_data;
};

template <typename T>
class VectorElementsGenerator final : public Catch::Generators::IGenerator<T> {
	std::vector<T> elems;
	size_t idx = 0;
public:
	VectorElementsGenerator(const std::vector<T>& elems) elems(elems) {}

	const T& get() override {
		return elems[idx];
	}

	bool next() override {
		++idx;
		return idx < elems.size();
	}
};

template <typename T>
auto VecElems(const std::vector<T>& vec) -> Catch::Generators::GeneratorWrapper<T> {
	return Catch::Generators::GeneratorWrapper<T>(
		Catch::Detail::make_unique<VectorElementsGenerator<T>>(vec);
	);
}

Sf2Data sf2_data {};

TEST_CASE("Generators") {
	using namespace SF2ML;
	SoundFont sf2;
	std::ifstream file(".sf2");
	sf2.Load(file);

	// convert to simple data structs
	std::vector<SmplData> sf2_smpls;
	std::vector<InstData> sf2_insts;
	std::vector<PresetData> sf2_presets;

	SECTION("Sample Generators") {
		auto smpl = GENERATE(VecElems(sf2_data.smpl_data));
		CHECK_THAT(sf2_smpls, Catch::Matchers::VectorContains(smpl));
	}

	SECTION("Inst Generators") {
		auto inst = GENERATE(VecElems(sf2_data.inst_data));
		CHECK_THAT(sf2_insts, Catch::Matchers::VectorContains(inst));
	}

	SECTION("Preset Generators") {
		auto preset = GENERATE(VecElems(sf2_data.preset_data));
		CHECK_THAT(sf2_presets, Catch::Matchers::VectorContains(preset));
	}
}