#include "robot_voice_tts_baidu_ability.h"
#include <iostream> 
#include <fstream> 
#include <codecvt>
#include <windows.h>

#define BAIDUPATH  "BaiduTTS\\"

robot_voice_tts_baidu_ability::robot_voice_tts_baidu_ability()
{
	_baiduaip = NULL;
	m_thrGuard = common_mutex_create_threadlock();
	_cache = true;
	_maxlen = 0;
	_shouldupdate = false;
}


robot_voice_tts_baidu_ability* robot_voice_tts_baidu_ability::getInstanse()
{
	static robot_voice_tts_baidu_ability m_baidu;
	return &m_baidu;
}

int robot_voice_tts_baidu_ability::InitEngine()
{
	LOCK_HELPER_AUTO_HANDLER(m_thrGuard);
	if (_baiduaip)return 0;
	std::string robot_id;
	int ret = common_get_robot_serialID(&robot_id);
	if (ret)
	{
		LOGE("error robot serial %d", ret);
	}
	if (ret)
	{
		common_get_guid(&robot_id);
		LOGI("use temp robot id %s ",robot_id.c_str());
	}


	std::string appkey;

	if (!common_config_get_field_string("baidu", "appkey", &appkey))
		appkey = "P9NrDI94OTWWIzZ3S5xYsUt8";

	std::string SecretKey;

	if (!common_config_get_field_string("baidu", "secretkey", &SecretKey))
		SecretKey = "LUzYUV3M7NBV6a9jjVhoLAOBntBbWiLt";


	//const char* per = "4", const char* spd = "5", const char* pit = "5")=0
	std::string temp;
	if (!common_config_get_field_string("baidu", "per", &temp))
		temp = "4";
	_baidumap["per"] = temp;


	if (!common_config_get_field_string("baidu", "spd", &temp))
		temp = "5";
	_baidumap["spd"] = temp;

	if (!common_config_get_field_string("baidu", "pit", &temp))
		temp = "5";
	_baidumap["pit"] = temp;


	if (!common_config_get_field_string("baidu", "cache", &temp))
		temp = "1";

	if (temp.compare("0") == 0)
		_cache = false;
	
	if (!common_config_get_field_int("baidu", "maxlen", &_maxlen))
		_maxlen = 38;

	_baiduaip = CreateSpeechEngine(appkey.c_str(), SecretKey.c_str(), robot_id.c_str());
	if (_baiduaip == NULL)
		return -1;

	if (_cache)
	{
		check_path();
		load_cacheindex();
	}
	_shouldupdate = false;
	return 0;
}

void robot_voice_tts_baidu_ability::check_path()
{
	std::string path;
	common_get_current_dir(&path);
	common_fix_path(&path);
	path.append(BAIDUPATH);//sql

	if (!common_isDirectory(path.c_str()))
		common_create_dir(path.c_str());

	std::string onesp = path;
	onesp.append("per");
	onesp.append(_baidumap["per"]);
	onesp.append("spd");
	onesp.append(_baidumap["spd"]);
	onesp.append("pit");
	onesp.append(_baidumap["pit"]);
	onesp.append("\\");

	if (!common_isDirectory(onesp.c_str()))
		common_create_dir(onesp.c_str());

	_ttscachepath = onesp;
}

void robot_voice_tts_baidu_ability::load_cacheindex()
{
	_history_cache.clear();
	std::string indexfile = _ttscachepath;
	indexfile.append("index.conf");
	std::ifstream ifs(indexfile);

	if (!ifs.is_open())
		return;
	std::string filecontect;
	ifs >> filecontect ;
	ifs.close();

	while (!filecontect.empty())
	{
		int endpos = filecontect.find("#");
		if (endpos == -1)
		{
			filecontect.clear();
			continue;
		}

		std::string info = filecontect.substr(0, endpos);

		int filetotts = info.find("|");
		if (filetotts != -1)
		{
			std::string tts = info.substr(0, filetotts);
			std::string file = info.substr(filetotts+1);
			if (!tts.empty() && !file.empty())
				_history_cache[tts] = file;
		}

		filecontect.erase(0, endpos+1);
	}

}


std::wstring UTF8ToUnicode(const std::string& str) {

	// 预算-缓冲区中宽字节的长度    

	int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);

	// 给指向缓冲区的指针变量分配内存    

	wchar_t *pUnicode = (wchar_t*)malloc(sizeof(wchar_t)*unicodeLen);

	// 开始向缓冲区转换字节    

	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, pUnicode, unicodeLen);

	std::wstring ret_str = pUnicode;

	free(pUnicode);

	return ret_str;

}

std::string UnicodeToUTF8(const std::wstring& wstr) {

	// 预算-缓冲区中多字节的长度    

	int ansiiLen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);

	// 给指向缓冲区的指针变量分配内存    

	char *pAssii = (char*)malloc(sizeof(char)*ansiiLen);

	// 开始向缓冲区转换字节    

	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, pAssii, ansiiLen, nullptr, nullptr);

	std::string ret_str = pAssii;

	free(pAssii);

	return ret_str;

}

void robot_voice_tts_baidu_ability::write_cacheindex()
{
	std::string indexfile = _ttscachepath;
	indexfile.append("index.conf");
	std::ofstream ofs(indexfile);

	if (!ofs.is_open())
		return;

	for (auto it = _history_cache.begin(); it != _history_cache.end();it++)
	{
		std::string context;
		context.append(it->first);
		context.append("|");
		context.append(it->second);
		context.append("#");
		ofs.write(context.data(), context.size());
	}

	ofs.close();
}


void robot_voice_tts_baidu_ability::UninitEngine()
{
	LOCK_HELPER_AUTO_HANDLER(m_thrGuard);
	if (!_baiduaip)return;

	if (_baiduaip)DestorySpeechEngine(_baiduaip);
	_baiduaip = NULL;
}

int robot_voice_tts_baidu_ability::RequestTTS(int mode, std::string textin)
{
	LOCK_HELPER_AUTO_HANDLER(m_thrGuard);
	if (!_baiduaip)return -50001;
	if (mode == 1)
	{
		if (_history_cache.find(textin) != _history_cache.end())
		{
			io_string_spit.push_back(textin);
			return 0;
		}
		return -50003;
	}

	pcm_cache.clear();
	io_string_spit.clear();

	int max_wait_count = _maxlen;
	std::string uf_str = textin;
	std::wstring text = UTF8ToUnicode(uf_str);
	if (max_wait_count!=0&&text.size() > max_wait_count)
	{
		while (text.size() > max_wait_count)
		{
			int endword[7] = { 0 };
			endword[0] = text.find(L'。', max_wait_count);
			endword[1] = text.find(L'！', max_wait_count);
			endword[2] = text.find(L'？', max_wait_count);
			endword[3] = text.find(L'.', max_wait_count);
			endword[4] = text.find(L'!', max_wait_count);
			endword[5] = text.find(L'?', max_wait_count);
			endword[6] = text.find(L'…', max_wait_count);
			int minpos = text.size()-1;
			for (int i = 0; i < 7;i++)
			{
				if (endword[i] != 0 && endword[i] != -1)
				{
					if (minpos > endword[i])minpos = endword[i];
				}
			}
			if (minpos+1 < text.size())
			{
				if (text.at(minpos + 1) == L'"' || text.at(minpos + 1) == L'”' || text.at(minpos + 1) == L'）' || text.at(minpos + 1) == L')'||
					text.at(minpos + 1) == L'】' || text.at(minpos + 1) == L']' || text.at(minpos + 1) == L'}' || text.at(minpos + 1) == L'`' ||
					text.at(minpos + 1) == L']' || text.at(minpos + 1) == L'…')
					minpos ++;
			}

			std::wstring inmap = text.substr(0, minpos + 1);
 			std::string inuf = UnicodeToUTF8(inmap);
			if (inuf.size()!=0)
				io_string_spit.push_back(inuf);
			else
			{
				io_string_spit.clear();
				io_string_spit.push_back(textin);
				return 0;
			}
			text.erase(0, minpos + 1);
			if (text.size() != 0 && text.size() <= max_wait_count)
			{
				std::string inufw = UnicodeToUTF8(text);
				if (inufw.size() != 0)
					io_string_spit.push_back(inufw);
				else
				{
					io_string_spit.clear();
					io_string_spit.push_back(textin);
					return 0;
				}
				text.clear();
			}

		}
	}
	else
	{
		io_string_spit.push_back(textin);
	}

	return 0;
}

void robot_voice_tts_baidu_ability::Cancel()
{
	if (!_baiduaip)return ;
	_baiduaip->CancelBTTS();
}

int robot_voice_tts_baidu_ability::read_from_cache(std::string text)
{
	if (_history_cache.find(text) != _history_cache.end())
	{
		std::string cachefile = _history_cache[text];
		std::ifstream ifs;

		std::string path = _ttscachepath;
		path.append(cachefile);
		path.append(".dat");
		bool removefile = true;
		ifs.open(path, std::ios::in | std::ios::binary);

		if (ifs.is_open())
		{
			ifs.seekg(0, ifs.end);
			unsigned long long filesize = ifs.tellg();
			ifs.seekg(0, ifs.beg);
			pcm_cache.resize(filesize);
			ifs.read((char*)pcm_cache.data(), filesize);
			int resi = ifs.gcount();
			if (resi != 0)
			{
				ifs.close();
				return 0;
			}
			ifs.close();
		}
		_history_cache.erase(text);
	}
	return 1;
}

void robot_voice_tts_baidu_ability::build_cache(std::string text)
{
	if (!_cache)return;
	if (text.find('|') != -1 || text.find('#') != -1)
		return;
	std::string guid, filename;
	common_get_guid(&guid);

	for (int i = 0; i < guid.size(); i++)
	{
		if (guid[i] != '{'&&guid[i] != '}'&&guid[i] != '-')
			filename.insert(filename.end(), guid[i]);
	}
	check_path();
	std::string path = _ttscachepath;
	path.append(filename); path.append(".dat");
	std::ofstream ofs;
	ofs.open(path, std::ios::out | std::ios::binary);
	if (ofs.is_open())
	{
		ofs.write((char*)pcm_cache.data(), pcm_cache.size());
		ofs.flush();
		ofs.close();
		_history_cache[text] = filename;
		_shouldupdate = true;
		LOGD("cache flush ok");
	}
}

void robot_voice_tts_baidu_ability::EndTTS()
{
	pcm_cache.clear();
	io_string_spit.clear();
	if (_shouldupdate)
	{
		write_cacheindex();
		_shouldupdate = false;
	}
}

int robot_voice_tts_baidu_ability::GetVoiceData(unsigned char *& data, unsigned int &len, TTS_STATUS &state)
{
	len = 0;
	state = TTS_STATUS::WAITTTSDATA;

	if (io_string_spit.size() == 0)return -1;
	
	std::string in = *io_string_spit.begin();

	if (read_from_cache(in))
	{
		TTSPCMDATA outinfo;
		LOGD("baidu tts request.");
		int ret = _baiduaip->BaiduTTS(in.c_str(), &outinfo, _baidumap["per"].c_str(), _baidumap["spd"].c_str(), _baidumap["pit"].c_str());
		if (ret == 0)
		{
			std::string tempout;
			common_utf8_2gbk(&in, &tempout);
			LOGI("[robot speech] speak :%s", tempout.c_str());
			pcm_cache.resize(outinfo.len);
			memcpy(pcm_cache.data(), outinfo.data, outinfo.len);
			build_cache(in);
		}
		else
		{
			if (ret == 42)
				ret = 0;
			else
				LOGE("BaiduTTS failed, error code: %d.\n", ret);
			return ret;

		}
		
	}
	data = pcm_cache.data();
	len = pcm_cache.size();
	io_string_spit.erase(io_string_spit.begin());
	if (io_string_spit.size() == 0)
	{
		state = TTS_STATUS::ENDTTSDATA;
	}
	else
	{
		state = TTS_STATUS::HASTTSDATA;
	}
	return 0;
}

int robot_voice_tts_baidu_ability::SetParam(int type, std::string key, std::string value)
{
	if (_baidumap.find(key) != _baidumap.end())
	{
		_baidumap[key] = value;
		common_config_set_field("baidu", key.c_str(), value.c_str());
		common_config_update_persistence();
		return 0;
	}
	return 1;
}

int robot_voice_tts_baidu_ability::GetParam(int type, std::string key, std::string &value)
{
	if (_baidumap.find(key) != _baidumap.end())
	{
		value = _baidumap[key];
		return 0;
	}
	return 0;
}

robot_voice_tts_baidu_ability::~robot_voice_tts_baidu_ability()
{
}
