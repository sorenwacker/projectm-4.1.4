#include "TimeKeeper.hpp"

#include <algorithm>
#include <random>

namespace libprojectM {

TimeKeeper::TimeKeeper(double presetDuration, double smoothDuration, double hardcutDuration, double easterEgg)
    : m_easterEgg(easterEgg)
    , m_presetDuration(presetDuration)
    , m_softCutDuration(smoothDuration)
    , m_hardCutDuration(hardcutDuration)
{
    UpdateTimers();
}

void TimeKeeper::UpdateTimers()
{
    auto currentTime = std::chrono::high_resolution_clock::now();

    double currentFrameTime = std::chrono::duration<double>(currentTime - m_startTime).count();

    // Initialize m_lastRealTime on first call
    if (m_lastRealTime == 0.0)
    {
        m_lastRealTime = currentFrameTime;
    }

    // Calculate real frame delta time
    double realDelta = currentFrameTime - m_lastRealTime;
    m_lastRealTime = currentFrameTime;

    // Apply time scale to get scaled delta
    m_secondsSinceLastFrame = realDelta * m_timeScale;
    m_currentTime += m_secondsSinceLastFrame;

    m_presetFrameA++;
    m_presetFrameB++;
}

void TimeKeeper::StartPreset()
{
    m_isSmoothing = false;
    m_presetTimeA = m_currentTime;
    m_presetFrameA = 1;
    m_presetDurationA = sampledPresetDuration();
}

void TimeKeeper::StartSmoothing()
{
    m_isSmoothing = true;
    m_presetTimeB = m_currentTime;
    m_presetFrameB = 1;
    m_presetDurationB = sampledPresetDuration();
}

void TimeKeeper::EndSmoothing()
{
    m_isSmoothing = false;
    m_presetTimeA = m_presetTimeB;
    m_presetFrameA = m_presetFrameB;
    m_presetDurationA = m_presetDurationB;
}

bool TimeKeeper::CanHardCut()
{
    return (m_currentTime - m_presetTimeA) > m_hardCutDuration;
}

double TimeKeeper::SmoothRatio()
{
    return (m_currentTime - m_presetTimeB) / m_softCutDuration;
}

bool TimeKeeper::IsSmoothing()
{
    return m_isSmoothing;
}

double TimeKeeper::GetRunningTime()
{
    return m_currentTime;
}

double TimeKeeper::PresetProgressA()
{
    if (m_isSmoothing)
    {
        return 1.0;
    }

    return std::min((m_currentTime - m_presetTimeA) / m_presetDurationA, 1.0);
}

double TimeKeeper::PresetProgressB()
{
    return std::min((m_currentTime - m_presetTimeB) / m_presetDurationB, 1.0);
}

int TimeKeeper::PresetFrameB()
{
    return m_presetFrameB;
}

int TimeKeeper::PresetFrameA()
{
    return m_presetFrameA;
}

double TimeKeeper::PresetTimeB()
{
    return m_presetTimeB;
}

double TimeKeeper::PresetTimeA()
{
    return m_presetTimeA;
}

double TimeKeeper::sampledPresetDuration()
{
    if (m_easterEgg < 0.001)
    {
        return m_presetDuration;
    }

    std::normal_distribution<double> gaussianDistribution(m_presetDuration, m_easterEgg);
    return std::max<double>(1.0, gaussianDistribution(m_randomGenerator));
}

} // namespace libprojectM
