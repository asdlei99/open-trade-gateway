/////////////////////////////////////////////////////////////////////
//@file main.cpp
//@brief	主程序
//@copyright	上海信易信息科技股份有限公司 版权所有
/////////////////////////////////////////////////////////////////////

#include "config.h"
#include "log.h"
#include "tradersim.h"
#include "ins_list.h"

#include <iostream>
#include <string>
#include <fstream>
#include <atomic>

#include <boost/asio.hpp>

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 2)
		{
			return -1;
		}

		std::string logFileName = argv[1];
		
		Log(LOG_INFO,"msg=trade sim %s init",logFileName.c_str());

		//加载配置文件
		if (!LoadConfig())
		{
			Log(LOG_WARNING,"msg=trade sim %s load config failed!"
				, logFileName.c_str());
			
			return -1;
		}

		boost::asio::io_context ioc;

		std::atomic_bool flag;
		flag.store(true);

		boost::asio::signal_set signals_(ioc);

		signals_.add(SIGINT);
		signals_.add(SIGTERM);
#if defined(SIGQUIT)
		signals_.add(SIGQUIT);
#endif

		tradersim tradeSim(ioc,logFileName);
		tradeSim.Start();
		signals_.async_wait(
			[&ioc, &tradeSim, &logFileName, &flag](boost::system::error_code, int sig)
		{
			tradeSim.Stop();
			flag.store(false);
			ioc.stop();
			Log(LOG_INFO,"msg=trade sim %s got sig %d", logFileName.c_str(), sig);
			Log(LOG_INFO,"msg=trade sim %s exit", logFileName.c_str());
		});
		
		while (flag.load())
		{
			try
			{
				ioc.run();
				break;
			}
			catch (std::exception& ex)
			{
				Log(LOG_ERROR,"msg=trade sim ioc run exception,%s"
					, ex.what());
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "trade sim exception: " << e.what() << std::endl;
	}	
}
