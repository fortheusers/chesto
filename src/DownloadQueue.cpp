#include "DownloadQueue.hpp"

#if defined(USE_RAMFS)
#define RAMFS "resin:/"
#else
#define RAMFS "resin/"
#endif

#define MAX_PARALLEL_DOWNLOADS	4


#define SOCU_ALIGN 0x1000
#define SOCU_BUFFERSIZE 0x100000

#ifndef SO_TCPSACK
#define SO_TCPSACK 0x00200 /* Allow TCP SACK (Selective acknowledgment) */
#endif

#ifndef SO_WINSCALE
#define SO_WINSCALE 0x00400 /* Set scaling window option */ 
#endif

#ifndef SO_RCVBUF
#define SO_RCVBUF 0x01002 /* Receive buffer size */
#endif

#ifndef NETWORK_MOCK
// networking optimizations adapted from:
//  - https://github.com/samdejong86/Arria-V-ADC-Ethernet-software/blob/master/ADC_Socket_bsp/iniche/src/h/socket.h
int sockopt_callback_chesto(void *clientp, curl_socket_t curlfd, curlsocktype purpose)
{
    int winscale = 1, rcvbuf = 0x20000, tcpsack = 1;
#ifndef WIN32
    setsockopt(curlfd, SOL_SOCKET, SO_WINSCALE, &winscale, sizeof(int));
    setsockopt(curlfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(int));
    setsockopt(curlfd, SOL_SOCKET, SO_TCPSACK, &tcpsack, sizeof(int));
#endif
	return 0;
}
#endif

DownloadQueue* DownloadQueue::downloadQueue = NULL;

void DownloadQueue::init()
{
	downloadQueue = new DownloadQueue();
}

void DownloadQueue::quit()
{
	delete downloadQueue;
}

static size_t WriteCallback(char *data, size_t n, size_t l, void *userp)
{
	DownloadOperation *download = (DownloadOperation *)userp;
	download->buffer.append(data, n * l);
	return n * l;
}

DownloadQueue::DownloadQueue()
{
#ifndef NETWORK_MOCK
	cm = curl_multi_init();
	curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, MAX_PARALLEL_DOWNLOADS);
#endif
}

DownloadQueue::~DownloadQueue()
{
#ifndef NETWORK_MOCK
	curl_multi_cleanup(cm);
	cm = NULL;
#endif
}

// add a new download operation
void DownloadQueue::downloadAdd(DownloadOperation *download)
{
	download->status = DownloadStatus::QUEUED;
	queue.push_back(download);
}

// cancel a download operation
void DownloadQueue::downloadCancel(DownloadOperation *download)
{
	if (download->status == DownloadStatus::DOWNLOADING)
		transferFinish(download);
	else if (download->status == DownloadStatus::QUEUED && queue.size() > 0)
		queue.remove(download);
}

#ifndef NETWORK_MOCK
void DownloadQueue::setPlatformCurlFlags(CURL* c)
{
	// from https://github.com/GaryOderNichts/wiiu-examples/blob/main/curl-https/romfs/cacert.pem
	curl_easy_setopt(c, CURLOPT_CAINFO, RAMFS "res/cacert.pem");

	curl_easy_setopt(c, CURLOPT_SOCKOPTFUNCTION, sockopt_callback_chesto);
}
#endif

// start a transfer operation
void DownloadQueue::transferStart(DownloadOperation *download)
{
#ifndef NETWORK_MOCK
	download->eh = curl_easy_init();

	setPlatformCurlFlags(download->eh);

	curl_easy_setopt(download->eh, CURLOPT_URL, download->url.c_str());
	curl_easy_setopt(download->eh, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(download->eh, CURLOPT_WRITEDATA, download);
	curl_easy_setopt(download->eh, CURLOPT_PRIVATE, download);
    
    curl_easy_setopt(download->eh, CURLOPT_FOLLOWLOCATION, 1L);

	curl_multi_add_handle(cm, download->eh);
#endif
	transfers++;
}

// finish a transfer operation
void DownloadQueue::transferFinish(DownloadOperation *download)
{
#ifndef NETWORK_MOCK
	if (cm != NULL) {
		curl_multi_remove_handle(cm, download->eh);
	}
	curl_easy_cleanup(download->eh);
#endif
	transfers--;
}

// start new transfers from the queue
void DownloadQueue::startTransfersFromQueue()
{
#ifndef NETWORK_MOCK
	while ((transfers < MAX_PARALLEL_DOWNLOADS) && (queue.size() > 0))
	{
		// remove the first element from the download queue
		DownloadOperation *download = queue.front();
		queue.pop_front();

		// start a new download
		download->status = DownloadStatus::DOWNLOADING;
		transferStart(download);
	}
#endif
}

// process finished and queued downloads
int DownloadQueue::process()
{
#ifndef NETWORK_MOCK
	DownloadOperation *download;
	int still_alive = 1;
	int msgs_left = -1;

	CURLMsg *msg;

	curl_multi_perform(cm, &still_alive);

	// handle completed or failed downloads
	while(msg = curl_multi_info_read(cm, &msgs_left))
	{
		long response_code = 404;

		if (msg->msg != CURLMSG_DONE)
			continue;

		curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &download);
		curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &response_code);

		transferFinish(download);
		startTransfersFromQueue();

		if (response_code == 200)
			download->status = DownloadStatus::COMPLETE;
		else
			download->status = DownloadStatus::FAILED;

		download->cb(download);
	}

	startTransfersFromQueue();

	return ((still_alive) || (msgs_left > 0) || (queue.size() > 0));
#else
	return false;
#endif
}
