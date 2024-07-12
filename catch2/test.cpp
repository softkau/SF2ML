#include <catch2/catch_test_macros.hpp>
// #include <catch2/matchers/catch_matchers_vector.hpp>
// #include <catch2/matchers/catch_matchers_predicate.hpp>
// #include <catch2/matchers/catch_matchers_contains.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <SF2ML/sf2ml.hpp>

#include <vector>
#include <string>
#include <optional>
#include <type_traits>

std::string src_dir = "../sf2src/";

inline bool MatchModDest(const std::variant<SF2ML::SFGenerator, SF2ML::ModHandle>& x,
                         const std::variant<SF2ML::SFGenerator, SF2ML::ModHandle>& y) {
    return x == y;
}

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

TEST_CASE("Load Samples from file #1", "[loader][sample]") {
    SF2ML::SoundFont sf2;
    std::ifstream sf2_ifs(src_dir + "SF2ML_TEST1.sf2", std::ios::binary);
    REQUIRE(sf2.Load(sf2_ifs) == SF2ML::SF2ML_SUCCESS);

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

TEST_CASE("Load Instruments from file #1", "[loader][inst]") {
    SF2ML::SoundFont sf2;
    std::ifstream sf2_ifs(src_dir + "SF2ML_TEST1.sf2", std::ios::binary);
    REQUIRE(sf2.Load(sf2_ifs) == SF2ML::SF2ML_SUCCESS);

    CHECK(sf2.AllInstruments().size() == 2);

    // checking kick instrument
    auto kick_h = sf2.FindInstrument([](const auto& inst) {
        return inst.GetName() == "Kick 1";
    });
    REQUIRE(kick_h.has_value());

    auto& kick = sf2.GetInstrument(*kick_h);
    CHECK(kick.CountZones() == 2);

    CHECK(kick.GetGlobalZone().GetSampleModes() == SF2ML::LoopMode::NoLoop);

    
    auto kick_zone1_h = kick.FindZone([](const auto& zone) {
        return zone.HasGenerator(SF2ML::SfGenSampleID);
    });
    REQUIRE(kick_zone1_h.has_value());
    auto& kick_zone1 = kick.GetZone(*kick_zone1_h);
    CHECK(kick_zone1.HasGenerator(SF2ML::SfGenOverridingRootKey));
    CHECK(kick_zone1.GetOverridingRootKey() == 60);
    CHECK(kick_zone1.GetKeynum() == 60);
    REQUIRE(kick_zone1.GetSample().has_value());
    auto kick_smpl_h = *kick_zone1.GetSample();
    CHECK(sf2.GetSample(kick_smpl_h).GetName() == "Kick 1");

    auto kick_global_mod_h
        = kick.GetGlobalZone().FindModulator(
            [](const SF2ML::SfModulator& mod) {
                return SF2ML::MatchController(
                    mod.GetSourceController(),
                    SF2ML::GeneralController::NoteOnVelocity
                );
            }
        );

    REQUIRE(kick_global_mod_h.has_value());
    auto& kick_global_mod = kick.GetGlobalZone().GetModulator(*kick_global_mod_h);
    CHECK(SF2ML::MatchController(kick_global_mod.GetAmtSourceController(), SF2ML::GeneralController::None));
    CHECK(kick_global_mod.GetSourceShape() == SF2ML::SfModSourceType::Concave);
    CHECK(kick_global_mod.GetSourcePolarity() == false);
    CHECK(kick_global_mod.GetSourceDirection() == true);
    CHECK(kick_global_mod.GetAmtSourceShape() == SF2ML::SfModSourceType::Linear);
    CHECK(kick_global_mod.GetAmtSourcePolarity() == false);
    CHECK(kick_global_mod.GetAmtSourceDirection() == false);
    CHECK(MatchModDest(kick_global_mod.GetDestination(), SF2ML::SfGenInitialFilterFc));
    CHECK(kick_global_mod.GetModAmount() == 4);
    CHECK(kick_global_mod.GetTransform() == SF2ML::SfModLinearTransform);

    // checking Lead instrument
    auto lead_h = sf2.FindInstrument([](const auto& inst) {
        return inst.GetName() == "Lead";
    });
    REQUIRE(lead_h.has_value());
    auto& lead = sf2.GetInstrument(*lead_h);
    CHECK(lead.CountZones() == 9);

    struct IZoneTestArgs {
        SF2ML::Ranges<SF2ML::BYTE> key_range;
        int16_t reverb_fx_send;
        int16_t panning;
        int16_t root_key;
        std::optional<SF2ML::SmplHandle> sample;
    };
    std::vector<IZoneTestArgs> izones;

    lead.ForEachZone([&izones](const SF2ML::SfInstrumentZone& zone) {
        izones.emplace_back(
            zone.GetKeyRange(),
            zone.GetReverbEffectsSend(),
            zone.GetPan(),
            zone.GetOverridingRootKey(),
            zone.GetSample()
        );
    });

    auto gz_it = std::find_if(izones.begin(), izones.end(), [](const IZoneTestArgs& iz) {
        return iz.sample.has_value() == false;
    });

    auto find_zone_by_name = [&](std::string_view name) {
        return std::find_if(
            izones.begin(),
            izones.end(),
            [&](const IZoneTestArgs& iz) {
                return
                    iz.sample.has_value() &&
                    sf2.GetSample(*iz.sample).GetName() == name;
            }
        );
    };
    auto lead_G3L_it = find_zone_by_name("Lead G3L");
    auto lead_G3R_it = find_zone_by_name("Lead G3R");
    auto lead_C4L_it = find_zone_by_name("Lead C4L");
    auto lead_C4R_it = find_zone_by_name("Lead C4R");
    auto lead_A4L_it = find_zone_by_name("Lead A4L");
    auto lead_A4R_it = find_zone_by_name("Lead A4R");
    auto lead_A5L_it = find_zone_by_name("Lead A5L");
    auto lead_A5R_it = find_zone_by_name("Lead A5R");

    REQUIRE_FALSE(gz_it == izones.end());
    REQUIRE_FALSE(lead_G3L_it == izones.end());
    REQUIRE_FALSE(lead_G3R_it == izones.end());
    REQUIRE_FALSE(lead_C4L_it == izones.end());
    REQUIRE_FALSE(lead_C4R_it == izones.end());
    REQUIRE_FALSE(lead_A4L_it == izones.end());
    REQUIRE_FALSE(lead_A4R_it == izones.end());
    REQUIRE_FALSE(lead_A5L_it == izones.end());
    REQUIRE_FALSE(lead_A5R_it == izones.end());

    const auto& lead_gz = *gz_it;
    const auto& lead_G3L = *lead_G3L_it;
    const auto& lead_G3R = *lead_G3R_it;
    const auto& lead_C4L = *lead_C4L_it;
    const auto& lead_C4R = *lead_C4R_it;
    const auto& lead_A4L = *lead_A4L_it;
    const auto& lead_A4R = *lead_A4R_it;
    const auto& lead_A5L = *lead_A5L_it;
    const auto& lead_A5R = *lead_A5R_it;

    CHECK(lead_gz.reverb_fx_send == 200);

    CHECK(lead_G3L.key_range.start == 0);
    CHECK(lead_G3L.key_range.end   == 56);
    CHECK(lead_G3L.panning == -500);
    CHECK(lead_G3L.root_key == 55);

    CHECK(lead_G3R.key_range.start == 0);
    CHECK(lead_G3R.key_range.end   == 56);
    CHECK(lead_G3R.panning == 500);
    CHECK(lead_G3R.root_key == 55);

    CHECK(lead_C4L.key_range.start == 57);
    CHECK(lead_C4L.key_range.end   == 67);
    CHECK(lead_C4L.panning == -500);
    CHECK(lead_C4L.root_key == 60);

    CHECK(lead_C4R.key_range.start == 57);
    CHECK(lead_C4R.key_range.end   == 67);
    CHECK(lead_C4R.panning == 500);
    CHECK(lead_C4R.root_key == 60);

    CHECK(lead_A4L.key_range.start == 68);
    CHECK(lead_A4L.key_range.end   == 79);
    CHECK(lead_A4L.panning == -500);
    CHECK(lead_A4L.root_key == 69);

    CHECK(lead_A4R.key_range.start == 68);
    CHECK(lead_A4R.key_range.end   == 79);
    CHECK(lead_A4R.panning == 500);
    CHECK(lead_A4R.root_key == 69);

    CHECK(lead_A5L.key_range.start == 80);
    CHECK(lead_A5L.key_range.end   == 127);
    CHECK(lead_A5L.panning == -500);
    CHECK(lead_A5L.root_key == 81);

    CHECK(lead_A5R.key_range.start == 80);
    CHECK(lead_A5R.key_range.end   == 127);
    CHECK(lead_A5R.panning == 500);
    CHECK(lead_A5R.root_key == 81);
}

TEST_CASE("Load Presets from file #1", "[loader][preset]") {
    SF2ML::SoundFont sf2;
    std::ifstream sf2_ifs(src_dir + "SF2ML_TEST1.sf2", std::ios::binary);
    REQUIRE(sf2.Load(sf2_ifs) == SF2ML::SF2ML_SUCCESS);

    CHECK(sf2.AllPresets().size() == 2);

    auto kick_h = sf2.FindPreset([](const SF2ML::SfPreset& preset) {
        return preset.GetName() == "Kick 1";
    });
    auto lead_h = sf2.FindPreset([](const SF2ML::SfPreset& preset) {
        return preset.GetName() == "Lead";
    });
    REQUIRE(kick_h.has_value());
    REQUIRE(lead_h.has_value());

    auto& kick = sf2.GetPreset(*kick_h);
    auto& lead = sf2.GetPreset(*lead_h);

    CHECK(kick.CountZones(false) == 1);
    CHECK(lead.CountZones(false) == 1);

    auto& kick_zone1 = kick.GetZone(kick.AllZoneHandles()[1]);
    auto& lead_zone1 = lead.GetZone(lead.AllZoneHandles()[1]);

    auto kick_inst_h = kick_zone1.GetInstrument();
    auto lead_inst_h = lead_zone1.GetInstrument();
    REQUIRE(kick_inst_h.has_value());
    REQUIRE(lead_inst_h.has_value());

    CHECK(sf2.GetInstrument(*kick_inst_h).GetName() == "Kick 1");
    CHECK(sf2.GetInstrument(*lead_inst_h).GetName() == "Lead");
    CHECK(kick_zone1.ModulatorCount() == 0);
    CHECK(lead_zone1.ModulatorCount() == 0);
}

TEST_CASE("Load from file #2", "[loader][inst][preset]") {
    SF2ML::SoundFont sf2;
    std::ifstream sf2_ifs(src_dir + "SF2ML_TEST2.sf2", std::ios::binary);
    REQUIRE(sf2.Load(sf2_ifs) == SF2ML::SF2ML_SUCCESS);

    auto insts = sf2.AllInstruments();
    auto presets = sf2.AllPresets();

    REQUIRE(insts.size() == 1);
    auto& sq_inst = sf2.GetInstrument(insts[0]);

    REQUIRE(presets.size() == 1);
    auto& sq_preset = sf2.GetPreset(presets[0]);

    CHECK(sq_preset.GetPresetNumber() == 2);
    CHECK(sq_preset.GetBankNumber() == 0);
    {
        auto zones = sq_preset.AllZoneHandles();
        REQUIRE(zones.size() == 3);

        

    }

    
}