
#include "VideostreamReader.h"

VideostreamReader::VideostreamReader()
{
	m_pm = NULL;
	m_ml=common_mutex_create_multisignal();
	m_c = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void VideostreamReader::set_stream(std::string streamname, std::string readername)
{
	m_streamname = streamname;
	m_readname = readername;
}


int VideostreamReader::startup_receive()
{
	if (m_streamname.empty())return 1;

	if (m_pm != NULL)
	{
		Processon_Memory_Close(m_pm);
		m_pm = NULL;
	}

	if (m_pm == NULL)
	{
		m_pm = Processon_Memory_Open(m_streamname.c_str(), m_readname.c_str());
		if (!m_pm)
		{
			LOGE("recorder cache process memory not create !!");
			return 1;
		}
	}

	clean_msgqueue();
	return start() ? 0 : 1;
}

bool VideostreamReader::get_media(VIDEO_MEDIADATA&data)
{
	while (true)
	{
		if (common_mutex_multisignal_wait(m_ml, 1000))
		{
			if (m_media.size() == 0)
			{
				common_mutex_multisignal_post(m_ml);
				if (WaitForSingleObject(m_c, 1000) != WAIT_OBJECT_0)
					return false;
				continue;
			}
			data = m_media.front();
			m_media.pop_front();
			common_mutex_multisignal_post(m_ml);
			return true;
		}
		else
			return false;
	}

}

VideostreamReader::~VideostreamReader()
{
	if (state())quit();
	common_mutex_multisignal_release(m_ml);
}


bool VideostreamReader::malloc_buffer(std::vector<unsigned char> &buffer)
{
	int pm_ret = -1;
	int outt = 60;
	while (pm_ret)
	{
		unsigned int data_len = buffer.size();
		pm_ret = Processon_Memory_Read(m_pm, buffer.data(), &data_len);
		if (pm_ret < 0)
		{
			LOGE("processon memory occur error %d!!", pm_ret);
			return false;
		}
		if (pm_ret > 0)
		{
			switch (pm_ret)
			{
			case 1001:
				if (outt-- <= 0)return false;
				break;
			case 1002:
				buffer.resize(data_len);
				break;
			default:
				break;
			}
			continue;

		}
		else
		{
			//VideoMemoryHeadFormat

		}
	}
	return true;
}


void VideostreamReader::run()
{
	std::vector<unsigned char> buffer;
	buffer.resize(256);
	if (!malloc_buffer(buffer))
		return;

	bool loop = true;
	while (loop)
	{
		Message msg = getq(0);
		switch (msg.msgType)
		{
		case _EXIT_THREAD_:
			loop = false;
			break;
		default:;
		}
		unsigned int data_len = buffer.size();

		int pm_ret = Processon_Memory_Read(m_pm, buffer.data(), &data_len);

		if (pm_ret < 0)
		{
			LOGE("processon memory occur error!!");
			common_thread_sleep(1000);
			continue;
		}
		if (pm_ret == 0)
		{
			if (common_mutex_multisignal_wait(m_ml, 1000))
			{
				VIDEO_MEDIADATA temp;
				temp.video_format = *(VideoMemoryHeadFormat *)buffer.data();
				temp.data.resize((data_len - sizeof(VideoMemoryHeadFormat)));
				memcpy(temp.data.data(), buffer.data() + sizeof(VideoMemoryHeadFormat), data_len - sizeof(VideoMemoryHeadFormat));
				if (m_media.size()>temp.video_format.fps)
					m_media.pop_front();
				m_media.push_back(temp);
				SetEvent(m_c);
				common_mutex_multisignal_post(m_ml);
			}
			else
			{
				LOGE("media video record wait failed ");
			}

		}


	}
	if (m_pm != NULL)
	{
		Processon_Memory_Close(m_pm);
		m_pm = NULL;
	}

	LOGW("recorder reader [%s] exit", m_streamname.c_str());
}
