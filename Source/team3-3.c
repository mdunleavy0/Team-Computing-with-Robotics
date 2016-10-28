#pragma config(Sensor, S1,     touchSensor,    sensorTouch)
#pragma config(Sensor, S3,     lightSensor,    sensorLightActive)
#pragma config(Sensor, S4,     sonarSensor,    sensorSONAR)
#pragma config(Motor,  motorB,          motorL,        tmotorNXT, PIDControl, driveRight, encoder)
#pragma config(Motor,  motorC,          motorR,        tmotorNXT, PIDControl, driveLeft, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//


#define DEFAULT_PWR 20
#define SPACING_THRESH 5.0
#define START 2
#define WHEEL_WIDTH 5.6

#define MAP_MAX_ROWS 100
#define MAP_MAX_COLS 100
#define MAP_PATH "team3-3_file1.dat"

#define OBST_FILE_SZ 100
#define OBST_PATH "team3-3_file2.dat"

/*
	Mathmematically correct modulo operation instead of that trucated
	nonsense C comes with.
*/
#define MOD(n, m) ((((n) % (m)) + (m)) % (m))


enum Compass {E = 0, S = 90, W = 180, N = 270};
enum LightBool {DARK, LIGHT};

typedef struct Coord {
	unsigned int x, y;
} Coord;


// initital calibrations
void calibrateLightThresh(void);
void calibrateRotationRatio(void);
void calibrateProximityTresh(void);
void calibrateSquareLen(void);
void initVariables(void);

// distance conversions
int cmToDeg(float cm);
float cmToRev(float cm);
float degToCm(int deg);
float revToCm(float rev);

// angle conversions
float degToRad(int deg);
int radToDeg(float rad);

// geometric formulae
int angleBetween(Coord p1, Coord p2);
float distBetween(Coord p1, Coord p2);

// turning
void face(int theta);
void pointTurn(int theta);
void pointTurn1(int theta);

// regular motion
void forwardNSquares(float n);
void moveNCm(float n, int pwrL, int pwrR);
void moveTo(Coord newPos);

// motion dependant on light sensor
float moveTilDark(int pwrL, int pwrR);
float moveTilDoubleLine(int spacingThresh, int pwrL, int pwrR);
float moveTilLight(int pwrL, int pwrR);
float moveTilLightChange(int pwrL, int pwrR);

// grid mapping
void mapCurrentPos(void);
void mapGrid(void);
void mapNSquares(int n);
void measureGrid(void);
void initGrid(void);
void writeGrid(void);

// obstacle detection
void setObstaclePos(void);
bool surveyGrid(void);
bool surveyNSquares(int n);
void writeObstaclePos(void);

// misc functions
void idleTilTouch(void);
void incrementRobotPos(void);


Coord robotPos;
int orientation;

unsigned int defaultPwr;
unsigned int wheelWdt;

float rotationRatio;
unsigned int lightThresh;
float proximityThresh;
unsigned int sqLen;

/*
	RobotC lacks dynamic memory allocation.
	Therefore the grid must be of a fixed, excessive size.
*/
char grid[MAP_MAX_ROWS][MAP_MAX_COLS];
unsigned int gridRows, gridCols;
unsigned int gridArea;

Coord startPos;

Coord obstaclePos;
unsigned int whiteC, blackC;


task main() {
	Coord origin;

	origin.x = 0;
	origin.y = 0;

	nxtDisplayTextLine(0, "Touch to begin");
	nxtDisplayTextLine(1, "phase 1:");
	nxtDisplayTextLine(2, "CALIBRATION");
	idleTilTouch();
	eraseDisplay();

	initVariables();
	calibrateLightThresh();
	calibrateRotationRatio();
	calibrateSquareLen();
	calibrateProximityTresh();

	measureGrid();
	/*gridRows = 7;
	gridCols = 9;
	gridArea = gridRows * gridCols;
	startPos.x = 0;
	startPos.y = 4;
	robotPos = startPos;*/

	initGrid();
	writeGrid();

	moveTo(startPos);
	face(E);

	nxtDisplayTextLine(0, "Touch to begin");
	nxtDisplayTextLine(1, "phase 2:");
	nxtDisplayTextLine(2, "OBSTACLE");
	nxtDisplayTextLine(3, "DETECTION");
	idleTilTouch();
	eraseDisplay();

	moveTo(origin);
	face(E);

	nxtDisplayTextLine(0, "Place obstacle");
	nxtDisplayTextLine(1, "on grid.");
	idleTilTouch();
	eraseDisplay();

	if (surveyGrid())
		writeObstaclePos();
}


/*
	Calculate the light sensor threshold.
	Sensor values above this threshold will be considered to be light.
	Values below it will be dark.
*/
void calibrateLightThresh(void) {
	int light, dark;

	eraseDisplay();

	// get light reading
	nxtDisplayTextLine(0, "Put me on a light");
	nxtDisplayTextLine(1, "surface.");
	idleTilTouch();
	light = SensorValue(lightSensor);
	wait1Msec(250);
	eraseDisplay();

	// get dark reading
	nxtDisplayTextLine(0, "Put me on a dark");
	nxtDisplayTextLine(1, "surface.");
	idleTilTouch();
	dark = SensorValue(lightSensor);
	wait1Msec(250);
	eraseDisplay();

	// calc average of ligh and dark
	lightThresh = (light + dark) / 2;
}


/*
	Calibrate how close an obstacle should be before it is detected.
*/
void calibrateProximityTresh(void) {
	eraseDisplay();

	nxtDisplayTextLine(0, "Place me half a");
	nxtDisplayTextLine(1, "square from an");
	nxtDisplayTextLine(2, "obstacle.");
	idleTilTouch();
	eraseDisplay();

	proximityThresh = SensorValue(sonarSensor);
}


/*
	Calibrates a figure called the rotation ratio.
	This is the ratio of the wheels rotaion (at default power) to the
	rotatation of the robot in a point turn.
	ie. the wheels must turn 'rotationRatio' degrees to turn the robot
	1 degree.
*/
void calibrateRotationRatio(void) {
	// current and previous light sensor readings
	bool currLight, prevLight;

	int encoderMean;

	// brake
	motor[motorL] = 0;
	motor[motorR] = 0;

	eraseDisplay();

	nxtDisplayTextLine(0, "Put me on the");
	nxtDisplayTextLine(1, "right edge of a");
	nxtDisplayTextLine(2, "straight line.");
	idleTilTouch();
	eraseDisplay();

	// reset motor encoders
	nMotorEncoder[motorL] = 0;
	nMotorEncoder[motorR] = 0;

	// convert light sensor reading to a boolean value
	currLight = (bool) (SensorValue(lightSensor) / lightThresh);

	// do until a new dark region is encountered
	do {
		// update sensor readings
		prevLight = currLight;
		currLight = (bool) (SensorValue(lightSensor) / lightThresh);

		// set power
		motor[motorL] = defaultPwr;
		motor[motorR] = -defaultPwr;
	} while (currLight == LIGHT || prevLight == DARK);

	// brake
	motor[motorL] = 0;
	motor[motorR] = 0;

	// calc average of the encoder readings
	encoderMean = (nMotorEncoder[motorL] + nMotorEncoder[motorL]) / 2;

	// calc rotation ratio
	rotationRatio = (float) encoderMean / 180.0;
}


/*
	Calibrate the length of 1 grid square.
	Includes the length of the border.
*/
void calibrateSquareLen(void) {
	eraseDisplay();

	nxtDisplayTextLine(0, "Put me on the");
	nxtDisplayTextLine(1, "border in front of");
	nxtDisplayTextLine(2, "2 white squares.");
	idleTilTouch();
	eraseDisplay();

	sqLen = 0;

	// move up to the inner edge of the border
	moveTilLight(defaultPwr, defaultPwr);

	// measure the length of the white square
	sqLen += moveTilDark(defaultPwr, defaultPwr);

	// measure the width of the border
	sqLen += moveTilLight(defaultPwr, defaultPwr);
}


/*
	Initialise a selection of the global variables with default values.
*/
void initVariables(void) {
	orientation = 0;

	defaultPwr = DEFAULT_PWR;
	wheelWdt = WHEEL_WIDTH;

	whiteC = 0;
	blackC = 0;

	nMotorPIDSpeedCtrl[motorL] = mtrSpeedReg;
	nMotorPIDSpeedCtrl[motorR] = mtrSpeedReg;
}


/*
	Convert from centimeters travelled to degrees rotated by wheels.
*/
int cmToDeg(float cm) {
	return (int) (cmToRev(cm) * 360);
}


/*
	Convert from centimeters travelled to revolutions rotated by
	wheels.
*/
float cmToRev(float cm) {
	return cm / (PI * wheelWdt);
}


/*
	Convert from degrees rotated by wheels to centimeters travelled.
*/
float degToCm(int deg) {
	return revToCm((float) deg / 360);
}


/*
	Convert from revolutions rotated by wheels to centimeters
	travelled.
*/
float revToCm(float rev) {
	return rev * (PI * wheelWdt);
}


/*
	Convert an angle from degrees to radians.
*/
float degToRad(int deg) {
	return (float) deg * (PI / 180);
}


/*
	Convert an angle from radians to degrees.
*/
int radToDeg(float rad) {
	return (int) (rad * 180 / PI);
}


/*
	Return the angle between two points.
*/
int angleBetween(Coord p1, Coord p2) {
	return radToDeg(atan2(p2.y - p1.y, p2.x - p1.x));
}


/*
	Return the distance between two points.
*/
float distBetween(Coord p1, Coord p2) {
	return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}


/*
	Face a given orientation (in degrees) by performing a point turn.
*/
void face(int theta) {
	int diff;

	// difference in orientation
	diff = MOD((theta - orientation), 360);

	// turn clockwise if it's faster to do so, else anti-clockwise
	if (diff < 180)
		pointTurn(diff);
	else
		pointTurn(-(360 - diff));

	// update robot's orientation
	orientation = theta;
}


/*
	Perform a point turn of a given angle in degrees.
	Positive angles yield clockwise rotations, and negative yield
	anti-clockwise rotations.
*/
void pointTurn(int theta) {
	if (!theta)
		return;

	// reset motor encoders
	nMotorEncoder[motorL] = 0;
	nMotorEncoder[motorR] = 0;

	// set motor encoder targets for more accurate movements
	nMotorEncoderTarget[motorL] = abs(theta) * rotationRatio;
	nMotorEncoderTarget[motorR] = abs(theta) * rotationRatio;

	// set power
	if (theta > 0) {
		motor[motorL] = -defaultPwr;
		motor[motorR] = defaultPwr;
	}
	else {
		motor[motorL] = defaultPwr;
		motor[motorR] = -defaultPwr;
	}

	// idle loop while motor encoders haven't met their target
	while (
		nMotorRunState[motorL] != runStateIdle ||
		nMotorRunState[motorR] != runStateIdle
	) {
	}
}


/*
	Perform a point turn of a given angle in degrees.
	Positive angles yield clockwise rotations, and negative yield
	anti-clockwise rotations.
	This implementation does not use motor encoder targets and is less
	accurate.
*/
void pointTurn1(int theta) {
	float distance;
	int encoderMean;
	float travelled;

	// reset motor encoders
	nMotorEncoder[motorL] = 0;
	nMotorEncoder[motorR] = 0;

	// distance the wheels must travel and how far they've already gone
	distance = abs(theta) * rotationRatio;
	travelled = 0;

	// while robot has not travelled the required distance
	while (travelled < distance) {
		// set power
		if (theta > 0) {
			motor[motorL] = -defaultPwr;
			motor[motorR] = defaultPwr;
		}
		else {
			motor[motorL] = defaultPwr;
			motor[motorR] = -defaultPwr;
		}

		// calc distance travelled so far
		encoderMean = (nMotorEncoder[motorL] + nMotorEncoder[motorR]) / 2;
		travelled = degToCm(encoderMean);
	}

	// brake
	motor[motorL] = 0;
	motor[motorR] = 0;
}


/*
	Move forward a given distance (measured in grid squares).
*/
void forwardNSquares(float n) {
	int encoderTarget;

	// reset motor encoders
	nMotorEncoder[motorL] = 0;
	nMotorEncoder[motorR] = 0;

	// set motor encoder target
	encoderTarget = n * cmToDeg(sqLen);
	nMotorEncoderTarget[motorL] = encoderTarget;
	nMotorEncoderTarget[motorR] = encoderTarget;

	// set motor power
	motor[motorL] = defaultPwr;
	motor[motorR] = defaultPwr;

	// idle loop while motor encoders haven't met their target
	while (
		nMotorRunState[motorL] != runStateIdle ||
		nMotorRunState[motorR] != runStateIdle
	) {
	}
}


/*
	Move the robot a given number of centimeters.
	The power levels of the motors are given as arguments.
*/
void moveNCm(float n, int pwrL, int pwrR) {
	int signedEncoderL, signedEncoderR;
	int encoderMean;
	float travelled;

	// reset motor encoders
	nMotorEncoder[motorL] = 0;
	nMotorEncoder[motorR] = 0;

	travelled = 0;

	// while robot has not travelled the n centimeters
	while (abs(travelled) < abs(n)) {
		// set power
		motor[motorL] = pwrL;
		motor[motorR] = pwrR;

		// convert motor encoder values to signed integers
		if (pwrL < 0)
			signedEncoderL = -nMotorEncoder[motorL];
		else
			signedEncoderL = nMotorEncoder[motorL];

		if (pwrR < 0)
			signedEncoderR = -nMotorEncoder[motorR];
		else
			signedEncoderR = nMotorEncoder[motorR];

		// calc distance travelled so far
		encoderMean = (signedEncoderL + signedEncoderR) / 2;
		travelled = degToCm(encoderMean);
	}

	// brake
	motor[motorL] = 0;
	motor[motorR] = 0;
}


/*
	Move to a given co-ordinate.
	Update the robot's position and orientation aftwerwards.
*/
void moveTo(Coord newPos) {
	int theta;
	float dist;

	// turn to face newPos
	theta = angleBetween(robotPos, newPos);
	face(theta);
	orientation = theta;

	// move forward to newPos
	dist = distBetween(robotPos, newPos);
	forwardNSquares(dist);
	robotPos = newPos;

}


/*
	Move until the light sensor detects a change from light to dark.
	Robot will still move if it starts on a dark area.
	Motor powers are given as arguments.
	Return the total distance travelled (in centimeters).
*/
float moveTilDark(int pwrL, int pwrR) {
	// current and previous light sensor readings
	bool currLight, prevLight;

	int encoderMean;
	float travelled;

	// reset motor encoders
	nMotorEncoder[motorL] = 0;
	nMotorEncoder[motorR] = 0;

	// convert light sensor reading to a boolean value
	currLight = (bool) (SensorValue(lightSensor) / lightThresh);

	// do until a new light region is encountered
	do {
		// update sensor readings
		prevLight = currLight;
		currLight = (bool) (SensorValue(lightSensor) / lightThresh);

		// set power
		motor[motorL] = pwrL;
		motor[motorR] = pwrR;
	} while (currLight == LIGHT || prevLight == DARK);

	// brake
	motor[motorL] = 0;
	motor[motorR] = 0;

	// measure distance travelled during function call
	encoderMean = (nMotorEncoder[motorL] + nMotorEncoder[motorL]) / 2;
	travelled = degToCm(encoderMean);

	return travelled;
}


/*
	Move robot until it passes two dark lines whose spacing is less
	than a given threshold. (This threshold includes the width of the
	first line because I'm lazy.)
	Return the total distance travelled (in centimeters).
*/
float moveTilDoubleLine(int spacingThresh, int pwrL, int pwrR) {
	float recentDist, totalDist;

	//moveNCm(sqLen, pwrL, pwrR);
	//totalDist = sqLen;

	do {
		recentDist = abs(moveTilDark(pwrL, pwrR));
		totalDist += recentDist;
		eraseDisplay();
		nxtDisplayTextLine(0, "Recent dist: %.1f", recentDist);
		nxtDisplayTextLine(1, "Total dist: %.1f", totalDist);
	} while (recentDist > spacingThresh || totalDist < 1.2 * sqLen);

	return totalDist;
}


/*
	Move until the light sensor detects a change from dark to light.
	Robot will still move if it starts on a light area.
	Motor powers are given as arguments.
	Return the total distance travelled (in centimeters).
*/
float moveTilLight(int pwrL, int pwrR) {
	// current and previous light sensor readings
	bool currLight, prevLight;

	int encoderMean;
	float travelled;

	// reset motor encoders
	nMotorEncoder[motorL] = 0;
	nMotorEncoder[motorR] = 0;

	// convert light sensor reading to a boolean value
	currLight = (bool) (SensorValue(lightSensor) / lightThresh);

	// do until a new light region is encountered
	do {
		// update sensor readings
		prevLight = currLight;
		currLight = (bool) (SensorValue(lightSensor) / lightThresh);

		// set power
		motor[motorL] = pwrL;
		motor[motorR] = pwrR;
	} while (currLight == DARK || prevLight == LIGHT);

	// brake
	motor[motorL] = 0;
	motor[motorR] = 0;

	// measure distance travelled during function call
	encoderMean = (nMotorEncoder[motorL] + nMotorEncoder[motorL]) / 2;
	travelled = degToCm(encoderMean);

	return travelled;
}


/*
	Move until the light sensor detects a change from light to dark or
	dark to light.
	Motor powers are given as arguments.
	Return the total distance travelled (in centimeters).
*/
float moveTilLightChange(int pwrL, int pwrR) {
	// current and previous light sensor readings
	bool currLight, prevLight;

	int encoderMean;
	float travelled;

	// reset motor encoders
	nMotorEncoder[motorL] = 0;
	nMotorEncoder[motorR] = 0;

	// convert light sensor reading to a boolean value
	currLight = (bool) (SensorValue(lightSensor) / lightThresh);

	// do until current and previous readings are different
	do {
		// update sensor readings
		prevLight = currLight;
		currLight = (bool) (SensorValue(lightSensor) / lightThresh);

		// set power
		motor[motorL] = pwrL;
		motor[motorR] = pwrR;
	} while (currLight == prevLight);

	// brake
	motor[motorL] = 0;
	motor[motorR] = 0;

	// measure distance travelled during function call
	encoderMean = (nMotorEncoder[motorL] + nMotorEncoder[motorL]) / 2;
	travelled = degToCm(encoderMean);

	return travelled;
}


/*
	Mark the robot's starting position on the grid,
	and initialise all other valid squares as white.
*/
void initGrid(void) {
	unsigned int i, j;

	// initialise each square to LIGHT
	for (i = 0; i < gridRows; i++) {
		for (j = 0; j < gridCols; j++)
			grid[i][j] = LIGHT;
	}

	// mark starting position
	grid[startPos.y][startPos.x] = 2;
}


/*
	Measure the dimensions of the grid and note the robot's starting
	position.
*/
void measureGrid(void) {
	int dist;

	gridRows = 0;
	gridCols = 0;

	nxtDisplayTextLine(0, "Put me on the");
	nxtDisplayTextLine(1, "starting position.");
	idleTilTouch();
	eraseDisplay();

	// move east
	dist = moveTilDoubleLine(SPACING_THRESH, defaultPwr, defaultPwr);
	moveNCm(dist, -defaultPwr, -defaultPwr);
	gridCols += dist / sqLen - 1;
	wait1Msec(1000);

	// move west
	dist = moveTilDoubleLine(SPACING_THRESH, -defaultPwr, -defaultPwr);
	moveNCm(dist, defaultPwr, defaultPwr);
	gridCols += dist / sqLen - 1;
	startPos.x = dist / sqLen;
	wait1Msec(1000);

	// move south
	face(S);
	dist = moveTilDoubleLine(SPACING_THRESH, defaultPwr, defaultPwr);
	moveNCm(dist, -defaultPwr, -defaultPwr);
	gridRows += dist / sqLen - 1;
	wait1Msec(1000);

	// move north
	dist = moveTilDoubleLine(SPACING_THRESH, -defaultPwr, -defaultPwr);
	moveNCm(dist, defaultPwr, defaultPwr);
	gridRows += dist / sqLen - 1;
	startPos.y = dist / sqLen;

	// update robot's current position
	robotPos = startPos;

	// measure area
	gridArea = gridCols * gridRows;

	eraseDisplay();
	nxtDisplayTextLine(0, "Width: %d", gridCols);
	nxtDisplayTextLine(1, "Height: %d", gridRows);
	nxtDisplayTextLine(2, "Area: %d", gridArea);
	wait1Msec(3000);
}


/*
	Take a light reading from the robot's current position and update
	the grid and colour counters with it.
*/
void mapCurrentPos(void) {
	bool reading;

	reading = (bool) (SensorValue(lightSensor) / lightThresh);

	if (reading == LIGHT)
		whiteC++;
	else
		blackC++;

	// add reading to grid
	if (grid[robotPos.y][robotPos.x] != START)
		grid[robotPos.y][robotPos.x] = (char) ((int) reading);

	eraseDisplay();
	nxtDisplayTextLine(
		0, "grid[%d][%d] = %d",
		robotPos.y, robotPos.x, (int) reading
	);

	writeGrid();
}


/*
	Traverse the entire grid and take a light reading on each square.
*/
void mapGrid(void) {
	unsigned int mapped;
	unsigned int step;

	mapCurrentPos();
	mapped = 1;

	step = 0;

	/*
		While the grid is not fully mapped,
		perform 1 step of a 4 step snaking traversal.
	*/
	while (mapped < gridArea) {
		switch (step % 4) {

			// step 0: east, gridCols squares
			case 0: {
				face(E);
				mapNSquares(gridCols);
				mapped += gridCols;
				break;
			}

			// step 1: south, 1 square
			case 1: {
				face(S);
				mapNSquares(1);
				mapped++;
				break;
			}

			// step 2: west, gridCols squares
			case 2: {
				face(W);
				mapNSquares(gridCols);
				mapped += gridCols;
				break;
			}

			// step 3: south, 1 square
			case 3: {
				face(S);
				mapNSquares(1);
				mapped++;
				break;
			}

			default: {
				//TODO error log
			}
		}

		step++;
	}
}


/*
	Move forward a given number of squares and take a light reading on
	each square passed.
	The robot's orientation must be a multiple of 90 degrees.
*/
void mapNSquares(int n) {
	int encoderTarget;
	int encoderMean;
	int marker;

	// reset motor encoders
	nMotorEncoder[motorL] = 0;
	nMotorEncoder[motorR] = 0;

	// set motor encoder target
	encoderTarget = n * cmToDeg(sqLen);
	nMotorEncoderTarget[motorL] = encoderTarget;
	nMotorEncoderTarget[motorR] = encoderTarget;

	// set motor power
	motor[motorL] = defaultPwr;
	motor[motorR] = defaultPwr;

	// mark where to take the next light reading
	marker = cmToDeg(sqLen) / 2;

	// while motor encoders haven't met their target
	while (
		nMotorRunState[motorL] != runStateIdle ||
		nMotorRunState[motorR] != runStateIdle
	) {
		encoderMean = (nMotorEncoder[motorL] + nMotorEncoder[motorR]) / 2;

		// if the robot passed the marker
		if (encoderMean > marker) {
			// update the robot's position
			incrementRobotPos();

			// take a light reading
			mapCurrentPos();

			// set a new marker
			marker += cmToDeg(sqLen);
		}
	}
}


/*
	Write the grid array to a text file in a readable format.
*/
void writeGrid(void) {
	const char whiteChar = ' ';
	const char blackChar = 'X';
	const char startChar = 'S';
	const char errorChar = 'E';

	TFileHandle file;
	TFileIOResult ioRet;
	short fileSize;
	unsigned int i, j;

	// delete old file
	Delete(MAP_PATH, ioRet);

	// open file
	fileSize = gridArea * 3 + gridRows;
	OpenWrite(file, ioRet, MAP_PATH, fileSize);

	// write each sqaure
	for (i = 0; i < gridRows; i++) {
		for (j = 0; j < gridCols; j++) {
			WriteByte(file, ioRet, '[');
			switch (grid[i][j]) {
				case DARK: {
					WriteByte(file, ioRet, blackChar);
					break;
				}
				case LIGHT: {
					WriteByte(file, ioRet, whiteChar);
					break;
				}
				case START: {
					WriteByte(file, ioRet, startChar);
					break;
				}
				default: {
					WriteByte(file, ioRet, errorChar);
					//TODO error log
				}
			}
			WriteByte(file, ioRet, ']');
		}
		WriteByte(file, ioRet, '\n');
	}

	// terminate and close file
	Close(file, ioRet);
}


/*
	Using the robot's current posiotion and orientation,
	and knowing that the obstacle is 1 tile in front of the robot,
	set the co-ordinates of the obstacle.
*/
void setObstaclePos(void) {
	obstaclePos = robotPos;

	switch (orientation) {
		case E: {
			obstaclePos.x++;
			break;
		}
		case S: {
			obstaclePos.y++;
			break;
		}
		case W: {
			obstaclePos.x--;
			break;
		}
		case N: {
			obstaclePos.y--;
			break;
		}
		default: {
			//TODO error log
		}
	}
}


bool surveyGrid(void) {
	unsigned int surveyed;
	unsigned int step;
	bool foundObstacle;

	mapCurrentPos();
	surveyed = 1;

	step = 0;

	/*
		While the grid is not fully surveyed,
		perform 1 step of a 4 step snaking traversal.
	*/
	while (surveyed < gridArea) {
		switch (step % 4) {

			// step 0: east, gridCols squares
			case 0: {
				face(E);
				foundObstacle = surveyNSquares(gridCols);
				surveyed += gridCols;
				break;
			}

			// step 1: south, 1 square
			case 1: {
				face(S);
				foundObstacle = surveyNSquares(1);
				surveyed++;
				break;
			}

			// step 2: west, gridCols squares
			case 2: {
				face(W);
				foundObstacle = surveyNSquares(gridCols);
				surveyed += gridCols;
				break;
			}

			// step 3: south, 1 square
			case 3: {
				face(S);
				foundObstacle = surveyNSquares(1);
				surveyed++;
				break;
			}

			default: {
				//TODO error log
			}
		}

		if (foundObstacle) {
			setObstaclePos();
			return true;
		}

		step++;
	}

	return false;
}


bool surveyNSquares(int n) {
	int encoderTarget;
	int encoderMean;
	int marker;

	// reset motor encoders
	nMotorEncoder[motorL] = 0;
	nMotorEncoder[motorR] = 0;

	// set motor encoder target
	encoderTarget = n * cmToDeg(sqLen);
	nMotorEncoderTarget[motorL] = encoderTarget;
	nMotorEncoderTarget[motorR] = encoderTarget;

	// set motor power
	motor[motorL] = defaultPwr;
	motor[motorR] = defaultPwr;

	// mark where to take the next ultrasonic reading
	marker = cmToDeg(sqLen) / 2;

	// while motor encoders haven't met their target
	while (
		nMotorRunState[motorL] != runStateIdle ||
		nMotorRunState[motorR] != runStateIdle
	) {
		// take an ultrasonic reading
		if (SensorValue(sonarSensor) < proximityThresh) {
			obstaclePos = robotPos;
			return true;
		}

		encoderMean = (nMotorEncoder[motorL] + nMotorEncoder[motorR]) / 2;

		// if the robot passed the marker
		if (encoderMean > marker) {
			// update the robot's position
			incrementRobotPos();

			// set a new marker
			marker += cmToDeg(sqLen);
		}
	}

	return false;
}


void writeObstaclePos(void) {
	TFileHandle file;
	TFileIOResult ioRet;
	short fileSize;

	// delete old file
	Delete(OBST_PATH, ioRet);

	// open file
	fileSize = OBST_FILE_SZ;
	OpenWrite(file, ioRet, OBST_PATH, fileSize);

	// write obstacle's co-ordinates
	WriteText(file, ioRet, "Obstacle position: ");
	WriteShort(file, ioRet, obstaclePos.x);
	WriteText(file, ioRet, ", ");
	WriteShort(file, ioRet, obstaclePos.y);
	WriteByte(file, ioRet, '\n');

	// write colour of obstacle's tile
	WriteText(file, ioRet, "Tile colour: ");
	switch (grid[obstaclePos.y][obstaclePos.x]) {
		case DARK: {
			WriteText(file, ioRet, "Black\n");
			break;
		}
		case LIGHT: {
			WriteText(file, ioRet, "White\n");
			break;
		}
		default: {
			WriteText(file, ioRet, "Unknown\n");
		}
	}

	// terminate and close file
	Close(file, ioRet);
}


/*
	Wait idly until the touch sensor is pressed.
*/
void idleTilTouch(void) {
	while (SensorValue(touchSensor) == 0) {
		// idle loop
	}
	wait1Msec(250);
}


/*
	Using the robot's orientation, update its current position to
	reflect moving forward one square.
*/
void incrementRobotPos(void) {
	switch (orientation) {
		case E: {
			robotPos.x++;
			break;
		}
		case S: {
			robotPos.y++;
			break;
		}
		case W: {
			robotPos.x--;
			break;
		}
		case N: {
			robotPos.y--;
			break;
		}
		default: {
			//TODO error log
		}
	}
}