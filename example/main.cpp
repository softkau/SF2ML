#include <iostream>
#include <fstream>
#include <string>

// include SF2ML library (you only need to include this specific file)
#include <SF2ML/sf2ml.hpp>

const std::string src_dir = "../test_src/Soundfont/";

int test3(int argc, char** argv) {
	// create (empty) SoundFont object
	SF2ML::SoundFont sf2;

	// set meta-datas (you can use method chaining here)
	sf2.Info()
		.SetSoundFontVersion({2, 1}) // set version to 2.1
		.SetBankName("SynthFile") // set Bank name to SynthFile
		.SetSoundEngine("EMU8000"); // set sound engine

	// add sample to SoundFont object
	std::ifstream tr_b1_file(src_dir + "Trombone B1.wav", std::ios::binary);
	if (tr_b1_file.is_open() == false) {
		std::cout << "failed to open Sample file." << std::endl;
		return 1;
	}
	auto [sample_handle, err] = sf2.AddMonoSample(tr_b1_file, "Trombone Sample");
	if (err) {
		// printing out errors
		std::cout << "error while adding a sample: " << SF2ML::ToStringView(err) << std::endl;
		return 1;
	}
	tr_b1_file.close();

	// add new instrument to SoundFont object
	auto handle = sf2.NewInstrument("Trombone Inst").GetHandle(); // you can get handles like this

	/// setting global zone
	sf2.GetInstrument(handle).GetGlobalZone()
		.SetReverbEffectsSend(500)
		.SetChorusEffectsSend(10);

	/// adding new zone & setting properties(generators)
	sf2.GetInstrument(handle).NewZone()
		.SetAttackVolEnv(0.01)
		.SetDecayVolEnv(0.5)
		.SetSustainVolEnv(70)
		.SetReleaseVolEnv(10)
		.SetSample(sample_handle); // links the sample that we just created

	// add new preset to SoundFont object
	sf2.NewPreset(57, 0, "Trombone Preset")
		.NewZone() // add new zone to Trombone Preset
			.SetInstrument(handle); // links instrument "Trombone Inst" to the zone


	// adding stereo sample seperately
	std::ifstream choir_l_file(src_dir + "Ahh Choir G#3_L.wav", std::ios::binary);
	std::ifstream choir_r_file(src_dir + "Ahh Choir G#3_R.wav", std::ios::binary);
	if (!choir_l_file.is_open()) {
		std::cout << "failed to open Sample file." << std::endl;
		return 1;
	}
	if (!choir_r_file.is_open()) {
		std::cout << "failed to open Sample file." << std::endl;
		return 1;
	}

	auto [sh1, err_sh1] = sf2.AddMonoSample(choir_l_file, "Ahh Choir G#3(L)", SF2ML::Ranges<SF2ML::DWORD>{28992,89051}, 45);
	auto [sh2, err_sh2] = sf2.AddMonoSample(choir_r_file, "Ahh Choir G#3(R)", SF2ML::Ranges<SF2ML::DWORD>{28992,89051}, 45);
	choir_l_file.close();
	choir_r_file.close();

	if (err_sh1 || err_sh2) {
		std::cout << "error while adding a sample<Ahh Choir G#3(L)>: " << SF2ML::ToStringView(err_sh1) << "\n";
		std::cout << "error while adding a sample<Ahh Choir G#3(R)>: " << SF2ML::ToStringView(err_sh2) << std::endl;
		return 1;
	}
	if (auto err = sf2.LinkSamples(sh1, sh2)) {
		std::cout << "error while linking samples: " << SF2ML::ToStringView(err) << "\n";
		// fall through because the error is not that fatal
	}

	auto ih = sf2.NewInstrument("Ahh Choir Stereo").GetHandle();
	auto iz1h = sf2.GetInstrument(ih).NewZone()
		.SetSampleModes(SF2ML::LoopMode::Loop)
		.SetPan(-500)
		.SetSample(sh1)
		.GetHandle();
	auto iz2h = sf2.GetInstrument(ih).NewZone()
		.SetSampleModes(SF2ML::LoopMode::Loop)
		.SetPan(500)
		.SetSample(sh2)
		.GetHandle();

	{ // adding modulators
		sf2.GetInstrument(ih).GetZone(iz1h).NewModulator()
			.SetSource(SF2ML::GeneralController::PitchWheel, 1, 0, SF2ML::SfModSourceType::Linear)
			.SetAmtSource(SF2ML::GeneralController::NoteOnVelocity, 0, 1, SF2ML::SfModSourceType::Concave)
			.SetDestination(SF2ML::SfGenInitialFilterFc)
			.SetTransform(SF2ML::SfModLinearTransform)
			.SetModAmount(-1);

		auto& iz2 = sf2.GetInstrument(ih).GetZone(iz2h);
		auto m1h = iz2.NewModulator()
			.SetSource(SF2ML::GeneralController::Link, 0, 0, SF2ML::SfModSourceType::Linear)
			.SetAmtSource(SF2ML::MidiController(4), 0, 0, SF2ML::SfModSourceType::Linear)
			.SetDestination(SF2ML::SfGenInitialFilterFc)
			.SetTransform(SF2ML::SfModLinearTransform)
			.SetModAmount(1)
			.GetHandle();
		iz2.NewModulator()
			.SetSource(SF2ML::GeneralController::NoteOnVelocity, 0, 0, SF2ML::SfModSourceType::Linear)
			.SetAmtSource(SF2ML::MidiController(2), 0, 0, SF2ML::SfModSourceType::Linear)
			.SetDestination(m1h)
			.SetModAmount(1)
			.SetTransform(SF2ML::SfModLinearTransform);
	}
	
	sf2.NewPreset(52, 0, "Ahh Choir Stereo").NewZone()
		.SetInstrument(
			sf2.FindInstrument([](const SF2ML::SfInstrument& i) {
				return i.GetName() == "Ahh Choir Stereo";
			})
		);
	
	std::ofstream output(src_dir + "test3.sf2");
	if (auto err = sf2.Save(output)) {
		std::cout << "error while saving: " << SF2ML::ToStringView(err) << std::endl;
		return 1;
	}

	//************MOAR advanced stuff!************

	// loading/reading existing sf2 file from disk (the SoundFont object is resetted)
	std::ifstream sf2_ifs(src_dir + "src.sf2", std::ios::binary);
	if (sf2_ifs.is_open() == false) {
		std::cout << "failed to open sound font file." << std::endl;
		return 1;
	}
	if (auto err = sf2.Load(sf2_ifs)) {
		std::cout << "error while loading sf2 file: " << SF2ML::ToStringView(err) << std::endl;
		return 1;
	}
	sf2_ifs.close(); // you can close the stream after loading is complete

	// you can search samples/instruments/presets like this
	auto search_rs1 = sf2.FindSample([](const SF2ML::SfSample& s) {
		return s.GetName() == "Sample 0";
	});

	if (search_rs1) {
		std::cout << "Sample rate of Sample 0: " <<
			sf2.GetSample(*search_rs1).GetSampleRate() << "Hz" << std::endl;
	} else {
		std::cout << "Sample 0 not found." << std::endl;
	}

	// this method returns a vector
	auto search_rs2 = sf2.FindInstruments([](const SF2ML::SfInstrument& i) {
		return i.CountZones() >= 3;
	});
	for (SF2ML::InstHandle ih : search_rs2) {
		const SF2ML::SfInstrument& inst = sf2.GetInstrument(ih);
		std::cout << "Visiting Instrument: " << inst.GetName() << "\n";

		int zone_idx = 0;
		// you can do "for-each" loop for zones (including global zones!)
		inst.ForEachZone([&](const SF2ML::SfInstrumentZone& iz) {
			std::cout << "  Zone(#" << zone_idx++ << "):\n";
			std::cout << "    VolAtk: " << iz.GetAttackVolEnv()  << "sec(s)\n";
			std::cout << "    VolDec: " << iz.GetDecayVolEnv()   << "sec(s)\n";
			std::cout << "    VolSus: " << iz.GetSustainVolEnv() << "cB\n";
			std::cout << "    VolRel: " << iz.GetReleaseVolEnv() << "sec(s)\n";
		});

		std::cout << std::endl;
	}

	return 0;
}


int main(int argc, char** argv) {
	return test3(argc, argv);
}