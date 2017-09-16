//==============================================================================
//
//  main.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
//  This determines what gets included in the build.  Each module resides
//  in its own static library, and all the files that belong to the library
//  reside in a folder with the same name.  The order of modules, from the
//  lowest to the highest layer, is
//
//                                              dependencies
//  namespace       module      library  nb nt ct nw sb st mb cb pb cn
//  ---------       ------      -------  -----------------------------
//  NodeBase        NbModule    nb
//  NodeTools       NtModule    nt       **
//  CodeTools       CtModule    ct       **
//  NodeBase        NwModule    nw       **
//  SessionBase     SbModule    sb       **       **
//  SessionTools    StModule    st       ** **    ** **
//  MediaBase       MbModule    mb       **       ** **
//  CallBase        CbModule    cb       ** **    ** ** ** **
//  PotsBase        PbModule    pb       ** **    ** ** ** ** **
//  ControlNode     CnModule    cn       **       ** **
//  RoutingNode     RnModule    rn       **       ** **    ** **
//  AccessNode      AnModule    an       **       ** **    ** ** **
//  ServiceNode     SnModule    sn       **       ** **    ** ** **
//  OperationsNode  OnModule    on       **       ** **    ** ** ** **
//  none            main.cpp    none     the desired subset of the above
//
//  RootThread is defined in NodeBase, so that namespace must be #included
//  here.  To build only NodeBase, create NbModule.  To include additional
//  layers, #include only the namespace, and create only the module, for the
//  uppermost layer that is required in the build.  That module's Register
//  function will, in turn, pull in the modules that it requires, and so on
//  transitively.
//
//  Compiler options (Windows)
//  --------------------------
//  In Properties > Configuration Properties, the following options are set
//  for all projects:
//  o C/C++ > General > Common Language RunTime Support: no CLR support
//    The CLR and managed code are Windows specific and degrade performance.
//  o C/C++ > General > Debug Information Format: /Z7
//    This provides full symbol information, even in a release build.
//  o C/C++ > General > Warning Level: /W4
//    This enables all compiler warnings.  A few are explicitly disabled using
//    the /wd option (see below).
//  o C/C++ > Optimization > Inline Function Expansion: /Ob1 (release builds)
//    This limits inlining to functions that are tagged as inline or that are
//    implemented in the class definition.  Aggressive inlining makes debugging
//    more difficult.
//  o C/C++ > Optimization > Omit Frame Pointers: Oy-
//    Keeping frame pointers ensures that SysThreadStack will work.
//  o C/C++ > Code Generation > Enable C++ Exceptions: /EHa
//    This is mandatory in order to catch Windows' structured exceptions.
//  o C/C++ > Code Generation > Basic Runtime Checks: /RTC1 (debug builds)
//    This detects things like the use of uninitialized variables and out of
//    bound array indices.
//  o C/C++ > Code Generation > Struct Member Alignment: /Z4
//    If the machine is 32 bits, then align to 32 bits.
//  o C/C++ > Browse Information > Enable Browse Information: /FR
//    Might as well enable browsing.
//  o C/C++ > Command Line > Additional Options:
//      /wd4100 /wd4127 /wd4244 /wd4481 /we4715
//    The first four are innocuous.  4715 is treated as an error.
//  o Librarian > General > Additional Dependencies:
//      Dbghelp.lib (for /nb), ws2_32.lib (for /nw)
//    These libraries contain DbgHelp.h and Winsock2.h, respectively.
//
#include <iostream>
#include <ostream>
#include <string>
#include "CfgParmRegistry.h"
#include "Debug.h"
#include "RootThread.h"
#include "Singleton.h"
#include "SysTypes.h"  // modules follow
#include "AnModule.h"
//& #include "CbModule.h"
#include "CnModule.h"
#include "CtModule.h"
//& #include "MbModule.h"
//& #include "NbModule.h"
//& #include "NwModule.h"
//& #include "NtModule.h"
#include "OnModule.h"
//& #include "PbModule.h"
#include "RnModule.h"
//& #include "SbModule.h"
#include "SnModule.h"
//& #include "StModule.h"

using std::string;

//------------------------------------------------------------------------------

using namespace NodeBase;
//& using namespace NodeTools;
using namespace CodeTools;
//& using namespace SessionBase;
//& using namespace MediaBase;
//& using namespace CallBase;
//& using namespace SessionTools;
//& using namespace PotsBase;
using namespace OperationsNode;
using namespace ControlNode;
using namespace RoutingNode;
using namespace ServiceNode;
using namespace AccessNode;

//------------------------------------------------------------------------------

fn_name main_cpp = "main";

main_t main(int argc, char* argv[])
{
   Debug::ft(main_cpp);

   //  Echo and save the arguments.
   //
   std::cout << "ENTERING main()" << CRLF;
   std::cout << "  argc: " << argc << CRLF;

   auto reg = Singleton< CfgParmRegistry >::Instance();

   for(auto i = 0; i < argc; ++i)
   {
      string arg(argv[i]);
      reg->AddMainArg(arg);
      std::cout << "  argv[" << i << "]: " << arg << CRLF;
   }

   std::cout << std::flush;

   //  Instantiate the desired modules.
   //
//& Singleton< NbModule >::Instance();
//& Singleton< NtModule >::Instance();
//& Singleton< SbModule >::Instance();
//& Singleton< StModule >::Instance();
//& Singleton< MbModule >::Instance();
//& Singleton< CbModule >::Instance();
//& Singleton< PbModule >::Instance();
   Singleton< CtModule >::Instance();
   Singleton< OnModule >::Instance();
   Singleton< CnModule >::Instance();
   Singleton< RnModule >::Instance();
   Singleton< SnModule >::Instance();
   Singleton< AnModule >::Instance();

   return RootThread::Main();
}