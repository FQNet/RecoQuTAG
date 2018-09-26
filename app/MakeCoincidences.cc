// include std libraries                                                                        
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cstring>
#include <string.h>
#include <sstream>
#include <assert.h>
// include ROOT libraries 
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TTree.h"
#include "TChain.h"
#include "TGraph.h"
#include "TStyle.h"
#include "TFolder.h"
#include "TCanvas.h"
#include "TRandom.h"
#include "TMath.h"
#include "TFile.h"
#include "TSystem.h"
#include "TProfile.h"

#define MAX_TOFPET_CHANNEL 64
#define REF_CHANNEL 1
using namespace std;

int main(int argc, char* argv[]){
   if(argc < 2) {
    cerr << "Please give 2 arguments " << "inputFileName " << " " << "outputFileName" <<endl;
    return -1;
  }
  const char *inputFileName = argv[1];
  const char *outFileName   = argv[2];
  
  TFile *outFile = new TFile(outFileName,"recreate");
  TTree *outTree = new TTree("data","data");
  outTree->SetAutoSave();

  ifstream inputfile (inputFileName);
  if (!inputfile.is_open()) assert(false);

  //=====================Initialize output tree variables=========================
  float chTime[4];

 //initialize for event 1
  int event=1;
  for(int k=0;k<4;k++){
    chTime[k]=-9999; 
  }
  
  //==============set Branch addresses for all the output variables================  
  outTree->Branch("event",&event,"event/I");
  outTree->Branch("chTime",&chTime,"chTime[4]/F");



 //========================Initialize local variables=================== 
  double co_window=1000; //(1000 ps)    
  Long64_t TPrev=-1;

  //======================== Look for coincidences =======================================

  vector< pair<long long, int> > eventData;
  int NEvents = 0 ;

  while ( !inputfile.eof()  ) {


    long long time = -1;
    int channel = -1;
    string tmp;
    inputfile >> tmp;
    if (inputfile.eof()) break;
    //cout << "Read : " << tmp << " | ";
    time = atoll( (tmp.substr(0,tmp.length()-1)).c_str() );
    inputfile >> tmp;
    //cout << tmp << "\n";
    channel = stoi( tmp);

    

    if (TPrev < 0) TPrev = time;    
    long long t_diff = time - TPrev;

    // cout << "TPrev = " << TPrev << "\n";
    // cout << "t_diff = " << t_diff << "\n";
    // cout << "NewLine : " << channel << " , " << time << "\n";
    if (t_diff < co_window) {
      eventData.push_back( pair<long long, int>(time, channel));
      //cout << "Add : " << channel << " , " << time << "\n";
    } else {
      if (NEvents % 1000 == 0) cout << "Processing Event " << NEvents << "\n";
      
       //now we hit the next event, so we need to process the previous event

      //cout << "New Event: " << time << " " << t_diff << "\n";

      //first find the ref time of the previous event
      long long tRef = -1;
      bool foundRef = false;
      for (int i=0; i<eventData.size();i++) {
	if (eventData[i].second == REF_CHANNEL) {
	  //TPrev = eventData[i].first;
	  tRef = eventData[i].first;
	  //cout << "set tRef = " << tRef << "\n";
	  foundRef = true;
	}
      }

      if (foundRef) {
	//fill the output timestamp arrays with time relative to tRef
	for (int i=0; i<eventData.size();i++) {
	  //cout << eventData[i].second << " : " << eventData[i].first << "\n";
	  //make sure the channel number is in range
	  assert( eventData[i].second >= 1 && eventData[i].second <= 4); 
	  chTime[ eventData[i].second ] = eventData[i].first - tRef;
	  //cout << "Data: " << eventData[i].first << " " << chTime[ eventData[i].second ] << "\n";	
	}
	outTree->Fill();
      }
      NEvents++;
      
      //clear eventData and add new entry
      eventData.clear();
      eventData.push_back( pair<long long, int>(time, channel));
      TPrev = time;

    }
   
  }

  cout << "NEvents Processed: " << NEvents << "\n";
  inputfile.close();
  outFile->Write();
  outFile->Close();

}
