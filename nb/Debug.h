//==============================================================================
//
//  Debug.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include <atomic>
#include <string>
#include "AssertionException.h"
#include "SysTypes.h"

//------------------------------------------------------------------------------

namespace NodeBase
{
//  Interface for generating debug information.  This interface must restrict
//  its use of headers so that it can be used by low level class templates.
//
class Debug
{
   friend class Thread;
   friend class ThreadAdmin;
   friend class TraceBuffer;
public:
   //  Any function that wishes to be included in a function trace must call
   //  Debug::ft(ClassName_FunctionName) as its first line of code, where the
   //  argument is defined as
   //
   //    fn_name ClassName_FunctionName = "ClassName.FunctionName";
   //
   //  at FILE SCOPE.  The pointer to this string is saved, so it must remain
   //  allocated!  DO NOT PASS SOMETHING THAT CAN BE DELETED.
   //
   //  Even small, "trivial" functions should be included in a function trace,
   //  as this can help to determine the execution flow when analyzing a trace.
   //  A function should only be omitted if it is invoked so frequently that it
   //  would fill the trace with noise.  "Get" functions usually fall into this
   //  category.
   //
   static void ft(fn_name_arg func);

   //  Generates a software error log.  FUNC is the function's exact name, in
   //  the same form as that used for Debug::ft above.  ERRVAL/ERRSTR provides
   //  debug information.  OFFSET is often a sequence number within a function,
   //  which makes it easy to see which occurrence of SwErr was invoked, but in
   //  some cases it provides debug information instead.  LEVEL specifies the
   //  severity of the error:
   //  o DebugLog and InfoLog generate a log
   //  o WarningLog includes a stack trace in the log
   //  o ErrorLog also throws an exception to clean up the work in progress
   //  If TITLE is provided, it replaces the usual title for software logs;
   //  this capability is used sparingly.
   //
   static void SwErr
      (fn_name_arg func, debug64_t errval, debug64_t offset,
      LogLevel level = WarningLog, fixed_string title = nullptr);
   static void SwErr
      (fn_name_arg func, const std::string& errstr, debug64_t offset,
      LogLevel level = WarningLog, fixed_string title = nullptr);

   //  Throws an exception if CONDITION is false.  ERRVAL is for debugging.
   //
   static void Assert(bool condition, debug32_t errval = 0)
   {
      if(!condition)
      {
         throw AssertionException(errval);
      }
   }

   //  Invokes CoutThread::Spool(s, eol) if the flag ShowToolProgress
   //  or FORCE is set, followed by Thread::Pause(10).
   //
   static void Progress(const std::string& s, bool eol, bool force = false);

   //  Does nothing.  Useful for defining a breakpoint or tracepoint.
   //
   static void noop();

   //  Returns true if a trace tool is currently active.
   //
   static bool TraceOn() { return FcFlags_.test(TracingActive); }

   //  Clears status flags that prevent infinite recursion.  This function
   //  is invoked during exception and signal handling so that Debug::ft
   //  and Debug::SwErr do not remain permanently disabled.
   //
   static void Reset();

   //  Returns true if the software flag identified by FID is on.
   //  Always returns false unless Element::RunningInLab() is true.
   //
   static bool SwFlagOn(FlagId fid);

   //  Sets the software flag identified by FID to VALUE.
   //
   static void SetSwFlag(FlagId fid, bool value);

   //  Clears all flags.
   //
   static void ResetSwFlags();

   //  Returns the entire set of flags.  Note that this is a copy.
   //
   static Flags GetSwFlags();
private:
   //  Flags that define actions performed when a function is invoked.
   //
   enum InvocationFlag
   {
      TracingActive,  // set when tracing is active
      TrapPending,    // set when a Raise() is pending on any thread
      StackChecking   // set when stack overflow prevention is active
   };

   //  Private because this class only has static members.
   //
   Debug();

   //  Used by the various versions of SwErr.
   //
   static void GenerateSwLog(fn_name_arg func, const std::string& errstr,
      debug64_t offset, LogLevel level, fixed_string title);

   //  Prevents reentry to Debug::ft.
   //
   static std::atomic_flag FtLock_;

   //  Flags associated with a function call.
   //
   static Flags FcFlags_;

   //  Flags for controlling the behavior of software during testing.
   //
   static Flags SwFlags_;
};
}
#endif