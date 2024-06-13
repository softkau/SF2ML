#include <iostream>
#include <fstream>
#include <string>
#include "sflib.hpp"

int test0(int argc, char** argv) {
	sflib::SoundFont sf2;
	sf2.GetInfo()
		.SetSoundFontVersion(2, 1)
		.SetBankName("SynthFile")
		.SetSoundEngine("EMU8000");

	std::ifstream tr_b1_file("./Soundfont/Trombone B1.wav", std::ios::binary);
	auto [sh, err] = sf2.AddMonoSample(tr_b1_file, "Trombone Sample");
	auto handle = sf2.NewInstrument("Trombone Inst").GetHandle();
	sf2.GetInstrument(handle).GetGlobalZone()
		.SetReverbEffectsSend(500)
		.SetChorusEffectsSend(10);

	sf2.GetInstrument(handle).NewZone()
		.SetAttackVolEnv(0.01)
		.SetDecayVolEnv(0.5)
		.SetSustainVolEnv(70)
		.SetReleaseVolEnv(10)
		.SetSampleHandle(sh);

	std::ofstream dumb("./Soundfont/lmao.sf2");
	sf2.Save(dumb);

	return 0;
}

int test2(int argc, char** argv) {
	sflib::SoundFont sf2;
	sf2.GetInfo()
		.SetSoundFontVersion(2, 1)
		.SetBankName("SynthFile")
		.SetSoundEngine("EMU8000");

	std::ifstream tr_b1_file("./Soundfont/Trombone B1.wav", std::ios::binary);
	std::ifstream tr_e2_file("./Soundfont/Trombone E2.wav", std::ios::binary);
	std::ifstream tr_c4_file("./Soundfont/Trombone C4.wav", std::ios::binary);
	
	auto [sh1, err1] = sf2.AddMonoSample(tr_b1_file, "Trombone Low");
	auto [sh2, err2] = sf2.AddMonoSample(tr_e2_file, "Trombone High");
	auto [sh3, err3] = sf2.AddMonoSample(tr_c4_file, "Trombone Mid");

	if (err1 | err2 | err3) {
		std::cout << "Sample 1: " << sflib::SflibErrorStr[err1] << std::endl;
		std::cout << "Sample 2: " << sflib::SflibErrorStr[err2] << std::endl;
		std::cout << "Sample 3: " << sflib::SflibErrorStr[err3] << std::endl;
		return 1;
	}

	auto handle = sf2.NewInstrument("Trombone Inst").GetHandle();
	sf2.GetInstrument(handle).GetGlobalZone()
		.SetReverbEffectsSend(500)
		.SetChorusEffectsSend(10);

	sf2.GetInstrument(handle).NewZone()
		.SetAttackVolEnv(0.01)
		.SetDecayVolEnv(0.5)
		.SetSustainVolEnv(70)
		.SetReleaseVolEnv(10)
		.SetKeyRange(sflib::Ranges<uint8_t>{ 0, 47 })
		.SetSampleHandle(sh1);

	auto zone2_handle = sf2.GetInstrument(handle).NewZone()
		.SetAttackVolEnv(0.001)
		.SetDecayVolEnv(0.2)
		.SetHoldVolEnv(1.0)
		.SetSustainVolEnv(70)
		.SetReleaseVolEnv(5)
		.SetKeyRange(sflib::Ranges<uint8_t>{ 48, 59 })
		.SetSampleHandle(sh2)
		.GetHandle();

	sf2.GetInstrument(handle).NewZone()
		.SetAttackVolEnv(0.001)
		.SetDecayVolEnv(0.2)
		.SetHoldVolEnv(1.0)
		.SetSustainVolEnv(70)
		.SetReleaseVolEnv(5)
		.SetKeyRange(sflib::Ranges<uint8_t>{ 60, 127 })
		.SetSampleHandle(sh3);

	sf2.RemoveSample(sh2);
	sf2.GetInstrument(handle)
		.RemoveZone(zone2_handle);

	sf2.NewPreset(0, 0, "Trombone Preset")
		.NewZone()
		.SetInstrument(handle);

	std::ofstream dumb("./Soundfont/lmao.sf2");
	if (auto err = sf2.Save(dumb)) {
		std::cout << "Error while serializing: " << sflib::SflibErrorStr[err] << std::endl;
		return 1;
	}

	return 0;
}

int test1(int argc, char** argv) {
	std::string sf2path = "./Soundfont/BANK_MUS_WB_ENDING.sf2";
	if (argc > 1) {
		sf2path.assign(argv[1]);
	}

	sflib::SoundFont sf2; {
		std::ifstream file;
		file.open(sf2path, std::ios_base::binary);
		if (sf2.Load(file)) {
			std::cout << "error while loading: " << sf2path << std::endl;
			return 1;
		}
		file.close();
	}

	// L / R sample link example (manually)
	auto smpl2_hand = sf2.FindSample([](const sflib::SfSample& smpl) { return smpl.GetName() == "Sample 2"; });
	auto smpl3_hand = sf2.FindSample([](const sflib::SfSample& smpl) { return smpl.GetName() == "Sample 3"; });
	if (smpl2_hand.has_value() && smpl3_hand.has_value()) {
		sf2.GetSample(*smpl2_hand)
			.SetLink(*smpl3_hand)
			.SetSampleMode(sflib::leftSample);
		sf2.GetSample(*smpl3_hand)
			.SetLink(*smpl2_hand)
			.SetSampleMode(sflib::rightSample);

		// break linkage
		sf2.GetSample(*smpl2_hand).SetLink(std::nullopt);
		sf2.GetSample(*smpl3_hand).SetLink(std::nullopt);
	}
	// L / R sample link example (slightly easier way)
	if (smpl2_hand.has_value() && smpl3_hand.has_value()) {
		// additionally, this will check bad linkage before actually linking (loops and stuff)
		auto err = sf2.LinkSamples(*smpl2_hand, *smpl3_hand);
		if (err) {
			std::cout << sflib::SflibErrorStr[err] << std::endl;
		}

		// you can break linkage the same way as mentioned above
	}

	// add trombone instrument/preset
	std::ifstream tr_b1_file("./Soundfont/Trombone B1.wav", std::ios::binary);
	std::ifstream tr_e2_file("./Soundfont/Trombone E2.wav", std::ios::binary);
	std::ifstream tr_c4_file("./Soundfont/Trombone C4.wav", std::ios::binary);
	if (tr_b1_file.is_open() == false) {
		std::cout << "ERROR" << std::endl;
	}
	auto [tr_b1_hand, err11] = sf2.AddMonoSample(tr_b1_file, "Trombone B1");
	auto [tr_e2_hand, err12] = sf2.AddMonoSample(tr_e2_file, "Trombone E2");
	auto [tr_c4_hand, err13] = sf2.AddMonoSample(tr_c4_file, "Trombone C4");
	sflib::SfHandle samples[] = { tr_b1_hand, tr_e2_hand, tr_c4_hand };
	sflib::Ranges<uint8_t> krange[] = { {0, 20}, {21, 40}, {41, 127} };

	// (you can close all sample files after adding)
	tr_b1_file.close();
	tr_e2_file.close();
	tr_c4_file.close();

	int i = 0;
	auto tr_inst_hand = sf2.NewInstrument("Trombone").GetHandle();
	for (auto smpl_hand : samples) {
		sf2.GetInstrument(tr_inst_hand).NewZone()
			.SetKeyRange(krange[i])
			.SetAttackVolEnv(0.001)
			.SetDecayVolEnv(0.1)
			.SetSustainVolEnv(200)
			.SetReleaseVolEnv(2.5)
			.SetSampleHandle(smpl_hand);

		i++;
	}

	{
		auto& new_preset = sf2.NewPreset(70, 0, "Trombone");
		new_preset.GetGlobalZone()
			.SetChorusEffectsSend(200)
			.SetReverbEffectsSend(250);
		new_preset.NewZone()
			.SetInstrument(tr_inst_hand);
	}

	// save the soundfont data into disk
	std::ofstream new_sf2("./Soundfont/sfoutput.sf2");
	if (new_sf2.is_open() == false) {
		std::cout << "failed to open output file" << std::endl;
		return 1;
	}
	if (auto err = sf2.Save(new_sf2)) {
		std::cout << sflib::SflibErrorStr[err] << std::endl;
		new_sf2.close();
		return 1;
	}
	new_sf2.close();
	
	#if 0
	auto smpl7_hand = sf2.FindSample([](const sflib::SfSample& x) {
		return x.GetName() == "Sample 7";
	});
	std::ofstream ex_wav("output_Sample 7.wav", std::ios::binary);
	sf2.ExportWav(ex_wav, *smpl7_hand);
	ex_wav.close();
	#endif

	return 0;
}

int main(int argc, char** argv) {
	test1(argc, argv);
	return 0;
}