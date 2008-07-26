#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "12";
	static const char MONTH[] = "07";
	static const char YEAR[] = "2008";
	static const double UBUNTU_VERSION_STYLE = 8.07;
	
	//Software Status
	static const char STATUS[] = "Custom";
	static const char STATUS_SHORT[] = "r";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 6;
	static const long BUILD = 3;
	static const long REVISION = 0;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 1;
	#define RC_FILEVERSION 1,6,3,0
	#define RC_FILEVERSION_STRING "1, 6, 3, 0\0"
	static const char FULLVERSION_STRING[] = "1.6.3.0";
	
	//SVN Version
	static const char SVN_REVISION[] = "0";
	static const char SVN_DATE[] = "unknown date";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 0;
	

#endif //VERSION_h
