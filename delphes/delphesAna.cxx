#include "classes/DelphesClasses.h"
#include "ExRootAnalysis/ExRootTreeReader.h"
#include "ExRootAnalysis/ExRootResult.h"

#include <TClonesArray.h>
#include <TLorentzVector.h>
#include <TSystem.h>
#include <unistd.h>
#include <iostream>

#include "DelphesUtils/DelphesNtuple.hpp"

using namespace std;

void AnalysisEvents(ExRootTreeReader* treeReader, DelphesNtuple* ntuple, bool debug=false) {
  // TODO: 
  // 1) add 4 vector of the generated tau
  // 2) label gen jets produced with tau
  // 3) match tracks to jets
  // TODO
  // TODO: add di-tau
  TClonesArray *branchParticle = treeReader->UseBranch("Particle");
  TClonesArray *branchElectron = treeReader->UseBranch("Electron");
  TClonesArray *branchPhoton = treeReader->UseBranch("Photon");
  TClonesArray *branchMuon = treeReader->UseBranch("Muon");
  
  TClonesArray *branchTrack = treeReader->UseBranch("Track");
  TClonesArray *branchTower = treeReader->UseBranch("Tower");

  TClonesArray *branchEFlowTrack = treeReader->UseBranch("EFlowTrack");
  TClonesArray *branchEFlowPhoton = treeReader->UseBranch("EFlowPhoton");
  TClonesArray *branchEFlowNeutralHadron = treeReader->UseBranch("EFlowNeutralHadron");

  TClonesArray *branchJet = treeReader->UseBranch("Jet");
  TClonesArray *branchGenJet = treeReader->UseBranch("GenJet");

  Long64_t allEntries = treeReader->GetEntries();
  printf("Total %lld entries\n", allEntries);

  Long64_t entry;
  Jet* jet;
  TObject *object;
  TLorentzVector momentum;

  GenParticle *particle;
  Track *track;
  Tower *tower;

  Int_t i, j, pdgCode;
  bool myevt = false;
  for(entry = 39; entry < allEntries; ++entry) {
    treeReader->ReadEntry(entry);
    ntuple->Clear();
    if (branchGenJet->GetEntriesFast() < 1) continue;
    if (debug) {
      cout << "Event " << entry << ", Electrons: " << branchElectron->GetEntriesFast() \
           << ", Muons: " << branchMuon->GetEntriesFast() \
           << ", Photons: " << branchPhoton->GetEntriesFast() \
           << ", Jets: " << branchJet->GetEntriesFast() \
           << ", Gen Jets:" << branchGenJet->GetEntriesFast() \
           << ", Tracks: " << branchTrack->GetEntriesFast() \
           << ", Towers: " << branchTower->GetEntriesFast() \
           << endl;
    }
    // cout << "HERE1, " << branchGenJet->GetEntriesFast() << ", " << branchJet->GetEntriesFast() <<  endl;
    // Loop over all truth jets in event
    int n_jets=0, n_bjets=0, n_taujets=0;
    for(i = 0; i < branchGenJet->GetEntriesFast(); ++i) {
      jet = (Jet*) branchGenJet->At(i);
      ntuple->FillGenJet(jet);
      n_jets ++;
      if(jet->BTag) n_bjets ++;
      if(jet->TauTag) n_taujets ++;
      // constituents
      momentum.SetPxPyPzE(0.0, 0.0, 0.0, 0.0);
      if (debug) printf("Truth Jet: %d, %d, %.2f %.2f %.2f\n", i, jet->Constituents.GetEntriesFast(), jet->PT, jet->Eta, jet->Phi);
      for(j = 0; j < jet->Constituents.GetEntriesFast(); ++j) {
        object = jet->Constituents.At(j);
        if(object == 0) continue;
        if(object->IsA() == GenParticle::Class())
        {
          particle = (GenParticle*) object;
          momentum += particle->P4();
          GenParticle* m1 = (GenParticle*) branchParticle->At(particle->M1);
          if(debug) cout << "    GenPart pt: " << particle->PT << ", eta: " << particle->Eta << ", phi: " \
            << particle->Phi << ", ID: " << particle->PID << ", M1: " << m1->PID << endl;

        }
        else if(object->IsA() == Track::Class())
        {
          track = (Track*) object;
          if(debug) cout << "    Track pt: " << track->PT << ", eta: " << track->Eta << ", phi: " << track->Phi << endl;
          momentum += track->P4();
        }
        else if(object->IsA() == Tower::Class())
        {
          tower = (Tower*) object;
          if(debug) cout << "    Tower pt: " << tower->ET << ", eta: " << tower->Eta << ", phi: " << tower->Phi << endl;
          momentum += tower->P4();
        }

      }

    }
    ntuple->FillGenJetsCnt(n_jets, n_bjets, n_taujets);

    // Loop over all reco jets in event
    n_jets=0, n_bjets=0, n_taujets=0;
    for(i = 0; i < branchJet->GetEntriesFast(); ++i) {
      jet = (Jet*) branchJet->At(i);
      ntuple->FillRecoJet(jet);
      n_jets ++;
      if(jet->BTag) n_bjets ++;
      if(jet->TauTag) n_taujets ++;
      if(debug) printf("Reco Jet: %d, %d, %.2f %.2f %.2f\n", i, jet->Constituents.GetEntriesFast(), jet->PT, jet->Eta, jet->Phi);
      // constituents
      momentum.SetPxPyPzE(0.0, 0.0, 0.0, 0.0);
      for(j = 0; j < jet->Constituents.GetEntriesFast(); ++j) {

        object = jet->Constituents.At(j);
        if(object == 0) continue;
        if(object->IsA() == GenParticle::Class())
        {
          particle = (GenParticle*) object;
          if (debug) cout << "    GenPart pt: " << particle->PT << ", eta: " << particle->Eta << ", phi: " \
            << particle->Phi << ", ID: " << particle->PID << ", M1: " << particle->M1 << endl;
          momentum += particle->P4();
          // GenParticle* m1 = (GenParticle*) branchParticle->At(particle->M1);
          // cout << "M1: " << m1->PID << endl;
        }
        else if(object->IsA() == Track::Class())
        {
          track = (Track*) object;
          if(debug) cout << "    Track pt: " << track->PT << ", eta: " << track->Eta << ", phi: " << track->Phi << endl;
          momentum += track->P4();
          myevt = true;
        }
        else if(object->IsA() == Tower::Class())
        {
          tower = (Tower*) object;
          if (debug) cout << "    Tower pt: " << tower->ET << ", eta: " << tower->Eta << ", phi: " << tower->Phi << endl;
          momentum += tower->P4();

        }
      }
    }
    ntuple->FillRecoJetCnt(n_jets, n_bjets, n_taujets);

    ntuple->Fill();
    if (myevt) { fprintf(stderr, "Found tracks in reco jets %lld\n", entry); break; }
    // break;
  }
}


int main(int argc, char** argv)
{
  bool help = false;
  bool debug = false;
  int opt;
  std::string file("/global/cfs/cdirs/m3443/usr/xju/TauStudies/run_Ztautau/Ntuple_ditau.root");
  std::string outname("test.root");

  while ((opt = getopt(argc, argv, "hf:o:d")) != -1) {
    switch(opt) {
    case 'f':
      file = optarg;
      break;
    case 'o':
      outname = optarg;
      break;
    case 'd':
      debug = true;
      break;
    case 'h':
      help = true;
    default:
      fprintf(stderr, "Usage: %s [-f FILENAME]\n", argv[0]);
      if (help) {
        printf("   -f FILE : read input file\n");
      }
    }
  }

  TChain *chain = new TChain("Delphes");
  chain->Add(file.c_str());
  printf("Processing %s\n", file.c_str());

  auto ntuple = new DelphesNtuple(outname);
  ntuple->BookGenJets();
  ntuple->BookRecoJets();

  ExRootTreeReader *treeReader = new ExRootTreeReader(chain);

  AnalysisEvents(treeReader, ntuple, debug);

  delete chain;
  delete ntuple;
  delete treeReader;
  return 0;
}