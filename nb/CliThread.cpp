//==============================================================================
//
//  CliThread.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "CliThread.h"
#include <algorithm>
#include <cctype>
#include <ios>
#include <sstream>
#include "CinThread.h"
#include "CliBuffer.h"
#include "CliCommand.h"
#include "CliRegistry.h"
#include "CliStack.h"
#include "Clock.h"
#include "CoutThread.h"
#include "Debug.h"
#include "Element.h"
#include "FileThread.h"
#include "Formatters.h"
#include "NbCliParms.h"
#include "NbIncrement.h"
#include "Singleton.h"
#include "Symbol.h"
#include "SymbolRegistry.h"
#include "TraceBuffer.h"

using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace NodeBase
{
const char CliThread::CliPrompt = '>';

//------------------------------------------------------------------------------

fn_name CliThread_ctor = "CliThread.ctor";

CliThread::CliThread() : Thread(OperationsFaction),
   ibuf(nullptr),
   obuf(nullptr),
   stack_(nullptr),
   skip_(false),
   command_(nullptr),
   result_(0),
   outIndex_(0),
   inIndex_(0)
{
   Debug::ft(CliThread_ctor);
}

//------------------------------------------------------------------------------

fn_name CliThread_dtor = "CliThread.dtor";

CliThread::~CliThread()
{
   Debug::ft(CliThread_dtor);
}

//------------------------------------------------------------------------------

const char* CliThread::AbbrName() const
{
   return "cli";
}

//------------------------------------------------------------------------------

fn_name CliThread_AllocResources = "CliThread.AllocResources";

void CliThread::AllocResources()
{
   Debug::ft(CliThread_AllocResources);

   skip_ = false;
   outIndex_ = 0;
   inIndex_ = 0;

   ibuf.reset(new CliBuffer);
   obuf.reset(new std::ostringstream);
   *obuf << std::boolalpha << std::nouppercase;
   stack_.reset(new CliStack);
   prompt_.reset(new string);
   outName_[0].reset(new string(CliRegistry::ConsoleFileName() + ".txt"));
}

//------------------------------------------------------------------------------

fixed_string YesNoChars = "yn";
fixed_string YesNoHelp = "Enter y(yes) or n(no): ";

fn_name CliThread_BoolPrompt = "CliThread.BoolPrompt";

bool CliThread::BoolPrompt(const string& prompt)
{
   Debug::ft(CliThread_BoolPrompt);

   return (CharPrompt(prompt, YesNoChars, YesNoHelp) == 'y');
}

//------------------------------------------------------------------------------

fn_name CliThread_CharPrompt = "CliThread.CharPrompt";

char CliThread::CharPrompt
   (const string& prompt, const string& chars, const string& help, bool upper)
{
   Debug::ft(CliThread_CharPrompt);

   //  If input is being taken from a file rather than the console,
   //  return the first character.
   //
   if(chars.empty()) return '\0';
   if(inIndex_ > 0) return chars.front();

   auto first = true;
   char text[80];

   //  Output the query until the user enters a character in CHARS.
   //
   while(true)
   {
      ostringstreamPtr stream(new std::ostringstream);

      if(first)
         *stream << prompt << " [" << chars << "]: ";
      else
         *stream << help;

      Flush();
      CoutThread::Spool(stream);
      first = false;

      auto count = CinThread::GetLine(text, 80);

      if(count < 0) return '\0';

      if(count == 1)
      {
         auto c = (upper ? text[0] : tolower(text[0]));
         if(chars.find(c) != string::npos) return c;
      }
   }
}

//------------------------------------------------------------------------------

fn_name CliThread_Cleanup = "CliThread.Cleanup";

void CliThread::Cleanup()
{
   Debug::ft(CliThread_Cleanup);

   ReleaseResources();
   Thread::Cleanup();
}

//------------------------------------------------------------------------------

fn_name CliThread_Destroy = "CliThread.Destroy";

void CliThread::Destroy()
{
   Debug::ft(CliThread_Destroy);

   Singleton< CliThread >::Destroy();
}

//------------------------------------------------------------------------------

void CliThread::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   Thread::Display(stream, prefix, options);

   auto lead1 = prefix + spaces(2);
   auto lead2 = prefix + spaces(4);

   stream << prefix << "ibuf : " << CRLF;
   ibuf->Display(stream, lead1, options);
   stream << prefix << "obuf : " << obuf.get() << CRLF;
   stream << prefix << "stack : " << CRLF;
   stack_->Display(stream, lead1, options);
   stream << prefix << "prompt   : " << *prompt_ << CRLF;
   stream << prefix << "skip     : " << skip_ << CRLF;
   stream << prefix << "command  : " << strObj(command_) << CRLF;
   stream << prefix << "cookie   : " << CRLF;
   cookie_.Display(stream, lead1, options);
   stream << prefix << "result   : " << result_ << CRLF;
   stream << prefix << "stream   : " << stream_.get() << CRLF;

   stream << prefix << "outIndex : " << outIndex_ << CRLF;
   stream << prefix << "outName  : " << CRLF;
   for(size_t i = 0; i <= outIndex_; ++i)
   {
      stream << lead1 << strIndex(i) << outName_[i].get() << CRLF;
   }

   stream << prefix << "inIndex  : " << inIndex_ << CRLF;
   stream << prefix << "in : " << CRLF;
   for(size_t i = 0; i <= inIndex_; ++i)
   {
      stream << lead1 << strIndex(i) << in_[i].get() << CRLF;
   }

   stream << prefix << "appData : " << CRLF;
   for(auto i = 0; i <= CliAppData::MaxId; ++i)
   {
      if(appData_[i] != nullptr)
      {
         stream << lead1 << strIndex(i) << CRLF;
         appData_[i]->Display(stream, lead2, options);
      }
   }
}

//------------------------------------------------------------------------------

fn_name CliThread_EndOfInput = "CliThread.EndOfInput";

bool CliThread::EndOfInput(bool error)
{
   Debug::ft(CliThread_EndOfInput);

   if(!ibuf->FindNextNonBlank()) return true;

   if(error)
      ibuf->ErrorAtPos(*this, "Error: extra input");
   else
      ibuf->ErrorAtPos(*this, "Extra input ignored");

   return false;
}

//------------------------------------------------------------------------------

fn_name CliThread_Enter = "CliThread.Enter";

void CliThread::Enter()
{
   Debug::ft(CliThread_Enter);

   //  Put the root increment on the stack and start reading commands.
   //
   stack_->SetRoot(*Singleton< NbIncrement >::Instance());
   while(true) ReadCommands();
}

//------------------------------------------------------------------------------

fn_name CliThread_Execute = "CliThread.Execute";

word CliThread::Execute(const string& input)
{
   Debug::ft(CliThread_Execute);

   auto result = -1;

   Flush();
   auto rc = ibuf->PutLine(*this, input);

   if(rc == StreamOk)
   {
      auto comm = ParseCommand();
      if(comm != nullptr) result = InvokeCommand(*comm);
   }

   Flush();
   return result;
}

//------------------------------------------------------------------------------

fn_name CliThread_FileStream = "CliThread.FileStream";

ostream* CliThread::FileStream()
{
   Debug::ft(CliThread_FileStream);

   stream_ = FileThread::CreateStream();
   return stream_.get();
}

//------------------------------------------------------------------------------

fn_name CliThread_Flush = "CliThread.Flush";

void CliThread::Flush()
{
   Debug::ft(CliThread_Flush);

   //  If output is being sent to the console transcript file, send it to
   //  the console before sending it to the destination file.
   //
   if((obuf != nullptr) && (obuf->tellp() > 0))
   {
      if(outIndex_ == 0)
      {
         auto stream =
            ostringstreamPtr(new std::ostringstream(obuf->str()));
         CoutThread::Spool(obuf);
         FileThread::Spool(*outName_[0], stream);
      }
      else
      {
         FileThread::Spool(*outName_[outIndex_], obuf);
      }
   }

   //  Create a new output buffer for the next command's results.
   //
   obuf.reset(new std::ostringstream);
   *obuf << std::boolalpha << std::nouppercase;
}

//------------------------------------------------------------------------------

fn_name CliThread_GenerateReportPreemptably =
   "CliThread.GenerateReportPreemptably";

bool CliThread::GenerateReportPreemptably()
{
   Debug::ft(CliThread_GenerateReportPreemptably);

   //  Generate the report preemptably unless tracing is on in the lab
   //  and the user specifically wants to trace report generation.
   //
   if(Debug::TraceOn())
   {
      if(!Element::RunningInLab()) return true;

      if(BoolPrompt(StopTracePrompt))
      {
         Singleton< TraceBuffer >::Instance()->StopTrace();
         return true;
      }

      if(BoolPrompt(TraceReportPrompt)) return false;
   }

   return true;
}

//------------------------------------------------------------------------------

fn_name CliThread_GetAppData = "CliThread.GetAppData";

CliAppData* CliThread::GetAppData(CliAppData::Id aid) const
{
   if(aid <= CliAppData::MaxId) return appData_[aid].get();

   Debug::SwErr(CliThread_GetAppData, aid, 0);
   return nullptr;
}

//------------------------------------------------------------------------------

fn_name CliThread_InvokeCommand = "CliThread.InvokeCommand";

word CliThread::InvokeCommand(const CliCommand& comm)
{
   Debug::ft(CliThread_InvokeCommand);

   //  Initialize the cookie so that it will look for the command's
   //  first parameter.
   //
   cookie_.Initialize();

   //  Execute the command and output the results.
   //
   Pause();
   command_ = &comm;
   SetResult(comm.ProcessCommand(*this));
   Flush();
   command_ = nullptr;
   return result_;
}

//------------------------------------------------------------------------------

fn_name CliThread_InvokeSubcommand = "CliThread.InvokeSubcommand";

word CliThread::InvokeSubcommand(const CliCommand& comm)
{
   Debug::ft(CliThread_InvokeSubcommand);

   auto prev = command_;
   command_ = &comm;
   SetResult(comm.ProcessCommand(*this));
   command_ = prev;
   return result_;
}

//------------------------------------------------------------------------------

fn_name CliThread_Notify = "CliThread.Notify";

void CliThread::Notify(CliAppData::Event evt) const
{
   Debug::ft(CliThread_Notify);

   for(auto i = 0; i <= CliAppData::MaxId; ++i)
   {
      if(appData_[i] != nullptr)
      {
         appData_[i]->EventOccurred(evt);
      }
   }
}

//------------------------------------------------------------------------------

fn_name CliThread_OpenInputFile = "CliThread.OpenInputFile";

word CliThread::OpenInputFile(const string& name, string& expl)
{
   Debug::ft(CliThread_OpenInputFile);

   if(++inIndex_ < InSize)
   {
      auto path = Element::InputPath() + PATH_SEPARATOR + name + ".txt";
      in_[inIndex_] = SysFile::CreateIstream(path.c_str());
      if(in_[inIndex_] != nullptr) return 0;

      expl = NoFileExpl;
      return -2;
   }

   --inIndex_;
   expl = TooManyInputStreams;
   return -7;
}

//------------------------------------------------------------------------------

fn_name CliThread_ParseCommand = "CliThread.ParseCommand";

const CliCommand* CliThread::ParseCommand()
{
   Debug::ft(CliThread_ParseCommand);

   string token1;
   string token2;
   string tag;
   bool inIncr;

   //  Record the user's input in the output file.
   //
   auto input = ibuf->Echo();
   FileThread::Spool(*outName_[outIndex_], input, true);

   //  Get the first token, which must be the name of a command or
   //  increment.
   //
   if(ibuf->GetStr(tag, token1) != CliParm::Ok) return nullptr;
   if(!tag.empty()) return nullptr;

   auto comm = stack_->FindCommand(token1);

   if(comm != nullptr)
   {
      //  <command>
      //
      return comm;
   }

   auto incr = stack_->FindIncrement(token1);

   if(incr != nullptr)
   {
      inIncr = true;
   }
   else
   {
      incr = Singleton< CliRegistry >::Instance()->FindIncrement(token1);

      if(incr == nullptr)
      {
         //  <junk>
         //
         *obuf << spaces(2) << NoCommandExpl << token1 << CRLF;
         return nullptr;
      }

      inIncr = false;
      stack_->Push(*incr);
   }

   if(ibuf->GetStr(tag, token2) != CliParm::Ok)
   {
      //  <increment>
      //
      if(inIncr) *obuf << AlreadyInIncrement << token1 << '.' << CRLF;
      return nullptr;
   }

   if(!tag.empty()) return nullptr;
   comm = incr->FindCommand(token2);

   if(comm == nullptr)
   {
      //  <increment> <junk>
      //
      *obuf << spaces(2) << NoCommandExpl << token1;
      *obuf << CliCommand::CommandSeparator << token2 << CRLF;
   }

   if(!inIncr) stack_->Pop();
   return comm;
}

//------------------------------------------------------------------------------

void CliThread::Patch(sel_t selector, void* arguments)
{
   Thread::Patch(selector, arguments);
}

//------------------------------------------------------------------------------

fn_name CliThread_ReadCommands = "CliThread.ReadCommands";

void CliThread::ReadCommands()
{
   Debug::ft(CliThread_ReadCommands);

   while(true)
   {
      //  Print the CLI prompt, which contains the name of the
      //  most recently entered increment (if any).
      //
      string prompt(stack_->Top()->Name());
      prompt.push_back(CliPrompt);
      *prompt_ = prompt;

      if(!skip_)
      {
         CoutThread::Spool(prompt_->c_str());
         FileThread::Spool(*outName_[outIndex_], *prompt_, false);
      }

      skip_ = false;

      //  Read the user's input and parse it.
      //
      auto rc = ibuf->GetLine(*this);

      if(rc > 0)
      {
         auto comm = ParseCommand();
         if(comm != nullptr) InvokeCommand(*comm);
         Flush();
      }
      else
      {
         switch(rc)
         {
         case StreamEmpty:
            //
            //  Display the prompt again only if reading from the console.
            //
            skip_ = (inIndex_ > 0);
            break;

         case StreamBadChar:
            //
            //  CliBuffer has displayed an error string.  Just loop around and
            //  prompt for new input.
            //
            break;

         case StreamEof:
         case StreamFailure:
            if(inIndex_ > 0)
            {
               //  End of input stream.  Delete the stream and resume input
               //  from the previous stream.
               //
               in_[inIndex_--].reset();
               skip_ = true;
               return;
            }

            //  StreamEof and StreamFailure are not reported when reading from
            //  the console.  Fall through and pause before continuing.
            //
            Debug::SwErr(CliThread_ReadCommands, rc, 1);
         case StreamInterrupt:
         case StreamRestart:
            //
            //  o StreamInterrupt occurs when we plan to exit during a restart.
            //    Pausing causes us to receive SIGCLOSE and exit.
            //  o StreamRestart can occur if InitThread traps during a restart,
            //    after we have been created.  Pausing is also appropriate in
            //    this case, as another restart should occur momentarily.
            //
            Pause(TIMEOUT_1_SEC);
            break;

         case StreamInUse:
         default:
            Debug::SwErr(CliThread_ReadCommands, rc, 0);
         }
      }
   }
}

//------------------------------------------------------------------------------

fn_name CliThread_Recreated = "CliThread.Recreated";

void CliThread::Recreated()
{
   Debug::ft(CliThread_Recreated);

   //  Recreation is caused by excessive trapping, so release and reacquire
   //  resources in case some type of corruption has occurred.
   //
   ReleaseResources();
   AllocResources();
}

//------------------------------------------------------------------------------

fn_name CliThread_ReleaseResources = "CliThread.ReleaseResources";

void CliThread::ReleaseResources()
{
   Debug::ft(CliThread_ReleaseResources);

   for(auto i = 0; i <= CliAppData::MaxId; ++i)
   {
      appData_[i].reset();
   }

   for(auto i = inIndex_; i > 0; --i)
   {
      in_[i].reset();
   }

   for(auto i = outIndex_; i > 0; --i)
   {
      outName_[i].reset();
   }

   stream_.reset();
   prompt_.reset();
   stack_.reset();
   obuf.reset();
   ibuf.reset();
}

//------------------------------------------------------------------------------

fn_name CliThread_Report = "CliThread.Report";

word CliThread::Report(word rc, const string& expl, col_t indent)
{
   Debug::ft(CliThread_Report);

   size_t maxlen = 79 - indent;  // maximum line length
   size_t len = expl.size();     // length of EXPL
   size_t pos = 0;               // current position in EXPL

   while(pos < len)
   {
      //  If the rest of EXPL fits on a line, output it and return.
      //
      if((len - pos + 1) <= maxlen)
      {
         *obuf << spaces(indent) << expl.substr(pos) << CRLF;
         return rc;
      }

      //  If the rest of EXPL contains an endline before reaching the
      //  maximum line length, output everything up to the endline and
      //  advance to the character aftere the endline.
      //
      auto ret = expl.find(CRLF, pos);

      if((ret != string::npos) && ((ret - pos + 1) <= maxlen))
      {
         *obuf << spaces(indent) << expl.substr(pos, ret - pos + 1);
         pos = ret + 1;
      }
      else
      {
         //  Starting at the last character that would fit on a line,
         //  work backwards to find a blank, and insert an endline at
         //  that point.
         //
         auto end = std::min(pos + maxlen - 1, len - 1);
         auto blank = expl.rfind(SPACE, end);

         if(blank != string::npos)
         {
            *obuf << spaces(indent) << expl.substr(pos, blank - pos) << CRLF;
            pos = blank + 1;
         }
         else
         {
            //  There are more characters without intervening blanks than
            //  fit on a line.  Just output the rest of it all and quit.
            //
            *obuf << spaces(indent) << expl.substr(pos, string::npos) << CRLF;
            return rc;
         }
      }
   }

   return rc;
}

//------------------------------------------------------------------------------

fn_name CliThread_SendToFile = "CliThread.SendToFile";

void CliThread::SendToFile(const string& name, bool purge)
{
   Debug::ft(CliThread_SendToFile);

   if(stream_->tellp() > 0)
   {
      FileThread::Spool(name, stream_, purge);
   }
}

//------------------------------------------------------------------------------

fn_name CliThread_SetAppData = "CliThread.SetAppData";

void CliThread::SetAppData(CliAppData* data, CliAppData::Id aid)
{
   Debug::ft(CliThread_SetAppData);

   if(aid <= CliAppData::MaxId)
   {
      appData_[aid].reset(data);
      return;
   }

   Debug::SwErr(CliThread_SetAppData, aid, 0);
}

//------------------------------------------------------------------------------

fn_name CliThread_SetResult = "CliThread.SetResult";

void CliThread::SetResult(word result)
{
   Debug::ft(CliThread_SetResult);

   result_ = result;
   auto reg = Singleton< SymbolRegistry >::Instance();
   auto sym = reg->EnsureSymbol("cli.result");
   if(sym != nullptr) sym->SetValue(strInt(result_), false);
}

//------------------------------------------------------------------------------

fn_name CliThread_Startup = "CliThread.Startup";

void CliThread::Startup(RestartLevel level)
{
   Debug::ft(CliThread_Startup);

   Thread::Startup(level);

   if(level < RestartReboot)
   {
      //  If we fail to exit during a restart, some of our resources would
      //  nevertheless have been released--and we still have pointers to them.
      //  These pointers must be cleared to avoid traps when reacquiring these
      //  resources.  STL objects, which reside in permanent memory, will have
      //  survived.  CLI objects, however, reside in temporary memory, so their
      //  heap is gone.
      //
      ibuf.release();
      stack_.release();
      for(auto i = 0; i <= CliAppData::MaxId; ++i) appData_[i].release();
   }

   AllocResources();
}
}