// EXCERPT — source: ProjectTemplate/libClearCore/src/MotorDriver.cpp
// EVIDENCE: E1 | symbol: MotorDriver::Refresh step path | lines: 470-495
    // Calculate the next S&D output step count
    if (Connector::m_mode == Connector::CPM_MODE_STEP_AND_DIR) {
        // Coordinated connectors stay in CoordinatedMotionMode(true) even when CONFIG SINGLE=0,
        // but we must only consume UpdateFast/SetCoordinatedSteps while the planner is driving.
        // Otherwise StepGenerator::Move() never runs StepsCalculated() on M0/M1 and bench-style
        // motion fails; when idle, fall back to per-motor step generation (same as SINGLE=1).
        const bool coordDriving =
            m_coordinatedController &&
            (m_coordinatedController->IsActive() || m_coordinatedController->MotionQueueCount() > 0);
        if (m_coordinatedMode && m_coordinatedController && coordDriving) {
            if (this == m_coordinatedMotorX) {
                m_coordinatedController->UpdateFast();
            }
        } else {
            // Independent motion (or coordinated planner idle on both motors)
            // Calculate the number of steps to send in the next sample time
            StepGenerator::StepsCalculated();
            // Check the status of the limits
            StepGenerator::CheckTravelLimits();

            m_bDutyCnt = StepGenerator::m_stepsPrevious;
            // Queue up the steps by writing the B duty value
            UpdateBDuty();
        }
    }
}
