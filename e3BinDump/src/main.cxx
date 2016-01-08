#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <limits>
#include <vector>

#include "TROOT.h"
#include "TApplication.h"

#include "anyoption.h"
#include "e3Event.h"

using namespace std;

enum STATUS {
  OK_STAT=0x01,
  BAD_STAT=0x00
};

int VLEVEL;
string BFNAME;
string DTSTR;
int BLKSTOREAD;

int GetUserOpt(int, char**);

//! Main routine
/*!
  Main routine.
*/
int main(int argc, char **argv)
{

  int _status=0;        ///< Process status
  int _ret=0;          

  int _readBlocks;
  string _inBinFileName;
  ifstream _inFile;
  e3DataBlock _e3DataBlock;
  
  //======================================== 
  // Set default values
  //========================================  
  
  VLEVEL = 0;
  DTSTR="";
  BLKSTOREAD=numeric_limits<int>::max();

  //======================================== 
  // Parse inline command options
  //========================================  

  _status=GetUserOpt(argc, argv);              
  if(_status!=OK_STAT){ 
    // if(_status == 0) 
    //   cerr<<"[e3Viewer.exe - ERROR] GetUserOpt returned error code "
    // 	  <<_status<<" - too few arguments. "<<endl;
    exit(EXIT_FAILURE);
  }

  // if(argc!=2){
  //   cerr<<"[e3Viewer - ERROR] Please specify data file path and list"<<endl;
  //   exit(EXIT_FAILURE);
  // }
  
  if(VLEVEL>0) _e3DataBlock.setVerbosityLevel(VLEVEL);
  if(!DTSTR.empty()) _e3DataBlock.setDTFilter(DTSTR);

  _inFile.open(BFNAME.c_str(), ios_base::in | ios_base::binary);  
  _e3DataBlock.openBinFile(&_inFile);

  _readBlocks=0;
  while(_readBlocks<BLKSTOREAD){
    _ret=_e3DataBlock.getNextBlock();
    if(!_ret) _readBlocks++;
    else if(_ret==1){
      cout<<"[WARNING] Read Blocks returned: "<<_ret<<". End of file reached. Total analysed blocks: "<<_readBlocks<<endl;
      exit(EXIT_SUCCESS);
    }
    else{
      cout<<"[ERROR] Read Blocks returned: "<<_ret<<". Exiting ..."<<endl;
      exit(EXIT_FAILURE);
    }  
  }

  exit(EXIT_SUCCESS);
  
}

//!AnyOption Interface function.
/*!

Command line option parsing function.

*/
int GetUserOpt(int argc, char* argv[]){

  AnyOption *opt = new AnyOption();

  //opt->setVerbose();                          // print warnings about unknown options
  //opt->autoUsagePrint(false);                 // print usage for bad options

  opt->addUsage( "Usage: e3Viewer.exe [options] [arguments]" );
  opt->addUsage( "" );
  opt->addUsage( "Options: " );
  opt->addUsage( "" );
  opt->addUsage( "  -h, --help               Print this help " );
  opt->addUsage( "  -v, --verbose <vlevel>   Change verbosity level" );
  opt->addUsage( "  -d, --data-type <datatypestr>   Dump only specific data block type" );
  opt->addUsage( "" );
  opt->addUsage( "Arguments: " );
  opt->addUsage( "  dstPath          DST files absolute path" );
  opt->addUsage( "  totBlocks       Number of data blocks to be dumped" );
  opt->addUsage( "" );
  //opt->addUsage( "Notes: ");
  //opt->addUsage( "" );

  opt->setFlag( "help", 'h' );  
  opt->setOption( "verbose", 'v' );
  opt->setOption( "data-type", 'd' );

  opt->processCommandArgs( argc, argv );      // go through the command line and get the options 

  if( ! opt->hasOptions()) {                  // print usage if no options 
    cout<<"[e3Viewer.exe - WARNING] No options specified. Using default values ..."<<endl;
    // cout<<"[e3Viewer.exe - WARNING] No options specified. Type 'e3Viewer.exe --help' for usage. Exiting ..."<<endl;
    // delete opt;
    // return BAD_STAT;
  }

  if( opt->getFlag( "help" ) || opt->getFlag( 'h' ) ){      //get the options and the flag
    opt->printUsage();
    return 2;
  }

  if( opt->getValue( "verbose" ) != NULL ){
    VLEVEL=atoi(opt->getValue( "verbose" )); 
  }

  if( opt->getValue( "data-type" ) != NULL ){
    DTSTR=opt->getValue( "data-type" ); 
  }

  switch(opt->getArgc()){                                   //get the actual arguments after the options
    
  case 2: 
    BFNAME = opt->getArgv(0);
    BLKSTOREAD = atoi(opt->getArgv(1));
    break;

  case 1: 
    BFNAME = opt->getArgv(0);
    break;

  default:
    delete opt;
    cout<<"[e3Viewer.exe - WARNING] No arguments specified. Using default settings. Type 'e3Viewer.exe --help' for usage."<<endl;
    return OK_STAT;
    break;
  }

  delete opt;
  return OK_STAT;

}
