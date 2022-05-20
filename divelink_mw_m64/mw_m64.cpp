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
    while((optc=getopt(argc, argv, "p:b:s:h"))!=-1)
    {
        switch(optc){
            case 'p': { /* Device Port */
                _device_port = optarg;
            } break;
            case 'b': { /* Baudrate */
                _baudrate = atoi(optarg);
            } break;
            case 's': { /* Share Data */
                _mqtt_broker = optarg;
            } break;
            case 'h':
            default:{
                cout << fmt::format("Divelink Middleware for M64 Acoustic Modem (built {}/{})", __DATE__, __TIME__) << endl;
                cout << "Usage: divelink_m64 [-p port] [-b baudrate] [-s Broker Address to share data] [-h help]" << endl;
                exit(EXIT_FAILURE);
            }
            break;
        }
    }

    /* show arguments by user */
    spdlog::info("> set device port : {}", _device_port);
    spdlog::info("> set port baudrate : {}", _baudrate);

    try {

        // mosquitto_lib_init();
        // _mqtt = mosquitto_new("divelink", true, 0);

        // if(_mqtt){
		//     mosquitto_connect_callback_set(_mqtt, connect_callback);
		//     mosquitto_message_callback_set(_mqtt, message_callback);
        //     mosquitto_subscribe_callback_set(_mqtt, subscribe_callback);
        //     _mqtt_rc = mosquitto_connect(_mqtt, _mqtt_broker.c_str(), 1883, 60);
        //     spdlog::info("mqtt connection : {}", _mqtt_rc);
        //     mosquitto_loop_start(_mqtt);
        // }
        
        if(!_serialbus){
            _serialbus = new divelink::serialbus(_device_port.c_str(), _baudrate);
            _serialbus->set_postprocess(postprocess);
            _serialbus->add_subport("sensor", new divelink::sensor());
            _serialbus->start();
        }

        ::pause();

    }
    catch(const std::exception& e){
        spdlog::error("Exception occurred : {}", e.what());
    }

    divelink::destroy();

    return 0;
}