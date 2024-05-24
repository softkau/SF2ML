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
	sflib::SountFont2 sf2(file);
	file.close();

	std::string smpl_mono_path = "./Soundfont/Trombone B1.wav";
	std::ifstream smpl_mono(smpl_mono_path, std::ios_base::binary);
	smpl_mono.seekg(0, std::ios::end);
	size_t size = smpl_mono.tellg();
	smpl_mono.seekg(0, std::ios::beg);
	size -= smpl_mono.tellg();

	char* buf = new char[size];
	smpl_mono.read(buf, size);
	smpl_mono.close();
	
	sflib::SfHandle smpl2 = *sf2.Samples()->FindSample("Sample 2");
	
	auto [handle, err] = sf2.Samples()->AddMono(buf, size, "dumb wave");
	delete[] buf;

	if (err) {
		std::cout << "Error while inserting wav" << std::endl;
	}

	sf2.Samples()->Remove(smpl2, sflib::RemovalMode::Restrict);

	std::size_t sz = sf2.Samples()->ChunkSize();
	std::vector<sflib::BYTE> data(sz, static_cast<sflib::BYTE>(0));
	sflib::BYTE* dst = data.data();
	sflib::BYTE* end = dst;
	sf2.Samples()->SerializeSDTA(dst, &end);
	sf2.Samples()->SerializeSHDR(end);

	

	std::ofstream ofs("output3.tmp", std::ios_base::binary);
	ofs.write(reinterpret_cast<const char*>(dst), data.size());
	ofs.close();

	return 0;
}