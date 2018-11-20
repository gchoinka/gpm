#define CATCH_CONFIG_MAIN
#include "../common/nodes.hpp"
#include "catch.hpp"

bool RPNDeserializationSerializationTest(char const* antRPNdefinition) {
  using namespace ant;
  auto ant =
      gpm::factory<ant::NodesVariant>(gpm::RPNTokenCursor{antRPNdefinition});
  auto antStr = boost::apply_visitor(gpm::RPNPrinter<std::string>{}, ant);
  return antStr == antRPNdefinition;
}

bool PNDeserializationSerializationTest(char const* antRPNdefinition) {
  using namespace ant;
  auto antFromRPN =
      gpm::factory<ant::NodesVariant>(gpm::RPNTokenCursor{antRPNdefinition});
  auto antFromRPNAsPNStr =
      boost::apply_visitor(gpm::PNPrinter<std::string>{}, antFromRPN);

  auto antFromPN = gpm::factory<ant::NodesVariant>(
      gpm::PNTokenCursor{antFromRPNAsPNStr.c_str()});
  auto antFromPNAsRPNStr =
      boost::apply_visitor(gpm::RPNPrinter<std::string>{}, antFromPN);
  return antFromPNAsRPNStr == antRPNdefinition;
}

TEST_CASE("RPN Deserialization / serialization test",
          "[PNDeserializationSerializationTest]") {
  REQUIRE(RPNDeserializationSerializationTest(
      "m r m if l l p3 r m if if p2 r p2 m if"));
  REQUIRE(PNDeserializationSerializationTest(
      "m r m if l l p3 r m if if p2 r p2 m if"));
}
