#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <cstdio>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h"

#include "gen/out.pb.h"

#include "mutator.h"

inline std::string slurp(const std::string &path)
{
	std::ostringstream buf;
	std::ifstream input(path.c_str());
	buf << input.rdbuf();
	return buf.str();
}

extern "C"
{
	void *afl_custom_init(void *afl, unsigned int seed);
	size_t afl_custom_fuzz(void *data, unsigned char *buf, size_t buf_size, unsigned char **out_buf,
						   unsigned char *add_buf, size_t add_buf_size, size_t max_size);
	size_t afl_custom_post_process(void *data, uint8_t *buf, size_t buf_size, uint8_t **out_buf);
	void afl_custom_deinit(void *data);
}

int main(int argc, char *argv[])
{
	menuctf::Choices msg;

	// 测试变异逻辑
	int rand_fd;
	if ((rand_fd = open("/dev/random", O_RDONLY)) < 0)
		abort();
	unsigned int seed;
	read(rand_fd, &seed, sizeof(seed));
	close(rand_fd);

	void *init_data = afl_custom_init(nullptr, seed);
	if (argc > 1)
	{
		if (!strcmp(argv[1], "gen"))
		{
			uint8_t *out_buf = nullptr;
			size_t new_size = afl_custom_fuzz(nullptr, nullptr, 0,
												&out_buf, nullptr, 0, 0x2000);
			uint8_t *new_str = nullptr;
			size_t new_str_size = afl_custom_post_process(init_data, out_buf, new_size, &new_str);
			std::string new_str_str((char *)new_str, new_str_size);
			std::cout << new_str_str << std::endl
						<< std::endl;
		}
		if (!strcmp(argv[1], "-d") && argc > 2)
		{
			struct stat statbuf;
			stat(argv[2], &statbuf);
			int proto_size = statbuf.st_size;

			char* proto_buf = new char [proto_size + 1];
			
			FILE* proto_fp = fopen(argv[2], "r");
			if (!proto_fp) { return -1; }
			fread(proto_buf, proto_size, 1, proto_fp);

			menuctf::Choices msg;
			std::stringstream stream;
			// 如果加载成功
			if (protobuf_mutator::libfuzzer::LoadProtoInput(true, (const unsigned char *) proto_buf, proto_size, &msg))
			{
				ProtoToDataHelper(stream, msg);
			}
			std::cout << stream.str();
		}
	}
	else
	{
		for (int i = 0; i < 5; i++)
		{
			uint8_t *out_buf = nullptr;
			size_t new_size = afl_custom_fuzz(nullptr, nullptr, 0,
												&out_buf, nullptr, 0, 0x2000);
			uint8_t *new_str = nullptr;
			size_t new_str_size = afl_custom_post_process(init_data, out_buf, new_size, &new_str);
			std::string new_str_str((char *)new_str, new_str_size);
			std::cout << i << " =============== " << std::endl;
			std::cout << new_str_str << std::endl
						<< std::endl;
		}
	}
	afl_custom_deinit(init_data);

	// std::cout << "msg DebugString: " << msg.DebugString() << std::endl;
//	std::stringstream stream;
//	ProtoToDataHelper(stream, msg);
//	std::cout << stream.str() << std::endl;

	return 0;
}