/*
 *      Copyright (C) 2016 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "DVDDemuxGame.h"
#include "DVDDemuxPacket.h"

CDVDDemuxGame::CDVDDemuxGame(CDVDInputStream *pInput) :
  m_pInput(pInput),
  m_IDemux(nullptr)
{
}

CDVDDemuxGame::~CDVDDemuxGame()
{
  Dispose();
}

bool CDVDDemuxGame::Open()
{
  Abort();

  m_IDemux = m_pInput->GetIDemux();
  if (!m_IDemux)
    return false;

  if (!m_IDemux->OpenDemux())
    return false;

  RequestStreams();

  return true;
}

void CDVDDemuxGame::Dispose()
{
  m_IDemux = nullptr;
}

void CDVDDemuxGame::Reset()
{
  Open();
}

void CDVDDemuxGame::Abort()
{
  if (m_IDemux)
    m_IDemux->AbortDemux();
}

void CDVDDemuxGame::Flush()
{
  if (m_IDemux)
    m_IDemux->FlushDemux();
}

DemuxPacket* CDVDDemuxGame::Read()
{
  DemuxPacket *pPacket = m_IDemux->ReadDemux();
  if (pPacket)
    ParsePacket(pPacket);

  return pPacket;
}

bool CDVDDemuxGame::SeekTime(int timems, bool backwards, double *startpts)
{
  if (m_IDemux)
    return m_IDemux->SeekTime(timems, backwards, startpts);

  return false;
}

void CDVDDemuxGame::SetSpeed(int speed)
{
  if (m_IDemux)
    m_IDemux->SetSpeed(speed);
}

int CDVDDemuxGame::GetStreamLength()
{
  return 0; // TODO
}

std::vector<CDemuxStream*> CDVDDemuxGame::GetStreams() const
{
  std::vector<CDemuxStream*> streams;

  if (m_IDemux)
    streams = m_IDemux->GetStreams();

  return streams;
}

CDemuxStream* CDVDDemuxGame::GetStream(int iStreamId) const
{
  if (m_IDemux)
    return m_IDemux->GetStream(iStreamId);

  return nullptr;
}

int CDVDDemuxGame::GetNrOfStreams() const
{
  if (m_IDemux)
    return m_IDemux->GetNrOfStreams();

  return 0;
}

std::string CDVDDemuxGame::GetFileName()
{
  if (m_pInput)
    return m_pInput->GetFileName();
  else
    return "";
}

std::string CDVDDemuxGame::GetStreamCodecName(int iStreamId)
{
  std::string strName;

  CDemuxStream *stream = GetStream(iStreamId);
  if (stream)
  {
    switch (stream->codec)
    {
    case AV_CODEC_ID_AC3:
      strName = "ac3";
      break;
    case AV_CODEC_ID_MP2:
      strName = "mpeg2audio";
      break;
    case AV_CODEC_ID_AAC:
      strName = "aac";
      break;
    case AV_CODEC_ID_DTS:
      strName = "dts";
      break;
    case AV_CODEC_ID_MPEG2VIDEO:
      strName = "mpeg2video";
      break;
    case AV_CODEC_ID_H264:
      strName = "h264";
      break;
    case AV_CODEC_ID_EAC3:
      strName = "eac3";
      break;
    case AV_CODEC_ID_HEVC:
      strName = "hevc";
      break;
    default:
      break;
    }
  }
  return strName;
}

void CDVDDemuxGame::EnableStream(int id, bool enable)
{
  if (m_IDemux)
    m_IDemux->EnableStream(id, enable);
}

void CDVDDemuxGame::ParsePacket(DemuxPacket* pkt)
{
  if (pkt->iStreamId == 1)
    ; // TODO: Video
  else if (pkt->iStreamId == 2)
    ; // TODO: Audio
}

void CDVDDemuxGame::RequestStreams()
{
  /* TODO
  int nbStreams = m_IDemux->GetNrOfStreams();

  int i;
  for (i = 0; i < nbStreams; ++i)
  {
    CDemuxStream *stream = m_IDemux->GetStream(i);
    if (!stream)
    {
      CLog::Log(LOGERROR, "CDVDDemuxGame::RequestStreams - invalid stream at pos %d", i);
      DisposeStreams();
      return;
    }

    if (stream->type == STREAM_AUDIO)
    {
      CDemuxStreamAudio *source = dynamic_cast<CDemuxStreamAudio*>(stream);
      if (!source)
      {
        CLog::Log(LOGERROR, "CDVDDemuxGame::RequestStreams - invalid audio stream at pos %d", i);
        DisposeStreams();
        return;
      }
      CDemuxStreamAudioClient* st = nullptr;
      if (m_streams[i])
      {
        st = dynamic_cast<CDemuxStreamAudioClient*>(m_streams[i]);
        if (!st || (st->codec != source->codec))
          DisposeStream(i);
      }
      if (!m_streams[i])
      {
        st = new CDemuxStreamAudioClient();
        st->m_parser = av_parser_init(source->codec);
        if(st->m_parser)
          st->m_parser->flags |= PARSER_FLAG_COMPLETE_FRAMES;
      }
      st->iChannels       = source->iChannels;
      st->iSampleRate     = source->iSampleRate;
      st->iBlockAlign     = source->iBlockAlign;
      st->iBitRate        = source->iBitRate;
      st->iBitsPerSample  = source->iBitsPerSample;
      if (source->ExtraSize > 0 && source->ExtraData)
      {
        st->ExtraData = new uint8_t[source->ExtraSize];
        st->ExtraSize = source->ExtraSize;
        for (unsigned int j=0; j<source->ExtraSize; j++)
          st->ExtraData[j] = source->ExtraData[j];
      }
      m_streams[i] = st;
      st->m_parser_split = true;
      st->changes++;
    }
    else if (stream->type == STREAM_VIDEO)
    {
      CDemuxStreamVideo *source = dynamic_cast<CDemuxStreamVideo*>(stream);
      if (!source)
      {
        CLog::Log(LOGERROR, "CDVDDemuxGame::RequestStreams - invalid video stream at pos %d", i);
        DisposeStreams();
        return;
      }
      CDemuxStreamVideoClient* st = nullptr;
      if (m_streams[i])
      {
        st = dynamic_cast<CDemuxStreamVideoClient*>(m_streams[i]);
        if (!st
            || (st->codec != source->codec)
            || (st->iWidth != source->iWidth)
            || (st->iHeight != source->iHeight))
          DisposeStream(i);
      }
      if (!m_streams[i])
      {
        st = new CDemuxStreamVideoClient();
        st->m_parser = av_parser_init(source->codec);
        if(st->m_parser)
          st->m_parser->flags |= PARSER_FLAG_COMPLETE_FRAMES;
      }
      st->iFpsScale       = source->irFpsScale;
      st->iFpsRate        = source->irFpsRate;
      st->iHeight         = source->iHeight;
      st->iWidth          = source->iWidth;
      st->fAspect         = source->fAspect;
      st->stereo_mode     = "mono";
      if (source->ExtraSize > 0 && source->ExtraData)
      {
        st->ExtraData = new uint8_t[source->ExtraSize];
        st->ExtraSize = source->ExtraSize;
        for (unsigned int j=0; j<source->ExtraSize; j++)
          st->ExtraData[j] = source->ExtraData[j];
      }
      m_streams[i] = st;
      st->m_parser_split = true;
    }
    else
    {
      if (m_streams[i])
        DisposeStream(i);
      m_streams[i] = new CDemuxStream();
    }

    m_streams[i]->codec = stream->codec;
    m_streams[i]->iId = i;
    m_streams[i]->iPhysicalId = stream->iPhysicalId;
    for (int j=0; j<4; j++)
      m_streams[i]->language[j] = stream->language[j];

    m_streams[i]->realtime = stream->realtime;

    CLog::Log(LOGDEBUG,"CDVDDemuxGame::RequestStreams(): added/updated stream %d:%d with codec_id %d",
        m_streams[i]->iId,
        m_streams[i]->iPhysicalId,
        m_streams[i]->codec);
  }
  */
}
