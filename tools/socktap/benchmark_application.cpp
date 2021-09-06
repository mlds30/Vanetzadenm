#include "benchmark_application.hpp"
#include <chrono>
#include <iostream>
#include <vanetza/asn1/cam.hpp>
#include <vanetza/asn1/packet_visitor.hpp>
#include <vanetza/facilities/cam_functions.hpp>
#include <boost/units/cmath.hpp>

// Benchmark application counts all incoming messages and calculates the message rate.

using namespace std::chrono;
using namespace vanetza;
using namespace vanetza::facilities;

BenchmarkApplication::BenchmarkApplication(boost::asio::io_service& io) :
    m_timer(io), m_interval(std::chrono::seconds(1))
{
    schedule_timer();
}

BenchmarkApplication::PortType BenchmarkApplication::port()
{
    return host_cast<uint16_t>(0);
}

Application::PromiscuousHook* BenchmarkApplication::promiscuous_hook()
{
    return this;
}

void BenchmarkApplication::tap_packet(const DataIndication& indication, const UpPacket& packet)
{
	asn1::PacketVisitor<asn1::Cam> visitor;
    std::shared_ptr<const asn1::Cam> cam = boost::apply_visitor(visitor, packet);

    std::cout << "CAM application received a packet with " << (cam ? "decodable" : "broken") << " content" << std::endl;
    if (cam && print_rx_msg_) {
        std::cout << "Received CAM contains\n";
        print_indented(std::cout, *cam, "  ", 1);
        std::stringstream ss;
        //print_intedented(ss, *cam, "   ",1);
        char test[256];
        ss.get(test,256);
        printf("%s1n",test);
    }
	
    ++m_received_messages;
}

void BenchmarkApplication::indicate(const DataIndication& indication, UpPacketPtr packet)
{
    // do nothing here
}

void BenchmarkApplication::schedule_timer()
{
    m_timer.expires_from_now(m_interval);
    m_timer.async_wait(std::bind(&BenchmarkApplication::on_timer, this, std::placeholders::_1));
}

void BenchmarkApplication::on_timer(const boost::system::error_code& ec)
{
    if (ec == boost::asio::error::operation_aborted) {
        return;
    }

    std::cout << "Received " << m_received_messages << " messages/second" << std::endl;
    m_received_messages = 0;

    schedule_timer();
}

void BenchmarkApplication::print_received_message(bool flag)
{
    print_rx_msg_ = flag;
}
