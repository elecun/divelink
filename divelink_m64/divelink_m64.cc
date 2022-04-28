/**
 * @file divelink_m64.cc
 * @author Byunghun Hwang (bh.hwang@iae.re.kr)
 * @brief M64 Acoustic Modem Interface Middleware
 * @version 0.1
 * @date 2022-04-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */




#include "../include/cxxopts.hpp"
#include "../include/json.hpp"
#include "../include/spdlog/spdlog.h"
#include "../include/spdlog/sinks/stdout_color_sinks.h"
#include "../include/serialbus.hpp"
#include <mosquitto.h>
#include <csignal>
#include <string>

using namespace std;
using namespace nlohmann;

namespace divelink {

    /* global variables */
    struct mosquitto* _mqtt = nullptr;
    divelink::serialbus* _serialbus = nullptr;

    /* before destroy */
    void destroy(){

        if(_serialbus){
            _serialbus->close();
            delete _serialbus;
            _serialbus = nullptr;
        }

        mosquitto_loop_stop(_mqtt, true);
        mosquitto_destroy(_mqtt);
        mosquitto_lib_cleanup();

        spdlog::info("Successfully destroyed");
        exit(EXIT_SUCCESS);
    }

    /* cleanup */
    void cleanup(int sig){
        switch(sig){
            case SIGSEGV: { spdlog::warn("Segmentation violation"); } break;
            case SIGABRT: { spdlog::warn("Abnormal termination"); } break;
            case SIGKILL: { spdlog::warn("Process killed"); } break;
            case SIGBUS: { spdlog::warn("Bus Error"); } break;
            case SIGTERM: { spdlog::warn("Termination requested"); } break;
            case SIGINT: { spdlog::warn("interrupted"); } break;
            default:
            spdlog::info("Cleaning up the program");
        }
    }

    /* catch signal */
    void set_signal(){
        const int signals[] = { SIGINT, SIGTERM, SIGBUS, SIGKILL, SIGABRT, SIGSEGV };
        for(const int& s:signals)
            signal(s, cleanup);

        //signal masking
        sigset_t sigmask;
        if(!sigfillset(&sigmask)){
            for(int signal:signals)
                sigdelset(&sigmask, signal); //delete signal from mask
        }
        else {
            spdlog::error("Signal Handling Error");
            divelink::destroy(); //if failed, do termination
        }

        if(pthread_sigmask(SIG_SETMASK, &sigmask, nullptr)!=0){ // signal masking for this thread(main)
            spdlog::error("Signal Masking Error");
            divelink::destroy();
        }
    } /* set_signal */
} /* namespace */

int main(int argc, char* argv[])
{
    spdlog::stdout_color_st("console");

    /* variables (with default) */
    int optc = 0;
    string _device_port = "/dev/ttyS0";
    string _mqtt_broker = "127.0.0.0";
    int _baudrate = 9600;

    /* command options */
    while((optc=getopt(argc, argv, "p:b:t:h"))!=-1)
    {
        switch(optc){
            case 'p': { /* device port */
                _device_port = optarg;
            } break;
            case 'b': { /* baudrate */
                _baudrate = atoi(optarg);
            } break;
            case 't': { /* target ip to pub */
                _mqtt_broker = optarg;
            } break;
            
            case 'h':
            default:{
                cout << fmt::format("Divelink Middleware for M64 Acoustic Modem (built {}/{})", __DATE__, __TIME__) << endl;
                cout << "Usage: divelink_m64 [-p port] [-b baudrate] [-t mqtt broker address] [-h help]" << endl;
                exit(EXIT_FAILURE);
            }
            break;
        }
    }

    /* show arguments */
    spdlog::info("> set device port : {}", _device_port);
    spdlog::info("> set port baudrate : {}", _baudrate);
    spdlog::info("> set broker IP : {}", _mqtt_broker);

    try {

    }
    catch(const std::exception& e){
        spdlog::error("Exception occurred : {}", e.what());
    }

    divelink::destroy();

    return 0;
}