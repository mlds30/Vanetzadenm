#include <vanetza/asn1/denm.hpp>
#include <vanetza/facilities/denm_functions.hpp>
#include <vanetza/facilities/path_history.hpp>
#include <vanetza/geonet/areas.hpp>
#include <boost/algorithm/clamp.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/systems/si/prefixes.hpp>
#include <boost/units/systems/angle/degrees.hpp>
#include <algorithm>
#include <limits>
#undef min

namespace vanetza
{
namespace facilities
{


void print_indented(std::ostream& os, const asn1::Denm& message, const std::string& indent, unsigned level)
{
    auto prefix = [&](const char* field) -> std::ostream& {
        for (unsigned i = 0; i < level; ++i) {
            os << indent;
        }
        os << field << ": ";
        return os;
    };

    const ItsPduHeader_t& header = message->header;
    prefix("ITS PDU Header") << "\n";
    ++level;
    prefix("Protocol Version") << header.protocolVersion << "\n";
    prefix("Message ID") << header.messageID << "\n";
    prefix("Station ID") << header.stationID << "\n";
    --level;

    const DecentralizedEnvironmentalNotificationMessage_t& denm = message->denm;
    prefix("Denm") << "\n";
    ++level;
    //prefix("Generation Delta Time") << cam.generationDeltaTime << "\n";

    prefix("Management Container") << "\n";
    ++level;
    const ManagementContainer_t& management = denm.management;
    prefix("StationID") << management.actionID.originatingStationID << "\n";     
    prefix("Sequence Number") << management.actionID.sequenceNumber << "\n";
    long temp;
    asn_INTEGER2long((INTEGER_t*)&management.detectionTime,&temp);
    prefix("Detection time") <<std::asctime(std::localtime(&temp))<< "\n";
    asn_INTEGER2long((INTEGER_t*)&management.referenceTime,&temp);
    prefix("Reference time") <<  std::asctime(std::localtime(&temp)) << "\n";
    
    
    //prefix("Reference time") << temptime << "\n";
    prefix("Longitude") << management.eventPosition.longitude << "\n";
    prefix("Latitude") << management.eventPosition.latitude << "\n";
    prefix("Altitude Value") << management.eventPosition.altitude.altitudeValue << "\n";
    prefix("Altitude Confidence") <<  management.eventPosition.altitude.altitudeConfidence << "\n";
    prefix("Validity Duration") << management.validityDuration << "\n";
    prefix("Station Type") << management.stationType << "\n";
    --level;
    --level;

    prefix("Situation Container") << "\n";
    const SituationContainer_t* situation = denm.situation;
    ++level;
    prefix("Information Quality") <<  situation->informationQuality << "\n";
    prefix("CauseCode") <<  situation->eventType.causeCode << "\n";
    prefix("subCauseCode") <<  situation->eventType.subCauseCode << "\n";
    --level;
}
} // namespace facilities
} // namespace vanetza
