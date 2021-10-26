#include "denm_application.hpp"
#include <vanetza/btp/ports.hpp>
#include <vanetza/asn1/denm.hpp>
#include <vanetza/asn1/packet_visitor.hpp>
#include <vanetza/facilities/denm_functions.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/systems/si/prefixes.hpp>
#include <chrono>
#include <exception>
#include <functional>
#include <iostream>
// This is a very simple CA application sending CAMs at a fixed rate.

using namespace vanetza;
using namespace vanetza::facilities;
using namespace std::chrono;

DenmApplication::DenmApplication(PositionProvider& positioning, Runtime& rt) :
    positioning_(positioning), runtime_(rt), denm_interval_(seconds(1))
{
    schedule_timer();
}




void DenmApplication::set_interval(Clock::duration interval)
{
    denm_interval_ = interval;
    runtime_.cancel(this);
    schedule_timer();
}

void DenmApplication::print_generated_message(bool flag)
{
    print_tx_msg_ = flag;
}

void DenmApplication::print_received_message(bool flag)
{
    print_rx_msg_ = flag;
}

DenmApplication::PortType DenmApplication::port()
{
    return btp::ports::DENM;
}

void DenmApplication::indicate(const DataIndication& indication, UpPacketPtr packet)
{
    asn1::PacketVisitor<asn1::Denm> visitor;
    std::shared_ptr<const asn1::Denm> denm = boost::apply_visitor(visitor, *packet);

    std::cout << "DENM application received a packet with " << (denm ? "decodable" : "broken") << " content" << std::endl;
    if (denm && print_rx_msg_) {
        std::cout << "Received DENM contains\n";
        print_indented(std::cout, *denm, "  ", 1);
    }
}

void DenmApplication::schedule_timer()
{
    runtime_.schedule(denm_interval_, std::bind(&DenmApplication::on_timer, this, std::placeholders::_1), this);
}

void DenmApplication::on_timer(Clock::time_point)
{
    schedule_timer();
    vanetza::asn1::Denm message;

    ItsPduHeader_t& header = message->header;
    header.protocolVersion = 2;
    header.messageID = ItsPduHeader__messageID_denm;
    header.stationID = 1; // some dummy value

    DecentralizedEnvironmentalNotificationMessage_t& denm = message->denm;
    
    auto position = positioning_.position_fix();

    if (!position.confidence) {
        std::cerr << "Skipping DENM, because no good position is available, yet." << std::endl;
        return;
    }

    //const auto time_now = duration_cast<milliseconds>(runtime_.now().time_since_epoch());
    const auto time_now  = duration_cast<seconds>(system_clock::now().time_since_epoch());
    uint64_t gen_delta_time = time_now.count();
    ManagementContainer_t& management = denm.management;
    management.actionID.originatingStationID = 1;
    management.actionID.sequenceNumber = 2220;
    //management.detectionTime = 4; //TimestampIts_t ?
    //management.referenceTime = 3; //TimestampIts_t ?
    
    /*
	// Seems like ASN1 supports 32 bit int (strange) and timestamp needs 42 bits.
	TimestampIts_t* timestamp = vanetza::asn1::allocate<TimestampIts_t>();

	//memset(timestamp,0,sizeof(TimestampIts_t));
	memcpy(timestamp, &time_now, sizeof(time_now)); //6bytes?
	
	//occorre inserire il tempo in timestampt
    message->denm.management.detectionTime = *timestamp;
    message->denm.management.referenceTime = *timestamp;
    */
    //memcpy(&management.referenceTime, &gen_delta_time, sizeof(time_now));
    int ret1 =  asn_uint642INTEGER((INTEGER_t*)&management.referenceTime, gen_delta_time);
    int ret2 =  asn_uint642INTEGER((INTEGER_t*)&management.detectionTime, gen_delta_time);
    assert(ret1+ret2==0);
    management.eventPosition.latitude = Latitude_unavailable;
    management.eventPosition.longitude = Longitude_unavailable;
    management.eventPosition.altitude.altitudeValue = AltitudeValue_unavailable;
    management.eventPosition.altitude.altitudeConfidence = AltitudeConfidence_unavailable;
    management.validityDuration = (ValidityDuration_t*) 600;
    management.stationType = StationType_passengerCar;
    //copy(position, management.referencePosition);
    
    //from artery TrafficJamUseCase
    message->denm.management.relevanceDistance = vanetza::asn1::allocate<RelevanceDistance_t>();
    *(message->denm.management.relevanceDistance) = RelevanceDistance_lessThan1000m;
    message->denm.management.relevanceTrafficDirection = vanetza::asn1::allocate<RelevanceTrafficDirection_t>();
    *(message->denm.management.relevanceTrafficDirection) = RelevanceTrafficDirection_upstreamTraffic;
    message->denm.management.validityDuration = vanetza::asn1::allocate<ValidityDuration_t>();
    *(message->denm.management.validityDuration) = 20;
    message->denm.management.stationType = StationType_unknown; // TODO retrieve type from SUMO
    //from artery
    
    message->denm.situation = vanetza::asn1::allocate<SituationContainer_t>();
    message->denm.situation->informationQuality = InformationQuality_unavailable;
    message->denm.situation->eventType.causeCode = 1;
    message->denm.situation->eventType.subCauseCode = 1;
    
    //situation.eventHistory.eventPosition =  DeltaReferencePosition; ? 
    //situation.eventHistory.eventDeltaTime = PathDeltaTime OPTIONAL; ?
    //situation.eventHistory.informationQuality = InformationQuality; ?
    

    std::string error;
    
    if (!message.validate(error)) {
        throw std::runtime_error("Invalid high frequency DENM: %s" + error);
    }

    if (print_tx_msg_) {
    	
        std::cout << "Generated DENM contains\n";
        print_indented(std::cout, message, "  ", 1);
	
    }

    DownPacketPtr packet { new DownPacket() };
    packet->layer(OsiLayer::Application) = std::move(message);

    DataRequest request;
    request.its_aid = aid::CA;
    
    request.transport_type = geonet::TransportType::SHB;
    request.communication_profile = geonet::CommunicationProfile::ITS_G5;
    auto confirm = Application::request(request, std::move(packet));

    if (!confirm.accepted()) {
        throw std::runtime_error("DENM application data request failed");
    }
   
}
