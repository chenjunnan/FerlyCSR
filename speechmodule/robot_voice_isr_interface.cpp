
#include "robot_voice_isr_interface.h"
#include "xCommon.h"
#include "robot_PostMessage.h"


robot_voice_isr_interface::robot_voice_isr_interface() :m_buffer_ev(NULL), m_buffer_lock(NULL)
{
	m_buffer_lock = common_mutex_create_threadlock();
	m_buffer_ev = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_buffer_ev == NULL || m_buffer_lock == NULL)
	{
		LOGE("create lock error");
		return;
	}
	m_maxcachevoice = 5;
	setCachelenght(10 * m_maxcachevoice);
	m_media_inindex = 0;
	m_media_outindex = 0;
	m_usermaxcahce = 3000;
}


void robot_voice_isr_interface::inFlush()
{
	LOCK_HELPER_AUTO_HANDLER(m_buffer_lock);
	setCachelenght(m_maxcachevoice*10);
	m_media_inindex = 0;
	m_media_outindex = 0;
}

void robot_voice_isr_interface::inData(robot_voice_media &data)
{
	LOCK_HELPER_AUTO_HANDLER(m_buffer_lock);
	m_media_cache[m_media_inindex++%m_media_cache.size()] = data;
	if (m_media_inindex >= m_media_cache.size())m_media_inindex = 0;
	SetEvent(m_buffer_ev);
}

robot_voice_isr_interface::~robot_voice_isr_interface()
{
	if (state())quit();
	common_mutex_threadlock_release(m_buffer_lock);
	CloseHandle(m_buffer_ev);
}


void robot_voice_isr_interface::setCachelenght(int lenght)
{
	LOCK_HELPER_AUTO_HANDLER(m_buffer_lock);
	m_media_cache.resize(lenght);
}

bool robot_voice_isr_interface::getCacheMedia(robot_voice_media& data, int timeout)
{
	if (getHeadMedia(data))
		return true;
	DWORD  ret=WaitForSingleObject(m_buffer_ev, timeout);//WAIT_OBJECT_00
	if (ret != WAIT_OBJECT_0)return false;
	if (getHeadMedia(data))
		return true;
	return false;
}

void robot_voice_isr_interface::Mediadone()
{
	LOCK_HELPER_AUTO_HANDLER(m_buffer_lock);
	m_media_outindex++;
	if (m_media_outindex >= m_media_cache.size())m_media_outindex = 0;
}

bool robot_voice_isr_interface::getHeadMedia(robot_voice_media &data)
{
	LOCK_HELPER_AUTO_HANDLER(m_buffer_lock);
	if (m_media_inindex != m_media_outindex)
	{
		data = m_media_cache[m_media_outindex];
		return true;
	}
	else
		return false;
}

int robot_voice_isr_interface::open_stored_file(const char * name)
{
	if (fdwav)return 0;
	std::string filename = name;
	filename.append("_speech.pcm");
	fdwav = fopen(filename.c_str(), "wb+");
	if (fdwav == NULL) {
		LOGE("error open file %s failed\n", filename.c_str());
		return -1;
	}
	return 0;
}

int robot_voice_isr_interface::loopwrite_to_file(char *data, size_t length)
{
	size_t wrt = 0, already = 0;
	int ret = 0;
	if (fdwav == NULL || data == NULL)
		return -1;

	while (1) {
		wrt = fwrite(data + already, 1, length - already, fdwav);
		if (wrt == (length - already))
			break;
		if (ferror(fdwav)) {
			ret = -1;
			break;
		}
		already += wrt;
	}
	if (ret != -1)
		fflush(fdwav);

	return ret;
}

void robot_voice_isr_interface::safe_close_file()
{
	if (fdwav) {
		fclose(fdwav);
		fdwav = NULL;
	}
}


void robot_voice_isr_interface::open_audio(std::string readername,std::string streamname)
{
	std::string reader = readername;
	reader.append("reader");
	Json::Value   json_object;
	Json::Reader  json_reader;
	Json::FastWriter  json_writer;
	json_object["msg_type"] = Json::Value("AUDIO_START_STREAMNAME_REQUEST");
	json_object["streamname"] = Json::Value(streamname);
	json_object["reader"] = Json::Value(reader);


	std::string report = json_writer.write(json_object);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, report.c_str(), report.size(), "audio_stream_module");
}


void robot_voice_isr_interface::close_audio(std::string readername, std::string streamname)
{
	std::string reader = readername;
	reader.append("reader");
	Json::Value   json_object;
	Json::Reader  json_reader;
	Json::FastWriter  json_writer;
	json_object["msg_type"] = Json::Value("AUDIO_STOP_STREAMNAME_REQUEST");
	json_object["reader"] = Json::Value(reader);
	json_object["streamname"] = Json::Value(streamname);
	std::string report = json_writer.write(json_object);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, report.c_str(), report.size(), "audio_stream_module");

}

void robot_voice_isr_interface::init_first_voicecache(std::string filed)
{
	if (!common_config_get_field_int(filed.c_str(), "maxcachevoice", &m_usermaxcahce))
		m_usermaxcahce = 3;

	m_usermaxcahce *= 1000;

	if (m_usermaxcahce == 0)m_usermaxcahce = 300;
}

