//------------------------------------------------------------------------------
// Class: TcpHandler
//------------------------------------------------------------------------------
#ifndef __Eaagles_Basic_TcpHandler_H__
#define __Eaagles_Basic_TcpHandler_H__

#include "openeaagles/basic/nethandlers/PosixHandler.h"

namespace Eaagles {
namespace Basic {

class String;

//------------------------------------------------------------------------------
// Class: TcpHandler
//
// Description: General TCP/IP support class.
//------------------------------------------------------------------------------
class TcpHandler : public PosixHandler
{
   DECLARE_SUBCLASS(TcpHandler, PosixHandler)

public:
   TcpHandler();

   bool hasBeenTerminated()          { return connectionTerminated; }

   // NetHandler interface
   virtual bool sendData(const char* const packet, const int size);
   virtual unsigned int recvData(char* const packet, const int maxSize);
   virtual bool isConnected() const;
   virtual bool closeConnection();

   // "Socket Only" constructor that will be used by one of our derived classes;
   // The socket should be an AF_INET, SOCK_STREAM type socket that has been
   // initialized, bound, connected, and is ready for use.
   TcpHandler(const LcSocket socketNum);

protected:
   virtual bool init();         // Initialize this socket handler

   bool connected;              // true if we are connected
   bool connectionTerminated;   // true if we had a connection, then lost it (or terminated it ourselves)

   bool  noWait;                // TCP's no wait I/O flag
};

} // End Basic namespace
} // End Eaagles namespace

#endif

