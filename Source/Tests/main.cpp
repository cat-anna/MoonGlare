
// #define DO_NOT_USE_WMAIN
#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

bool gGenerateDiagrams = false;
bool gVerbose = false;

int main( int argc, char* argv[] )
{
  Catch::Session session; // There must be exactly one instance
  
  // Build a new parser on top of Catch's
  using namespace Catch::clara;
  auto cli 
    = session.cli() // Get Catch's composite command line parser
    | Opt( gGenerateDiagrams ) ["--generate-diagrams"] ("Generate diagrams")
    | Opt( gVerbose ) ["--verbose"] ("Be verbose"); 
        
  // Now pass the new composite back to Catch so it uses that
  session.cli( cli ); 


  // Let Catch (using Clara) parse the command line
  int returnCode = session.applyCommandLine( argc, argv );
  if( returnCode != 0 ) // Indicates a command line error
  	return returnCode;
      
  return session.run();
}
