
task main() {
	nMotorEncoder[motorB] = 0;
	nMotorEncoder[motorC] = 0;

	nMotorEncoderTarget[motorB] = 1800;
	nMotorEncoderTarget[motorC] = 1800;

	motor[motorB] = 50;
	motor[motorC] = 50;

	while (
		nMotorRunState[motorB] != runStateIdle ||
		nMotorRunState[motorC] != runStateIdle
	) {
		// idle loop
	}
}
