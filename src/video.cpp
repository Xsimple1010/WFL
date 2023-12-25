#include "Video.hpp"

WFLGlClass WFLGl;

//esse arquivo sera usando para o swap de APIs gráficas
void VideoClass::init(retro_game_geometry* geometry) {
	WFLGl.init(geometry);
	videoInfo = &WFLGl.videoInfo;
}

void VideoClass::deinit() {
	WFLGl.deinit();
}

bool VideoClass::setPixelFormat(unsigned format) {
	return WFLGl.setPixelFormat(format);
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

void VideoClass::setToDefaultValues() {
	WFLGl.setToDefaultValues();
}
