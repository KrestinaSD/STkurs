#include "interface.h"
#include "userbase.h"
#include <iostream>
#include "communicator.h"
#include <vector>
#include <string>
#include <map>
#include "programmerror.h"
#include "logger.h"


int main(int argc, char **argv)
{
	interface op;
	op.Opts(argc, argv);
	return 0;
}



