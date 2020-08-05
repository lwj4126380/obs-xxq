#include "VideoDecoder.h"
#include <obs.hpp>

VideoDecoder::VideoDecoder()
{
	memset(&m_sVideoFrameOri, 0, sizeof(SFgVideoFrame));
}

VideoDecoder::~VideoDecoder()
{
	uninitFFMPEG();
}

int VideoDecoder::docode(uint8_t *data, size_t data_len, bool is_key,
			 uint64_t ts)
{
	int canOutput = -1;
	if (is_key) {
		uninitFFMPEG();

		if (initFFMPEG(data, data_len) != 0)
			blog(LOG_ERROR, "decoder init fail!!!!!");
	} else {
		if (m_bCodecOpened) {
			AVPacket pkt1, *packet = &pkt1;
			int frameFinished;
			AVFrame *pFrame;

			pFrame = av_frame_alloc();

			av_new_packet(packet, data_len);
			memcpy(packet->data, data, data_len);

			int ret =
				avcodec_send_packet(this->m_pCodecCtx, packet);
			frameFinished = avcodec_receive_frame(this->m_pCodecCtx,
							      pFrame);

			av_packet_unref(packet);

			// Did we get a video frame?
			if (frameFinished == 0) {
				if (m_sVideoFrameOri.width != pFrame->width ||
				    m_sVideoFrameOri.height != pFrame->height) {
					if (m_sVideoFrameOri.data) {
						delete[] m_sVideoFrameOri.data;
						m_sVideoFrameOri.data = NULL;
					}
				}

				m_sVideoFrameOri.width = pFrame->width;
				m_sVideoFrameOri.height = pFrame->height;
				m_sVideoFrameOri.pts = ts;
				m_sVideoFrameOri.isKey = pFrame->key_frame;
				int ySize =
					pFrame->linesize[0] * pFrame->height;
				int uSize =
					pFrame->linesize[1] * pFrame->height >>
					1;
				int vSize =
					pFrame->linesize[2] * pFrame->height >>
					1;
				m_sVideoFrameOri.dataTotalLen =
					ySize + uSize + vSize;
				m_sVideoFrameOri.dataLen[0] = ySize;
				m_sVideoFrameOri.dataLen[1] = uSize;
				m_sVideoFrameOri.dataLen[2] = vSize;
				if (!m_sVideoFrameOri.data) {
					m_sVideoFrameOri.data = new uint8_t
						[m_sVideoFrameOri.dataTotalLen];
				}
				memcpy(m_sVideoFrameOri.data, pFrame->data[0],
				       ySize);
				memcpy(m_sVideoFrameOri.data + ySize,
				       pFrame->data[1], uSize);
				memcpy(m_sVideoFrameOri.data + ySize + uSize,
				       pFrame->data[2], vSize);
				m_sVideoFrameOri.pitch[0] = pFrame->linesize[0];
				m_sVideoFrameOri.pitch[1] = pFrame->linesize[1];
				m_sVideoFrameOri.pitch[2] = pFrame->linesize[2];

				canOutput = 1;
			}
			av_frame_free(&pFrame);
		}
	}

	return canOutput;
}

int VideoDecoder::initFFMPEG(const void *privatedata, int privatedatalen)
{
	if (m_pCodec == NULL) {
		m_pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
		m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
	}
	if (m_pCodec == NULL) {
		return -1;
	}

	m_pCodecCtx->extradata = (uint8_t *)av_malloc(privatedatalen);
	m_pCodecCtx->extradata_size = privatedatalen;
	memcpy(m_pCodecCtx->extradata, privatedata, privatedatalen);
	m_pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	int res = avcodec_open2(m_pCodecCtx, m_pCodec, NULL);
	if (res < 0) {

		printf("Failed to initialize decoder\n");
		return -1;
	}

	m_bCodecOpened = true;

	return 0;
}

void VideoDecoder::uninitFFMPEG()
{
	if (m_pCodecCtx) {
		if (m_pCodecCtx->extradata) {
			av_freep(&m_pCodecCtx->extradata);
		}
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = NULL;
		m_pCodec = NULL;
	}
	if (m_pSwsCtx) {
		sws_freeContext(m_pSwsCtx);
		m_pSwsCtx = NULL;
	}
}