#include <cstring>
#include <algorithm>

#include "e3Event.h"

using namespace std;

ClassImp(e3DataBlock)
ClassImp(e3GPSData)
ClassImp(e3SORData)
ClassImp(e3GEOData)
ClassImp(e3WADData)
ClassImp(e3VWSData)
ClassImp(HPTDCEvent)
ClassImp(e3RawHit)
ClassImp(e3RawEvent)

//---------------------------------------------------------
// e3GPSData creator
//---------------------------------------------------------
e3GPSData::e3GPSData(){

  init();

};

//---------------------------------------------------------
// e3GPSData init function
//---------------------------------------------------------
void e3GPSData::init(){

  for(unsigned int iTime=0; iTime<4; iTime++) _time[iTime]=0;

};

//---------------------------------------------------------
// e3GPSData dump function
//---------------------------------------------------------
void e3GPSData::dump(ofstream *logFile){

  cout<<"[e3GPSData::dump - INFO] e3GPSData dump BEGIN"<<endl;

  cout<<"EventID: "<<dec<<getEventID()<<" | ";
  cout<<"YY: "<<getYear()<<" | ";
  cout<<"DOY: "<<getDayOfYear()<<" | ";
  cout<<"sOD: "<<getSecondsOfDay()<<" | ";
  cout<<"ns: "<<getNanoSeconds()<<" | ";
  cout<<"NVS: "<<getNumOfVisSat()<<" | ";
  cout<<"LAT: "<<getLatitudeDeg()<<"° "<<getLatitudeMin()<<"',"<<getLatitudeMinFrac()<<" NSF "<<getNSFlag()<<" | ";
  cout<<"LON: "<<getLongitudeDeg()<<"° "<<getLongitudeMin()<<"',"<<getLongitudeMinFrac()<<" EWF "<<getEWFlag()<<" | ";
  cout<<"ALT: "<<getAltitude();

  cout<<endl<<"[e3GPSData::dump - INFO] e3GPSData dump END"<<endl;

};

//---------------------------------------------------------
// e3SORData creator
//---------------------------------------------------------
e3SORData::e3SORData(){

  init();

};

//---------------------------------------------------------
// e3SORData init function
//---------------------------------------------------------
void e3SORData::init(){

  _edgeDetectionMode = 0;
  _edgeTimeResolution = 0;
  _pulseWidthResolution = 0;
  _maxTime1 = 0;
  _maxTime2 = 0;

  _comments.clear();

};

//---------------------------------------------------------
// e3SORData dump function
//---------------------------------------------------------
void e3SORData::dump(ofstream *logFile){

  char detectionMode[][32]={"Pair mode","only trailing","only leading","leading and trailing"};
  char resolutionOfEdgeArrival[][16]={"100 ps","200 ps","400 ps","800 ps","1.6 ns","3.12 ns","6.25 ns","12.5 ns"};
  char resolutionOfPulseWidth[][16]={"100 ps","200 ps","400 ps","800 ps","1.6 ns","3.2 ns","6.25 ns","12.5 ns","25 ns","50 ns","100 ns","200 ns","400 ns","800 ns","Not Valid","Not valid"};

  cout<<"[e3SORData::dump - INFO] e3SORData dump BEGIN"<<endl;

  cout<<"DM: "<<detectionMode[getEdgeDetectionMode()]<<" ( 0x"<<hex<<setfill('0')<<setw(2)<<getEdgeDetectionMode()<<" ) | ";
  cout<<"ET Res: "<<resolutionOfEdgeArrival[getEdgeTimeResolution()]<<" ( 0x"<<hex<<setfill('0')<<setw(2)<<getEdgeTimeResolution()<<" ) | ";
  cout<<"PW Res: "<<resolutionOfPulseWidth[getPulseWidthResolution()]<<" ( 0x"<<hex<<setfill('0')<<setw(2)<<getPulseWidthResolution()<<") | ";
  cout<<"V1190A Max ETTT: "<<dec<<getMaxTime1()<<" | ";
  cout<<"V1190B Max ETTT: "<<dec<<getMaxTime2()<<" | ";
  cout<<"Comments: "<<getComments();

  cout<<endl<<"[e3SORData::dump - INFO] e3SORData dump END"<<endl;

};

//---------------------------------------------------------
// e3GEOData creator
//---------------------------------------------------------
e3GEOData::e3GEOData(){

  init();

};

//---------------------------------------------------------
// e3GEOData init function
//---------------------------------------------------------
void e3GEOData::init(){

  _angle = 0.;
  _grangle = 0.;
  _dist12 = 0.;
  _dist23 = 0.;

  for(unsigned int iMRPC=0; iMRPC<3; iMRPC++){
    for(unsigned int iSide=0; iSide<2; iSide++){
      _fecType[iMRPC][iSide] = 0;
      _cableLen[iMRPC][iSide] = 0.;
    }
  }

  _stationID=0;
  _stationReg.clear();


};

//---------------------------------------------------------
// e3GEOData dump function
//---------------------------------------------------------
void e3GEOData::dump(ofstream *logFile){

  char MRPCSide[][16]={"Left","Right"};

  cout<<fixed;
  cout<<"[e3GEOData::dump - INFO] e3GEOData dump BEGIN"<<endl;

  cout<<"Angle: "<<dec<<setprecision(1)<<getGrAngle()<<" deg. | ";
  cout<<"Dist12: "<<dec<<setprecision(2)<<getDist12()<<" m | ";
  cout<<"Dist23: "<<dec<<setprecision(2)<<getDist23()<<" m | ";

  for(unsigned int iMRPC=0; iMRPC<3; iMRPC++){
    for(unsigned int iSide=0; iSide<2; iSide++){
      cout<<"FECType MRPC"<<iMRPC+1<<MRPCSide[iSide]<<" : "<<_fecType[iMRPC][iSide]<<" | ";
    }
  }

  for(unsigned int iMRPC=0; iMRPC<3; iMRPC++){
    for(unsigned int iSide=0; iSide<2; iSide++){
      cout<<"CableLen MRPC"<<iMRPC+1<<MRPCSide[iSide]<<" : "<<setprecision(2)<<_cableLen[iMRPC][iSide]<<" m | ";
    }
  }

  cout<<"StationID "<<getStationIDComp();

  cout<<endl<<"[e3GEOData::dump - INFO] e3GEOData dump END"<<endl;

};

//---------------------------------------------------------
// e3WADData creator
//---------------------------------------------------------
e3WADData::e3WADData(){

  init();

};

//---------------------------------------------------------
// e3WADData init function
//---------------------------------------------------------
void e3WADData::init(){

  _windowWidth=0;
  _windowOffset=0;
  _searchMargin=0;
  _rejectMargin=0;

  _delayBetTrgAndCrst=0;
  _triggerSub=0;

};

//---------------------------------------------------------
// e3WADData dump function
//---------------------------------------------------------
void e3WADData::dump(ofstream *logFile){

  cout<<fixed; cout<<dec;
  cout<<"[e3WADData::dump - INFO] e3WADData dump BEGIN"<<endl;

  cout<<"WW: "<<getWindowWidth()<<" ns | ";
  cout<<"WO: "<<getWindowOffset()<<" ns | ";
  cout<<"SM: "<<getSearchMargin()<<" ns | ";
  cout<<"RM: "<<getRejectMargin()<<" ns | ";
  cout<<"DelTrgCrst: "<<getDelayBetTrgAndCrst()<<" ns | ";
  cout<<"TrgSub: "<<getTriggerSub();

  cout<<endl<<"[e3WADData::dump - INFO] e3WADData dump END"<<endl;

};

//---------------------------------------------------------
// e3VWSData creator
//---------------------------------------------------------
e3VWSData::e3VWSData(){

  init();

};

//---------------------------------------------------------
// e3VWSData init function
//---------------------------------------------------------
void e3VWSData::init(){

  _year=0;
  _month=0;
  _day=0;
  _hours=0;
  _minutes=0;

  _indoorTemperature=0.;
  _outdoorTemperature=0.;
  _slBarometer=0;

};

//---------------------------------------------------------
// e3VWSData dump function
//---------------------------------------------------------
void e3VWSData::dump(ofstream *logFile){

  cout<<fixed; cout<<dec;
  cout<<"[e3VWSData::dump - INFO] e3VWSData dump BEGIN"<<endl;

  cout<<"YY: "<<getYear()<<" | ";
  cout<<"MM: "<<getMonth()<<" | ";
  cout<<"DD: "<<getDay()<<" | ";
  cout<<"hh: "<<getHours()<<" | ";
  cout<<"mm: "<<getMinutes()<<" | ";
  cout<<"INTEMP: "<<getIndoorTemperature()<<" °C | ";
  cout<<"OUTTEMP: "<<getOutdoorTemperature()<<" °C | ";
  cout<<"SLBAR: "<<getSlBarometer()<<" mbar";

  cout<<endl<<"[e3VWSData::dump - INFO] e3VWSData dump END"<<endl;

};

//---------------------------------------------------------
// block getBinData method
//---------------------------------------------------------
int e3DataBlock::getBinData(char *buffer, unsigned int nBytes) {

  unsigned int nLeftBytes,nReadBytes;

  nLeftBytes = nBytes;

  while(nLeftBytes>0){

    _inFile->read(buffer, nLeftBytes);
    nReadBytes = _inFile->gcount();
    nLeftBytes -= nReadBytes;

    if(nLeftBytes>0) {
      buffer += nReadBytes;
      _inFile->clear();
    }

  }

  if(nLeftBytes!=0){
    cerr<<"[e3DataBlock::getBinData - ERROR] Read "<<nReadBytes<<" out of "<<nBytes<<" from the binary file."<<endl;
  }
  else{
    if(_vLevel>=5) cout<<dec<<"[e3DataBlock::getBinData - DEBUG] Read "<<nBytes - nLeftBytes<<" bytes."<<endl;
  }

  return (nBytes-nLeftBytes);
};

//---------------------------------------------------------
// block getBinData method
//---------------------------------------------------------
int e3DataBlock::getBlockData(char *&buffer, unsigned int nBytes, unsigned int offSet) {

  // buffer = new char(nBytes);
  buffer = _buffer+_offSet; 
  if(_vLevel>=5) cout<<dec<<"[e3DataBlock::getBlockData - DEBUG] Copied "<<nBytes<<" bytes from buffer"<<endl;
  
  return nBytes;

};

//---------------------------------------------------------
// block openBinFile method
//---------------------------------------------------------
int e3DataBlock::openBinFile(ifstream *inFile){

  int readBytes, byteSize;
  int curCharPos;
  
  _inFile = inFile;  
  if(_inFile->is_open()) {
    
    //------------------------------------------------------------------------------------------
    //Check if input data stream is just an EEE data file (first 8 byte - 4 words: _EEE Data)
    //------------------------------------------------------------------------------------------

    readBytes = getBinData((char *)&_blockSize, 4);
    if(readBytes!=4){
      cerr<<"[e3DataBlock::openBinFile - ERROR] Read "<<readBytes<<" out of 4 from the binary file."<<endl;
      return 2;
    }
    
    readBytes = getBinData((char *)&_dataType, 4);
    if(readBytes!=4){
      cerr<<"[e3DataBlock::openBinFile - ERROR] Read "<<readBytes<<" out of 4 from the binary file."<<endl;
      return 2;
    }
    
    if(_blockSize!=1162167584 || _dataType!=1147237473){
      cerr<<"[e3DataBlock::openBinFile - ERROR] No valid EEE data in the binary file."<<endl;
      return 1;
    }
    else{
      if(_vLevel>=4){
	stringstream e3DataString;
	e3DataString<<(char) ((_blockSize>>0x18)&0xff)<<(char) ((_blockSize>>0x10)&0xff);
	e3DataString<<(char) ((_blockSize>>0x8)&0xff)<<(char) (_blockSize&0xff);
	e3DataString<<(char) ((_dataType>>0x18)&0xff)<<(char) ((_dataType>>0x10)&0xff);
	e3DataString<<(char) ((_dataType>>0x8)&0xff)<<(char) (_dataType&0xff);
	cout<<"[e3DataBlock::openBinFile - INFO] Valid EEE data found. Read EEE string: "<<e3DataString.str()<<endl;
      }
    }

    //------------------------------------------------------------------------------------------
    // Read up to the first data block (block size is expressed in 32 bits words)
    // When the first Data block has been found the binary data file content is loaded in memory
    //------------------------------------------------------------------------------------------
          
    curCharPos = _inFile->tellg();
    _inFile->seekg(0, _inFile->end);
    byteSize = (int)_inFile->tellg()-curCharPos;
    _inFile->seekg (curCharPos);
      
    //Check on block size
    if(byteSize > BUFSIZE) {
      cerr<<"[e3DataBlock::openBinFile - ERROR] Data block size too big: "<<byteSize<<" bytes."<<endl;
      return 3;
    }
    else{
      readBytes = getBinData((char *)&_buffer, byteSize);	
      if(_vLevel>=4) cout<<"[e3DataBlock::openBinFile - INFO] Binary data content loaded in memory. Size: "<<byteSize<<" bytes."<<endl;
      _offSet = 0;
    }
    
    _inFile->seekg (curCharPos);
    readBytes = getBinData((char *)&_blockSize, 4);
    if(readBytes!=4){
      cerr<<"[e3DataBlock::openBinFile - ERROR] Read "<<readBytes<<" out of 4 from the binary file."<<endl;
      return 2;
    }
    
    readBytes = getBinData((char *)&_dataType, 4);
    if(readBytes!=4){
      cerr<<"[e3DataBlock::openBinFile - ERROR] Read "<<readBytes<<" out of 4 from the binary file."<<endl;
      return 2;
    }
    if(_dataType!=0x3){
      cerr<<"[e3DataBlock::openBinFile - ERROR] First block type is: "<<_dataType<<endl;
      return 4;
    }
            
    //Check on block size
    if((_blockSize-2)*4 > BUFSIZE) {
      cerr<<"[e3DataBlock::openBinFile - ERROR] Data block size too big: "<<(_blockSize-2)*4<<" bytes."<<endl;
      return 3;
    }

    if(_vLevel>=4) cout<<"[e3DataBlock::openBinFile - INFO] First BLOCK found."<<endl;
    // readBytes = getBinData((char *)&_buffer, (_blockSize-2)*4);
    // if(_vLevel>=10) dump((char *)&_buffer, (_blockSize-2)*4, _dataType);


    //------------------------------------------------------------------------------------------

  }
  else {
    cerr<<"[e3DataBlock::openBinFile - ERROR] Data file is not open."<<endl;
    return 1;
  }

  return 0;

};

//---------------------------------------------------------
// block dump method
//---------------------------------------------------------
void e3DataBlock::dump(char *buffer, unsigned int nBytes, unsigned int dataType){

  cout<<"[e3DataBlock::dump] Block size: "<<dec<<nBytes<<" bytes - Data type: 0x"<<hex<<dataType;

  for(unsigned int iByte=0; iByte<nBytes; iByte++){
    if((iByte%8)==0 && iByte!=0) cout<<" ";
    if((iByte%16)==0){
      cout<<endl;
      cout.width(8);
      cout.fill('0');
      cout<<hex<<iByte<<"  ";
    }
    
    cout.width(2);
    cout.fill('0');
    cout<<hex<<(buffer[iByte]&0xff)<<" ";
    
  }
  cout<<dec<<endl;
};

//---------------------------------------------------------
// e3DataBlock getGPSData method
//---------------------------------------------------------
int e3DataBlock::getGPSData(e3GPSData *gpsData){

  if(_vLevel>=5) dump(_sngBlock, (_blockSize-2)*4, _dataType);

  gpsData->setEventID(((_sngBlock[0*4+3]&0xff)<<24)|((_sngBlock[0*4+2]&0xff)<<16)|((_sngBlock[0*4+1]&0xff)<<8)|(_sngBlock[0*4+0]&0xff));
  gpsData->setTime(1,((_sngBlock[1*4+1]&0xff)<<8)|(_sngBlock[1*4+0]&0xff));
  gpsData->setTime(2,((_sngBlock[2*4+1]&0xff)<<8)|(_sngBlock[2*4+0]&0xff));
  gpsData->setTime(3,((_sngBlock[3*4+1]&0xff)<<8)|(_sngBlock[3*4+0]&0xff));
  gpsData->setTime(4,((_sngBlock[4*4+1]&0xff)<<8)|(_sngBlock[4*4+0]&0xff));

  gpsData->setStatus(1,((_sngBlock[5*4+1]&0xff)<<8)|(_sngBlock[5*4+0]&0xff));
  gpsData->setStatus(2,((_sngBlock[6*4+1]&0xff)<<8)|(_sngBlock[6*4+0]&0xff));
  gpsData->setPRN(1,((_sngBlock[7*4+1]&0xff)<<8)|(_sngBlock[7*4+0]&0xff));
  gpsData->setPRN(2,((_sngBlock[8*4+1]&0xff)<<8)|(_sngBlock[8*4+0]&0xff));

  gpsData->setPosition(1,((_sngBlock[9*4+1]&0xff)<<8)|(_sngBlock[9*4+0]&0xff));
  gpsData->setPosition(2,((_sngBlock[10*4+1]&0xff)<<8)|(_sngBlock[10*4+0]&0xff));
  gpsData->setPosition(3,((_sngBlock[11*4+1]&0xff)<<8)|(_sngBlock[11*4+0]&0xff));
  gpsData->setPosition(4,((_sngBlock[12*4+1]&0xff)<<8)|(_sngBlock[12*4+0]&0xff));

  return 0;

};

//---------------------------------------------------------
// e3DataBlock getSORData method
//---------------------------------------------------------
int e3DataBlock::getSORData(){

  if(_vLevel>=5) dump(_sngBlock, (_blockSize-2)*4, _dataType);

  sorData.setEdgeDetectionMode((int) _sngBlock[3]|_sngBlock[2]|_sngBlock[1]|_sngBlock[0]); 
  sorData.setEdgeTimeResolution((int) _sngBlock[7]|_sngBlock[6]|_sngBlock[5]|_sngBlock[4]); 
  sorData.setPulseWidthResolution((int) _sngBlock[11]|_sngBlock[10]|_sngBlock[9]|_sngBlock[8]); 
  sorData.setMaxTime1((int) _sngBlock[15]|_sngBlock[14]|_sngBlock[13]|_sngBlock[12]); 
  sorData.setMaxTime2((int) _sngBlock[19]|_sngBlock[18]|_sngBlock[17]|_sngBlock[16]); 

  for(unsigned int iword=5; iword<_blockSize-5; iword++){
    sorData.setComments(_sngBlock[iword*4+3]);
    sorData.setComments(_sngBlock[iword*4+2]);
    sorData.setComments(_sngBlock[iword*4+1]);
    sorData.setComments(_sngBlock[iword*4+0]);
  }

  return 0;

};

//---------------------------------------------------------
// e3DataBlock getGEOData method
//---------------------------------------------------------
int e3DataBlock::getGEOData(){

  if(_vLevel>=5) dump(_sngBlock, (_blockSize-2)*4, _dataType);

  geoData.setGrAngle((double) (((_sngBlock[3]&0xff)<<24)|((_sngBlock[2]&0xff)<<16)|((_sngBlock[1]&0xff)<<8)|(_sngBlock[0]&0xff))/100.);
  geoData.setDist12((double) (((_sngBlock[7]&0xff)<<24)|((_sngBlock[6]&0xff)<<16)|((_sngBlock[5]&0xff)<<8)|(_sngBlock[4]&0xff))/100.);
  geoData.setDist23((double) (((_sngBlock[11]&0xff)<<24)|((_sngBlock[10]&0xff)<<16)|((_sngBlock[9]&0xff)<<8)|(_sngBlock[8]&0xff))/100.);

  for(unsigned int iMRPC=0; iMRPC<3; iMRPC++){
    for(unsigned int iSide=0; iSide<2; iSide++){

      geoData.setCableLen(iMRPC,iSide,(double) (((_sngBlock[12+3+iMRPC*4+iSide*4]&0xff)<<24)|((_sngBlock[12+2+iMRPC*4+iSide*4]&0xff)<<16)|((_sngBlock[12+1+iMRPC*4+iSide*4]&0xff)<<8)|(_sngBlock[12+0+iMRPC*4+iSide*4]&0xff))/100.);
      geoData.setFecType(iMRPC,iSide,(double) (((_sngBlock[36+3+iMRPC*4+iSide*4]&0xff)<<24)|((_sngBlock[36+2+iMRPC*4+iSide*4]&0xff)<<16)|((_sngBlock[36+1+iMRPC*4+iSide*4]&0xff)<<8)|(_sngBlock[36+0+iMRPC*4+iSide*4]&0xff)));

    }
  }

  //STATION ID
  geoData.setStationID(((_sngBlock[15*4+3]&0xff)<<24)|((_sngBlock[15*4+2]&0xff)<<16)|((_sngBlock[15*4+1]&0xff)<<8)|(_sngBlock[15*4+0]&0xff));

  //STATION REGION
  geoData.setStationReg(_sngBlock[16*4+3]);
  geoData.setStationReg(_sngBlock[16*4+2]);
  geoData.setStationReg(_sngBlock[16*4+1]);
  geoData.setStationReg(_sngBlock[16*4+0]);

  return 0;

};

//---------------------------------------------------------
// e3DataBlock getWADData method
//---------------------------------------------------------
int e3DataBlock::getWADData(){

  if(_vLevel>=5) dump(_sngBlock, (_blockSize-2)*4, _dataType);

  wadData.setWindowWidth(((_sngBlock[0*4+3]&0xff)<<24)|((_sngBlock[0*4+2]&0xff)<<16)|((_sngBlock[0*4+1]&0xff)<<8)|(_sngBlock[0*4+0]&0xff));
  wadData.setWindowOffset(((_sngBlock[1*4+3]&0xff)<<24)|((_sngBlock[1*4+2]&0xff)<<16)|((_sngBlock[1*4+1]&0xff)<<8)|(_sngBlock[1*4+0]&0xff));
  wadData.setSearchMargin(((_sngBlock[2*4+3]&0xff)<<24)|((_sngBlock[2*4+2]&0xff)<<26)|((_sngBlock[2*4+1]&0xff)<<8)|(_sngBlock[2*4+0]&0xff));
  wadData.setRejectMargin(((_sngBlock[3*4+3]&0xff)<<24)|((_sngBlock[3*4+2]&0xff)<<26)|((_sngBlock[3*4+1]&0xff)<<8)|(_sngBlock[3*4+0]&0xff));

  wadData.setDelayBetTrgAndCrst(((_sngBlock[4*4+3]&0xff)<<24)|((_sngBlock[4*4+2]&0xff)<<26)|((_sngBlock[4*4+1]&0xff)<<8)|(_sngBlock[4*4+0]&0xff));
  wadData.setTriggerSub(((_sngBlock[5*4+3]&0xff)<<24)|((_sngBlock[5*4+2]&0xff)<<26)|((_sngBlock[5*4+1]&0xff)<<8)|(_sngBlock[5*4+0]&0xff));

  return 0;

};

//---------------------------------------------------------
// e3DataBlock getVWSData method
//---------------------------------------------------------
int e3DataBlock::getVWSData(){

  if(_vLevel>=5) dump(_sngBlock, (_blockSize-2)*4, _dataType);

  vwsData.setYear(((_sngBlock[0*4+3]&0xff)<<24)|((_sngBlock[0*4+2]&0xff)<<16)|((_sngBlock[0*4+1]&0xff)<<8)|(_sngBlock[0*4+0]&0xff));
  vwsData.setMonth(((_sngBlock[1*4+3]&0xff)<<24)|((_sngBlock[1*4+2]&0xff)<<16)|((_sngBlock[1*4+1]&0xff)<<8)|(_sngBlock[1*4+0]&0xff));
  vwsData.setDay(((_sngBlock[2*4+3]&0xff)<<24)|((_sngBlock[2*4+2]&0xff)<<26)|((_sngBlock[2*4+1]&0xff)<<8)|(_sngBlock[2*4+0]&0xff));
  vwsData.setHours(((_sngBlock[3*4+3]&0xff)<<24)|((_sngBlock[3*4+2]&0xff)<<26)|((_sngBlock[3*4+1]&0xff)<<8)|(_sngBlock[3*4+0]&0xff));
  vwsData.setMinutes(((_sngBlock[4*4+3]&0xff)<<24)|((_sngBlock[4*4+2]&0xff)<<26)|((_sngBlock[4*4+1]&0xff)<<8)|(_sngBlock[4*4+0]&0xff));

  vwsData.setIndoorTemperature((float) (((_sngBlock[5*4+3]&0xff)<<24)|((_sngBlock[5*4+2]&0xff)<<26)|((_sngBlock[5*4+1]&0xff)<<8)|(_sngBlock[5*4+0]&0xff))/100.);
  vwsData.setOutdoorTemperature((float) (((_sngBlock[6*4+3]&0xff)<<24)|((_sngBlock[6*4+2]&0xff)<<26)|((_sngBlock[6*4+1]&0xff)<<8)|(_sngBlock[6*4+0]&0xff))/100.);
  vwsData.setSlBarometer(((_sngBlock[7*4+3]&0xff)<<24)|((_sngBlock[7*4+2]&0xff)<<26)|((_sngBlock[7*4+1]&0xff)<<8)|(_sngBlock[7*4+0]&0xff));

  return 0;

};

//---------------------------------------------------------
// e3DataBlock getNextBlock method
//---------------------------------------------------------
int e3DataBlock::getNextBlock(){

  unsigned int readBytes, readBlocks; //blockSize, dataType;
  unsigned int anBytes;

  e3GPSData _gpsData;
    
  if(_vLevel>=5) cout<<"[e3DataBlock::getNextBlock - DEBUG] Buffer offset: "<<dec<<_offSet<<endl;

  //------------------------------------------------------------------------------------------
  // Get Next Block data
  //------------------------------------------------------------------------------------------
  readBlocks = 0;
  while(readBlocks<1){

    _blockSize = (int) ((_buffer[3+_offSet]&0xff)<<24)|((_buffer[2+_offSet]&0xff)<<16)|((_buffer[1+_offSet]&0xff)<<8)|(_buffer[0+_offSet]&0xff);
    _dataType = (int) ((_buffer[7+_offSet]&0xff)<<24)|((_buffer[6+_offSet]&0xff)<<16)|((_buffer[5+_offSet]&0xff)<<8)|(_buffer[4+_offSet]&0xff);
    _offSet+= 8;

    //Check on block size
    if(_blockSize==0){
      if(_vLevel>0) cout<<"[e3DataBlock::getNextBlock - WARNING] Data block size: 0 bytes."<<endl;
      return 1;
    }

    if((_blockSize-2)*4 > BUFSIZE) {
      cerr<<"[e3DataBlock::getNextBlock - ERROR] Data block size too big: "<<(_blockSize-2)*4<<" bytes."<<endl;
      return 2;
    }

    //Check on data type
    if(_dataType<0x0 || _dataType>0x7){
      cerr<<"[e3DataBlock::getNextBlock - ERROR]  Found anomalous DT: 0x"<<hex<<_dataType<<dec<<endl;
      return 3;
    }

    readBytes = getBlockData(_sngBlock, (_blockSize-2)*4, _offSet);
    if(_vLevel>=4) cout<<"[e3DataBlock::getNextBlock - INFO] Next BLOCK read."<<endl;    //mettere protezione su readBytes?
    if(_vLevel>=10) dump(_sngBlock, (_blockSize-2)*4, _dataType);
    _offSet+= readBytes;
    
    readBlocks++;

    //OK, now we examine the data type
    switch(_dataType) {
      
    case 0x0:            //GPS data
      
      if(_dtFilterStr.find("GPS")!=string::npos){

	if(_vLevel>=4) cout<<"[e3DataBlock::getNextBlock - INFO] New \"GPS\" BLOCK found."<<endl;
	
	_gpsData.init();
	getGPSData(&_gpsData);
	if(!_specialDump) _gpsData.dump();
	//	else if(_gpsData.getSecondsOfDay()-(int)(_gpsData.getSecondsOfDay()/60)*60==0) {       
	else{

	  //Dump only one GPS block per minute
	  cout<<fixed;
	  cout<<_inFileName<<",";
	  cout.precision(6); cout<<_gpsData.getLatitudeDeg()+(float)((_gpsData.getLatitudeMin()+_gpsData.getLatitudeMinFrac()/1000.)/60.)<<",";
	  cout.precision(6); cout<<_gpsData.getLongitudeDeg()+(float)((_gpsData.getLongitudeMin()+_gpsData.getLongitudeMinFrac()/1000.)/60.)<<",";
	  cout.precision(0); cout<<_gpsData.getAltitude()<<",";
	  cout.fill('0'); cout.width(3); cout.precision(2); cout<<geoData.getDist12()<<",";
	  cout.fill('0'); cout.width(3); cout.precision(2); cout<<geoData.getDist23()<<",";
	  cout.fill('0'); cout.width(4); cout.precision(1); cout<<geoData.getGrAngle()<<",";
	  time_t loctime;
	  struct tm timeinfo, *loctimeinfo;
	  /* initialize timeinfo and modify it to the user's choice */
	  bzero(&timeinfo, sizeof(struct tm));
	  timeinfo.tm_isdst = -1;  /* Allow mktime to determine DST setting. */
	  timeinfo.tm_mon   = 0;
	  timeinfo.tm_mday = _gpsData.getDayOfYear();
	  timeinfo.tm_year = _gpsData.getYear() - 1900;

	  loctime = mktime (&timeinfo);
	  loctimeinfo = localtime(&loctime);
	  cout<<loctimeinfo->tm_year+1900<<"-"; 
	  cout.fill('0'); cout.width(2); cout<<loctimeinfo->tm_mon+1<<"-";
	  cout.fill('0'); cout.width(2); cout<<loctimeinfo->tm_mday<<",";
	  cout.fill('0'); cout.width(2); cout<<(int)(_gpsData.getSecondsOfDay()/3600)<<":";
	  cout.fill('0'); cout.width(2); cout<<(int)(_gpsData.getSecondsOfDay()/60)-(int)((_gpsData.getSecondsOfDay()/3600)*60)<<":";
	  if(_gpsData.getNanoSeconds()>5E8){ cout.fill('0'); cout.width(2); cout<<_gpsData.getSecondsOfDay()-(int)(_gpsData.getSecondsOfDay()/60)*60+1; }
	  else{ cout.fill('0'); cout.width(2); cout<<_gpsData.getSecondsOfDay()-(int)(_gpsData.getSecondsOfDay()/60)*60; }

	  char *telID = (char*) geoData.getStationIDComp().c_str();
	  for(int ichar=0; telID[ichar]; ichar++)
	    telID[ichar]=toupper(telID[ichar]);
	  cout<<","<<telID;
	  cout<<endl;
	}
	
      }

      break;

    case 0x1:
    case 0x2:

      if(_vLevel>=4) cout<<"[e3DataBlock::getNextBlock - INFO] New \"TDC\" BLOCK found."<<endl;

      break;
      
    case 0x3:            //Start of run data
      
      if(_dtFilterStr.find("SOR")!=string::npos){

	if(_vLevel>=4) cout<<"[e3DataBlock::getNextBlock - INFO] New \"Start of Run\" BLOCK found."<<endl;
	
	sorData.init();
	getSORData();
	if(!_specialDump) sorData.dump();

      }      

      break;
      
    case 0x5:            //Geometry data

      if(_dtFilterStr.find("GEO")!=string::npos){

	if(_vLevel>=4) cout<<"[e3DataBlock::getNextBlock - INFO] New \"Geometry\" BLOCK found."<<endl;
      
	geoData.init();
	getGEOData();
	if(!_specialDump) geoData.dump();

      }
      
      break;
      
    case 0x6:            //Window and delay data
      
      if(_dtFilterStr.find("WAD")!=string::npos){

	if(_vLevel>=4) cout<<"[e3DataBlock::getNextBlock - INFO] New \"Window and Delay\" BLOCK found."<<endl;
	
	wadData.init();
	getWADData();
	if(!_specialDump) wadData.dump();

      }

      break;
      
    case 0x7:            //Wheater station data

      if(_dtFilterStr.find("VWS")!=string::npos){

	if(_vLevel>=2) cout<<"[e3DataBlock::getNextBlock - INFO] New \"Weather Station\" BLOCK found."<<endl;
	
	vwsData.init();
	getVWSData();
	if(!_specialDump) vwsData.dump();

      }

      break;
      
    default:
      break;
      
    }

  }

  return 0;

};

//---------------------------------------------------------
// e3DataBlock getNextEvent method
//---------------------------------------------------------
int e3DataBlock::getNextEvent(e3RawEvent *rawEvent){

  unsigned int readBytes, readBlocks, blockSize, dataType;
  unsigned int anBytes;
  HPTDCEvent *_HPTDCEvent = NULL;
  V1190Event *_V1190Event = NULL;
    
  if(_vLevel>=5) cout<<"[e3DataBlock::getNextEvent - DEBUG] Buffer offset: "<<dec<<_offSet<<endl;

  //------------------------------------------------------------------------------------------
  // Get Next Event block data
  //------------------------------------------------------------------------------------------
  readBlocks = 0;
  while(readBlocks<2){

    blockSize = (int) _buffer[3+_offSet]|_buffer[2+_offSet]|_buffer[1+_offSet]|_buffer[0+_offSet];
    dataType = (int) _buffer[7+_offSet]|_buffer[6+_offSet]|_buffer[5+_offSet]|_buffer[4+_offSet];
    _offSet+= 8;
    
    readBytes = getBlockData(_evntBlock, (blockSize-2)*4, _offSet);
    if(_vLevel>1) dump(_evntBlock, (blockSize-2)*4, dataType);
    _offSet+= readBytes;
    
    readBlocks+=1;
    //------------------------------------------------------------------------------------------
    // Store Data from event block to e3GPSEvents
    //------------------------------------------------------------------------------------------
    if(dataType==0x0){
      break;
    }

    //------------------------------------------------------------------------------------------
    // Store Data from event block to e3RawEvents
    //------------------------------------------------------------------------------------------
    if(dataType==0x1 || dataType==0x2){
      
      _V1190Event = new V1190Event();
      _V1190Event->setDT(dataType);

      anBytes = 0;      
      while(anBytes<((blockSize-2)*4)){
	
	memcpy(&_data, _evntBlock+anBytes, sizeof(sData));
	
	switch(_data.type) {
	case 0x08:      //Global Header Event Count packet type   START OF A NEW EVENT
	  
	  memcpy(&_V1190Event->_globalHeader,&_data,sizeof(sGlobalHeader));
	  _V1190Event->setID(_V1190Event->_globalHeader.EventCount);
	  cout<<"Event ID: "<<dec<<_V1190Event->_globalHeader.EventCount<<endl;
	  break;
	  
	case 0x01:      //TDC Header packet type
	  //INSERIRE CONTROLLO SU EVENT ID!

	  _HPTDCEvent = new HPTDCEvent();
	  memcpy(&_HPTDCEvent->_tdcHeader,&_data,sizeof(sTdcHeader));
	  cout<<"Event ID: "<<dec<<_HPTDCEvent->_tdcHeader.EventID<<endl;
	  cout<<"Bunch ID: "<<dec<<_HPTDCEvent->_tdcHeader.BunchID<<endl;
	  cout<<"HPTDC ID: "<<dec<<_HPTDCEvent->_tdcHeader.ID<<endl;
	  break;
	  
	case 0x00:      //TDC Measure
	  _HPTDCEvent->_tdcMeasure.Edge     = ((_data.dummy & 0x04000000) >> 26);                     //0=leading, 1=trailing
	  _HPTDCEvent->_tdcMeasure.Channel  = ((_data.dummy & 0x03F80000) >> 19) + 128*(dataType-1);  //TDC channel
	  _HPTDCEvent->_tdcMeasure.Time     = ((_data.dummy & 0x0007FFFF));                           //Time unit: 1/10 ns
	  // _HPTDCEvent->_tdcMeasure.CameraID = channel[_HPTDCEvent->_tdcMeasure.Channel].camera;
	  // _HPTDCEvent->_tdcMeasure.Strip    = channel[_HPTDCEvent->_tdcMeasure.Channel].strip;
	  // _HPTDCEvent->_tdcMeasure.Side     = channel[_HPTDCEvent->_tdcMeasure.Channel].side;
	  // _HPTDCEvent->_tdcMeasure.Partner  = channel[_HPTDCEvent->_tdcMeasure.Channel].partner;
	  // _HPTDCEvent->_tdcMeasure.EventID  = EventID;
	  _HPTDCEvent->_tdcMeasure.pw=0;
	  _HPTDCEvent->_tdcMeasure.Used=0;
	  _HPTDCEvent->_tdcMeasure.ForCalib=true;
	  
	  cout<<"Find new measurement "<<dec<<_HPTDCEvent->_tdcMeasure.Channel<<endl;
	  _HPTDCEvent->_tdcMeasureVec.push_back(_HPTDCEvent->_tdcMeasure);
	  break;
	  
	case 0x03:      //TDC Trailer packet type
	  memcpy(&_HPTDCEvent->_tdcTrailer,&_data,sizeof(sTdcTrailer));
	  _V1190Event->_hptdcEventVec.push_back(*_HPTDCEvent);
	  delete _HPTDCEvent;
	  cout<<"Word Count: "<<dec<<_HPTDCEvent->_tdcTrailer.WordCount<<endl;
	  break;
	  
	case 0x11:      //Raw Extended Trigger Time Tag (data.type = 0x11 according to V1190 manual)
	  _V1190Event->setRawETTT(_data.dummy);
	  cout<<"Raw Extended Trigger Time Tag: "<<dec<<_data.dummy<<endl;
	  cout<<"Raw Extended Trigger Time Tag data type: 0x"<<hex<<setw(2)<<setfill('0')<<_data.type<<endl;
	  break;
	  
	case 0x10:        //Global Trailer
	  memcpy(&_V1190Event->_globalTrailer, &_data, sizeof(sGlobalTrailer));
	  cout<<"Global trailer->  Status: 0x"<<hex<<_V1190Event->_globalTrailer.status<<" (GEO: 0x"<<hex<<_V1190Event->_globalTrailer.GEO<<" & 0x1f = 0x"<<(_V1190Event->_globalTrailer.GEO&0x1F)<<")"<<endl;
	  break;
	  
	case 0x04:        //TDC Error
	  if(_vLevel>0) cout<<"[e3DataBlock::getNextEvent - WARNING] Found error for TDC "<<dataType<<"!!!"<<endl;
	  break;
	  
	default:
	  if(_vLevel>0) cout<<"[e3DataBlock::getNextEvent - WARNING] Unknown type: 0x"<<hex<<_data.type<<endl;
	  break;
	}
	
  	anBytes += sizeof(sData);
      }

      rawEvent->_v1190EventVec.push_back(*_V1190Event);      
    }

  }

  //METTERE DEI CHECK QUI!!!!
  rawEvent->fillRawHitVec();
  rawEvent->fillHitVec();

  return 0;

};

//---------------------------------------------------------
// e3Event default constructor
//---------------------------------------------------------
e3RawEvent::e3RawEvent(){

  // Create a raw Event object.
  _id = 0;

};

//---------------------------------------------------------
// e3Event destructor
//---------------------------------------------------------
e3RawEvent::~e3RawEvent(){

};

//---------------------------------------------------------
// e3RawHit comparison operator
//---------------------------------------------------------
struct rawHitHasSame
{
  unsigned int _mrpc, _strip;
  string _side;

  rawHitHasSame(unsigned int mrpc, unsigned int strip, string side) : _mrpc(mrpc), _strip(strip), _side(side) {}
    bool operator () ( const e3RawHit& _rawHit ) const
    {
      if(_rawHit.getMRPC()==_mrpc && _rawHit.getStrip()==_strip && _rawHit.getSide()==_side)
      return kTRUE;
      else return kFALSE;
    }
};

//---------------------------------------------------------
// e3Event fillRawHitVec method
//---------------------------------------------------------
int e3RawEvent::fillRawHitVec(){

  e3RawHit *_rawHit = NULL;

  for(vector<V1190Event>::iterator v1190EventIt = _v1190EventVec.begin(); v1190EventIt != _v1190EventVec.end(); ++v1190EventIt) {

    for(vector<HPTDCEvent>::iterator HPTDCEventIt = v1190EventIt->_hptdcEventVec.begin(); HPTDCEventIt != v1190EventIt->_hptdcEventVec.end(); ++HPTDCEventIt) {

      for(vector<sTdcMeasure>::iterator tdcMeasureIt = HPTDCEventIt->_tdcMeasureVec.begin(); tdcMeasureIt != HPTDCEventIt->_tdcMeasureVec.end(); ++tdcMeasureIt) {

	if(tdcMeasureIt->Edge!=0) continue; //Push back leading times only

	_rawHit = new e3RawHit();
	if(tdcMeasureIt->Channel>=0 && tdcMeasureIt->Channel<(24)){    //MRPC 0 Right Hit
	  _rawHit->setMRPC(0);
	  _rawHit->setStrip(tdcMeasureIt->Channel-32*((int)tdcMeasureIt->Channel/32));
	  _rawHit->setSide("right");
	}
	else if(tdcMeasureIt->Channel>=32 && tdcMeasureIt->Channel<(32+24)){    //MRPC 0 Left Hit
	  _rawHit->setMRPC(0);
	  _rawHit->setStrip(tdcMeasureIt->Channel-32*((int)tdcMeasureIt->Channel/32));
	  _rawHit->setSide("left");
	}

	else if(tdcMeasureIt->Channel>=(32*2) && tdcMeasureIt->Channel<(32*2+24)){    //MRPC 2 Right Hit
	  _rawHit->setMRPC(2);
	  _rawHit->setStrip(tdcMeasureIt->Channel-32*((int)tdcMeasureIt->Channel/32));
	  _rawHit->setSide("right");
	}
	else if(tdcMeasureIt->Channel>=(32*3) && tdcMeasureIt->Channel<(32*3+24)){    //MRPC 2 Left Hit
	  _rawHit->setMRPC(2);
	  _rawHit->setStrip(tdcMeasureIt->Channel-32*((int)tdcMeasureIt->Channel/32));
	  _rawHit->setSide("left");
	}

	else if(tdcMeasureIt->Channel>=(32*4) && tdcMeasureIt->Channel<(32*4+24)){    //MRPC 1 Right Hit
	  _rawHit->setMRPC(1);
	  _rawHit->setStrip(tdcMeasureIt->Channel-32*((int)tdcMeasureIt->Channel/32));
	  _rawHit->setSide("right");
	}
	else if(tdcMeasureIt->Channel>=(32*5) && tdcMeasureIt->Channel<(32*5+24)){    //MRPC 1 Left Hit
	  _rawHit->setMRPC(1);
	  _rawHit->setStrip(tdcMeasureIt->Channel-32*((int)tdcMeasureIt->Channel/32));
	  _rawHit->setSide("left");
	}
	else{
	  cerr<<"[e3RawEvent::fillRawHitVec - ERROR] Found BAD Channel number: "<<tdcMeasureIt->Channel<<endl;
	  return -1;
	}

	_rawHit->setLeadTime(tdcMeasureIt->Time/10.);
	_rawHitVec.push_back(*_rawHit);
	delete _rawHit;
	
      }

      for(vector<sTdcMeasure>::iterator tdcMeasureIt = HPTDCEventIt->_tdcMeasureVec.begin(); tdcMeasureIt != HPTDCEventIt->_tdcMeasureVec.end(); ++tdcMeasureIt) {

	if(tdcMeasureIt->Edge!=1) continue; //Push back trailing times only

	_rawHit = new e3RawHit();
	if(tdcMeasureIt->Channel>=0 && tdcMeasureIt->Channel<(24)){    //MRPC 0 Right Hit
	  _rawHit->setMRPC(0);
	  _rawHit->setStrip(tdcMeasureIt->Channel-32*((int)tdcMeasureIt->Channel/32));
	  _rawHit->setSide("right");
	}
	else if(tdcMeasureIt->Channel>=32 && tdcMeasureIt->Channel<(32+24)){    //MRPC 0 Left Hit
	  _rawHit->setMRPC(0);
	  _rawHit->setStrip(tdcMeasureIt->Channel-32*((int)tdcMeasureIt->Channel/32));
	  _rawHit->setSide("left");
	}

	else if(tdcMeasureIt->Channel>=(32*2) && tdcMeasureIt->Channel<(32*2+24)){    //MRPC 2 Right Hit
	  _rawHit->setMRPC(2);
	  _rawHit->setStrip(tdcMeasureIt->Channel-32*((int)tdcMeasureIt->Channel/32));
	  _rawHit->setSide("right");
	}
	else if(tdcMeasureIt->Channel>=(32*3) && tdcMeasureIt->Channel<(32*3+24)){    //MRPC 2 Left Hit
	  _rawHit->setMRPC(2);
	  _rawHit->setStrip(tdcMeasureIt->Channel-32*((int)tdcMeasureIt->Channel/32));
	  _rawHit->setSide("left");
	}

	else if(tdcMeasureIt->Channel>=(32*4) && tdcMeasureIt->Channel<(32*4+24)){    //MRPC 1 Right Hit
	  _rawHit->setMRPC(1);
	  _rawHit->setStrip(tdcMeasureIt->Channel-32*((int)tdcMeasureIt->Channel/32));
	  _rawHit->setSide("right");
	}
	else if(tdcMeasureIt->Channel>=(32*5) && tdcMeasureIt->Channel<(32*5+24)){    //MRPC 1 Left Hit
	  _rawHit->setMRPC(1);
	  _rawHit->setStrip(tdcMeasureIt->Channel-32*((int)tdcMeasureIt->Channel/32));
	  _rawHit->setSide("left");
	}
	else{
	  cerr<<"[e3RawEvent::fillRawHitVec - ERROR] Found BAD Channel number: "<<tdcMeasureIt->Channel<<endl;
	  return -1;
	}

	vector<e3RawHit>::iterator _rawHitIt = find_if(_rawHitVec.begin(),_rawHitVec.end(), 
						       rawHitHasSame(_rawHit->getMRPC(),_rawHit->getStrip(),_rawHit->getSide()));
	if(_rawHitIt!=_rawHitVec.end()) (*_rawHitIt).setTrailTime(tdcMeasureIt->Time/10.);
	else{
	  cerr<<"[e3RawEvent::fillRawHitVec - ERROR] Missing corresponding Leading Time for raw hit: ";
	  cerr<<" MRPC "<<_rawHit->getMRPC()<<" strip "<<_rawHit->getStrip()<<" side "<<_rawHit->getSide()<<endl;
	  delete _rawHit;
	  return -2;
	}

      }

    }

  }

  // delete _rawHit;

  return _rawHitVec.size();

};

//---------------------------------------------------------
// e3Hit comparison operator
//---------------------------------------------------------
struct hitHasSame
{
  unsigned int _mrpc, _strip;

  hitHasSame(unsigned int mrpc, unsigned int strip) : _mrpc(mrpc), _strip(strip) {}
    bool operator () ( const e3RawHit& _Hit ) const
    {
      if(_Hit.getMRPC()==_mrpc && _Hit.getStrip()==_strip)
      return kTRUE;
      else return kFALSE;
    }
};

//---------------------------------------------------------
// e3Event fillHitVec method
//---------------------------------------------------------
int e3RawEvent::fillHitVec(){

  e3Hit *_Hit = NULL;
  // vector<e3RawHit> rawHitVec(_rawHitVec);

  for(vector<e3RawHit>::iterator _rawHitIt=_rawHitVec.begin(); _rawHitIt!=_rawHitVec.end(); ++_rawHitIt){
    vector<e3RawHit>::iterator _foundRawHit = find_if(_rawHitIt+1,_rawHitVec.end(), hitHasSame(_rawHitIt->getMRPC(),_rawHitIt->getStrip()));
    if(_foundRawHit!=_rawHitVec.end()){
      cout<<_rawHitIt->getMRPC()<<","<<dec<<_rawHitIt->getStrip()<<" ";
      cout<<_rawHitIt->getSide()<<" "<<_rawHitIt->getLeadTime()<<" ";
      cout<<_foundRawHit->getSide()<<" "<<_foundRawHit->getLeadTime()<<endl;

      _Hit = new e3Hit();
      _Hit->setXCoo((MAXXLENGHT+(_rawHitIt->getLeadTime()-_foundRawHit->getLeadTime())*SIGNALSPEED)/2);
      _Hit->setYCoo((float) _rawHitIt->getStrip());
      _Hit->setZCoo((float) -50+50*_rawHitIt->getMRPC());
      _HitVec.push_back(*_Hit);
      delete _Hit;

    }
  }

  return _HitVec.size();

}

//---------------------------------------------------------
// e3Event dump method
//---------------------------------------------------------
void e3RawEvent::dump(){

  cout<<"[e3RawEvent::dump - INFO] e3RawEvent #"<<dec<<getID()<<" dump BEGIN"<<endl;

  for(vector<V1190Event>::iterator v1190EventIt = _v1190EventVec.begin(); v1190EventIt != _v1190EventVec.end(); ++v1190EventIt) {

    cout<<endl<<" - V1190 ";
    if(v1190EventIt->getDT()==0x1) cout<<"A "; else cout<<"B ";
    cout<<"Event -- ";
    cout<<"Data type: 0x"<<hex<<setfill('0')<<setw(2)<<v1190EventIt->getDT()<<" ";
    cout<<endl;

    cout<<" -- Global header  -- ";
    cout<<"GEO addr: 0x"<<hex<<setfill('0')<<setw(2)<<v1190EventIt->_globalHeader.GEO<<" ";
    cout<<"Event counter: 0x"<<hex<<setfill('0')<<setw(4)<<v1190EventIt->_globalHeader.EventCount<<" ";
    cout<<"Data type: 0x"<<hex<<setfill('0')<<setw(2)<<v1190EventIt->_globalHeader.type<<" ";
    cout<<endl;
    
    for(vector<HPTDCEvent>::iterator HPTDCEventIt = v1190EventIt->_hptdcEventVec.begin(); HPTDCEventIt != v1190EventIt->_hptdcEventVec.end(); ++HPTDCEventIt) {
      cout<<" +- HPTDC Event    -- ";
      cout<<endl;
      cout<<"  -- HPTDC Header  -- ";
      cout<<"Bunch ID: 0x"<<hex<<setfill('0')<<setw(4)<<HPTDCEventIt->_tdcHeader.BunchID<<" ";
      cout<<"Event ID: 0x"<<hex<<setfill('0')<<setw(4)<<HPTDCEventIt->_tdcHeader.EventID<<" ";
      cout<<"ID: "<<dec<<setfill('0')<<setw(1)<<HPTDCEventIt->_tdcHeader.ID<<" "; 
      cout<<"Data type: 0x"<<hex<<setfill('0')<<setw(2)<<HPTDCEventIt->_tdcHeader.type<<" ";
      cout<<endl;

      for(vector<sTdcMeasure>::iterator tdcMeasureIt = HPTDCEventIt->_tdcMeasureVec.begin(); tdcMeasureIt != HPTDCEventIt->_tdcMeasureVec.end(); ++tdcMeasureIt) {
	cout<<"  -- HPTDC Measure -- ";
	cout<<"Edge: "<<dec<<setfill('0')<<setw(1)<<tdcMeasureIt->Edge<<" ";
	cout<<"Channel: "<<dec<<setfill('0')<<setw(8)<<tdcMeasureIt->Channel<<" ";
	cout<<"Time: "<<dec<<tdcMeasureIt->Time<<" ";
	cout<<endl;
      }

      cout<<"  -- HPTDC Trailer -- ";
      cout<<"Word counter: 0x"<<hex<<setfill('0')<<setw(4)<<HPTDCEventIt->_tdcTrailer.WordCount<<" ";
      cout<<"Event ID: 0x"<<hex<<setfill('0')<<setw(4)<<HPTDCEventIt->_tdcTrailer.EventID<<" ";
      cout<<"ID: "<<dec<<setfill('0')<<setw(1)<<HPTDCEventIt->_tdcTrailer.ID<<" ";
      cout<<"Data type: 0x"<<hex<<setfill('0')<<setw(2)<<HPTDCEventIt->_tdcTrailer.type<<" ";
      cout<<endl;
    }
    
    cout<<" -- ETTT -- ";
    cout<<"Raw (i.e. 5LSB missing): "<<dec<<v1190EventIt->getRawETTT()<<"";
    cout<<" Complete:  "<<dec<<v1190EventIt->getETTT()<<"";
    cout<<endl;

    cout<<" -- Global trailer -- ";
    cout<<"GEO addr: 0x"<<hex<<setfill('0')<<setw(2)<<v1190EventIt->_globalTrailer.GEO<<" ";
    cout<<"Word counter: 0x"<<hex<<setfill('0')<<setw(4)<<v1190EventIt->_globalTrailer.WordCount<<" ";
    cout<<"Data type: 0x"<<hex<<setfill('0')<<setw(2)<<v1190EventIt->_globalTrailer.type<<" ";
    cout<<"Status: 0x"<<hex<<setfill('0')<<setw(2)<<v1190EventIt->_globalTrailer.status<<" ";
    cout<<endl;

  }

  cout<<endl<<" - e3 Raw Hits vector --"<<endl;
  for(vector<e3RawHit>::iterator e3RawHitIt = _rawHitVec.begin(); e3RawHitIt != _rawHitVec.end(); ++e3RawHitIt) {
    cout<<" -- e3RawHit -- ";
    cout<<"MRPC: "<<setw(1)<<dec<<e3RawHitIt->getMRPC()<<"|";
    cout<<"Strip: "<<setfill('0')<<setw(2)<<dec<<e3RawHitIt->getStrip()<<"|";
    cout<<"Side: "<<setfill(' ')<<setw(5)<<e3RawHitIt->getSide()<<"|";
    cout<<fixed<<setprecision(1);
    cout<<"Edge times [ns]: L "<<e3RawHitIt->getLeadTime()<<" ";
    cout<<"T "<<e3RawHitIt->getTrailTime()<<" ";
    cout<<"(deltaT "<<e3RawHitIt->getTrailTime() - e3RawHitIt->getLeadTime()<<") ";
    cout<<endl;
  }

  cout<<endl<<" - e3 Hits vector --"<<endl;
  for(vector<e3Hit>::iterator e3HitIt = _HitVec.begin(); e3HitIt != _HitVec.end(); ++e3HitIt) {
    cout<<" -- e3Hit -- ";
    cout<<fixed<<setprecision(1);
    cout<<"Coo [cm]: ";
    cout<<"x "<<e3HitIt->getXCoo()<<" ";
    cout<<"y "<<e3HitIt->getYCoo()<<" ";
    cout<<"z "<<e3HitIt->getZCoo()<<" ";
    cout<<endl;
  }

  cout<<endl<<"[e3RawEvent::dump - INFO] e3RawEvent #"<<getID()<<" dump END"<<endl;

};
