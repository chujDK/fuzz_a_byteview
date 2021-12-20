#include "mutator.h"

#include <iostream>
#include <unistd.h>

#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/reflection.h"

#include "libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h"

#include "gen/out.pb.h"

#include <algorithm>
#include <random>
#include <cstdio>

void ProtoToDataHelper(std::stringstream &out, const google::protobuf::Message &msg)
{
	const google::protobuf::Descriptor *desc = msg.GetDescriptor();
	const google::protobuf::Reflection *refl = msg.GetReflection();

	const unsigned fields = desc->field_count();
	for (unsigned i = 0; i < fields; ++i)
	{
		const google::protobuf::FieldDescriptor *field = desc->field(i);

		if (field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE)
		{
			if (field->is_repeated())
			{
				// maybe choice list or data list
				const google::protobuf::RepeatedFieldRef<google::protobuf::Message> &ptr = refl->GetRepeatedFieldRef<google::protobuf::Message>(msg, field);
				for (const auto &child : ptr)
				{
					ProtoToDataHelper(out, child);
				}
			}
			else if (refl->HasField(msg, field))
			{
				// maybe one choice or one data
				const google::protobuf::Message &child = refl->GetMessage(msg, field);
				ProtoToDataHelper(out, child);
			}
		}
		else if (field->cpp_type() ==
				 google::protobuf::FieldDescriptor::CPPTYPE_INT32)
		{
			out.width(8);
			out << refl->GetInt32(msg, field);
		}
		else if (field->cpp_type() ==
				 google::protobuf::FieldDescriptor::CPPTYPE_STRING)
		{
			out << refl->GetString(msg, field) << "\n";
		}
		else
		{
			abort();
		}
	}
}

// Apparently the textual generator kinda breaks?
DEFINE_BINARY_PROTO_FUZZER(const menuctf::Choices &root)
{
	std::stringstream stream;
	ProtoToDataHelper(stream, root);
	std::string data = stream.str();
	std::replace(data.begin(), data.end(), '\n', '|');
	puts(("ProtoToDataHelper: " + data).c_str());
	puts("====================================================================================================");
	// std::string dbg = root.DebugString();
	// std::replace(dbg.begin(), dbg.end(), '\n', ' ');
}

static int sum_of_datas = 0;
static int size_of_data[11];
static char content_buf[0xE0];

void new_content(menuctf::Choices& msg)
{
	std::random_device rd;
	auto choice = new menuctf::New_content();
	int sum = rd() % 11;
	choice->set_sum_of_datas(sum);

	if (sum < 10)
	{
		sum_of_datas = sum;
		for (int i = 0; i < sum_of_datas; i++)
		{
			auto data = choice->add_datas();
			data->set_key(i);
			int size = rd() % 0xCF + 1;
			size_of_data[i] = size;
			data->set_size(size);
		}
	}

	msg.add_choice()->set_allocated_new_content(choice);
}

void edit_message(menuctf::Choices& msg)
{
	if (!sum_of_datas) 
	{
		return;
	}
	auto choice = new menuctf::Edit_message();
	std::random_device rd;
	int idx = rd() % sum_of_datas;
	choice->set_key(idx);
	for (int i = 0; i < size_of_data[idx]; i++)
	{
		content_buf[i] = rd() % 127 + 1;
		while (content_buf[i] == '\n')
		{
			content_buf[i] = rd() % 127 + 1;
		}
	}
	content_buf[size_of_data[idx]] = 0;
	choice->set_content(content_buf);

	msg.add_choice()->set_allocated_edit_message(choice);
}

void show_message(menuctf::Choices& msg)
{
	if (!sum_of_datas)
	{
		return;
	}
	auto choice = new menuctf::Show_message();
	std::random_device rd;
	int idx = rd() % sum_of_datas;
	choice->set_key(idx);

	msg.add_choice()->set_allocated_show_message(choice);
}

void old_content(menuctf::Choices& msg)
{
	auto choice = new menuctf::Old_content();

	msg.add_choice()->set_allocated_old_content(choice);
}

void content_info(menuctf::Choices& msg)
{
	auto choice = new menuctf::Content_info();

	msg.add_choice()->set_allocated_content_info(choice);
}

void exit_choice(menuctf::Choices& msg)
{
	auto choice = new menuctf::Exit();

	msg.add_choice()->set_allocated_exit(choice);
}

// AFLPlusPlus interface
extern "C"
{
	static std::default_random_engine engine_pro;
	static std::uniform_int_distribution<unsigned int> dis(0, UINT32_MAX);

	void *afl_custom_init(void *afl, unsigned int seed)
	{
#pragma unused(afl)
		engine_pro.seed(seed);
		return nullptr;
	}

	void afl_custom_deinit(void *data)
	{
		assert(!data);
	}

	/**
	 * Perform custom mutations on a given input
	 *
	 * (Optional for now. Required in the future)
	 *
	 * @param[in] data pointer returned in afl_custom_init for this fuzz case
	 * @param[in] buf Pointer to input data to be mutated
	 * @param[in] buf_size Size of input data
	 * @param[out] out_buf the buffer we will work on. we can reuse *buf. NULL on
	 * error.
	 * @param[in] add_buf Buffer containing the additional test case
	 * @param[in] add_buf_size Size of the additional test case
	 * @param[in] max_size Maximum size of the mutated output. The mutation must not
	 *     produce data larger than max_size.
	 * @return Size of the mutated output.
	 */
	// afl_custom_fuzz
	size_t afl_custom_fuzz(void *data, unsigned char *buf, size_t buf_size, unsigned char **out_buf,
						   unsigned char *add_buf, size_t add_buf_size, size_t max_size)
	{
#pragma unused(data)
#pragma unused(buf)
#pragma unused(buf_size)
#pragma unused(add_buf)
#pragma unused(add_buf_size)

		menuctf::Choices msg;
		std::random_device rd;

		int i = 0;
		while (i++ < 100)
		{
			if (rd() % 100 > 80)
			{
				exit_choice(msg);
				break;
			}
			switch (rd() % 5)
			{
			case 0:
				new_content(msg);
				break;
			case 1:
				edit_message(msg);
				break;
			case 2:
				show_message(msg);
				break;
			case 3:
				old_content(msg);
				break;
			case 4:
				content_info(msg);
				break;
			default:
				break;
			}
		}

		static uint8_t *saved_buf = nullptr;

		assert(buf_size <= max_size);

		uint8_t *new_buf = (uint8_t *)realloc((void *)saved_buf, max_size);
		saved_buf = new_buf;

		std::string serialize_data;
		msg.SerializePartialToString(&serialize_data);
		memcpy(new_buf, serialize_data.c_str(), msg.ByteSizeLong());
		*out_buf = new_buf;

		return msg.ByteSizeLong();
	}

	size_t afl_custom_post_process(void *data, uint8_t *buf, size_t buf_size, uint8_t **out_buf)
	{
#pragma unused(data)
		// new_data is never free'd by pre_save_handler
		// I prefer a slow but clearer implementation for now

		static uint8_t *saved_buf = NULL;

		menuctf::Choices msg;
		std::stringstream stream;
		// 如果加载成功
		if (protobuf_mutator::libfuzzer::LoadProtoInput(true, buf, buf_size, &msg))
		{
			ProtoToDataHelper(stream, msg);
		}
		else
		{
			// printf("[afl_custom_post_process] LoadProtoInput Error\n");
			// std::ofstream err_bin("err.bin");
			// err_bin.write((char*)buf, buf_size);

			// abort();

			// 如果加载失败，则返回 Exit Choice
			/// NOTE: 错误的变异 + 错误的 trim 将会导致 post process 加载失败，尤其是 trim 逻辑。
			/// TODO: 由于默认的 trim 会破坏样例，因此需要手动实现一个 trim，这里实现了一个空 trim，不进行任何操作
			ProtoToDataHelper(stream, menuctf::Exit());
		}
		const std::string str = stream.str();

		uint8_t *new_buf = (uint8_t *)realloc((void *)saved_buf, str.size());
		if (!new_buf)
		{
			*out_buf = buf;
			return buf_size;
		}
		*out_buf = saved_buf = new_buf;

		memcpy((void *)new_buf, str.c_str(), str.size());

		return str.size();
	}

	int32_t afl_custom_init_trim(void *data, uint8_t *buf, size_t buf_size)
	{
		/// NOTE: disable trim
		return 0;
	}

	size_t afl_custom_trim(void *data, uint8_t **out_buf)
	{
		/// NOTE: unreachable
		return 0;
	}
}
