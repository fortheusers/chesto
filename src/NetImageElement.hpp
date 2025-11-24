#pragma once

#include <string>
#include <functional>
#include "Texture.hpp"
#include "DownloadQueue.hpp"

namespace Chesto {

class NetImageElement : public Texture
{
public:
	/// Creates a new image element, downloading the image from url
	/// If the url is not cached, getImageFallback will be called to
	/// get a Texture to be shown while downloading the correct image;
	/// the provided Texture is free'd when the download is complete
	/// or the destructor is called
	/// If immediateLoad is set to false, the loading won't begin until
	/// load() is called
	NetImageElement(const char *url, std::function<Texture *(void)> getImageFallback = NULL, bool immediateLoad = true);
	~NetImageElement();

	/// Start downloading the image (called in the constructor unless immediateLoad is false)
	void fetch();

	// TODO: introduce a boolean to control auto-loading described below
	/// Checks if we're onscreen and start download if needed
	bool process(InputEvents* event) override;

	/// Render the image
	void render(Element* parent) override;
	bool loaded = false;
	bool updateSizeAfterLoad = false;

private:
	void imgDownloadComplete(DownloadOperation *download);

	DownloadOperation *imgDownload = nullptr;
	Texture *imgFallback = nullptr;
	bool downloadStarted = false;
};

} // namespace Chesto
