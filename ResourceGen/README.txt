Source: Jeff Weinstein/Architekt (Dev forums) upon request
Content: Source code to the Resource Generator program

You won't be able to build this on modern VS without these changes:
Common/DefinesParser.cpp:
  #include "fstream.h" -> #include <fstream>
ResourceGen/ResourceManager.cpp:
  std::string &aDir = *anItr; -> std::string aDir = *anItr;
