/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2004 Live Networks, Inc.  All rights reserved.
// A data structure that represents a session that consists of
// potentially multiple (audio and/or video) sub-sessions
// (This data structure is used for media *streamers* - i.e., servers.
//  For media receivers, use "MediaSession" instead.)
// C++ header

#ifndef _SERVER_MEDIA_SESSION_HH
#define _SERVER_MEDIA_SESSION_HH

#ifndef _MEDIA_HH
#include "Media.hh"
#endif
#ifndef _GROUPEID_HH
#include "GroupEId.hh"
#endif

class ServerMediaSubsession; // forward

class ServerMediaSession: public Medium {
public:
  static ServerMediaSession* createNew(UsageEnvironment& env,
				       char const* streamName = NULL,
				       char const* info = NULL,
				       char const* description = NULL,
				       Boolean isSSM = False,
				       char const* miscSDPLines = NULL);
			       
  virtual ~ServerMediaSession();

  static Boolean lookupByName(UsageEnvironment& env,
                              char const* mediumName,
                              ServerMediaSession*& resultSession);

  char* generateSDPDescription(); // based on the entire session
      // Note: The caller is responsible for freeing the returned string

  char const* streamName() const { return fStreamName; }

  Boolean addSubsession(ServerMediaSubsession* subsession);

  float duration() const { return fDuration; }

  unsigned referenceCount() const { return fReferenceCount; }
  void incrementReferenceCount() { ++fReferenceCount; }
  void decrementReferenceCount() { if (fReferenceCount > 0) --fReferenceCount; }
  Boolean& deleteWhenUnreferenced() { return fDeleteWhenUnreferenced; }

private:
  ServerMediaSession(UsageEnvironment& env, char const* streamName,
		     char const* info, char const* description,
		     Boolean isSSM, char const* miscSDPLines);
  // called only by "createNew()"

private: // redefined virtual functions
  virtual Boolean isServerMediaSession() const;

private:
  Boolean fIsSSM;

  // Linkage fields:
  friend class ServerMediaSubsessionIterator;
  ServerMediaSubsession* fSubsessionsHead;
  ServerMediaSubsession* fSubsessionsTail;
  unsigned fSubsessionCounter;

  char* fStreamName;
  char* fInfoSDPString;
  char* fDescriptionSDPString;
  char* fMiscSDPLines;
  struct timeval fCreationTime;
  float fDuration;
    // fDuration == 0 means an unbounded session (the default)
    // fDuration < 0 means: use subsession durations instead (because they differ)
    // fDuration > 0 means: this is the duration of a bounded session
  unsigned fReferenceCount;
  Boolean fDeleteWhenUnreferenced;
};


class ServerMediaSubsessionIterator {
public:
  ServerMediaSubsessionIterator(ServerMediaSession& session);
  virtual ~ServerMediaSubsessionIterator();
  
  ServerMediaSubsession* next(); // NULL if none
  void reset();
  
private:
  ServerMediaSession& fOurSession;
  ServerMediaSubsession* fNextPtr;
};


class ServerMediaSubsession: public Medium {
public:
  virtual ~ServerMediaSubsession();

  unsigned trackNumber() const { return fTrackNumber; }
  char const* trackId();
  virtual char const* sdpLines(ServerMediaSession& parentSession) = 0;
  virtual void getStreamParameters(unsigned clientSessionId, // in
				   netAddressBits clientAddress, // in
				   Port const& clientRTPPort, // in
				   Port const& clientRTCPPort, // in
				   int tcpSocketNum, // in (-1 means use UDP, not TCP)
				   unsigned char rtpChannelId, // in (used if TCP)
				   unsigned char rtcpChannelId, // in (used if TCP)
				   netAddressBits& destinationAddress, // in out
				   u_int8_t& destinationTTL, // in out
				   Boolean& isMulticast, // out
				   Port& serverRTPPort, // out
				   Port& serverRTCPPort, // out
				   void*& streamToken // out
				   ) = 0;
  virtual void startStream(unsigned clientSessionId, void* streamToken,
			   unsigned short& rtpSeqNum,
			   unsigned& rtpTimestamp) = 0;
  virtual void pauseStream(unsigned clientSessionId, void* streamToken);
  virtual void deleteStream(unsigned clientSessionId, void*& streamToken);

  virtual float duration() const;
    // returns 0 for an unbounded session (the default)
    // returns > 0 for a bounded session

protected: // we're a virtual base class
  ServerMediaSubsession(UsageEnvironment& env);

  char const* rangeSDPLine(ServerMediaSession& parentSession) const;
      // returns a string to be delete[]d

private:
  friend class ServerMediaSession;
  friend class ServerMediaSubsessionIterator;
  ServerMediaSubsession* fNext;

  unsigned fTrackNumber; // within an enclosing ServerMediaSession
  char const* fTrackId;
};

#endif
