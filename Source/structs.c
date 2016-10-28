enum Axes {X, Y};
enum LightBool {DARK, LIGHT};


typedef struct Robot {
	unsigned int pos[2];
	unsigned int direction;
	unsigned int defaultSpd;
	unsigned int wheelWdt;
	unsigned int axelWdt;
	float rotationRatio;
} Robot;


typedef struct Grid {
	unsigned int sqLen;
	unsigned int width, height;
	unsigned int area;
	unsigned int lightThresh;
	unsigned int startPos[2];
	unsigned int obstaclePos[2];
	char obstacleColour;
	unsigned int whiteC, blackC;
	char **map;
} Grid;
