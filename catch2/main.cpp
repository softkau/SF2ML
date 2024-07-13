#include <catch2/catch_session.hpp>
#include <iostream>

std::string src_dir;

int main(int argc, char* argv[]) {
	Catch::Session session;

	auto cli = session.cli()
		| Catch::Clara::Opt(src_dir, "path")
			["-D"]["--src-directory"]
			("path where test sf2 files exists.");

	session.cli(cli);

	int ret_code = session.applyCommandLine(argc, argv);
	if (ret_code != 0) {
		return ret_code;
	}

	if (src_dir.empty()) {
		src_dir = ".";
	}

	std::cout << "SF2 Source Directory: " << src_dir << std::endl;
	src_dir += "/";

	int num_failed = session.run();

	return num_failed;
}