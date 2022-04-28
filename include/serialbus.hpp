/**
 * @file serialbus.hpp
 * @author Byunghun Hwang (bh.hwang@iae.re.kr)
 * @brief Divelink serial bus interface
 * @version 0.1
 * @date 2022-04-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _DIVELINK_SERIAL_BUS_HPP_
#define _DIVELINK_SERIAL_BUS_HPP_

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <spdlog/spdlog.h>
#include <functional>
#include <atomic>
#include <boost/smart_ptr.hpp>
#include <map>
#include <queue>
#include <vector>
#include "json.hpp"
#include "safeq.hpp"

using namespace std;
using namespace nlohmann;

namespace divelink {

    class serialbus {
        public:
            typedef void(*ptrPostProcess)(json&);

            serialbus(const char* dev, int baudrate):_service(), _port(_service, dev), _operation(false){

                _port.set_option(boost::asio::serial_port_base::parity());	// default none
                _port.set_option(boost::asio::serial_port_base::character_size(8));
                _port.set_option(boost::asio::serial_port_base::stop_bits());	// default one
                _port.set_option(boost::asio::serial_port_base::baud_rate(baudrate));
                _port.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none)); // default none

            }
            virtual ~serialbus(){
                
            }

            /* set external post process function pointer */
            void set_postprocess(ptrPostProcess ptr){ 
                _post_proc = ptr;
            }

            /* write to bus */
            void write(unsigned char* data, int size){

            }

            void open(){ /* start read */
                _operation = true;
                _worker.reset(new boost::asio::io_service::work(_service));
                assign();
                _tg.create_thread(boost::bind(&boost::asio::io_service::run, boost::ref(_service)));
                _tg.create_thread([&](void){_service.run();});
            }

            void close(){ /* stop read */
                _operation = false;
                _worker.reset();
                _service.stop();
                _tg.join_all();
                _service.reset();
                _port.close();
            }
            

            /* push the write data */
            void push_write(unsigned char* buffer, int size){
                vector<unsigned char> data(buffer, buffer+size);
                _write_buffer.produce(std::move(data));
                spdlog::info("write buffer size : {}", _write_buffer.size());
            }

        private:
            /* function assign for thread */
            void assign(){
                boost::function<void(void)> read_handler = [&](void) {
                    while(_operation){
                        if(_port.is_open()){
                            // for(auto& sub: _subport_container){

                            //     //read only
                            //     json response;
                            //     sub.second->readsome(&_port, response);
                            //     call_post(response);
                            // }
                        }
                        else
                            spdlog::error("port is not opened");
                    }
                };
                _service.post(read_handler);
            }

            /* post process function */
            void call_post(json& response){
                if(_post_proc){
                    _post_proc(response);
                }
            }

        private:
            char _rbuffer[2048];
            boost::asio::io_service _service;
            boost::shared_ptr<boost::asio::io_service::work> _worker;
            boost::thread_group _tg;
            boost::scoped_ptr<boost::thread> _t;
            boost::asio::serial_port _port;
            ptrPostProcess _post_proc;
            atomic<bool> _operation;

            safeQueue<vector<unsigned char>> _write_buffer;

    };

} /* end of namespace */

#endif