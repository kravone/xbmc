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

#include "GameClientReversiblePlayback.h"
#include "games/addons/GameClient.h"
#include "games/addons/savestates/BasicMemoryStream.h"
#include "games/addons/savestates/DeltaPairMemoryStream.h"
#include "settings/Settings.h"
#include "threads/SingleLock.h"

#include <algorithm>

using namespace GAME;

#define REWIND_FACTOR  0.25  // Rewind at 25% of gameplay speed

CGameClientReversiblePlayback::CGameClientReversiblePlayback(CGameClient* gameClient, double fps, size_t serializeSize) :
  m_gameClient(gameClient),
  m_gameLoop(this, fps)
{
  const bool bRewindEnabled = CSettings::GetInstance().GetBool(CSettings::SETTING_GAMES_ENABLEREWIND);
  if (bRewindEnabled)
    m_memoryStream.reset(new CDeltaPairMemoryStream);
  else
    m_memoryStream.reset(new CBasicMemoryStream);

  unsigned int frameCount = 1;
  if (bRewindEnabled)
  {
    unsigned int rewindBufferSec = CSettings::GetInstance().GetInt(CSettings::SETTING_GAMES_REWINDTIME);
    if (rewindBufferSec < 10)
      rewindBufferSec = 10;
    frameCount = static_cast<unsigned int>(rewindBufferSec * m_gameLoop.FPS() + 0.5);
  }

  m_memoryStream->Init(serializeSize, frameCount);

  m_gameLoop.Start();
}

CGameClientReversiblePlayback::~CGameClientReversiblePlayback()
{
  m_gameLoop.Stop();
}

bool CGameClientReversiblePlayback::IsPaused() const
{
  return m_gameLoop.GetSpeed() == 0.0;
}

void CGameClientReversiblePlayback::PauseUnpause()
{
  if (IsPaused())
    m_gameLoop.SetSpeed(1.0);
  else
    m_gameLoop.SetSpeed(0.0);
}

unsigned int CGameClientReversiblePlayback::GetTimeMs() const
{
  const unsigned int played = m_memoryStream->PastFramesAvailable() + (m_memoryStream->CurrentFrame() ? 1 : 0);

  return static_cast<unsigned int>(1000.0 * played / m_gameLoop.FPS() + 0.5);
}

unsigned int CGameClientReversiblePlayback::GetTotalTimeMs() const
{
  const unsigned int total = m_memoryStream->MaxFrameCount();

  return static_cast<unsigned int>(1000.0 * total / m_gameLoop.FPS() + 0.5);
}

unsigned int CGameClientReversiblePlayback::GetCacheTimeMs() const
{
  const unsigned int cached = m_memoryStream->FutureFramesAvailable();

  return static_cast<unsigned int>(1000.0 * cached / m_gameLoop.FPS() + 0.5);
}

void CGameClientReversiblePlayback::SeekTimeMs(unsigned int timeMs)
{
  const int offsetTimeMs = timeMs - GetTimeMs();
  const int offsetFrames = static_cast<int>(offsetTimeMs / 1000.0 * m_gameLoop.FPS() + 0.5);

  if (offsetFrames > 0)
  {
    const unsigned int frames = std::min(static_cast<unsigned int>(offsetFrames), m_memoryStream->FutureFramesAvailable());
    if (frames > 0)
    {
      m_gameLoop.SetSpeed(0.0);
      {
        CSingleLock lock(m_memoryMutex);
        m_memoryStream->AdvanceFrames(frames);
        m_gameClient->Deserialize(m_memoryStream->CurrentFrame(), m_memoryStream->FrameSize());
      }
      m_gameLoop.SetSpeed(1.0);
    }
  }
  else if (offsetFrames < 0)
  {
    const unsigned int frames = std::min(static_cast<unsigned int>(-offsetFrames), m_memoryStream->PastFramesAvailable());
    if (frames > 0)
    {
      m_gameLoop.SetSpeed(0.0);
      {
        CSingleLock lock(m_memoryMutex);
        m_memoryStream->RewindFrames(frames);
        m_gameClient->Deserialize(m_memoryStream->CurrentFrame(), m_memoryStream->FrameSize());
      }
      m_gameLoop.SetSpeed(1.0);
    }
  }
}

void CGameClientReversiblePlayback::SetSpeed(float speedFactor)
{
  if (speedFactor >= 0.0f)
    m_gameLoop.SetSpeed(static_cast<double>(speedFactor));
  else
    m_gameLoop.SetSpeed(static_cast<double>(speedFactor) * REWIND_FACTOR);
}

void CGameClientReversiblePlayback::FrameEvent()
{
  m_gameClient->RunFrame();

  {
    CSingleLock lock(m_memoryMutex);
    if (m_gameClient->Serialize(m_memoryStream->BeginFrame(), m_memoryStream->FrameSize()))
      m_memoryStream->SubmitFrame();
  }
}

void CGameClientReversiblePlayback::RewindEvent()
{
  bool bSuccess = false;

  {
    CSingleLock lock(m_memoryMutex);

    // Need to rewind 2 frames so that RunFrame() will update the screen
    m_memoryStream->RewindFrames(2);

    bSuccess = m_gameClient->Deserialize(m_memoryStream->CurrentFrame(), m_memoryStream->FrameSize());
  }

  if (bSuccess)
    FrameEvent();
}
