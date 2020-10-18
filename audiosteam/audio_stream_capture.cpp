#include "audio_stream_capture.h"


audio_stream_capture::audio_stream_capture()
{
	m_buffer = NULL;
	m_buffersize = 0;
	m_recorder = NULL;
	m_pm = NULL;
	m_run = false;
	wavfmt = DEFAULT_FORMAT;
}

static void recorder_cb(char *data, unsigned long len, void *user_para)
{

	int ret = 0;
	audio_stream_capture *object;

	if (len == 0 || data == NULL)
		return;
	object = (audio_stream_capture *)user_para;

	if (!object)
		return;

	object->pushmedia(data, len);
}




bool audio_stream_capture::init(AUDIO_INFO &config)
{
	bool ok = false;
	int errcode = 0;

	wavfmt.nSamplesPerSec = config.nSampleRate;
	wavfmt.nAvgBytesPerSec = wavfmt.nBlockAlign * wavfmt.nSamplesPerSec;

	if (get_input_dev_num() == 0 || get_input_dev_num() < config.nDeviceID)
	{
		LOGE("Audio stream error device totle :%d,want open %d.", get_input_dev_num(), config.nDeviceID);
		return false;
	}
	if (m_recorder==NULL)
	{
		errcode = create_recorder(&m_recorder, recorder_cb, (void*)this);
		if (m_recorder == NULL || errcode != 0)
		{
			LOGE("Audio stream create recorder failed: %d\n", errcode);
			return false;
		}
	}
	int buffersize = wavfmt.nBlockAlign *(wavfmt.nSamplesPerSec / 100) * 10*2; // 100 100 ms ,,50 200ms

	if (!Processon_Memory_Create(config.streamsname.c_str(), buffersize + sizeof(AudioMemoryHeadFormat), 90))
	{
		LOGE("cant create %s process memory!", config.streamsname.c_str());
		return false;
	}
	m_pm = Processon_Memory_Open(config.streamsname.c_str(), "writer");
	if (!m_pm)
	{
		LOGE("cant open %s process memory!", config.streamsname.c_str());
		return false;
	}
	if (buffersize > m_buffersize)
	{
		if (m_buffer)delete m_buffer;
		m_buffer = new unsigned char[buffersize + sizeof(AudioMemoryHeadFormat)];
		m_buffersize = buffersize;

	}
	LOGI("audio build success [%s]", config.streamsname.c_str());
	memset(m_buffer, 0x00, m_buffersize + sizeof(AudioMemoryHeadFormat));
	m_info = config;
	return true;
}

bool audio_stream_capture::start()
{
	if (m_run)return true;
	if (!m_pm)return false;
	int errcode = 0;
	errcode = open_recorder(m_recorder, m_info.nDeviceID, &wavfmt);
	if (errcode != 0) {
		close_recorder(m_recorder);
		LOGE("Audio stream recorder open failed: %d\n", errcode);
		return false;
	}

	errcode = start_record(m_recorder);
	if (errcode != 0) {
		stop_record(m_recorder);
		close_recorder(m_recorder);
		LOGE("Audio start recorder failed: %d", errcode);
		return false;
	}
	LOGI("start record.");
	m_run = true;
	return true;
}

void audio_stream_capture::stop()
{
	if (m_recorder)
	{
		if (is_record_stopped(m_recorder))
			stop_record(m_recorder);
		close_recorder(m_recorder);
	}
	m_run = false;

}

void audio_stream_capture::pushmedia(char *buffer, int len)
{
	AudioMemoryHeadFormat head_info;

	head_info.nDeviceID = m_info.nDeviceID;
	head_info.nSampleRate = m_info.nSampleRate;

	head_info.stime = common_get_longcur_time_stampms();
	memcpy(m_buffer, &head_info, sizeof(AudioMemoryHeadFormat));
	memcpy(m_buffer + sizeof(AudioMemoryHeadFormat), buffer, len);
	len += sizeof(AudioMemoryHeadFormat);
	if (!Processon_Memory_Write(m_pm, m_buffer, /*m_buffersize + sizeof(AudioMemoryHeadFormat)*/len))
	{
		LOGE("Write processon memory error!!!");
	}
	


}


audio_stream_capture::~audio_stream_capture()
{
	stop();
	SAFE_DELETE(m_buffer);
	if (m_recorder)
		destroy_recorder(m_recorder);
	m_recorder = NULL;
	if (m_pm)	
		Processon_Memory_Close(m_pm);
}
