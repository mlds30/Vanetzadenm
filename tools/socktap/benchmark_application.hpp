#ifndef BENCHMARK_APPLICATION_HPP_EUIC2VFR
#define BENCHMARK_APPLICATION_HPP_EUIC2VFR

#include "application.hpp"
#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <mosquitto.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

class BenchmarkApplication : public Application, private Application::PromiscuousHook
{
public:
    BenchmarkApplication(boost::asio::io_service&);
    PortType port() override;
    void indicate(const DataIndication&, UpPacketPtr) override;
    Application::PromiscuousHook* promiscuous_hook() override;
    void print_received_messagec(bool flag);
    void print_received_messaged(bool flag);
    struct mosquitto *mosq;
    int connetti(char* payload);

private:
    void schedule_timer();
    void on_timer(const boost::system::error_code& ec);
    void tap_packet(const DataIndication&, const vanetza::UpPacket&) override;

    boost::asio::steady_timer m_timer;
    std::chrono::milliseconds m_interval;
    unsigned m_received_messages;
    bool print_rx_msg_c = false;
    bool print_rx_msg_d = false;
    
};

#endif /* BENCHMARK_APPLICATION_HPP_EUIC2VFR */
