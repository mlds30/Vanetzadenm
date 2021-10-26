#ifndef DENM_FUNCTIONS_HPP_PUFKBEM8
#define DENM_FUNCTIONS_HPP_PUFKBEM8

#include <vanetza/asn1/its/AltitudeConfidence.h>
#include <vanetza/asn1/its/AltitudeValue.h>
#include <vanetza/asn1/its/Heading.h>
#include <vanetza/asn1/its/ReferencePosition.h>
#include <vanetza/common/position_fix.hpp>
#include <vanetza/security/cam_ssp.hpp>
#include <vanetza/units/angle.hpp>
#include <vanetza/units/length.hpp>

// forward declaration of asn1c generated struct
struct BasicVehicleContainerLowFrequency;

namespace vanetza
{

// forward declaration of CAM message wrapper
namespace asn1 { class Denm; }

namespace facilities
{


/**
 * Print CAM content with indentation of nested fields
 * \param os output stream
 * \param cam CA message
 * \param indent indentation marker, by default one tab per level
 * \param start initial level of indentation
 *
 * This function is an idea of Erik de Britto e Silva (erikbritto@github)
 * from University of Antwerp - erik.debrittoesilva@uantwerpen.be
 */
void print_indented(std::ostream& os, const asn1::Denm& denm, const std::string& indent = "\t", unsigned start = 0);

} // namespace facilities
} // namespace vanetza

#endif /* DEMN_FUNCTIONS_HPP_PUFKBEM8 */
