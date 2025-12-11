// ------------------- RACE_PROGRAM_STATE.c -------------------
/* Muködése megegyezik a struck_v10.c programmal
Új: külön fájba ír és olvas. Így nem tanul, hanem adatot gyüjt, és lejátsza amit kézzel beírunk!
Ha ciklusról ciklusra akarjuk tanulómódra állítani, ugyanazt a fájltnevet kell beírni track_save és load_track függvénekbe
Nincs learn, és debug változó!
Van egy DEBUG_PID.c program a kezdo beállításokhoz!
*/

// ------------------- STRUKTÚRÁK -------------------
enum Direction { RIGHT=1, LEFT=-1 };
enum State { LINE=0, CORR=1, ERROR=2 };

typedef struct { byte S1,S2,S3,S4; byte S1MAX,S2MAX,S3MAX,S4MAX; } Sensors;
typedef struct { float Kp,Ki,Kd; float I,D; int error,prevError; Direction lastDir; } PID;

#define MAX_TRACKPOINTS 100
typedef struct { int loaded[MAX_TRACKPOINTS]; int measured[MAX_TRACKPOINTS]; int countLoaded,countMeasured; long startTime; int playbackIndex; long shiftTime; } TrackMap;
typedef struct { State prev,curr; long changeTime; } StateTransition;

// ------------------- GLOBALIS VÁLTOZÓK -------------------
Sensors s;
PID pid;
State state;
TrackMap tm;

bool midSeenSinceLastShift = false;
long errorStartTime = 0;
long shiftLog[MAX_TRACKPOINTS];
int shiftLogCount = 0;

StateTransition transition;

long dbg_ciklus = 1;
long maxCycle = 0;
long prevCycleTime = 0;

// ------------------- FUNKCIÓK -------------------
string sensorERROR;
// Szenzor olvasás
bool kikapcsolt=false;
void readSensors1234() {
	  /*if (SensorValue[S1]==0 ||SensorValue[S2]==0||SensorValue[S3]==0||SensorValue[S4]==0){
	  	kikapcsolt=true;
	  	return;
	  }*/


    s.S1 = (SensorValue[S1] * 100) / s.S1MAX;
    s.S2 = (SensorValue[S2] * 100) / s.S2MAX;
    s.S3 = (SensorValue[S3] * 100) / s.S3MAX;
    s.S4 = (SensorValue[S4] * 100) / s.S4MAX;

    if (s.S1 ==0)strcat(sensorERROR, "1");
		if (s.S2 ==0)strcat(sensorERROR, "2");
		if (s.S3 ==0)strcat(sensorERROR, "3");
		if (s.S4 ==0)strcat(sensorERROR, "4");

    if (s.S1 > 100) s.S1 = 100;
    if (s.S2 > 100) s.S2 = 100;
    if (s.S3 > 100) s.S3 = 100;
    if (s.S4 > 100) s.S4 = 100;

    if (s.S2 < 40 || s.S3 < 40)
        midSeenSinceLastShift = true;

    if (midSeenSinceLastShift) {
        if (s.S1 < 40) { pid.lastDir = RIGHT; midSeenSinceLastShift = false; }
        else if (s.S4 < 40) { pid.lastDir = LEFT; midSeenSinceLastShift = false; }
    }
}

// PID számítás
int computePID() {
    if (state == ERROR) pid.I = 0;
    else if (state == LINE) pid.I += pid.error;

    pid.D = pid.error - pid.prevError;

    float output = pid.Kp * pid.error + pid.Ki * pid.I + pid.Kd * pid.D;
    pid.prevError = pid.error;

    if (output > 50) output = 50;
    else if (output < -50) output = -50;

    return (int)output;
}

// Error számítás
int compute_error() {
    if (s.S1<70 && s.S2<70 && s.S3<70 && s.S4<70) {
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

// Track inicializálás
void track_init() {
    tm.countLoaded = 0;
    tm.countMeasured = 0;
    tm.startTime = nSysTime;
    tm.playbackIndex = 0;
    tm.shiftTime = 0;

    for (int i=0; i<MAX_TRACKPOINTS; i++) {
        tm.loaded[i] = 0;
        tm.measured[i] = 0;
        shiftLog[i] = 0;
    }
    shiftLogCount = 0;
}

// Track betöltés fájlból
void track_load(string fileName) {
    int handle = fileOpenRead(fileName);
    if (handle < 0) return;

    byte ch;
    string token = "";
    static int index = 0;

    while (fileReadData(handle, &ch, 1) == 1) {
        if (ch == ',' || ch == '\n' || ch == '\r') {
            if (strlen(token) > 0) {
                tm.loaded[index++] = atoi(token);
                token = "";
                if (index >= MAX_TRACKPOINTS) break;
            }
        } else {
            string tmp;
            stringFormat(tmp, "%c", ch);
            token += tmp;
        }
    }

    if (strlen(token) > 0 && index < MAX_TRACKPOINTS)
        tm.loaded[index++] = atoi(token);

    fileClose(handle);
    tm.countLoaded = index;
}

/*void track_load2(string fileName, int appendNum, int lepesKoz) {
    int handle = fileOpenRead(fileName);
    if (handle < 0) return;

    byte ch;
    string token = "";
    static int index = 0;

    while (fileReadData(handle, &ch, 1) == 1) {
        if (ch == ',' || ch == '\n' || ch == '\r') {
            if (strlen(token) > 0) {
                tm.loaded[index++] = atoi(token);
                int kPoz = index-1;
                for(int i = 0; i < appendNum; i++)
                {
                		tm.loaded[index++] = tm.loaded[index-1] + lepesKoz;
                }

                tm.loaded[index++] = tm.loaded[kPoz] - lepesKoz;
                for(int i = 0; i < appendNum - 1; i++)
                {
                		tm.loaded[index++] = tm.loaded[index-1] -lepesKoz;
                }

                token = "";
                if (index >= MAX_TRACKPOINTS) break;
            }
        } else {
            string tmp;
            stringFormat(tmp, "%c", ch);
            token += tmp;
        }
    }

    if (strlen(token) > 0 && index < MAX_TRACKPOINTS)
        tm.loaded[index++] = atoi(token);

    fileClose(handle);
    tm.countLoaded = index;
}
*/
void track_load2(string fileName, int appendNum, int lepesKoz) {
    int handle = fileOpenRead(fileName);
    if (handle < 0) return;

    byte ch;
    string token = "";
    int index = tm.countLoaded ;

    while (fileReadData(handle, &ch, 1) == 1) {
        if (ch == ',' || ch == '\n' || ch == '\r') {
            if (strlen(token) > 0 && index < MAX_TRACKPOINTS) {
                int base = atoi(token);      // az eredeti szám
                tm.loaded[index++] = base;

                // növekvo sorozat
                for(int i = 1; i <= appendNum && index < MAX_TRACKPOINTS; i++)
                    tm.loaded[index++] = base + i * lepesKoz;

                // csökkeno sorozat
                for(int i = 1; i <= appendNum && index < MAX_TRACKPOINTS; i++)
                    tm.loaded[index++] = base - i * lepesKoz;

                token = "";
            }
        } else {
            string tmp;
            stringFormat(tmp, "%c", ch);
            token += tmp;
        }
    }

    // végso token
    if (strlen(token) > 0 && index < MAX_TRACKPOINTS) {
        int base = atoi(token);
        tm.loaded[index++] = base;

        for(int i = 1; i <= appendNum && index < MAX_TRACKPOINTS; i++)
            tm.loaded[index++] = base + i * lepesKoz;

        for(int i = 1; i <= appendNum && index < MAX_TRACKPOINTS; i++)
            tm.loaded[index++] = base - i * lepesKoz;
    }

    fileClose(handle);
    tm.countLoaded = index;
}

void sortArray(int* arr, int size) {
    for(int i = 0; i < size - 1; i++) {
        int minIndex = i;

        for(int j = i + 1; j < size; j++) {
            if(arr[j] < arr[minIndex]) {
                minIndex = j;
            }
        }

        // Csere
        int temp = arr[i];
        arr[i] = arr[minIndex];
        arr[minIndex] = temp;
  }
}
// Track mentés fájlba
void track_save(string fileName) {
    int handle = fileOpenWrite(fileName);
    if (handle < 0) return;

    for (int i=0; i<tm.countMeasured; i++) {
        string tmp;
        stringFormat(tmp, "%d,", tm.measured[i]);
        fileWriteData(handle, tmp, strlen(tmp));
    }

    fileWriteChar(handle, '\n');
    fileClose(handle);
}

// Fékezés
void doBrakeforSlow(int fek) {
    for(int i=0;i<fek;i++){ motor[motorA]=motor[motorD]=0; wait1Msec(1);}
}
void doBrakeforPlay(int fek) {
    for(int i=0;i<fek;i++){ motor[motorA]=motor[motorD]=0; wait1Msec(1);}
}
void doBrakeforError(int fek) {
    for(int i=0;i<fek;i++){
        if(pid.lastDir==LEFT){ motor[motorA]=-100; motor[motorD]=100; }
        else{ motor[motorA]=100; motor[motorD]=-100; }
        readSensors1234();
        wait1Msec(1);
    }
}

// Diagnosztika
void printDiagnostics() {
    long elapsed = nSysTime - tm.startTime;
    float avgCycle = (float)elapsed / dbg_ciklus;

    eraseDisplay();
    displayBigTextLine(0,"AVG: %.2f ms", avgCycle);
    displayBigTextLine(2,"MAX: %ld ms", maxCycle);
    displayBigTextLine(4,"Battery: %.2f V", nAvgBatteryLevel/1000.0);
		string txt = kikapcsolt ? "true" : "false";
		displayBigTextLine(6, "S %s", sensorERROR);

    wait1Msec(10000);
}

// Állapotátmenet frissítése
void setTransition(){
    if(transition.curr != state){
        transition.prev = transition.curr;
        transition.curr = state;
        transition.changeTime = nSysTime;
    }
}
// ------------------- Long abs függvény RobotC-hez ----------------
long longAbs(long x) {
    return (x < 0) ? -x : x;
}

// Detect brake pont
bool detectBrakePoint() {
    static long blockUntil = 0;
    static State prevState = LINE; // kezdeti állapot
    if(nSysTime < blockUntil) return false;

    if (state == ERROR && prevState != ERROR) {

        if(tm.countMeasured < MAX_TRACKPOINTS)
        {
            // relatív ido startTime-hoz képest (ms)
            int ment = (int)(nSysTime - tm.startTime - 350);
            tm.measured[tm.countMeasured++] = ment;

            // ---- LEGKÖZELEBBI páros keresése ----
            long bestDiff = 999999;
            int bestIndex = -1;

            for(int i = 0; i < tm.countLoaded; i++){
                long diff = ment - tm.loaded[i];
                long adiff = longAbs(diff);

                // csak ±SHIFT_TOLERANCE-ig tekintjük érvényesnek
                if(adiff < bestDiff && adiff <= 600){
                    bestDiff = adiff;
                    bestIndex = i;
                }
            }

            if(bestIndex >= 0) {
                long diff = ment - tm.loaded[bestIndex];
                tm.shiftTime = diff;

                // shift log mentése
                if(shiftLogCount < MAX_TRACKPOINTS){
                    shiftLog[shiftLogCount++] = tm.shiftTime;
                }
            }
        }

        doBrakeforError(1);
        blockUntil = nSysTime + 600; // 600 ms blokkolás
        setLEDColor(ledRed);
        playImmediateTone(50,1);
        prevState = state;
        return true;
    }

    prevState = state;
    return false;
}

// Playback fékezés
void checkPlaybackBrake(){
    if (tm.playbackIndex >= tm.countLoaded) return;

    long t = nSysTime - tm.startTime;
    long corrected = tm.loaded[tm.playbackIndex] + tm.shiftTime;

    if (t >= corrected) {
        doBrakeforPlay(3);
        setLEDColor(ledGreen);
        playImmediateTone(1000,1);
        tm.playbackIndex++;
    }
}

// Ciklusido frissítése
void updateCycleTime(){
    long now = nSysTime;
    long cycleTime = now - prevCycleTime;
    prevCycleTime = now;

    dbg_ciklus++;

    if (cycleTime > maxCycle)
        maxCycle = cycleTime;
}

task main(){
    // *** KRITIKUS HIBA JAVÍTVA ***
    state = LINE;
    transition.prev = LINE;
    transition.curr = LINE;
		int lastMotorA=100;
		int lastMotorD=100;
		int targetA;
		int targetD;
    //Aktuális FÉNY adatok
    //s.S1MAX = 54; s.S2MAX = 49; s.S3MAX = 45; s.S4MAX = 51; // 1. PÁLYA
    //s.S1MAX = 55; s.S2MAX = 46; s.S3MAX = 43; s.S4MAX = 45; // 2. PÁLYA
    s.S1MAX = 52; s.S2MAX = 47; s.S3MAX = 48; s.S4MAX = 47; // 3. PÁLYA
    //s.S1MAX = 50; s.S2MAX = 45; s.S3MAX = 42; s.S4MAX = 45; // 4. PÁLYA

    transition.changeTime = nSysTime;
    track_init();

    string fileName2 = "ujPalya.txt";
    track_load(fileName2);
    string fileName3 = "UP.txt";
    track_load2(fileName3, 5, 100);
    sortArray(tm.loaded,tm.countLoaded);

    /*motor[motorA] = 90;
    motor[motorD] = 100;
    wait1Msec(150);*/


    tm.startTime = prevCycleTime = nSysTime;

    while (nSysTime - tm.startTime < 15000) {

        readSensors1234();
        pid.error = compute_error();
        int correction = computePID();

        setTransition();

        float speed = 1.6;	//1.6 volt

        if (state == ERROR){
						motor[motorA] = (40 - correction)*2;
						motor[motorD] = (40 + correction)*2;
        //}else if( state!=ERROR  && transition.changeTime+50>nSysTime /*&& (s.S2 >= 80 || s.S3 >= 80)*/){
        		 //pid.Kp = 0.5; pid.Ki = 0.0; pid.Kd = 20;
        		 /*pid.Kp = 0.5; pid.Ki = 0.0; pid.Kd = 1000;//50volt
            motor[motorA] = (50 - correction)*speed;
						motor[motorD] = (50 + correction)*speed;
        		 //playImmediateTone(500,1);*/
        }else {
            pid.Kp = 0.45; pid.Ki = 0.0015; pid.Kd = 9;
						motor[motorA] = (50 - correction)*speed;
						motor[motorD] = (50 + correction)*speed;
        }

        lastMotorA = motor[motorA] = (motor[motorA]*2+lastMotorA)/3;
        lastMotorD = motor[motorD] = (motor[motorD]*2+lastMotorD)/3;



        detectBrakePoint();
        checkPlaybackBrake();
        updateCycleTime();

        //if (false/*dbg_ciklus % 500 == 0*/)
          //  doBrakeforSlow(1);

        if(state == LINE && dbg_ciklus % 10 == 0)
        {
        		wait1Msec(2);
        }else
        	wait1Msec(1);
    }

    motor[motorA] = motor[motorD] = 0;

    string fileName1 = "ujPalya.txt";
    track_save(fileName1);

    printDiagnostics();
}
