// EXCERPT — source: ProjectTemplate/libClearCore/src/MotorManager.cpp
// EVIDENCE: E1 | symbol: MotorModeSet | lines: 140-169
bool MotorManager::MotorModeSet(MotorPair motorPair,
                                Connector::ConnectorModes newMode) {
    if (motorPair == MOTOR_ALL) {
        return MotorModeSet(MOTOR_M0M1, newMode) &&
               MotorModeSet(MOTOR_M2M3, newMode);
    }

    switch (newMode) {
        case Connector::CPM_MODE_A_DIRECT_B_DIRECT:
        case Connector::CPM_MODE_STEP_AND_DIR:
        case Connector::CPM_MODE_A_DIRECT_B_PWM:
        case Connector::CPM_MODE_A_PWM_B_PWM:
            m_motorModes[motorPair] = newMode;
            MotorConnectors[motorPair * 2]->Mode(newMode);
            MotorConnectors[motorPair * 2 + 1]->Mode(newMode);

            if (newMode == Connector::CPM_MODE_STEP_AND_DIR) {
                PMUX_ENABLE(m_stepPorts[motorPair],
                            m_stepDataBits[motorPair]);
            }
            else {
                PMUX_DISABLE(m_stepPorts[motorPair],
                             m_stepDataBits[motorPair]);
            }
            break;
        default:
            break;
    }
    return (m_motorModes[motorPair] == newMode);
}
