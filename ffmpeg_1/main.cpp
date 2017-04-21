
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <iostream>
using namespace std;

int main(int argc, char** argv)
{
	av_register_all();

	// auto video_file = argv[1];
	auto video_file = "video.avi";

	do {
		AVFormatContext* format_ctx = nullptr;
		if (avformat_open_input(&format_ctx, video_file, nullptr, nullptr) != 0) {
			cerr << "Failed to avformat_open_input" << endl;
			break;
		}

		if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
			cerr << "Failed to avformat_find_stream_info" << endl;
			break;
		}

		av_dump_format(format_ctx, 0, video_file, 0);
		avformat_close_input(&format_ctx);

	} while (0);

	getchar();
}

