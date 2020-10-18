#include "AudioManager.h"
#include "xCommon.h"
#include "json\json.h"
#include "robot_PostMessage.h"


AudioManager::AudioManager()
{
}



bool AudioManager::add_audio(AUDIO_INFO &config)
{
	if (m_list.find(config.streamsname)!=m_list.end())
		return true;

	m_list[config.streamsname] = new audio_stream_capture();
	if (!m_list[config.streamsname] || !m_list[config.streamsname]->init(config))
	{
		SAFE_DELETE(m_list[config.streamsname]);
		m_list.erase(config.streamsname);
		return false;
	}
	return true;
	return m_list[config.streamsname]->start();
}

int AudioManager::handle_message(AUDIO_MESSAGE &acm)
{
	Json::Value   json_object;
	Json::Reader  json_reader;
	Json::FastWriter  json_writer;
	bool ret_value = false;
	int error = 1;
	std::string stream_name, reader_name;
	bool b_json_read = json_reader.parse(acm.context, json_object);
	if (b_json_read)
	{
		if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
		{
			std::string msgtype = json_object["msg_type"].asString();
			common_trim(&msgtype);

			if (json_object.isMember("streamname") && json_object["streamname"].isString())
				stream_name = json_object["streamname"].asString();
			if (json_object.isMember("reader") && json_object["reader"].isString())
				reader_name = json_object["reader"].asString();
			if (msgtype.compare("AUDIO_START_STREAMNAME_REQUEST") == 0)
			{
				if (m_list.find(stream_name) != m_list.end())
				{
					if (m_listenlist.find(stream_name) != m_listenlist.end())
						error = 0;
					else
						error = m_list[stream_name]->start()?0:1;
				}
				if (error == 0)
					m_listenlist[stream_name].insert(reader_name);
				json_object.clear();
				json_object["msg_type"] = Json::Value("AUDIO_START_STREAMNAME_RESPONSE");
			}
			if (msgtype.compare("AUDIO_STOP_STREAMNAME_REQUEST") == 0)
			{
				if (m_list.find(stream_name) != m_list.end())
				{
					if (m_listenlist.find(stream_name) != m_listenlist.end())
					{
						error = 0;
						m_listenlist[stream_name].erase(reader_name);
						if (m_listenlist[stream_name].size() == 0)
						{
							m_list[stream_name]->stop();
							m_listenlist.erase(stream_name);
						}
					}
				}
				json_object.clear();
				json_object["msg_type"] = Json::Value("AUDIO_STOP_STREAMNAME_RESPONSE");
			}

			
		}
	}
	json_object["reader"] = Json::Value(reader_name);
	json_object["streamname"] = Json::Value(stream_name);
	json_object["error"] = Json::Value(error);
	std::string report = json_writer.write(json_object);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, report.c_str(), report.size(), acm.from.c_str(), NULL, 0);
	return 0;

}

void AudioManager::remove_audio(std::string streamsname)
{
	if (m_list.find(streamsname) == m_list.end())
		return ;
	m_list[streamsname]->stop();
	delete m_list[streamsname];
	m_list.erase(streamsname);
}

void AudioManager::stopall()
{
	while (m_list.size()!=0)
		remove_audio(m_list.begin()->first);
		
	m_list.clear();
}

AudioManager::~AudioManager()
{
	stopall();
}
