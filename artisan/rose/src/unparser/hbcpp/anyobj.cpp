// Program code written by Steven De Toni ACBC 11

#include "anyobj.h"

// ###############################################################################
// #### ANYOBJECT Destructor ####
// ##############################
// Set to virtual so that objects can call there own destructors
// without becoming confused.
namespace bcpp {
ANYOBJECT::~ANYOBJECT (void) {}
}


