// EXCERPT — source: ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/clearcore_clearlink_bridge.cpp
// EVIDENCE: E1 | symbol: BoardMotorMode_Request | lines: 1204-1229
int BoardMotorMode_Request(int step_and_dir_nonzero) {
  ClearCore::Connector::ConnectorModes const mode =
      step_and_dir_nonzero != 0
          ? ClearCore::Connector::CPM_MODE_STEP_AND_DIR
          : ClearCore::Connector::CPM_MODE_A_PWM_B_PWM;
  const bool applied = MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL, mode);
  if (!applied) {
    return 0;
  }
  // Both personalities can use PWM HLFB measurement for consistent telemetry.
  for (unsigned axis = 0U; axis < 4U; ++axis) {
    MotorDriver *const motor = MotorAxisPtr(axis);
    if (motor == nullptr) {
      continue;
    }
    (void)motor->HlfbMode(MotorDriver::HLFB_MODE_HAS_BIPOLAR_PWM);
    (void)motor->HlfbCarrier(MotorDriver::HLFB_CARRIER_482_HZ);
  }
  return 1;
}

void BoardVoltageSamples(float *auxiliary_volts, float *supply_volts) {
  if (auxiliary_volts == nullptr || supply_volts == nullptr) {
    return;
  }
  /* ADC_VSUPPLY_MON: main rail; ADC_5VOB_MON: 5 V off-board (auxiliary) per Teknic naming. */
