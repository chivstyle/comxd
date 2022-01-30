
#define BLITZ_INDEX__ F1
#include "C:\Users\chiv\GitRepos\comxd\comxd\resource.cpp"
#ifdef IMAGECLASS
#undef IMAGECLASS
#endif
#ifdef IMAGEFILE
#undef IMAGEFILE
#endif
#undef BLITZ_INDEX__

#define BLITZ_INDEX__ F2
#include "C:\Users\chiv\GitRepos\comxd\comxd\ioimpls\SerialPort.cpp"
#undef BLITZ_INDEX__

#define BLITZ_INDEX__ F3
#include "C:\Users\chiv\GitRepos\comxd\comxd\ioimpls\SSHPort.cpp"
#undef BLITZ_INDEX__

#define BLITZ_INDEX__ F4
#include "C:\Users\chiv\GitRepos\comxd\comxd\ioimpls\SSHDevsDialog.cpp"
#ifdef CONFIG_MAX_RECENT_RECS_NUMBER
#undef CONFIG_MAX_RECENT_RECS_NUMBER
#endif
#undef BLITZ_INDEX__

#define BLITZ_INDEX__ F5
#include "C:\Users\chiv\GitRepos\comxd\comxd\ioimpls\SerialDevsDialog.cpp"
#undef BLITZ_INDEX__

#define BLITZ_INDEX__ F6
#include "C:\Users\chiv\GitRepos\comxd\comxd\ConnCreateFactory.cpp"
#undef BLITZ_INDEX__

#define BLITZ_INDEX__ F7
#include "C:\Users\chiv\GitRepos\comxd\comxd\main.cpp"
#undef BLITZ_INDEX__
