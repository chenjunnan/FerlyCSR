#pragma once
#include <string>
#include "json/json.h"
#include "robot_voice_struct.h"

class robot_json_rsp
{
public:
	robot_json_rsp(){};
	~robot_json_rsp(){};

	std::string robot_json_rsp::start_voice_service(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("START_SPEECH_SERVICE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::stop_voice_service(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("STOP_SPEECH_SERVICE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::tts_begin(int c, int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("START_TTS_RESPONSE");
		json_object["channel"] = Json::Value(c);
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);

	}
	std::string robot_json_rsp::tts_stop(int c, int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("STOP_TTS_RESPONSE");
		json_object["channel"] = Json::Value(c);
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);

	}
	std::string robot_json_rsp::tts_set(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("SET_TTS_PARAMETER_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);

	}
	std::string robot_json_rsp::isr_set(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("SET_ISR_PARAMETER_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);

	}
	std::string robot_json_rsp::tts_get(int errorcode, std::string name, std::string value)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("GET_ISR_PARAMETER_RESPONSE");
		if (errorcode == 0)
		{
			json_object["paramName"] = name;
			json_object["paramValue"] = value;
		}
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);

	}
	std::string robot_json_rsp::isr_get(int errorcode,std::string name,std::string value)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("GET_ISR_PARAMETER_RESPONSE");
		if (errorcode == 0)
		{
			json_object["paramName"] = name;
			json_object["paramValue"] = value;
		}
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);

	}
	std::string robot_json_rsp::tts_pause(int c,int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("PAUSE_TTS_RESPONSE");
		json_object["channel"] = Json::Value(c);
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);

	}
	std::string robot_json_rsp::tts_resume(int c, int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("RESUME_TTS_RESPONSE");
		json_object["channel"] = Json::Value(c);
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);

	}
	std::string robot_json_rsp::start_once_isr(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("START_ONCE_ISR_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::stop_once_isr(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["error"] = Json::Value(errorcode);
		json_object["msg_type"] = Json::Value("STOP_ONCE_ISR_RESPONSE");
		return std::string(json_writer.write(json_object).c_str());


	}	
	std::string robot_json_rsp::irs_enhance_mic(int error)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("ENHANCE_MIC_RESPONSE");
		json_object["error"] = Json::Value(error);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::irs_reset_mic(int error)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("RESET_MIC_RESPONSE");
		json_object["error"] = Json::Value(error);
		return json_writer.write(json_object);
	}


	std::string robot_json_rsp::voice_error(int error_code)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("ISR_ERROR_NOTIFY");
		json_object["error"] = Json::Value(error_code);
		return std::string(json_writer.write(json_object).c_str());
	}


	std::string robot_json_rsp::internet_status_notify(int error_code)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("TTS_INTERNET_NOTIFY");
		json_object["status"] = Json::Value((bool)error_code);
		return std::string(json_writer.write(json_object).c_str());
	}


	std::string robot_json_rsp::tts_error(int error_code)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("TTS_ERROR_NOTIFY");
		json_object["error"] = Json::Value(error_code);
		return std::string(json_writer.write(json_object).c_str());
	}

	std::string robot_json_rsp::read_over_ntf(int channels,std::string text)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("TTS_READ_OVER_NOTIFY");
		json_object["channels"] = Json::Value(channels);
		json_object["text"] = Json::Value(text);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::voice_wakeup_ntf(int code, int angle,int score,std::string key)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("SPEECH_WAKEUP_NOTIFY");
		json_object["error"] = Json::Value(code);
		json_object["angle"] = Json::Value(angle);
		json_object["score"] = Json::Value(score);
		json_object["keyword"] = Json::Value(key);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::voice_wakeup_ntf(int code, int angle)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("SPEECH_WAKEUP_NOTIFY");
		json_object["error"] = Json::Value(code);
		json_object["angle"] = Json::Value(angle);
		return json_writer.write(json_object);
	}
	
	std::string robot_json_rsp::irs_result_semantics_ntf(std::string &text, std::string name="")
	{
		Json::Value   json_object;
		Json::Value   json_result;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("ISR_RESULT_NOTIFY");
		json_object["error"] = Json::Value(0);
		json_result["text"] = Json::Value(text);
		if (name.empty())
			json_result["result_type"] = Json::Value(0);
		else
			json_result["result_type"] = Json::Value(name);
		json_object["result"] = json_result;
		return json_writer.write(json_object);
	}



	std::string robot_json_rsp::irs_result_semantics_ntf(std::string &text,std::vector<slot_id> &data, std::string name = "")
	{
		Json::Value   json_object;
		Json::Value   json_result;

		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("ISR_RESULT_NOTIFY");
		json_object["error"] = Json::Value(0);
		json_result["text"] = Json::Value(text);
		if (name.empty())
			json_result["result_type"] = Json::Value(0);
		else
			json_result["result_type"] = Json::Value(name);
		json_object["result"] = json_result;

		for (int i = 0; i < data.size();i++)
		{
			Json::Value   json_solt;
			if (data[i].indata)
			{
				for (int j = 0; j < data[i].slot_context.size();j++)
				{
					Json::Value   one;
					one["id"] = Json::Value(data[i].slot_context[j].id);
					one["context"] = Json::Value(data[i].slot_context[j].context);
					one["score"] = Json::Value(data[i].slot_context[j].sc);
					json_solt["idmap"].append(one);
				}

				json_solt["slot_name"] = Json::Value(data[i].slot_name);
				json_object["slot"].append(json_solt);
			}

		}
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::irs_result_section_ntf(std::string &text)
	{
		Json::Value   json_object;
		Json::Value   json_result;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("ISR_RESULT_SECTION_NOTIFY");
		json_object["error"] = Json::Value(0);
		json_result["text"] = Json::Value(text);
		json_object["result"] = json_result;
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::irs_result_readly_wait_ntf()
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("ISR_RESULT_WAIT_NOTIFY");
		json_object["error"] = Json::Value(0);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::irs_result_readly_start_ntf(int error)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("ISR_RESULT_START_NOTIFY");
		json_object["error"] = Json::Value(error);
		return json_writer.write(json_object);
	}

};