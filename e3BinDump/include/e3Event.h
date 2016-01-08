#ifndef E3_EVENT
#define E3_EVENT

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>

#include "TObject.h"

using namespace std;

const int BUFSIZE =10e6;         //Max size (bytes) of input buffer
const int MAXXLENGHT=158;        //Lunghezza camera asse X in cm.
const double STRIPWIDTH=2.5;     //Larghezza della strip lungo Y in cm.
const double STRIPGAP=0.7;       //Distanza tra due strip contigue in cm.
const double SIGNALSPEED=16.4;   //Velocita' del segnale lungo la strip in cm/ns
//FARE UN INCLUDE FILE - NAMESPACE DI TUTTE QUESTE VARIABILI!!!!

class e3GPSData;            //0x0
class e3RawEvent;           //0x1 or 0x2
class e3SORData;            //0x3
class e3GEOData;            //0x5
class e3WADData;           //0x6
class e3VWSData;           //0x7

struct sData {
  unsigned int dummy : 27;
  unsigned int type : 5;
};

enum dataType {
  GPS=0x0,
  SOR=0x3,
  GEO=0x5,
  WAD=0x6,
  VWS=0x7
};

class e3DataBlock : public TObject {

 private:

  unsigned int _vLevel;
  ifstream *_inFile;
  char _buffer[BUFSIZE];
  unsigned int _offSet;
  int _blockSize, _dataType;
  char *_evntBlock, *_sngBlock;
  sData _data;
  string _dtFilterStr;

 public:

  e3DataBlock(){ _vLevel = 0; _dtFilterStr="GPS!EV1|EV2|SOR|GEO|WAD|VWS"; }
  int openBinFile(ifstream *);
  void closeBinFile(){};
  int getBinData(char *, unsigned int);
  int getBlockData(char *&, unsigned int, unsigned int offSet = 0);
  int getGPSData(e3GPSData *);
  int getSORData(e3SORData *);
  int getGEOData(e3GEOData *);
  int getWADData(e3WADData *);
  int getVWSData(e3VWSData *);
  int getNextBlock();
  int getNextEvent(e3RawEvent *);
  void dump(char *, unsigned int, unsigned int);

  inline void setVerbosityLevel(unsigned int vLevel){ _vLevel = vLevel; }
  inline void setDTFilter(string dtFilterStr){ _dtFilterStr=dtFilterStr; }

  ClassDef(e3DataBlock,1)
};

class e3GPSData : public TObject {

 private:

  unsigned int _eventID;
  unsigned int _time[4];
  unsigned int _status[2];
  unsigned int _PRN[2];
  unsigned int _position[4];

 public:

  e3GPSData();
  void init();
  void dump(ofstream *logFile=NULL);

  inline void setEventID(unsigned int eventID){ _eventID = eventID; }
  inline unsigned int getEventID(void){ return _eventID; }

  inline void setTime(unsigned int iTime, unsigned int time){ _time[iTime-1]=time; }
  inline unsigned int getTime(unsigned int iTime){ return _time[iTime-1]; }
  inline unsigned int getYear(void){ return ((_time[3]>>5)&0x1f)+1996; }
  inline unsigned int getDayOfYear(void){ return ((_time[3]&0x1f)<<4)|((_time[2]>>12)&0x0f); }
  inline unsigned int getSecondsOfDay(void){ return ((_time[2]&0xfff)<<5)|((_time[1]>>11)&0x1f); }
  inline unsigned int getNanoSeconds(void){ return ((_time[1]&0x7ff)<<16)|(_time[0]&0xffff)*10; }

  inline bool getSchedSatVis(void){ return ((_time[3]>>10)&0x1); }                  //not used in EEE
  inline unsigned int getNumOfVisSat(void){ return ((_time[3]>>11)&0x7); }
  inline unsigned int getNavMode(void){ return ((_time[3]>>14)&0x3); }          //not used in EEE

  inline void setStatus(unsigned int iStatus, unsigned int status){ _status[iStatus-1]=status; } 
  inline unsigned int getStatus(unsigned int iStatus){ return _status[iStatus-1]; } //not used in EEE
  inline void setPRN(unsigned int iPRN, unsigned int PRN){ _PRN[iPRN-1]=PRN; }
  inline unsigned int getPRN(unsigned int iPRN){ return _PRN[iPRN-1]; } //not used in EEE

  inline void setPosition(unsigned int iPosition, unsigned int position){ _position[iPosition-1]=position; }
  inline unsigned int getPosition(unsigned int iPosition){ return _position[iPosition-1]; }
  inline unsigned int getLatitudeDeg(void){ return ((_position[0]>>0)&0x7f); }
  inline unsigned int getLatitudeMin(void){ return ((_position[0]>>7)&0x3f); }
  inline unsigned int getLatitudeMinFrac(void){ return ((_position[1]&0x7f)<<3)|((_position[0]>>13)&0x7); }
  inline unsigned int getNSFlag(void){ return ((_position[1]>>7)&0x1); }
  inline unsigned int getLongitudeDeg(void){ return ((_position[1]>>8)&0x7f); }
  inline unsigned int getLongitudeMin(void){ return ((_position[2]>>0)&0x3f); }
  inline unsigned int getLongitudeMinFrac(void){ return ((_position[2]>>6)&0x3ff); }
  inline unsigned int getEWFlag(void){ return ((_position[3]>>0)&0x1); }
  inline unsigned int getAltitude(void){ return ((_position[3]>>1)&0x3ff); }
  inline unsigned int getABSL(void){ return ((_position[3]>>11)&0x1); } //not used in EEE

  ClassDef(e3GPSData,1)
};

class e3SORData : public TObject {

 private:

  unsigned int _edgeDetectionMode;
  unsigned int _edgeTimeResolution;
  unsigned int _pulseWidthResolution;
  unsigned int _maxTime1;
  unsigned int _maxTime2;
  stringstream _comments;

 public:

  e3SORData();
  void init();
  void dump(ofstream *logFile=NULL);

  inline void setEdgeDetectionMode(unsigned int edgeDetectionMode){ _edgeDetectionMode = edgeDetectionMode; }
  inline unsigned int getEdgeDetectionMode(void){ return _edgeDetectionMode; }
  inline void setEdgeTimeResolution(unsigned int edgeTimeResolution){ _edgeTimeResolution = edgeTimeResolution; }
  inline unsigned int getEdgeTimeResolution(void){ return _edgeTimeResolution; }
  inline void setPulseWidthResolution(unsigned int pulseWidthResolution){ _pulseWidthResolution = pulseWidthResolution; }
  inline unsigned int getPulseWidthResolution(void){ return _pulseWidthResolution; }
  inline void setMaxTime1(unsigned int maxTime1){ _maxTime1 = maxTime1; }
  inline unsigned int getMaxTime1(void){ return _maxTime1; }
  inline void setMaxTime2(unsigned int maxTime2){ _maxTime2 = maxTime2; }
  inline unsigned int getMaxTime2(void){ return _maxTime2; }
  
  inline void setComments(char buffer){ _comments<<buffer; }
  inline string getComments(void){ return _comments.str(); }

  ClassDef(e3SORData,1)
};

class e3GEOData : public TObject {

 private:

  unsigned int _stationID;
  stringstream  _stationReg;
  unsigned int _fecType[3][2];
  float _cableLen[3][2];
  float _dist23;
  float _dist12;
  float _angle;
  float _grangle;

 public:

  e3GEOData();
  void init();
  void dump(ofstream *logFile=NULL);

  inline void setFecType(unsigned int MRPC, unsigned int side, unsigned int fecType){ _fecType[MRPC][side] = fecType; } //METTERE PROTEZIONI
  inline unsigned int getFecType(unsigned int MRPC, unsigned int side){ return _fecType[MRPC][side]; } //METTERE PROTEZIONI
  inline void setCableLen(unsigned int MRPC, unsigned int side, float cableLen){ _cableLen[MRPC][side] = cableLen; } //METTERE PROTEZIONI
  inline float getCableLen(unsigned int MRPC, unsigned int side){ return _cableLen[MRPC][side]; } //METTERE PROTEZIONI
  inline void setDist23(float dist23){ _dist23 = dist23; }
  inline float getDist23(void){ return _dist23; }
  inline void setDist12(float dist12){ _dist12 = dist12; }
  inline float getDist12(void){ return _dist12; }
  inline float getAngle(void){ return _angle; }
  inline void setGrAngle(float grangle){ _grangle = grangle; }
  inline float getGrAngle(void){ return _grangle; }
  
  inline void setStationID(unsigned int stationID){ _stationID = stationID; }
  inline void setStationReg(char buffer){ _stationReg<<buffer; }
  inline unsigned int getStationID(void){ return _stationID; }
  inline string getStationReg(void){ return _stationReg.str(); }
  inline string getStationIDComp(void){ stringstream compID; compID.fill('0'); compID<<_stationReg.str()<<"-"<<setw(2)<<_stationID; return compID.str(); }

  ClassDef(e3GEOData,1)
};

class e3WADData : public TObject {

 private:

  unsigned int _windowWidth;
  int _windowOffset;
  unsigned int _searchMargin;
  unsigned int _rejectMargin;

  unsigned int _delayBetTrgAndCrst;
  unsigned int _triggerSub;


 public:

  e3WADData();
  void init();
  void dump(ofstream *logFile=NULL);
 
  inline void setWindowWidth(unsigned int windowWidth){ _windowWidth = windowWidth; }
  inline unsigned int getWindowWidth(void){ return _windowWidth; }
  inline void setWindowOffset(int windowOffset){ _windowOffset = windowOffset; }
  inline int getWindowOffset(void){ return _windowOffset; }
  inline void setSearchMargin(unsigned int searchMargin){ _searchMargin = searchMargin; }
  inline unsigned int getSearchMargin(void){ return _searchMargin; }
  inline void setRejectMargin(unsigned int rejectMargin){ _rejectMargin = rejectMargin; }
  inline unsigned int getRejectMargin(void){ return _rejectMargin; }

  inline void setDelayBetTrgAndCrst(unsigned int delayBetTrgAndCrst){ _delayBetTrgAndCrst = delayBetTrgAndCrst; }
  inline unsigned int getDelayBetTrgAndCrst(void){ return _delayBetTrgAndCrst; }
  inline void setTriggerSub(unsigned int triggerSub){ _triggerSub = triggerSub; }
  inline unsigned int getTriggerSub(void){ return _triggerSub; }

  ClassDef(e3WADData,1)
};

class e3VWSData : public TObject {

 private:

  unsigned int _year;
  unsigned int _month;
  unsigned int _day;
  unsigned int _hours;
  unsigned int _minutes;

  float _indoorTemperature;
  float _outdoorTemperature;
  float _slBarometer;

  public:

  e3VWSData();
  void init();
  void dump(ofstream *logFile=NULL);
 
  inline void setYear(unsigned int year){ _year = year; }
  inline unsigned int getYear(void){ return _year; }
  inline void setMonth(unsigned int month){ _month = month; }
  inline unsigned int getMonth(void){ return _month; }
  inline void setDay(unsigned int day){ _day = day; }
  inline unsigned int getDay(void){ return _day; }
  inline void setHours(unsigned int hours){ _hours = hours; }
  inline unsigned int getHours(void){ return _hours; }
  inline void setMinutes(unsigned int minutes){ _minutes = minutes; }
  inline unsigned int getMinutes(void){ return _minutes; }

  inline void setIndoorTemperature(float indoorTemperature){ _indoorTemperature = indoorTemperature; }
  inline float getIndoorTemperature(void){ return _indoorTemperature; }
  inline void setOutdoorTemperature(float outdoorTemperature){ _outdoorTemperature = outdoorTemperature; }
  inline float getOutdoorTemperature(void){ return _outdoorTemperature; }
  inline void setSlBarometer(float slBarometer){ _slBarometer = slBarometer; }
  inline float getSlBarometer(void){ return _slBarometer; }

  ClassDef(e3VWSData,1)
};

struct sGlobalHeader {
  unsigned int GEO : 5;
  unsigned int EventCount : 22;
  unsigned int type : 5;
};
struct sGlobalTrailer {
  unsigned int GEO : 5;
  unsigned int WordCount : 16;
  unsigned int dummy : 3;
  unsigned int status : 3;
  unsigned int type : 5;
};
struct sTdcHeader {
  unsigned int BunchID : 12;
  unsigned int EventID : 12;
  unsigned int ID : 2;
  unsigned int dummy : 1;
  unsigned int type : 5;
};
struct sTdcChannel {
  unsigned int camera;
  unsigned int strip;
  unsigned int edge;
};
struct sTdcMeasure {
  unsigned int Edge;
  unsigned int Channel;
  double Time;
  unsigned int CameraID;
  unsigned int Strip;
  unsigned int Side;
  unsigned int Partner;
  int pw;
  int EventID;
  int Used;
  bool ForCalib;
};
struct sTdcTrailer {
  unsigned int WordCount : 12;
  unsigned int EventID : 12;
  unsigned int ID : 2;
  unsigned int dummy : 1;
  unsigned int type : 5;
};

class HPTDCEvent : public TObject {

 private:

  unsigned int _vLevel;
  unsigned int _id;

 public:

  HPTDCEvent(){};
  virtual ~HPTDCEvent(){};
  
  sTdcHeader _tdcHeader;
  sTdcMeasure _tdcMeasure;
  sTdcTrailer _tdcTrailer;

  vector<sTdcMeasure> _tdcMeasureVec;

  ClassDef(HPTDCEvent,1)
};

class V1190Event : public TObject {

 private:

  unsigned int _vLevel;
  unsigned int _id;
  unsigned int _dataType;
  unsigned int _rawETTT, _eTTT;

 public:

  sGlobalHeader _globalHeader;
  sGlobalTrailer _globalTrailer;

  vector<HPTDCEvent> _hptdcEventVec;

  V1190Event(){};
  virtual ~V1190Event(){};
  inline unsigned int getID(){ return _id; }
  inline void setID(unsigned id){ _id = id; }
  inline unsigned int getDT(){ return _dataType; }
  inline void setDT(unsigned dataType){ _dataType = dataType; }
  inline unsigned int getRawETTT(){ return _rawETTT; }
  inline unsigned int getETTT(){ return (_rawETTT<<0x5)|_globalTrailer.GEO; }
  inline void setRawETTT(unsigned rawETTT){ _rawETTT = rawETTT; }

  ClassDef(V1190Event,1)
};

class e3RawHit : public TObject {

 private:

  unsigned int _vLevel;
  unsigned int _mrpc;         //0-2 from bottom MRPC
  unsigned int _strip;        //0-23
  string _side;               //"left" or "right" respect to NINO cables exit direction
  float _leadTime, _trailTime;           

 public:

  e3RawHit(){};
  virtual ~e3RawHit(){}; 

  inline unsigned int getMRPC() const { return _mrpc; }
  inline void setMRPC(unsigned int mrpc){ _mrpc = mrpc; }
  inline unsigned int getStrip() const { return _strip; }
  inline void setStrip(unsigned int strip){ _strip = strip; }
  inline string getSide() const { return _side; }
  inline void setSide(string side){ _side = side; } //METTERE UNA PROTEZIONE (SOLO LEFT O RIGHT)
  inline float getLeadTime(){ return _leadTime; }
  inline void setLeadTime(float leadTime){ _leadTime = leadTime; }
  inline float getTrailTime(){ return _trailTime; }
  inline void setTrailTime(float trailTime){ _trailTime = trailTime; }

  ClassDef(e3RawHit,1)
};

class e3Hit : public TObject {

 private:

  unsigned int _vLevel;
  float _x,_y,_z;
  //METTERE IL TEMPO DELL'HIT???

 public:

  e3Hit(){};
  virtual ~e3Hit(){}; 

  inline float getXCoo() const { return _x; }
  inline void setXCoo(float x){ _x = x; }
  inline float getYCoo() const { return _y; }
  inline void setYCoo(float y){ _y = y; }
  inline float getZCoo() const { return _z; }
  inline void setZCoo(float z){ _z = z; }

  ClassDef(e3Hit,1)
};

class e3RawEvent : public TObject {

 private:

  unsigned int _vLevel;
  unsigned int _id;

 public:

  vector<V1190Event> _v1190EventVec;
  vector<e3RawHit> _rawHitVec;
  vector<e3Hit> _HitVec;

  e3RawEvent();
  virtual ~e3RawEvent();  

  inline unsigned int getID(){ return _id; }
  inline void setID(unsigned id){ _id = id; }
  int fillRawHitVec();
  int fillHitVec();
  void dump();

  ClassDef(e3RawEvent,1)
};

#endif
