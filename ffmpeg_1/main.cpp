
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
using namespace std;

void save_frame(AVFrame* frame, int width, int height, int iframe)
{
	auto filename = "frame" + to_string(iframe) + ".ppm";
	ofstream out(filename, ios::binary);
	if (!out.is_open()) {
		cerr << "Failed to open file " << filename << endl;
		abort();
	}

	// write header
	auto header = "P6\n" + to_string(width) + " " + to_string(height) + "\n255\n";
	out.write(header.c_str(), header.length());

	// write pixel data
	for (int y = 0; y < height; y++) {
		out.write(reinterpret_cast<const char*>(frame->data[0] + y * frame->linesize[0]), width * 3);
	}

	out.close();
}

int main(int argc, char** argv)
{
	av_register_all();

	// auto video_file = argv[1];
	//auto video_file = "video.avi";
	//auto video_file = "Gee.mp4";
	auto video_file = "thoseyears.mp4";

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

		unsigned int stream_index = -1;
		for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
			if (format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
				stream_index = i;
				break;
			}
		}

		if (stream_index == -1) {
			cerr << " Failed to find a video stream" << endl;
			break;
		}

		// get a pointer to the codec context for the video stream
		auto codec_ctx = format_ctx->streams[stream_index]->codec;

		// find the decoder for the video stream
		auto codec = avcodec_find_decoder(codec_ctx->codec_id);
		if (!codec) {
			cerr << "unsupported codec" << endl;
			break;
		}

		// open codec
		AVDictionary* options = nullptr;
		if (avcodec_open2(codec_ctx, codec, &options) < 0) {
			cerr << "couldn't open codec" << endl;
			break;
		}

		// allocate video frame
		auto frame = av_frame_alloc();

		// allocate an AVFrame structure
		auto frame_rgb = av_frame_alloc();
		if (!frame_rgb) {
			cerr << "Failed to allocate frame_rgb" << endl;
			break;
		}

		// determine required buffer size and allocate buffer
		auto numbytes = avpicture_get_size(AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height);
		auto buffer = reinterpret_cast<uint8_t*>(av_malloc(numbytes * sizeof(uint8_t)));

		auto sws_ctx = sws_getContext(codec_ctx->width, 
									  codec_ctx->height, 
									  codec_ctx->pix_fmt, 
									  codec_ctx->width, 
									  codec_ctx->height, 
									  AV_PIX_FMT_RGB24, 
									  SWS_BILINEAR, 
									  nullptr, 
									  nullptr, 
									  nullptr);

		// assign appropriate parts of buffer to image planes in frame_rgb
		// note that frame_rgb is an AVFrame, but AVFrame is a superset of AVPicture
		avpicture_fill(reinterpret_cast<AVPicture*>(frame_rgb), buffer, AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height);

		// read frames and save first five frames to disk
		AVPacket packet = {};
		int iframe = 0;
		while (av_read_frame(format_ctx, &packet) >= 0) {
			// is this a packet from the video stream?
			if (packet.stream_index == stream_index) {
				// decode video frame
				int frame_finished = 0;
				avcodec_decode_video2(codec_ctx, frame, &frame_finished, &packet);

				// did we get a video frame?
				if (frame_finished) {
					// convert the image from its native format to RGB
					sws_scale(sws_ctx, 
							  reinterpret_cast<const uint8_t* const*>(frame->data),
							  frame->linesize,
							  0,
							  codec_ctx->height,
							  frame_rgb->data,
							  frame_rgb->linesize);

					// save frame to disk
					save_frame(frame_rgb, codec_ctx->width, codec_ctx->height, iframe);
					cout << "decode " << iframe << " frame" << endl;
					iframe++;
				}
			}

			// free the packet allocated by av_read_frame
			av_free_packet(&packet);
		}

		// free the rgb image
		av_free(buffer);
		av_free(frame_rgb);

		// free the yuv frame
		av_free(frame);

		// close codec
		avcodec_close(codec_ctx);

		// close video file
		avformat_close_input(&format_ctx);

	} while (0);

	getchar();
}

