#include "Video.hpp"

WFLGlClass WFLGl;

//esse arquivo sera usando para o swap de APIs gráficas

void VideoClass::init(libretro_external_data* data, retro_game_geometry* geometry) {
	WFLGl.init(data, geometry);
}

void VideoClass::deinit() {
	WFLGl.deinit();
}

bool VideoClass::setPixelFormat(unsigned format, libretro_external_data* data) {
	return WFLGl.setPixelFormat(format, data);
}

void VideoClass::refreshVertexData() {
	WFLGl.refreshVertexData();
}

void VideoClass::resizeToAspect(double ratio, int sw, int sh, int* dw, int* dh) {
	WFLGl.resizeToAspect(ratio, sw, sh, dw, dh);
}

void VideoClass:: videoRefresh(const void* data, unsigned width, unsigned height, unsigned pitch) {
	WFLGl.videoRefresh(data, width, height, pitch);
}
