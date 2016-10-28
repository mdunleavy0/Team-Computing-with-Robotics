
#define TYRE_RAD 2.8


float cmToDeg(float cm, float rad);
float cmToRev(float cm, float rad);
float degToCm(float deg, float rad);
float revToCm(float rev, float rad);


task main() {
	nxtDisplayTextLine(0, "5 cm = %.1f rev", cmToRev(5, TYRE_RAD));
	nxtDisplayTextLine(1, "5 cm = %.0f deg", cmToDeg(5, TYRE_RAD));
	nxtDisplayTextLine(2, "1 rev = %.1f cm", revToCm(1, TYRE_RAD));
	nxtDisplayTextLine(3, "360 deg = %.1f cm", degToCm(360, TYRE_RAD));
}


float cmToDeg(float cm, float rad) {
	return cmToRev(cm, rad) * 360;
}


float cmToRev(float cm, float rad) {
	return cm / (2 * PI * rad);
}


float degToCm(float deg, float rad) {
	return revToCm(deg, rad) / 360;
}


float revToCm(float rev, float rad) {
	return rev * (2 * PI * rad);
}
