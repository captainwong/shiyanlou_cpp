extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <SDL.h>
#include <SDL_thread.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <memory>
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

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	struct sdl_destroy {
		~sdl_destroy() {
			SDL_Quit();
		}
	}sdl_destroyer;

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
		auto av_frame_deleter = [](AVFrame* frame) {av_free(frame); };
		auto frame = std::unique_ptr<AVFrame, decltype(av_frame_deleter)>(av_frame_alloc(), av_frame_deleter);

		// make a scren to put our video
		auto sdl_win_deleter = [](SDL_Window* win) { SDL_DestroyWindow(win); };
		auto window = std::unique_ptr<SDL_Window, decltype(sdl_win_deleter)>
			(SDL_CreateWindow("Hello World!",
							  SDL_WINDOWPOS_UNDEFINED,
							  SDL_WINDOWPOS_UNDEFINED,
							  codec_ctx->width,
							  codec_ctx->height,
							  SDL_WINDOW_SHOWN),
			 sdl_win_deleter);

		if (window == nullptr) {
			std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
			break;
		}

		auto sdl_renderer_deleter = [](SDL_Renderer* renderer) {SDL_DestroyRenderer(renderer); };
		auto renderer = std::unique_ptr<SDL_Renderer, decltype(sdl_renderer_deleter)>(
			SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
			sdl_renderer_deleter);
		if (renderer == nullptr) {
			std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
			break;
		}

		// Allocate a place to put our YUV image on that screen
		auto sdl_texture_deleter = [](SDL_Texture* texture) {SDL_DestroyTexture(texture); };
		auto texture = std::unique_ptr<SDL_Texture, decltype(sdl_texture_deleter)>(
			SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, codec_ctx->width, codec_ctx->height),
			sdl_texture_deleter);
		if (!texture) {
			std::cout << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
			break;
		}

		// set up YV12 pixel array (12 bits per pixel)
		auto yPlaneSz = codec_ctx->width * codec_ctx->height;
		auto uvPlaneSz = codec_ctx->width * codec_ctx->height / 4;
		auto yPlane = (Uint8*)malloc(yPlaneSz);
		auto uPlane = (Uint8*)malloc(uvPlaneSz);
		auto vPlane = (Uint8*)malloc(uvPlaneSz);
		if (!yPlane || !uPlane || !vPlane) {
			fprintf(stderr, "Could not allocate pixel buffers - exiting\n");
			exit(1);
		}

		auto uvPitch = codec_ctx->width / 2;

		auto sws_ctx = sws_getContext(codec_ctx->width,
									  codec_ctx->height,
									  codec_ctx->pix_fmt,
									  codec_ctx->width,
									  codec_ctx->height,
									  AV_PIX_FMT_YUV420P,
									  SWS_BILINEAR,
									  nullptr,
									  nullptr,
									  nullptr);

		

		// read frames and save first five frames to disk
		AVPacket packet = {};
		int iframe = 0;
		while (av_read_frame(format_ctx, &packet) >= 0) {
			// is this a packet from the video stream?
			if (packet.stream_index == stream_index) {
				// decode video frame
				int frame_finished = 0;
				avcodec_decode_video2(codec_ctx, frame.get(), &frame_finished, &packet);

				// did we get a video frame?
				if (frame_finished) {
					AVPicture pict;
					pict.data[0] = yPlane;
					pict.data[1] = uPlane;
					pict.data[2] = vPlane;
					pict.linesize[0] = codec_ctx->width;
					pict.linesize[1] = uvPitch;
					pict.linesize[2] = uvPitch;

					// Convert the image into YUV format that SDL uses
					sws_scale(sws_ctx, (uint8_t const * const *)frame->data,
							  frame->linesize, 0, codec_ctx->height, pict.data,
							  pict.linesize);

					SDL_UpdateYUVTexture(
						texture.get(),
						nullptr,
						yPlane,
						codec_ctx->width,
						uPlane,
						uvPitch,
						vPlane,
						uvPitch
					);

					SDL_RenderClear(renderer.get());
					SDL_RenderCopy(renderer.get(), texture.get(), nullptr, nullptr);
					SDL_RenderPresent(renderer.get());
				}
			}

			// free the packet allocated by av_read_frame
			av_free_packet(&packet);

			bool exiting = false;
			SDL_Event se;
			SDL_PollEvent(&se);
			switch (se.type) {
			case SDL_QUIT:
				exiting = true;
				break;
			default:
				SDL_Delay(20);
				break;
			}
			
			if (exiting) {
				break;
			}
		}

		// close codec
		avcodec_close(codec_ctx);

		// close video file
		avformat_close_input(&format_ctx);

		return 0;

	} while (0);

	system("pause");

	return 1;
}

