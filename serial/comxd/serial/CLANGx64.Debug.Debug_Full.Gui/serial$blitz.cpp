
#define BLITZ_INDEX__ F1
#include "C:\Users\chiv\GitRepos\comxd\serial\src\impl\list_ports\list_ports_linux.cc"
#undef BLITZ_INDEX__

#define BLITZ_INDEX__ F2
#include "C:\Users\chiv\GitRepos\comxd\serial\src\impl\list_ports\list_ports_osx.cc"
#ifdef HARDWARE_ID_STRING_LENGTH
#undef HARDWARE_ID_STRING_LENGTH
#endif
#undef BLITZ_INDEX__
