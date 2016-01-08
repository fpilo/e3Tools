#include <iostream>
#include <fstream>
#include <string>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include "../include/define.h"
#include "../include/c_evento.h"
#include "../include/ExternalHisto.h"

using namespace std;

void printhex(char *,int);
void printbin(char *,int);
void printlog(char *,ofstream *);
int Legge(ifstream * , char * ,int , int);
void setFilesNames(char *, char *);
void init(char *);
long int Now();
int get_file_list(char *, int);
void print_channel();
void HistoBook(TObjArray *,char *);
int RemoveHistograms(TObjArray *);
void write_pw_file(vector <c_evento * > *);

bool LOGG=true;
bool LOGGPS=false;

int STIMEOUT=0;            //;Valore in secondi per il timeout nella lettura dei dati dalla porta seriale
int RTIMEOUT=30;            //;Valore in secondi per il timeout nella lettura dei dati dal file binario
int BUFSIZE=1000000;       //;Size del buffer utilizzato per leggere dal file binario

char MYDATADIR[256];       //;Directory specifica dove cercare i files di dati
char EEEDATADIR[256];      //;Directory dove sono i dati binari.
char LOGFILEDIR[256];      //;Eventuale path per il file di log (se nullo nessun log, default no log)
bool LOG=true;             //;Default nessun log

double MAX_CHI2XZ=0.5;       //;Valore massimo accettabile per definire buona la traccia (YZ) default 5.8D
double MAX_CHI2YZ=1.0;       //;Valore massimo accettabile per definire buona la traccia (XZ) default 1.1D
int MAXXLENGHT=158;        //;Lunghezza camera asse X in cm.

double STRIPWIDTH=2.5;     //;Larghezza della strip lungo Y in cm.
double STRIPGAP=0.7;       //;Distanza tra due strip contigue in cm.
double SIGNALSPEED=16.4;   //;Velocita' del segnale lungo la strip in cm/ns
double IWAIT=0;            //;Conta le volte in cui si deve aspettare che i dati arrivino dal DAQ

double ZETA[NUMEROCAMERE]; //;Coordinata Zeta della camere

struct s_channel channel[NCHANNEL];

char * filelist[MAXFILE];
char EEEDataType[][32]={"GPS Data","TDC 1 Data","TDC 2 Data","Start Of Run","4","Geometry","Window And Delay","Weather"};

ifstream *infile=NULL;
ifstream *incalib=NULL;
ofstream *outcalib=NULL;
ofstream *outfile=NULL;
ofstream *outbin=NULL;
ofstream *outputFileJAVA;

/*
  void checkcopia(char * s,int n) {
  int br,bw;
  outbin->flush();
  br=outbin->tellp();
  bw=infile->tellg();
  cout<<s<<" "<<n<<": input bytes <-> output bytes : "<<br<<"  <-> "<<bw<<endl;
  if(br!=bw) {

  exit(1);
  }
  }
*/

c_SOR SOR;
c_GEO GEO;
c_WAD WAD;
c_VWS VWS;

int main(int argc, char * argv[])
{

  //ROOT object definitions
  TStyle * style = new TStyle("style","myStyle");
  style->SetOptStat(111111);
  style->cd();

  TROOT simple("Check","Distribuzione tempi L-R");
  TObjArray Hlist(0);
  Hlist.Add(style);

  //Instanzia il vettore per la calibrazione delle strip
  vector< s_calib> calib;

  //Instanzia le tre camere
  c_camera * camere[NUMEROCAMERE];

  //Instanzia il vettore delle misure
  vector<s_measure *> Misure;

  //Instanzia il vettore degli eventi
  vector<c_evento *> Eventi;
  c_evento * Evento;

  //Instanzia il vettore degli eventi GPS
  vector<c_GPS *> EventiGPS;
  c_GPS * GPS;

  string line;
  bool need_new_file=true, writeCalib=false;// commenti=false;
  bool good,UsedEvent;

  char strLog[256],rootFile[256],newdatafile[256];
  char fn[256],filemap[128],buffer[BUFSIZE],LogFile[256],previous_LogFile[256],JAVAfile[256];
  char calibfile[256];
  char ProgramName[256];

  char LF[2];
  LF[1]=0x00;
  sprintf(LF,"\n");

  //    int canali[NUMEROCAMERE][192][2];

  int ifile,nfiles;
  int bsize,datatype,size_to_read,this_bsize,this_datatype;
  bool to_be_log;

  struct s_data data;
  struct s_global_header * global_header;
  struct s_tdc_header * tdc_header;
  struct s_measure * measure;
  struct s_tdc_trailer * tdc_trailer;
  struct s_global_trailer * global_trailer;

  int TDCID;
  bool EventoPending;
  int PPSExtendedTriggerTimeTag1,PPSExtendedTriggerTimeTag2;
  int OldEventCount,EventID,LastGPSEventID;
  int BunchID;
  int TotalEvent=0;
  int s_data_size=0;
  unsigned int newdummy;

  //Inizializza le camere e imposta il defaul per la ccordinata Zeta (equispaziate di 60 cm)
  for (int i=0;i<NUMEROCAMERE;i++) {
    camere[i] = new c_camera(i);
    ZETA[i]=i*60;
  }

  s_data_size=sizeof(s_data);
  strcpy(MYDATADIR,"");
  strcpy(ProgramName,argv[0]);

  strcpy(filemap,"/home/terreni/EEE/EEEmyAnalyzer_txt/EEEmyAnalyzer.ini");

  for(int i=0;i<MAXFILE;i++) {
    filelist[i]=(char *)malloc(256);
  }

  //Inizializza le strutture channel
  for(int i=0;i<NCHANNEL;i++) {
    channel[i].camera=0;
    channel[i].strip=0;
    channel[i].side=-1;
    channel[i].partner=0;
    channel[i].tcorr=0.;
  }

  //    cout<<"Leggo il file "<<filemap<<endl<<flush;
  init(filemap);

  //Legge la lista di files da analizzare
  nfiles=get_file_list(MYDATADIR,0);

  //Main loop
  previous_LogFile[0]=0x00;
  ifile=0;
  if(LOG) {
    cout<<"LOG = true"<<endl<<flush;
  } else {
    cout<<"LOG = false"<<endl<<flush;
  }

  //Print the channel mapping
  //    cout<<"Canale\t"<<"camera\t"<<"Strip\t"<<"side\t"<<"partner"<<endl<<flush;
  //    for(int i=0;i<NCHANNEL;i++) {
  //      cout<<i<<"\t"<<channel[i].camera<<"\t"<<channel[i].strip<<"\t"<<channel[i].side<<"\t"<<channel[i].partner<<endl<<flush;
  //    }

  cout<<"Analizzo "<<nfiles<<" files trovati in "<<MYDATADIR<<" ..."<<endl<<flush;

  while(ifile<nfiles) {                       //For all data file
    need_new_file=false;
    cout<<"File: "<<filelist[ifile]<<endl<<flush;
    HistoBook(&Hlist,filelist[ifile]);
    strcpy(fn,filelist[ifile]);
    setFilesNames(ProgramName,LogFile);
    ifile++;
    if(strcmp(LogFile,previous_LogFile)!=0) {
      if(outfile) {
	if(outfile->is_open()) {
	  outfile->close();
	  delete outfile;
	}
      }
    }

    //Genera nomi files output
    int ilen=strlen(fn)-4;
    memset(LogFile,'\0',256);
    memset(rootFile,'\0',256);
    memset(newdatafile,'\0',256);
    memset(JAVAfile,'\0',256);
    memset(calibfile,'\0',256);

    strncpy(LogFile,fn,ilen);
    strncpy(rootFile,fn,ilen);
    strncpy(newdatafile,fn,ilen);
    strncpy(JAVAfile,fn,ilen);

    strcat(LogFile,".log");
    strcat(rootFile,".root");
    strcat(newdatafile,".bin_calib");
    strcat(JAVAfile,"-pw-time.txt");

    //Apre il nuovo file di dati convertito
    outbin = new ofstream(newdatafile,ios_base::out | ios_base::binary);

    //Apertura del file di log
    outfile = new ofstream(LogFile,ios_base::out);
    strcpy(previous_LogFile,LogFile);
    cout<<"Log file: "<<LogFile<<endl<<flush;

    //Apre i file per il viewer Java
    outputFileJAVA = new ofstream(JAVAfile,ios_base::out);

    //Apertura file di dati
    if(LOG) {
      sprintf(strLog,"Opening data file %s\n",fn);
      printlog(strLog,outfile);
    }
    infile = new ifstream(fn,ios_base::in | ios_base::binary);

    IWAIT=0;
    for (int i=0;i<NUMEROCAMERE;i++) {
      camere[i]->Reset();
    }
    //      Hlist.Delete();
    //      cout<<"Hlist is empty="<<Hlist.IsEmpty()<<endl<<flush;

    if(infile->is_open()) {
      //Check if input data stream is just an EEE data file
      if(Legge(infile,(char *)&bsize,4,RTIMEOUT)!=4) {
	RTIMEOUT=0;
	need_new_file=true;
	break;
      }
      if(outbin) {outbin->write((char *)&bsize,4);}

      if(Legge(infile,(char *)&datatype,4,RTIMEOUT)!=4) {
	need_new_file=true;
	break;
      }
      if(outbin) {outbin->write((char *)&datatype,4);}
      if(bsize!=1162167584 || datatype!=1147237473) {
	cout<<"No EEE data file ... exit!"<<endl<<flush;
	break;
      }

      //Inizializza i contatori degli eventi
      TotalEvent=0;
      OldEventCount=-1;
      LastGPSEventID=0;
      EventoPending=false;

      //Until data are available ...
      while(!infile->eof() && !need_new_file)  {

	//Legge le dimensioni ed il tipo del blocco
	RTIMEOUT=1;
	if(Legge(infile,(char *)&bsize,4,RTIMEOUT)!=4) {
	  need_new_file=true;
	  break;
	}
	if(outbin) {outbin->write((char *)&bsize,4);}
	RTIMEOUT=2;
	if(Legge(infile,(char *)&datatype,4,RTIMEOUT)!=4) {
	  need_new_file=true;
	  break;
	}
	if(outbin) {outbin->write((char *)&datatype,4);}

	size_to_read=(bsize-2)*4;
	/*
          if(LOG && datatype != 1 && datatype != 2) {
	  sprintf(strLog,"Size:\t%d\nType:\t%d\t%s\n",bsize,datatype,EEEDataType[datatype]);
	  printlog(strLog,outfile);
          }
	*/
	//Se i dati del blocco sonno eccessivi lo segnala ed esce
	if(size_to_read > BUFSIZE) {
	  cout<<"Warning: data block size very big: "<<size_to_read<<endl<<flush;
	  need_new_file=true;
	  break;
	}

	//OK we examine now the data type
	switch(datatype) {
	case 3:            //Start Of Run
	  SOR.init(infile,outfile,outbin);
	  if(SOR.get(size_to_read)) {
	    SOR.print();
	  } else {
	    need_new_file=true;
	    break;
	  }
	  break;
	case 5:            //Geometry
	  GEO.init(infile,outfile,outbin);
	  if(GEO.get(size_to_read)) {
	    ZETA[1]=ZETA[0]+GEO.get_dist12()*100.;
	    ZETA[2]=ZETA[1]+GEO.get_dist23()*100.;

	    //                ZETA2=ZETA1+GEO.get_dist12()*100.;
	    //                ZETA3=ZETA2+GEO.get_dist23()*100.;
	    camere[0]->SetZeta(ZETA[0]);
	    camere[1]->SetZeta(ZETA[1]);
	    camere[2]->SetZeta(ZETA[2]);
	    GEO.print();
	    sprintf(calibfile,"%s/%s-%02d-calib.txt",EEEDATADIR,GEO.getRegion(),GEO.getStationID());

	    //Apre il file delle calibrazioni in lettura
	    cout<<"Calibration file: "<<calibfile<<endl<<flush;
	    int thisCamera,thisStrip;
	    double tcorr;
	    ifstream incalib(calibfile);
	    if(incalib.is_open()) {
	      cout<<"Calibration found ... now reading data ..."<<endl;
	      do {
		incalib>>thisCamera>>thisStrip>>tcorr;
		camere[thisCamera]->SetSingleStripTimeCorr(thisStrip,tcorr);
		//                    cout<<thisCamera<<"  "<<thisStrip<<"  "<<tcorr<<endl;
	      } while(!incalib.eof());
	      writeCalib=false;
	      incalib.close();
	      cout<<"End reading from calibration file ..."<<endl;
	    } else {
	      cout<<"Calibration file not available ..."<<endl;
	      writeCalib=true;
	    }
	  } else {
	    need_new_file=true;
	    break;
	  }
	  break;
	case 6:            //Window And Delay
	  WAD.init(infile,outfile,outbin);
	  if(WAD.get(size_to_read)) {
	    WAD.print();
	  } else {
	    need_new_file=true;
	    break;
	  }
	  break;
	case 7:            //Weather VWS
	  VWS.init(infile,outfile,outbin);
	  if(VWS.get(size_to_read)) {
	    VWS.print();
	  } else {
	    need_new_file=true;
	    break;
	  }
	  break;
	case 0:            //GPS data
	  GPS = new c_GPS(infile,outfile,outbin);
	  if(outbin) {outbin->flush();}
	  if(GPS->get(size_to_read)) {
	    if(EventoPending) {
	      PPSExtendedTriggerTimeTag1=Evento->GetETTT(1);
	      PPSExtendedTriggerTimeTag2=Evento->GetETTT(2);
	      Eventi.push_back(Evento);
	      EventoPending=false;
	      if(LOG) {
		Evento->Print();
	      }

	      for (int n=LastGPSEventID;n<(int)Eventi.size();n++) {
		Eventi[n]->SetPPSExtendedTriggerTimeTag(PPSExtendedTriggerTimeTag1,1);
		Eventi[n]->SetPPSExtendedTriggerTimeTag(PPSExtendedTriggerTimeTag2,2);

		if(LOGGPS) {
		  Eventi[n]->PrintSummary();
		}
	      }
	    }
	    Evento = new c_evento(outfile,GPS->getEventID());    //Alloca un nuovo evento
	    Evento->SetGPS(GPS);
	    Evento->SetWAD((c_WAD *) &WAD);
	    Eventi.push_back(Evento);
	    LastGPSEventID=Eventi.size();
	    if(LOG) {
	      Evento->Print();
	    }
	  } else {
	    need_new_file=true;
	    break;
	  }
	  break;
	case 1:            //TDC 1 data
	case 2:            //TDC 2 data
	  this_datatype=datatype;
	  this_bsize=bsize;
	  to_be_log=true;
	  while(size_to_read >= 4) {
	    if(Legge(infile,(char *)&data,s_data_size,RTIMEOUT)!=s_data_size) {
	      need_new_file=true;
	      break;
	    }
	    size_to_read-=s_data_size;

	    switch(data.tipo) {
	    case 8:                     //Global Header Event Count packet type   START OF A NEW EVENT
	      if(outbin) {
		outbin->write((char *)&data,s_data_size);
	      }
	      global_header = (s_global_header *)&data;
	      if(global_header->EventCount != OldEventCount) {
		if(EventoPending) {
		  Eventi.push_back(Evento);
		  EventoPending=false;
		}
		Evento = new c_evento(outfile,global_header->EventCount);    //Alloca un nuovo evento
		Evento->SetWAD((c_WAD *) &WAD);
		EventoPending=true;
		Evento->reset(global_header->EventCount);
		OldEventCount=global_header->EventCount;
		TotalEvent++;
		if(TotalEvent%10000==0) {
		  cout<<TotalEvent<<" events read"<<endl<<flush;
		}
	      }
	      if(LOG) {
		if(to_be_log) {
		  sprintf(strLog,"Size:\t%d\nType:\t%d\t%s\n",this_bsize,this_datatype,EEEDataType[this_datatype]);
		  printlog(strLog,outfile);
		  to_be_log=false;
		}
		sprintf(strLog,"\nGlobal Header->EventCount: %d  GEO: %d\n",global_header->EventCount,global_header->GEO);
		printlog(strLog,outfile);
		//                      printbin((char*)&data,s_data_size);
	      }
	      Evento->Add_global_header(global_header);
	      break;
	    case 1:                                    //TDC Header packet type
	      if(outbin) {outbin->write((char *)&data,s_data_size);}
	      //checkcopia("\t\tTDC HEader",1);
	      tdc_header = (s_tdc_header *)&data;
	      TDCID=tdc_header->TDC;
	      EventID=tdc_header->EventID;
	      BunchID=tdc_header->BunchID;
	      if(LOG) {
		sprintf(strLog,"\tTDC Header-> EventID: %d  ChipID: %d  BunchID: %d\n",EventID,TDCID,BunchID);
		printlog(strLog,outfile);
		//                      printbin((char*)&data,s_data_size);
	      }
	      Evento->Add_tdc_header(tdc_header);
	      break;
	    case 0:                                                                    //TDC Measure
	      measure=new s_measure;
	      measure->Edge     = ((data.dummy & 0x04000000) >> 26);                     //0=leading, 1=trailing
	      measure->Channel  = ((data.dummy & 0x03F80000) >> 19) + 128*(datatype-1);  //Canale del TDC
	      measure->Time     = ((data.dummy & 0x0007FFFF));                           //Tempo in decimi di ns
	      measure->CameraID = channel[measure->Channel].camera;
	      measure->Strip    = channel[measure->Channel].strip;
	      measure->Side     = channel[measure->Channel].side;
	      measure->Partner  = channel[measure->Channel].partner;
	      measure->EventID  = EventID;
	      measure->pw=0;
	      measure->Used=0;
	      measure->ForCalib=true;

	      Misure.push_back(measure);
	      Evento->AddMeasure(measure);

	      newdummy=0;
	      newdummy = (measure->Edge << 26);
	      newdummy = newdummy + ((channel[measure->Channel].newchannel - 128*(datatype-1)) << 19);
	      if(measure->Side==0) {
		newdummy = newdummy + measure->Time - camere[measure->CameraID]->GetTcorr(measure->Strip)/2.;
	      } else {
		newdummy = newdummy + measure->Time + camere[measure->CameraID]->GetTcorr(measure->Strip)/2.;
	      }

	      if(outbin) {
		outbin->write((char *)&newdummy,s_data_size);
		outbin->flush();
	      }
	      if(LOG) {
		sprintf(strLog,"\t\t%d\t%d\t%d\t%5.0f\t%d\n",measure->CameraID,measure->Strip,measure->Side,measure->Time,measure->Edge);
		printlog(strLog,outfile);
	      }
	      break;

	    case 3:                                            //TDC trailer
	      if(outbin) {outbin->write((char *)&data,s_data_size);}
	      tdc_trailer=(s_tdc_trailer *)&data;

	      if(LOG){
		sprintf(strLog,"\tTDC Trailer->EventID: %d  ChipID: %d  Word count: %d\n",tdc_trailer->EventID,tdc_trailer->TDC,tdc_trailer->WordCount);
		printlog(strLog,outfile);
		//                      printbin((char*)&data,s_data_size);
		//                      printlog(LF,outfile);
	      }
	      Evento->Add_tdc_trailer(tdc_trailer);
	      break;
	    case 17:                                        //Extended Trigger Time Tag
	      if(outbin) {outbin->write((char *)&data,s_data_size);}
	      if(LOG) {
		sprintf(strLog,"\tRaw Extended Trigger Time Tag: %d\n",data.dummy);
		printlog(strLog,outfile);
	      }
	      Evento->AddRawETTT(data.dummy,datatype);

	      break;
	    case 16:                                          //Global Trailer
	      if(outbin) {outbin->write((char *)&data,s_data_size);}
	      global_trailer=(s_global_trailer *)&data;
	      Evento->SetGeo(global_trailer->GEO,datatype);
	      if(LOG) {
		sprintf(strLog,"Global trailer->  Status: %d  (GEO: %d & 0x1F = %02X)\n",global_trailer->status,
			global_trailer->GEO,(global_trailer->GEO & 0x1F));
		printlog(strLog,outfile);
		//                      printbin((char*)&data,s_data_size);
	      }
	      Evento->Add_global_trailer(global_trailer);
	      break;
	    case 4:                      //TDC Error
	      if(outbin) {outbin->write((char *)&data,s_data_size);}
	      sprintf(strLog,"TDC ERROR!!!!\n");
	      printlog(strLog,outfile);
	      cout<<"TDC ERROR!!!!"<<endl;
	      break;
	    default:
	      if(outbin) {outbin->write((char *)&data,s_data_size);}
	      sprintf(strLog,"Unknown measure code ...\n");
	      printlog(strLog,outfile);
	      cout<<"Codice sconosciuto..."<<endl;
	    }
	  }

	  if(need_new_file) break;
	  break;                       //close case 1 and 2
	default:
	  if(outbin) {outbin->write((char *)&data,s_data_size);}
          //checkcopia("Unknown code",101);
	  sprintf(strLog,"Unknown block code ...\n");
	  printlog(strLog,outfile);
	  while(size_to_read > BUFSIZE) {
	    RTIMEOUT=4;
	    if(Legge(infile,buffer,BUFSIZE,RTIMEOUT)!=BUFSIZE) {
	      break;
	    }
	    if(outbin) {outbin->write(buffer,BUFSIZE);}
	    //checkcopia("Skipping data",110);
	    size_to_read-=BUFSIZE;
	  }
          RTIMEOUT=5;
	  if(Legge(infile,buffer,size_to_read,RTIMEOUT)!=size_to_read) {
	    break;
	  }
	  if(outbin) {outbin->write(buffer,size_to_read);}
          //checkcopia("Skipping data",111);
	}                //End primary switch
      }                    //End of while(!infile->eof() && !need_new_file)
      infile->close();
      //        delete infile;
      if(outbin) {
	outbin->close();
	//          delete outbin;
      }
      if(EventoPending) {
	Eventi.push_back(Evento);
	EventoPending=false;
      }
      for (int n=LastGPSEventID;n<(int)Eventi.size();n++) {
	Eventi[n]->SetPPSExtendedTriggerTimeTag(PPSExtendedTriggerTimeTag1,1);
	Eventi[n]->SetPPSExtendedTriggerTimeTag(PPSExtendedTriggerTimeTag2,2);
	if(LOGGPS) {
	  Eventi[n]->PrintSummary();
	}
      }

      //This data file has been read. Now start event analysis
      cout<<"End reading binary file ... start TDC calibration for "<<Eventi.size()<<" events ..."<<endl<<flush;

      //We calibrate the TDC time measure
      int TotaleMisureTuttiEventiSide[NUMEROCAMERE][NSTRIP][2];
      for(int i=0;i<NUMEROCAMERE;i++) {
	for(int j=0;j<NSTRIP;j++) {
	  TotaleMisureTuttiEventiSide[i][j][0]=0;
	  TotaleMisureTuttiEventiSide[i][j][1]=0;
	}
      }
      int skipped,noskipped;
      skipped=0;
      noskipped=0;

      cout<<"Time calibartion for any channel (strip) ..."<<endl;
      s_calib thisCalib;

      for(unsigned int ev=0;ev<Eventi.size();ev++) {                 //Per tutti gli eventi
	c_evento * it = Eventi[ev];
	it->Print();
	if(!it->IsGPS()) {
	  it->SelectMeasureForCalib();
	  UsedEvent=false;
	  for(int c=NUMEROCAMERE-1;c>=0;c--) {                       //Per ogni camera
	    int Tutte=it->GetNumberOfMeasure(c,0);
	    for(int n=0;n<Tutte ;n++) {
	      s_measure * m = it->GetMeasure(c,n);
	      if( m->ForCalib ) {
		UsedEvent=true;
		for(int k=n+1;k<Tutte;k++) {
		  s_measure * mm = it->GetMeasure(c,k);
		  if(mm->Channel == m->Partner) {
		    thisCalib.EventID=it->GetEventID();
		    thisCalib.Camera=c;
		    thisCalib.Strip=m->Strip;
		    if(m->Side == 0) {
		      thisCalib.TimeL=m->Time;
		      thisCalib.TimeR=mm->Time;
		    } else {
		      thisCalib.TimeR=m->Time;
		      thisCalib.TimeL=mm->Time;
		    }
		    calib.push_back(thisCalib);
		  }
		}
	      }
	    }
	  }
	  if(UsedEvent) {
	    noskipped++;
	  } else {
	    skipped++;
	  }
	}
      }

      //        cout<<" \tEv.\tCam.\tStrip\tTimeL\tTimeR\tTL+TR\tTL-TR\tX-coord"<<endl;
      for(unsigned int i=0;i<calib.size();i++) {
	int c=calib[i].Camera;
	calib[i].TLpiuTR=calib[i].TimeL+calib[i].TimeR;
	calib[i].TLmenoTR=calib[i].TimeL-calib[i].TimeR;
	calib[i].x=(MAXXLENGHT+calib[i].TLmenoTR*0.1*SIGNALSPEED)/2.;

	//          cout<<"C\t"<<calib[i].EventID<<"\t"<<calib[i].Camera<<"\t"<<calib[i].Strip<<"\t"<<calib[i].TimeL<<"\t"
	//              <<calib[i].TimeR<<"\t"<<calib[i].TLpiuTR<<"\t"<<calib[i].TLmenoTR<<"\t"<<calib[i].x<<endl;
	camere[c]->Statistic(calib[i].Strip,0,calib[i].TimeL);
	camere[c]->Statistic(calib[i].Strip,1,calib[i].TimeR);
      }

      cout<<endl<<"Events Skipped: "<<skipped<<"  no skipped: "<<noskipped<<endl<<endl;
      for(int c=NUMEROCAMERE-1;c>=0;c--) {
	camere[c]->SetStripTimeNewCorr();
	camere[c]->PrintSummary();
	camere[c]->StatisticReset();
      }

      if(writeCalib) {
	cout<<"Now writing calibration file: "<<calibfile<<endl;
	outcalib = new ofstream(calibfile,ios_base::out);
	for(int c=NUMEROCAMERE-1;c>=0;c--) {
	  for(int s=0;s<NSTRIP;s++) {
	    sprintf(strLog,"%d %d %6.2f\n",c,s,camere[c]->GetNewTcorr(s));
	    printlog(strLog,outcalib);
	  }
	}
	outcalib->close();
      }


      //We report the time calib on channel
      for(int i=0; i<NCHANNEL;i++) {
	int c=channel[i].camera;
	int strip=channel[i].strip;
	if(channel[i].side == 0) {
	  channel[i].tcorr=-camere[c]->GetTcorr(strip)/2;
	} else if(channel[i].side == 1) {
	  channel[i].tcorr=camere[c]->GetTcorr(strip)/2;
	}
      }

      //Printout channel data
      //        cout<<"Printing channel data on log file"<<endl;
      //        print_channel();

      //We write the data file to be used by the Viewer (java)
      //        cout<<"writing output file for java Viewer"<<endl;
      //        write_pw_file(&Eventi);
      outputFileJAVA->close();

      //We analyze the events
      cout<<"Start events analysis..."<<endl;
      for(unsigned int ev=0;ev<Eventi.size();ev++) {
        c_evento * it = Eventi[ev];
	if( !it->IsGPS()) {
	  //            cout<<endl<<"*******************************************************"<<endl;
	  double val_1_3=0., val_2_3=0., val_1_2=0.;
	  int mStrip;
	  //            int tdc;
	  int h[3];

	  it->Elabora(camere);

	  //Histogram for TOFs
	  //            double t3off=5.4;
	  //            printf("\nEv. %5d      chi2XZ    chi2YZ      chi2\n",it->GetEventID());
	  for(int i=0;i<it->GetNumberOfFit();i++) {
	    c_fit * fit = it->GetFit(i);
	    s_hit * h1 = fit->GetHit(0);
	    s_hit * h2 = fit->GetHit(1);
	    s_hit * h3 = fit->GetHit(2);
	    //              point_ * p1 = h1->GetPoint();
	    //              point_ * p2 = h2->GetPoint();
	    //              point_ * p3 = h3->GetPoint();

	    val_1_2=h1->GetAbsTime()-h2->GetAbsTime();
	    val_1_3=h1->GetAbsTime()-h3->GetAbsTime();
	    val_2_3=h2->GetAbsTime()-h3->GetAbsTime();
	    TOF_1_2->Fill(val_1_2);
	    TOF_1_3->Fill(val_1_3);
	    TOF_2_3->Fill(val_2_3);

	    Chi2XZ->Fill(fit->Getchi2XZ());
	    Chi2YZ->Fill(fit->Getchi2YZ());
	    Chi2->Fill(fit->Getchi2());

	    if(fit->IsGood()) {
	      CHI2TOF_1_2->Fill(val_1_2);
	      CHI2TOF_1_3->Fill(val_1_3);
	      CHI2TOF_2_3->Fill(val_2_3);
	    }
	    //              printf("           %10.2f%10.2f%10.2f\n",fit->Getchi2XZ(),fit->Getchi2YZ(),fit->Getchi2());
	  }
	  //            cout<<"Ev. "<<it->GetEventID()<<"\tCamera\thit\tx\ty\t   AbsTime\t\tRETT\tETTT\tPPSETTT\t\tdETTT\t\ttmisura(ns)"<<endl;
	  for (int c=NUMEROCAMERE-1;c>=0;c--) {                                              //Per ogni camera
	    //              if(c==1) {tdc=2;} else {tdc=1;}
	    int maxMeasure=(it->GetNumberOfMeasure(c,0));
	    //              int E1=it->GetRawExtendedTriggerTimeTag(tdc);
	    //              int E2=it->GetETTT(tdc);
	    //              int E3=it->GetPPSExtendedTriggerTimeTag(tdc);
	    //              double E4=it->GetdETTT(tdc);

	    for (int m=0;m < maxMeasure;m++) {                                            //Per ogni misura
	      s_measure * im = it->GetMeasure(c,m);
	      TotaleMisureTuttiEventiSide[c][im->Strip][im->Side]++;
	      StripTime[c][im->Strip][im->Side]->Fill(im->Time);
	    }
	    int maxHit=it->GetNumberOfHit(c);
	    for (int ht=0;ht < maxHit;ht++) {                              //Per ogni hit
	      s_hit * ih = it->GetHit(c,ht);
	      mStrip=ih->GetStrip() ;
	      StripT[c][mStrip]->Fill(ih->Getdt());
	      StripX[c][mStrip]->Fill(ih->Getx());
	      HistDiff[c]->Fill(ih->Getdt());
	      point_ * P;
	      P=ih->GetPoint();
	      Hit2D[c]->Fill(P->x(),P->y());
	      Hit2D[c]->Fill(ih->Getrawx(),P->y()+90);
	      //                sprintf(strLog,"%11.0f %10d %10d %10d %11.0f %11.2f  (%d,%d)",ih->AbsTime,E1,E2,E3,E4,ih->mtime,ih->measure[0]->Used,ih->measure[1]->Used);
	      //                cout<<"\t ";
	      if(ht==h[c] && good) {
		//                  cout<<"    ->\t";
	      }
	      //                cout<<c+1<<"\t"<<ht<<"\t"<<ih->x<<"\t"<<ih->y<<"\t"<<strLog<<endl;
	    }
	  }
	  //            cout<<"chi2=("<<it->GetMinCHI2_1()<<","<<it->GetMinCHI2_2()<<")"<<endl;
	  if((it->GetNumberOfMeasure(3,0)) > 0) {
	    MeasureForEvent->Fill(it->GetNumberOfMeasure(3,0));
	    MeasureForEventInC0->Fill(it->GetNumberOfMeasure(0,0));
	    MeasureForEventInC1->Fill(it->GetNumberOfMeasure(1,0));
	    MeasureForEventInC2->Fill(it->GetNumberOfMeasure(2,0));
	  }
	}
	//          cout<<endl;
      }
      /*
        cout<<"Camera"<<"\t"<<"Strip"<<"\t"<<"Side L"<<"\t"<<"Side R"<<endl;
        for(int i=0;i<NUMEROCAMERE;i++) {
	for(int j=0;j<NSTRIP;j++) {
	cout<<i<<"\t"<<j<<"\t"<<TotaleMisureTuttiEventiSide[i][j][0]<<"\t"<<TotaleMisureTuttiEventiSide[i][j][1]<<endl;
	}
        }
      */
      //Open root output file
      cout<<"Root output data file: "<<rootFile<<endl<<flush;
      TFile *H_file = new TFile(rootFile,"recreate");
      Hlist.Write();
      H_file->Close();
      Hlist.SetOwner(kTRUE);
      cout<<"Reset all histograms"<<endl;
      Hlist.Delete();

      need_new_file=true;
    } else {
      cout<<"Data file is not open...."<<endl;
    }
  }           //End while(1)

  return 0;
}

int get_file_list(char * datadir, int i)
{
  char newdir[1024];
  char *indx;
  int this_error;
  DIR *d;
  struct dirent *dir;

  //   cout<<"(i="<<i<<") Esamino la directory: "<<datadir<<endl<<flush;
  d=opendir(datadir);
  if(d) {
    while( ((dir = readdir(d)) != NULL) && (i<MAXFILE) ) {
      //       printf("\n\ndir->d_type=%02X  DT_DIR=%02X   dir->d_name=%s\n",dir->d_type,DT_DIR,dir->d_name);
      if (dir->d_type & DT_DIR) {
	if( (strcmp(dir->d_name,".") != 0) && (strcmp(dir->d_name,"..") != 0) ) {
	  sprintf(newdir,"%s/%s",datadir,dir->d_name);
	  //           cout<<"(i="<<i<<") Chiamo get_file_list("<<newdir<<","<<i<<")"<<endl<<flush;
	  i=get_file_list(newdir,i);
	}
      } else {
	//         cout<<"Esamino il file "<<dir->d_name<<endl<<flush;
	indx=strstr(dir->d_name,".bin");
	if(indx) {
	  if(strlen(indx)==4) {
	    //             cout<<"(i="<<i<<") Memorizzo il file "<<dir->d_name<<endl<<flush;
	    strcpy(filelist[i],datadir);
	    strcat(filelist[i],"/");
	    strcat(filelist[i],dir->d_name);
	    //             cout<<"(i="<<i<<") Memorizzato il file "<<filelist[i]<<endl<<flush;
	    i++;
	  }
	}
      }
    }
    closedir(d);
  } else {
    this_error=errno;
    cout<<"Error looking for file in dir "<<datadir<<" errno= "<<this_error;
    exit(1);
  }

  return i;
}

long int Now() {
  struct timeval t0;
  gettimeofday(&t0,NULL);
  return (long int)(t0.tv_sec);
}

int Legge(ifstream * infile, char * buffer,int n, int timeout) {
  int nrest,nread;
  long int sec=Now();
  nrest=n;

  while(nrest>0 && (Now()-sec)<=timeout) {
    infile->read(buffer,nrest);
    nread=infile->gcount();
    nrest-=nread;
    IWAIT--;
    if(nrest>0) {
      buffer+=nread;
      //        Sleep(10);
      IWAIT=0;
      infile->clear();
    }
  }
  //cout<<"Legge ("<<timeout<<") bytes letti: "<<n-nrest<<endl;
  return (n-nrest);
}

void printhex(char *p,int nch) {
  for (int i=0; i<nch;i++) {
    if(i%8==0 && i>0) {
      printf("\n");
    }
    printf("%02X ",*p);
    p++;
  }
  printf("\n");
}

void printbin(char *p, int chr) {
  int mask=0;
  char strLog[256];
  char *c;
  int val;

  val=0;
  c=(char *)&strLog;
  p=p+chr-1;

  for(int j=0;j<chr;j++) {
    for (int i=0; i<8;i++) {
      mask=0x80>>i;
      val<<=1;
      if((*p & mask) != 0x00) {
        sprintf(c,"1");
        val=val | 1;
      } else {
        sprintf(c,"0");
      }
      c++;
    }
    p--;
    //    sprintf(c," ");
    //    c++;
  }
  printlog(strLog,outfile);
  cout<<strLog<<endl;
  sprintf(strLog,"    %d\n",val);
  printlog(strLog,outfile);


}

void printlog(char *p, ofstream *fout) {
  fout->write(p,strlen(p));
  fout->flush();
}

void init(char *filemap) {

  string line;
  char cstr[128],myline[128];
  char *p;
  char parName[256];
  int parVal,pp;
  double dparVal;
  unsigned int partner;
  int side;

  ifstream mymap(filemap);

  if(mymap.is_open()) {
    int im=0;
    do {
      getline(mymap,line);
      if(line.at(0)!=0x3B) {             //Primo carattere diverso da ";" riga valida
	strcpy(cstr,line.c_str());
	for(int i=0;i<(int)strlen(cstr);i++) {if(cstr[i]<0x20) {cstr[i]=0x00;}}
	p=strtok(cstr," ");
	pp=0;
	if(*p==0x43) {pp=1;}     //C  assegnazione dei canali dei TDC alle strip delle camere
	if(*p==0x53) {pp=2;}     //S  variabile di tipo string
	if(*p==0x49) {pp=3;}     //I  variabile di tipo integer
	if(*p==0x44) {pp=4;}     //D  variabile di tipo double
	if(*p==0x42) {pp=5;}     //B  variabile di tipo boolean
	if(*p==0x52) {pp=6;}     //R  channelid re-mapping
	switch(pp) {
	case 1:                //The first character is 'C' so data are for channel-strip mapping
	  p=strtok(NULL," ");
	  im=atoi(p);
	  p=strtok(NULL," ");
	  channel[im].camera=atoi(p);
	  p=strtok(NULL," ");
	  channel[im].strip=atoi(p);
	  p=strtok(NULL," ");
	  if(*p==0x4C) {channel[im].side=0;}      //Left  side
	  if(*p==0x52) {channel[im].side=1;}      //Right side
	  p=strtok(NULL," ");
	  channel[im].partner=atoi(p);

	  partner=channel[im].partner;
	  side=-1;
	  if(channel[im].side==0) {side=1;}
	  if(channel[im].side==1) {side=0;}
	  channel[partner].camera=channel[im].camera;
	  channel[partner].strip=channel[im].strip;
	  channel[partner].side=side;
	  channel[partner].partner=im;
	  break;
	case 2:                 //The first character is 'S' so data are for string parameter
	  strcpy(parName,strtok(NULL," "));
	  if(strcmp(parName,"EEEDATADIR")==0) {strcpy(EEEDATADIR,strtok(NULL," "));break;}
	  if(strcmp(parName,"MYDATADIR")==0) {strcpy(MYDATADIR,strtok(NULL," "));break;}
	  if(strcmp(parName,"LOGFILEDIR")==0) {strcpy(LOGFILEDIR,strtok(NULL," "));break;}
	  break;
	case 3:                 //The first character is 'I' so data are for integer parameter
	  strcpy(parName,strtok(NULL," "));
	  parVal=atoi(strtok(NULL," "));
	  if(strcmp(parName,"STIMEOUT")==0) {STIMEOUT=parVal;break;}
	  if(strcmp(parName,"RTIMEOUT")==0) {RTIMEOUT=parVal;break;}
	  if(strcmp(parName,"BUFSIZE")==0) {BUFSIZE=parVal;break;}
	  //                    if(strcmp(parName,"NUMEROCAMERE")==0) {NUMEROCAMERE=parVal;break;}
	  //                    if(strcmp(parName,"MAXMEASURE")==0) {MAXMEASURE=parVal;break;}
	  //                    if(strcmp(parName,"MAXHIT")==0) {MAXHIT=parVal;break;}
	  //                    if(strcmp(parName,"MAXFIT")==0) {MAXFIT=parVal;break;}
	  break;
	case 4:                 //The first character is 'D' so data are for double type parameter
	  strcpy(parName,strtok(NULL," "));
	  dparVal=atof(strtok(NULL," "));
	  if(strcmp(parName,"MAX_CHI2XZ")==0) {MAX_CHI2XZ=dparVal;break;}
	  if(strcmp(parName,"MAX_CHI2YZ")==0) {MAX_CHI2YZ=dparVal;break;}
	  if(strcmp(parName,"MAXXLENGHT")==0) {MAXXLENGHT=dparVal;break;}
	  if(strcmp(parName,"ZETA1")==0) {ZETA[0]=dparVal;break;}
	  if(strcmp(parName,"ZETA2")==0) {ZETA[1]=dparVal;break;}
	  if(strcmp(parName,"ZETA3")==0) {ZETA[2]=dparVal;break;}

	  if(strcmp(parName,"STRIPWIDTH")==0) {STRIPWIDTH=dparVal;break;}
	  if(strcmp(parName,"STRIPGAP")==0) {STRIPGAP=dparVal;break;}
	  if(strcmp(parName,"SIGNALSPEED")==0) {SIGNALSPEED=dparVal;break;}
	  break;
	case 5:                //The first character is 'B' so data are boolean
	  strcpy(parName,strtok(NULL," "));
	  if(strcmp(parName,"LOG")==0) {
	    if(strcmp(strtok(NULL," "),"true")==0) {LOG=true;} else {LOG=false;}
	  }
	  break;
	case 6:
	  p=strtok(NULL," ");
	  im=atoi(p);
	  p=strtok(NULL," ");
	  channel[im].newchannel=atoi(p);
	}
      }
      strcpy(myline,line.c_str());
    } while(!mymap.eof() && strncmp(myline,";END",4) != 0);
    mymap.close();
  } else {
    cout<<"Mapping data file "<<filemap<<" not found! Program exit!"<<endl;
    exit(1);
  }
}
void setFilesNames(char * ProgramName, char * LogFile) {

  struct tm *loctime;
  time_t curtime;
  char mydate[11];

  curtime=time(NULL);
  loctime=localtime(&curtime);
  if(strlen(LOGFILEDIR) > 0) {
    sprintf(mydate,"%04d-%02d-%02d",loctime->tm_year+1900,loctime->tm_mon+1,loctime->tm_mday);
    sprintf(LogFile,"%s\\%s-%s.log",LOGFILEDIR,ProgramName,mydate);
  }
  if(strlen(MYDATADIR)==0) {
    sprintf(MYDATADIR,"%s\\%04d-%02d-%02d",EEEDATADIR,loctime->tm_year+1900,loctime->tm_mon+1,loctime->tm_mday);
    //  cout<<ProgramName<<": LogFile = "<<LogFile<<"   MYDATADIR = "<<MYDATADIR<<endl;
  }
}

void print_channel() {
  char strLog[256];
  int i;
  for(i=0;i<NCHANNEL;i++) {
    sprintf(strLog,"channel[%d]:\t%d\t%d\t%d\t%d\t%f\n",i,channel[i].camera,channel[i].strip,channel[i].side,channel[i].partner,channel[i].tcorr);
    printlog(strLog,outfile);
  }
}

void write_pw_file( vector <c_evento *> * Events ) {
  char line[256];
  unsigned int canale;
  //    unsigned int side;
  double tcorr;

  sprintf(line,"#TDC data output format:\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#===========================================================\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#EventID; ChannelID; Time (ns); Width (ns)\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#===========================================================\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#\n#\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#Negative values in channelID give special meaning to data:\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#===========================================================\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#EventID; -1; TDC-128_Extended_Trigger_Time_Tag (ns); 0\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#EventID; -2; TDC-64_Extended_Trigger_Time_Tag (ns); 0\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#EventID; -3; x1; y1; x2; y2; chi2XZ; chi2YZ\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#             Track reconstructed by Analyzer\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#             (x1,y1) Bottom Chamber, (x2,y2) Top Chamber\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#             chi2XZ chi2 plane XZ, chi2YZ chi2 plane YZ\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#===========================================================\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#EventID; -100; GPS_year; 0\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#EventID; -101; GPS_day; 0\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#EventID; -102; GPS_second; 0\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#EventID; -103; GPS_nanosecond; 0\n");
  printlog(line,outputFileJAVA);
  sprintf(line,"#===========================================================\n");
  printlog(line,outputFileJAVA);

  GEO.print_pw(outputFileJAVA);

  sprintf(line,"#===========================================================\n");
  printlog(line,outputFileJAVA);

  for(unsigned int i=0;i<Events->size();i++) {
    c_evento * ev = Events->at(i);

    if(ev->IsGPS()) {
      c_GPS * g = ev->GetGPS();
      g->print_pw(outputFileJAVA);

    } else {

      for(int c=0;c<3;c=c+2) {
	int maxMeasure=(ev->GetNumberOfMeasure(c,0));
	for (int m=0;m < maxMeasure;m++) {                             //Per ogni misura on leading edge
	  s_measure * im = ev->GetMeasure(c,m);

	  canale=im->Channel;
	  //            side=im->Side;
	  tcorr=channel[canale].tcorr;

	  sprintf(line,"%d\t%d\t%6.1f\t%4.1f\n",ev->GetEventID(),canale,((double)(im->Time)-tcorr)/10.,(double)(im->pw)/10. );
	  printlog(line,outputFileJAVA);

	}
      }
      sprintf(line,"%d\t-1\t%10.1f\t0\n",ev->GetEventID(),ev->GetdETTT(1) );
      printlog(line,outputFileJAVA);
      int maxMeasure=(ev->GetNumberOfMeasure(1,0));
      for (int m=0;m < maxMeasure;m++) {                             //Per ogni misura on leading edge
	s_measure * im = ev->GetMeasure(1,m);
	canale=im->Channel;
	//          side=im->Side;
	tcorr=channel[canale].tcorr;

	sprintf(line,"%d\t%d\t%6.1f\t%4.1f\n",ev->GetEventID(),canale,((double)(im->Time)-tcorr)/10.,(double)(im->pw)/10. );
	printlog(line,outputFileJAVA);

      }
      sprintf(line,"%d\t-2\t%10.1f\t0\n",ev->GetEventID(),ev->GetdETTT(2) );
      printlog(line,outputFileJAVA);
    }
  }
}
