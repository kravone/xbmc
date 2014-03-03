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
#pragma once

#include "IGameClientPlayback.h"
#include "GameLoop.h"
#include "threads/CriticalSection.h"

#include <memory>
#include <stddef.h>

namespace GAME
{
  class CGameClient;
  class IMemoryStream;

  class CGameClientReversiblePlayback : public IGameClientPlayback,
                                        public IGameLoopCallback
  {
  public:
    CGameClientReversiblePlayback(CGameClient* gameClient, double fps, size_t serializeSize);

    virtual ~CGameClientReversiblePlayback();

    // implementation of IGameClientPlayback
    virtual bool CanPause() const override { return true; }
    virtual bool CanSeek() const override { return true; }
    virtual bool IsPaused() const override;
    virtual void PauseUnpause() override;
    virtual unsigned int GetTimeMs() const override;
    virtual unsigned int GetTotalTimeMs() const override;
    virtual unsigned int GetCacheTimeMs() const override;
    virtual void SeekTimeMs(unsigned int timeMs) override;
    virtual void SetSpeed(float speedFactor) override;

    // implementation of IGameLoopCallback
    virtual void FrameEvent() override;
    virtual void RewindEvent() override;

  private:
    // Construction parameter
    CGameClient* const m_gameClient;

    // Gameplay members
    CGameLoop                      m_gameLoop;
    std::unique_ptr<IMemoryStream> m_memoryStream;
    CCriticalSection               m_memoryMutex;
  };
}
