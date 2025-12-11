// ------------------- DEBUG_LOG.c -------------------

// ------------------- STRUKTÚRÁK -------------------
enum Direction {
    RIGHT = 1,
    LEFT  = -1
};

enum State {
    LINE = 0,
    CORR = 1,
    ERROR = 2
};

typedef struct {
    byte S1;
    byte S2;
    byte S3;
    byte S4;
    byte S1MAX;
    byte S2MAX;
    byte S3MAX;
    byte S4MAX;
} Sensors;

typedef struct {
    long startTime;
    int ciklus;
} Debug;

typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float I;
    float D;
    int error;
    int prevError;
    Direction lastDir;
} PID;

// ------------------- GLOBALIS VÁLTOZÓK -------------------
Sensors s;
PID pid;
Debug dbg;
State state;
State nextState;

// ------------------- FUNKCIÓK -------------------
bool midSeenSinceLastShift = false;
// Szenzor olvasás (4 szenzor)
void readSensors1234(Sensors *s) {
    s->S1 = (SensorValue[S1] * 100) / s->S1MAX;
    s->S2 = (SensorValue[S2] * 100) / s->S2MAX;
    s->S3 = (SensorValue[S3] * 100) / s->S3MAX;
    s->S4 = (SensorValue[S4] * 100) / s->S4MAX;
    if(s->S1 > 100) s->S1 = 100;
    if(s->S2 > 100) s->S2 = 100;
    if(s->S3 > 100) s->S3 = 100;
    if(s->S4 > 100) s->S4 = 100;
	if(s->S2<40||s->S3<40)midSeenSinceLastShift=true;
	if(midSeenSinceLastShift){
		if(s->S1<40){ pid.lastDir=RIGHT; midSeenSinceLastShift=false; }
		else if(s->S4<40){ pid.lastDir=LEFT; midSeenSinceLastShift=false; }
	}
}

// Error számítás
int compute_error() {
    if (s.S1<80 && s.S2<80 && s.S3<80 && s.S4<80) {
        state = LINE; pid.I = 0; return 0;
    }
    if (s.S1>80 && s.S2>80 && s.S3>80 && s.S4>80) {
        state = ERROR; pid.I = 0; return 100 * pid.lastDir;
    }
    if (s.S2>80 && s.S3>80) {
        state = CORR;
        if (s.S1>80) { pid.I = 0; return -60; }
        else if (s.S4>80) { pid.I = 0; return 60; }
    }
    state = LINE;
    return (int)(-s.S1*3 - s.S2*1 + s.S3*1 + s.S4*3) / 4;
}

// PID számítás
int computePID(PID *pid) {
    if(nextState == ERROR)
        pid->I = 0;
    else if(nextState == LINE)
        pid->I += pid->error;

    pid->D = pid->error - pid->prevError;
    float output = pid->Kp*pid->error + pid->Ki*pid->I + pid->Kd*pid->D;
    pid->prevError = pid->error;
    if(output > 50) output = 50;
    else if(output < -50) output = -50;
    return (int)output;
}

// ------------------- MAIN PROGRAM -------------------
task main() {
    // Inicializálás
    s.S1MAX = 54; s.S2MAX = 44; s.S3MAX = 41; s.S4MAX = 50;
    pid.Kp = 0.5; pid.Ki = 0.0015; pid.Kd = 1000;
    pid.I = 0; pid.D = 0; pid.error = 0; pid.prevError = 0; pid.lastDir = LEFT;

    dbg.startTime = nSysTime;
    dbg.ciklus = 1;
    state = LINE;
    nextState = LINE;
		while(true) {
		    readSensors1234(&s);
		    pid.error = compute_error();
		    int correction = computePID(&pid);

		    // csak minden 100. ciklusban frissítjük a kijelzot
		    if(dbg.ciklus % 100 == 0) {
		        displayBigTextLine(0, "%d %d %d %d", s.S1, s.S2, s.S3, s.S4);
		        displayBigTextLine(2, "Error:%d", pid.error);
		        displayBigTextLine(4, "Corr:%d", correction);
		        displayBigTextLine(6, "V%d %d %d %d", SensorValue[S1], SensorValue[S2], SensorValue[S3], SensorValue[S4]);

		        if(nextState == ERROR)
		            displayBigTextLine(8, "%.2fV ERROR", nAvgBatteryLevel/1000.0);
		        else if(nextState == CORR)
		            displayBigTextLine(8, "%.2fV CORR", nAvgBatteryLevel/1000.0);
		        else
		            displayBigTextLine(8, "%.2fV LINE", nAvgBatteryLevel/1000.0);
		    }

		    dbg.ciklus++;
		    wait1Msec(1); // gyors ciklus
		}
}
