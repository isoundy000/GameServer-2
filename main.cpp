/*
 * main.cpp
 *
 * Created on: 2013-01-17 15:38
 *     Author: glendy
 */


#include "GameDefine.h"
#include "DaemonServer.h"
#include "DebugServer.h"
#include <iostream>

int main(int argc, char *argv[])
{
	Time_Value nowtime = Time_Value::gettimeofday();
	std::srand(nowtime.sec() + nowtime.usec());

#ifdef LOCAL_DEBUG
    DEBUG_SERVER->init();
    DEBUG_SERVER->start();

    DebugServerSingle::destroy();
    std::cerr << "server stopped\n" << std::endl;
    return 0;
#else
    DAEMON_SERVER->init(argc, argv);
    return DAEMON_SERVER->start(argc, argv);
#endif
}


