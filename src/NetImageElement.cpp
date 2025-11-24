#include "NetImageElement.hpp"

namespace Chesto {

NetImageElement::NetImageElement(const char *url, std::function<Texture *(void)> getImageFallback, bool immediateLoad)
{
	std::string key = std::string(url);
	// printf("Key: %s\n", key.c_str());
	if (loadFromCache(key)) {
		loaded = true;

		// if we're using the cache, we can update the size now
		// printf("The size of the image is %d x %d\n", texW, texH);
		width = texW;
		height = texH;
	}
	else {
		// setup a temporary image fallback
		if (getImageFallback)
			imgFallback = getImageFallback();

		// start downloading the correct image
		imgDownload = new DownloadOperation();
		imgDownload->url = std::string(url);
		imgDownload->cb = std::bind(&NetImageElement::imgDownloadComplete, this, std::placeholders::_1);

		// load immediately
		if (immediateLoad)
			fetch();
	}
}

NetImageElement::~NetImageElement()
{
	if (imgFallback)
		delete imgFallback;

	if (imgDownload)
	{
		DownloadQueue::downloadQueue->downloadCancel(imgDownload);
		delete imgDownload;
	}
}

void NetImageElement::fetch()
{
	if (!downloadStarted && imgDownload) {
		downloadStarted = true; // guard against re-entry
		DownloadQueue::downloadQueue->downloadAdd(imgDownload);
	}
}

bool NetImageElement::process(InputEvents* event)
{
	bool ret = Texture::process(event); // updates xAbs/yAbs
	
	// fetch the image if we're visible onscreen
	if (!downloadStarted && !loaded && imgDownload) {
		CST_Rect rect = { this->xAbs, this->yAbs, this->width, this->height };
		if (!CST_isRectOffscreen(&rect)) {
			fetch();
		}
	}
	
	return ret;
}

void NetImageElement::imgDownloadComplete(DownloadOperation *download)
{
	bool success = false;

	if (download->status == DownloadStatus::COMPLETE)
	{
		CST_Surface *surface = IMG_Load_RW(SDL_RWFromMem((void*)download->buffer.c_str(), download->buffer.size()), 1);
		success = loadFromSurfaceSaveToCache(download->url, surface);
		CST_FreeSurface(surface);
	}

	if (success)
	{
		this->needsRedraw = true;
		loaded = true;

		delete imgFallback;
		imgFallback = nullptr;

		if (updateSizeAfterLoad) {
			width = texW;
			height = texH;
		}
	}

	delete imgDownload;
	imgDownload = nullptr;
}


void NetImageElement::render(Element* parent)
{
	// if we're hidden, don't render
	if (hidden) return;

	if (mTexture)
	{
		Texture::render(parent);
	}
	else if (imgFallback)
	{
		// we need to apply our constraints
		Element::render(parent); // not texture's
		
		imgFallback->x = x;
		imgFallback->y = y;
		imgFallback->width = width;
		imgFallback->height = height;
		imgFallback->render(parent);
	}
}

} // namespace Chesto
