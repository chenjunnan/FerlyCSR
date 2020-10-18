#include "robot_print_module.h"
#include "xCommon.h"
#include "robot_PostMessage.h"
#include "json\value.h"

extern const char *MODULE_NAME;
robot_print_module::robot_print_module()
{
	m_run_print_usb = 1;
}


robot_print_module::~robot_print_module()
{
}


bool robot_print_module::message_handler(print_message& rcs)
{
	unsigned char recoil_buffer[3] = { 0x1D, 0x42, 0x00 };
	unsigned char bold_buffer[3] = { 0x1B, 0x45, 0x00 };
	unsigned char cutcmmand_buffer[3] = { 0x1B, 0x6D, 0x00 };

	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(rcs.content, json_object);
	if (b_json_read)
	{
		std::string gbkstring;
		//common_utf8_2gbk(&rcs.content, &gbkstring);
		//LOGD("Receive face message request %s", gbkstring.c_str());
		if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
		{
			std::string msgtype = json_object["msg_type"].asString();
			common_trim(&msgtype);
			if (msgtype.compare("PRINT_TEXT_REQUEST") == 0)
			{
				if (json_object.isMember("font_size") && json_object["font_size"].isInt())
					rcs.font_size = json_object["font_size"].asInt();
				if (json_object.isMember("text") && json_object["text"].isString())
					rcs.text = json_object["text"].asString();
				if (json_object.isMember("under_line") && json_object["under_line"].isBool())
					rcs.under_line = json_object["under_line"].asBool();
				if (json_object.isMember("recoil") && json_object["recoil"].isBool())
					rcs.recoil = json_object["recoil"].asBool();
				if (json_object.isMember("align") && json_object["align"].isString())
				{
					std::string al = json_object["align"].asString();
					if (al.find("center") != -1)
						rcs.align = 1;
					if (al.find("left") != -1)
						rcs.align = 0;
					if (al.find("right") != -1)
						rcs.align = 2;
				}
				if (json_object.isMember("bold") && json_object["bold"].isBool())
					rcs.bold = json_object["bold"].asBool();



				open_device();
				if (!m_run_print_usb)
				{
					int error = 0;
					do
					{
						int state = YkGetStatus(2);
						//int state = YkGetStatus(4);
						//LOGT("%x",state);
						if ((state & 0x60) != 0)
						{
							error = 7004;
							LOGW("State error [0x%02x]!!!", state);
							break;
						}

						if (YkSetAlign(rcs.align))
						{
							LOGE("YkSetAlign error!!!");
							error = 7005;
							break;
						}
						if (rcs.recoil)
							recoil_buffer[2] = 0x01;
						else
							recoil_buffer[2] = 0x00;

						if (YkDirectIO(recoil_buffer, 3))
						{
							LOGE("YkDirectIO error!!!");
							error = 7006;
							break;
						}
						if (rcs.bold)
							bold_buffer[2] = 0x01;
						else
							bold_buffer[2] = 0x00;

						if (YkDirectIO(bold_buffer, 3))
						{
							LOGE("YkDirectIO error!!!");
							error = 7006;
							break;
						}

						if (YkEnableUnderLine(rcs.under_line))
						{
							LOGE("YkEnableUnderLine error!!!");
							error = 7007;
							break;
						}

						if (YkSetCharSize(rcs.font_size % 8, rcs.font_size%8))
						{
							LOGE("YkSetCharSize error!!!");
							error = 7008;
							break;
						}
						
						std::string print_data;
						common_utf8_2gbk(&rcs.text, &print_data);
						//LOGD("print text data:%s", print_data.c_str());
						if (YkPrintStr((char*)print_data.c_str()))
						///if (YkPrintStr("test"))
						{
							LOGE("YkPrintStr error!!!");
							error = 7009;
							break;
						}
						LOGI("[print] text: %s", print_data.c_str());
						// 					if (YkFeedPaper())
						// 					{
						// 						LOGE("YkFeedPaper error!!!");
						// 						error = 7010;
						// 						break;
						// 					}

					} while (0);

					rcs.return_state = error;			
				}
				else
				{
					LOGW("device open error !!!");
					rcs.return_state = 1;
				}
					
				
				rcs.content = robot_json.print_text(rcs.return_state);
			}
			else if (msgtype.compare("PRINT_FEEDPAPER_REQUEST") == 0)
			{
				if (json_object.isMember("lines") && json_object["lines"].isInt())
					rcs.font_size = json_object["lines"].asInt();

				open_device();
				if (!m_run_print_usb)
				{
					int error = 0;
					do
					{
						int state = YkGetStatus(2);
						if ((state & 0x60) != 0)
						{
							LOGW("State error [0x%02x]!!!", state);
							error = 7004;
							break;
						}
						if (rcs.font_size == 0)
						{
							if (YkEnter())
							{
								LOGE("YkEnter error!!!");
								error = 7011;
								break;
							}
							LOGI("[print] enter");

						}
						if (rcs.font_size == 1)
						{
							if (YkFeedPaper())
							{
								LOGE("YkFeedPaper error!!!");
								error = 7010;
								break;
							}
							LOGI("[print] feed paper line");
						}

						if (rcs.font_size > 1)
						{
							if (YkPrnAndFeedPaper(rcs.font_size))
							{
								LOGE("YkPrnAndFeedLine error!!!");
								error = 7011;
								break;
							}
							LOGI("[print] feed paper %d pixel", rcs.font_size);
						}


					} while (0);

					rcs.return_state = error;
				}
				else
				{
					LOGW("device open error !!!");
					rcs.return_state = 1;
				}

				rcs.content = robot_json.print_feedpapaer(rcs.return_state);
			}
			else if (msgtype.compare("PRINT_CUTPAPER_REQUEST") == 0)
			{
				open_device();
				if (!m_run_print_usb)
				{
					int error = 0;

					int state = YkGetStatus(2);
					if ((state & 0x60) != 0)
					{
						LOGW("State error [0x%02x]!!!", state);
						error = 7004;
					}
					else
					{
						YkFeedPaper();

						//CutPaper(1);
						if (YkPrnAndFeedPaper(90))
						{
							LOGE("YkPrnAndFeedLine error!!!");
							error = 7011;
						}
						else
						{
							if (YkDirectIO(cutcmmand_buffer, 3))
							{
								LOGE("YkCutPaper error!!!");
								error = 7012;
							}
						}
						LOGI("[print] cut paper ");
						// 					if (YkCutPaper(66, 0))
						// 					{
						// 						LOGE("YkCutPaper error!!!");
						// 						error = 7012;
						// 					}
					}
					rcs.return_state = error;
				}
				else
				{
					LOGW("device open error !!!");
					rcs.return_state = 1;
				}

				rcs.content = robot_json.print_cutpapaer(rcs.return_state);
				close_device();
			}
			else if (msgtype.compare("PRINT_BMPFILE_REQUEST") == 0)
			{
				if (json_object.isMember("path") && json_object["path"].isString())
					rcs.text = json_object["path"].asString();
				if (json_object.isMember("size") && json_object["size"].isInt())
					rcs.font_size = json_object["size"].asInt();

				open_device();
				if (!m_run_print_usb)
				{
					int error = 0;

					int state = YkGetStatus(2);
					if ((state & 0x60) != 0)
					{
						LOGW("State error [0x%02x]!!!", state);
						error = 7004;
					}
					else
					{

						std::string gbkstring;
						common_utf8_2gbk(&rcs.text, &gbkstring);
						
						//if(PrintBmp((char*)gbkstring.c_str())
						if (YkDownloadBitmapAndPrint((char*)gbkstring.c_str(), 0))
						{
							LOGE("YkDownloadBitmapAndPrint error!!!");
							error = 7013;
						}
						LOGI("[print] print bmp file %s .", gbkstring.c_str());
					}

					rcs.return_state = error;
				}
				else
				{
					LOGW("device open error !!!");
					rcs.return_state = 1;
				}

				rcs.content = robot_json.print_bmpfile(rcs.return_state);
			}
			else if (msgtype.compare("PRINT_STATUS_REQUEST") == 0)
			{
				open_device();
				if (!m_run_print_usb)
				{
					int error = YkGetStatus(2);
					error = error & 0x60;
					int state = YkGetStatus(4);
					LOGT("print status %02x", state);
					rcs.nopaper = ((state & 0x60)) == 0 ? false : true;
					rcs.outofpaper = ((state & 0x0C)) == 0 ? false : true;
					if (rcs.nopaper)error = 0;
					rcs.return_state = error;
				}
				else
					rcs.return_state = 1;

				rcs.content = robot_json.print_status(rcs.return_state,rcs.nopaper,rcs.outofpaper);
			}
			else if (msgtype.compare("PRINT_CLOSE_REQUEST") == 0)
			{
				rcs.return_state = 0;
				close_device();
				rcs.content = robot_json.print_close(rcs.return_state);
			}
			else{
				rcs.return_state = 1;
				rcs.content = robot_json.print_text(rcs.return_state);
				LOGE("unknow print message");
			}
			robot_PostMessage::getPostManager()->post(MODULE_NAME, rcs.content.c_str(), rcs.content.length(), rcs.fromguid.c_str());
		}
	}
	return rcs.return_state;
}

void robot_print_module::close_device()
{
	if (!m_run_print_usb)
	{
		YkCloseDevice();
	}
	m_run_print_usb = 1;
}

void robot_print_module::open_device()
{
	if (m_run_print_usb == 0)return;
	if (YkOpenDevice(USB, 0))
	{
		LOGE("print init error");
		return ;
	}
	if (YkSetPaperSensor(0x08 | 0x02))
	{
		LOGE("Set Paper Sensor error");
	}
	//YkSetSensorToStopPrint(2);
	LOGI("print id:%d", YkGetPrinterID(1));
	LOGI("print style id:%d", YkGetPrinterID(2));
	LOGI("print class id:%d", YkGetPrinterID(3));
	LOGI("print firmware id:%d", YkGetPrinterID(4));
	LOGW("init print running");
	m_run_print_usb = YkInitPrinter();
}

int robot_print_module::init()
{
	return 0;
}

void robot_print_module::stop()
{
	close_device();
}
