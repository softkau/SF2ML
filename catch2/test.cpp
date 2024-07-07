#include <catch2/catch_test_macros.hpp>
#include <SF2ML/sf2ml.hpp>

std::string src_dir = "../sf2src/";


#define CheckSampleProperties(smpl,                           \
                              name,                           \
                              bit_depth,                      \
                              loop_start,                     \
                              loop_end,                       \
                              smpl_count,                     \
                              root_key,                       \
                              sample_rate,                    \
                              pitch_correction,               \
                              sample_mode,                    \
                              link) do {                      \
    CHECK((smpl).GetName()            == (name));             \
    CHECK((smpl).GetBitDepth()        == (bit_depth));        \
    CHECK((smpl).GetLoop().first      == (loop_start));       \
    CHECK((smpl).GetLoop().second     == (loop_end));         \
    CHECK((smpl).GetSampleCount()     == (smpl_count));       \
    CHECK((smpl).GetRootKey()         == (root_key));         \
    CHECK((smpl).GetSampleRate()      == (sample_rate));      \
    CHECK((smpl).GetPitchCorrection() == (pitch_correction)); \
    CHECK((smpl).GetSampleMode()      == (sample_mode));      \
    CHECK((smpl).GetLink() == (link));                        \
} while(0)

TEST_CASE("Loading SoundFont2 files", "[loader]") {
    SF2ML::SoundFont sf2;
    std::ifstream sf2_ifs(src_dir + "SF2ML_TEST1.sf2", std::ios::binary);
    REQUIRE(sf2.Load(sf2_ifs) == SF2ML::SF2ML_SUCCESS);

    // check if it correctly loaded samples
    CHECK(sf2.AllSamples().size() == 9);

    auto kick_h = sf2.FindSample([](const auto& smpl) { return smpl.GetName() == "Kick 1"; });
    REQUIRE(kick_h.has_value());
    auto& kick = sf2.GetSample(*kick_h);
    CheckSampleProperties(kick,
        "Kick 1",
        SF2ML::SampleBitDepth::Signed16,
        75,
        17875,
        17876,
        60,
        44100,
        0,
        SF2ML::monoSample,
        std::nullopt
    );

    auto lead_A4L_h = sf2.FindSample([](const auto& smpl) { return smpl.GetName() == "Lead A4L"; }); 
    auto lead_A4R_h = sf2.FindSample([](const auto& smpl) { return smpl.GetName() == "Lead A4R"; }); 
    REQUIRE(lead_A4L_h.has_value());
    REQUIRE(lead_A4R_h.has_value());

    auto& lead_A4L = sf2.GetSample(*lead_A4L_h);
    auto& lead_A4R = sf2.GetSample(*lead_A4R_h);
    CheckSampleProperties(lead_A4L,
        "Lead A4L",
        SF2ML::SampleBitDepth::Signed16,
        0,
        2000,
        60828,
        60,
        44100,
        0,
        SF2ML::leftSample,
        lead_A4R_h
    );

    CheckSampleProperties(lead_A4R,
        "Lead A4R",
        SF2ML::SampleBitDepth::Signed16,
        0,
        2000,
        60828,
        60,
        44100,
        0,
        SF2ML::rightSample,
        lead_A4L_h
    );

}