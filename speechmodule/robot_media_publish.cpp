
#include "robot_media_publish.h"
#include "audiohead.h"



robot_media_publish::robot_media_publish(robot_voice_data_interface* thp) :m_client(NULL)
{
	m_pm = NULL;
	m_samplerate = DEFINE_SAMPLE_RATE;
	m_client = thp;

}

int robot_media_publish::init(std::string streamname, std::string client_name)
{
	if (streamname.empty() || client_name.empty())
		return -1;

	if (streamname.compare(m_streamstring) == 0 && client_name.compare(m_name) == 0)
	{
		if (state())return 0;
	}

	close_memory();

	m_pm = Processon_Memory_Open(streamname.c_str(), client_name.c_str());
	if (!m_pm)
	{
		LOGE("audio process memory not create !!");
		return -1;
	}

	int pm_ret = -1;
	int outt = 60;
	while (pm_ret)
	{
		unsigned int data_len = m_buffer.GetMediaLen();
		pm_ret = Processon_Memory_Read(m_pm, m_buffer.GetMedia(), &data_len);
		if (pm_ret < 0)
		{
			LOGE("processon memory occur error %d!!", pm_ret);
			return -1;
		}
		if (pm_ret > 0)
		{
			switch (pm_ret)
			{
			case 1001:
				if (outt-- <= 0)return 1;
				break;
			case 1002:
				m_buffer.Resize(data_len);
				break;
			default:
				break;
			}
			continue;

		}
		else
		{
			AudioMemoryHeadFormat *head_info = (AudioMemoryHeadFormat *)m_buffer.GetMedia();
			m_samplerate = head_info->nSampleRate;

		}
	}
	m_streamstring = streamname;
	m_name = client_name;
	return start()?0:1;
}

void robot_media_publish::close_memory()
{
	if (state())quit();
	m_pm = NULL;
}

int robot_media_publish::get_samplerate()
{
	return m_samplerate;
}



robot_media_publish::~robot_media_publish()
{
	close_memory();
}


void robot_media_publish::run()
{
	bool loop = true;
	int fps = 0;
	Timer_helper fps_timer;
	robot_voice_media temp;
	if (m_client)
		m_client->inFlush();
	while (loop)
	{
		Message xMSG = getq(0);

		switch (xMSG.msgType)
		{
		case _EXIT_THREAD_:
			loop = false;
			break;
		default:
			;
		}
		unsigned int data_len = m_buffer.GetMediaLen();
		int ret=Processon_Memory_Read(m_pm, m_buffer.GetMedia(), &data_len);

		if (ret)
		{
			if (ret < 0)
			{
				LOGE("processon memory occur error!!");
			}
			if (ret == 1002)
				m_buffer.Resize(data_len);
			continue;
		}

		AudioMemoryHeadFormat *head_info = (AudioMemoryHeadFormat *)m_buffer.GetMedia();

		if (m_client)
		{
			temp.Set(head_info->stime, m_buffer.GetMedia() + sizeof(AudioMemoryHeadFormat), data_len - sizeof(AudioMemoryHeadFormat));
			m_client->inData(temp);			
		}

	}
	
	Processon_Memory_Close(m_pm);
	m_pm = NULL;
	m_streamstring.clear();
	m_name.clear();

}


