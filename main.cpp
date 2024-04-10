#include <iostream>
#include <fstream>
#include <string>
#include "sflib.hpp"

int main(int argc, char** argv) {
	std::string sf2path = "./Soundfont/BANK_MUS_WB_ENDING.sf2";
	if (argc > 1) {
		sf2path.assign(argv[1]);
	}

	std::ifstream file;

	file.open(sf2path, std::ios_base::binary);
	sflib::SoundFont2 sf2(file);
	file.close();

	std::string smpl_mono_path = "./Soundfont/mono.wav";
	std::ifstream smpl_mono(smpl_mono_path, std::ios_base::binary);
	smpl_mono.seekg(0, std::ios::end);
	size_t size = smpl_mono.tellg();
	smpl_mono.seekg(0, std::ios::beg);
	size -= smpl_mono.tellg();

	char* buf = new char[size];
	smpl_mono.read(buf, size);
	smpl_mono.close();
	
	auto [id, err] = sf2.Samples()->AddMono(buf, size, "dumb wave");
	delete[] buf;

	std::cout << sf2.Samples()->GetName(id) << std::endl;
	std::cout << sf2.Samples()->GetSampleRate(id) << std::endl;
	sf2.Samples()->Remove(id, sflib::RemovalMode::Restrict);

	return 0;
}