// now in options
//=============================================//
//const char* centralityEstimator = "V0M";
//const char* centralityEstimator = "CL1";
//const char* centralityEstimator = "TRK";
//=============================================//
//Bool_t gRunShuffling = kFALSE;
//Bool_t gRunShuffling = kTRUE;
//=============================================//

//PID config
Bool_t kUseNSigmaPID = kTRUE;
Double_t nSigmaMax = 3.0;
Bool_t kUseBayesianPID = kFALSE;
Double_t gMinAcceptedProbability = 0.7;

//_________________________________________________________//
AliAnalysisTaskBF *AddTaskBalanceCentralityTrain(TString analysisType = "ESD",
						 Double_t centrMin=0.,
						 Double_t centrMax=80.,  //100.
						 Bool_t gRunShuffling=kFALSE,
						 TString centralityEstimator="V0M",
						 Double_t vertexZ=10.,
						 Double_t DCAxy=-1,
						 Double_t DCAz=-1,
						 Double_t ptMin=0.3,
						 Double_t ptMax=1.5,
						 Double_t etaMin=-0.8,
						 Double_t etaMax=0.8,
						 Double_t maxTPCchi2 = -1, 
						 Int_t minNClustersTPC = -1,
						 Bool_t kUsePID = kFALSE,
						 Int_t AODfilterBit = 128,
						 Bool_t bCentralTrigger = kFALSE,
						 Bool_t bHBTcut = kFALSE,
						 Bool_t bConversionCut = kFALSE,
						 TString fileNameBase="AnalysisResults") {

  // Creates a balance function analysis task and adds it to the analysis manager.
  // Get the pointer to the existing analysis manager via the static access method.
  TString centralityName("");
  centralityName+=Form("%.0f",centrMin);
  centralityName+="-";
  centralityName+=Form("%.0f",centrMax);
  centralityName+="_";
  centralityName+=Form("%s",centralityEstimator.Data());
  centralityName+="_";
  centralityName+=Form("vZ%.1f",vertexZ);
  centralityName+="_";
  centralityName+=Form("DCAxy%.1f",DCAxy);
  centralityName+="_";
  centralityName+=Form("DCAz%.1f",DCAz);
  centralityName+="_Pt";
  centralityName+=Form("%.1f",ptMin);
  centralityName+="-";
  centralityName+=Form("%.1f",ptMax);
  centralityName+="_Eta";
  centralityName+=Form("%.1f",etaMin);
  centralityName+="-";
  centralityName+=Form("%.1f",etaMax);
  centralityName+="_Chi";
  centralityName+=Form("%.1f",maxTPCchi2);
  centralityName+="_nClus";
  centralityName+=Form("%d",minNClustersTPC);
  centralityName+="_Bit";
  centralityName+=Form("%d",AODfilterBit);
  if(bCentralTrigger)   centralityName+="_withCentralTrigger";
  if(bHBTcut)           centralityName+="_withHBTcut";
  if(bConversionCut)    centralityName+="_withConversionCut";
  if(kUsePID)           centralityName+="_PID";





  TString outputFileName(fileNameBase);
  outputFileName.Append(".root");

  //===========================================================================
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) {
    ::Error("AddTaskBF", "No analysis manager to connect to.");
    return NULL;
  }

  // Check the analysis type using the event handlers connected to the analysis manager.
  //===========================================================================
  if (!mgr->GetInputEventHandler()) {
    ::Error("AddTaskBF", "This task requires an input event handler");
    return NULL;
  }
  TString analysisType = mgr->GetInputEventHandler()->GetDataType(); // can be "ESD" or "AOD" or "MC"
  if(dynamic_cast<AliMCEventHandler*> (AliAnalysisManager::GetAnalysisManager()->GetMCtruthEventHandler())) analysisType = "AOD";


  // for local changed BF configuration
  //gROOT->LoadMacro("./configBalanceFunctionAnalysis.C");
  gROOT->LoadMacro("$ALICE_PHYSICS/PWGCF/EBYE/macros/configBalanceFunctionAnalysis.C");
  AliBalance *bf  = 0;  // Balance Function object
  AliBalance *bfs = 0;  // shuffled Balance function objects

  if (analysisType=="ESD"){
    bf  = GetBalanceFunctionObject("ESD",centralityEstimator,centrMin,centrMax,kFALSE,bHBTcut,bConversionCut,kUsePID);
    if(gRunShuffling) bfs = GetBalanceFunctionObject("ESD",centralityEstimator,centrMin,centrMax,kTRUE,bHBTcut,bConversionCut,kUsePID);
  }
  else if (analysisType=="MCESD"){
    bf  = GetBalanceFunctionObject("MCESD",centralityEstimator,centrMin,centrMax,kFALSE,bHBTcut,bConversionCut,kUsePID);
    if(gRunShuffling) bfs = GetBalanceFunctionObject("MCESD",centralityEstimator,centrMin,centrMax,kTRUE,bHBTcut,bConversionCut,kUsePID);
  }  
  else if (analysisType=="AOD"){
    bf  = GetBalanceFunctionObject("AOD",centralityEstimator,centrMin,centrMax,kFALSE,bHBTcut,bConversionCut,kUsePID);
    if(gRunShuffling) bfs = GetBalanceFunctionObject("AOD",centralityEstimator,centrMin,centrMax,kTRUE,bHBTcut,bConversionCut,kUsePID);
  }
  else if (analysisType=="MC"){
    bf  = GetBalanceFunctionObject("MC",centralityEstimator,centrMin,centrMax,kFALSE,bHBTcut,bConversionCut,kUsePID);
    if(gRunShuffling) bfs = GetBalanceFunctionObject("MC",centralityEstimator,centrMin,centrMax,kTRUE,bHBTcut,bConversionCut,kUsePID);
  }
  else{
    ::Error("AddTaskBF", "analysis type NOT known.");
    return NULL;
  }

  // Create the task, add it to manager and configure it.
  //===========================================================================
  AliAnalysisTaskBF *taskBF = new AliAnalysisTaskBF("TaskBF");
  taskBF->SetAnalysisObject(bf);
  if(gRunShuffling) taskBF->SetShufflingObject(bfs);

  taskBF->SetCentralityPercentileRange(centrMin,centrMax);
  if((analysisType == "ESD")||(analysisType == "MCESD")) {
    AliESDtrackCuts *trackCuts = GetTrackCutsObject(ptMin,ptMax,etaMin,etaMax,maxTPCchi2,DCAxy,DCAz,minNClustersTPC);
    taskBF->SetAnalysisCutObject(trackCuts);
    if(kUsePID) {
      if(kUseBayesianPID)
	taskBF->SetUseBayesianPID(gMinAcceptedProbability);
      else if(kUseNSigmaPID)
	taskBF->SetUseNSigmaPID(nSigmaMax);
      taskBF->SetParticleOfInterest(AliAnalysisTaskBF::kPion);
      taskBF->SetDetectorUsedForPID(AliAnalysisTaskBF::kTPCpid);
    }
  }
  else if(analysisType == "AOD") {
    // pt and eta cut (pt_min, pt_max, eta_min, eta_max)
    taskBF->SetAODtrackCutBit(AODfilterBit);
    taskBF->SetKinematicsCutsAOD(ptMin,ptMax,etaMin,etaMax);

    // set extra DCA cuts (-1 no extra cut)
    taskBF->SetExtraDCACutsAOD(DCAxy,DCAz);

    // PID
    if(kUsePID) {
      if(kUseBayesianPID)
	taskBF->SetUseBayesianPID(gMinAcceptedProbability);
      else if(kUseNSigmaPID)
	taskBF->SetUseNSigmaPID(nSigmaMax);
      taskBF->SetParticleOfInterest(AliAnalysisTaskBF::kPion);
      taskBF->SetDetectorUsedForPID(AliAnalysisTaskBF::kTPCpid);
    }

    // set extra TPC chi2 / nr of clusters cut
    taskBF->SetExtraTPCCutsAOD(maxTPCchi2, minNClustersTPC);
    
  }
  else if(analysisType == "MC") {
    taskBF->SetKinematicsCutsAOD(ptMin,ptMax,etaMin,etaMax); 
  }

  // offline trigger selection (AliVEvent.h)
  // taskBF->UseOfflineTrigger(); // NOT used (selection is done with the AliAnalysisTaskSE::SelectCollisionCandidates()) 
  // with this only selected events are analyzed (first 2 bins in event QA histogram are the same))
  // documentation in https://twiki.cern.ch/twiki/bin/viewauth/ALICE/PWG1EvSelDocumentation
  if(bCentralTrigger) taskBF->SelectCollisionCandidates(AliVEvent::kMB | AliVEvent::kCentral | AliVEvent::kSemiCentral);
  else                taskBF->SelectCollisionCandidates(AliVEvent::kMB);

  // centrality estimator (default = V0M)
  taskBF->SetCentralityEstimator(centralityEstimator);
  
  // vertex cut (x,y,z)
  taskBF->SetVertexDiamond(.3,.3,vertexZ);
  


  //bf->PrintAnalysisSettings();
  mgr->AddTask(taskBF);
  
  // Create ONLY the output containers for the data produced by the task.
  // Get and connect other common input/output containers via the manager as below
  //==============================================================================
  TString outputFileName = AliAnalysisManager::GetCommonFileName();
  outputFileName += ":PWGCFEbyE.outputBalanceFunctionAnalysis";
  AliAnalysisDataContainer *coutQA = mgr->CreateContainer(Form("listQA_%s",centralityName.Data()), TList::Class(),AliAnalysisManager::kOutputContainer,outputFileName.Data());
  AliAnalysisDataContainer *coutBF = mgr->CreateContainer(Form("listBF_%s",centralityName.Data()), TList::Class(),AliAnalysisManager::kOutputContainer,outputFileName.Data());
  if(gRunShuffling) AliAnalysisDataContainer *coutBFS = mgr->CreateContainer(Form("listBFShuffled_%s",centralityName.Data()), TList::Class(),AliAnalysisManager::kOutputContainer,outputFileName.Data());
  if(kUsePID) AliAnalysisDataContainer *coutQAPID = mgr->CreateContainer(Form("listQAPID_%s",centralityName.Data()), TList::Class(),AliAnalysisManager::kOutputContainer,outputFileName.Data());

  mgr->ConnectInput(taskBF, 0, mgr->GetCommonInputContainer());
  mgr->ConnectOutput(taskBF, 1, coutQA);
  mgr->ConnectOutput(taskBF, 2, coutBF);
  if(gRunShuffling) mgr->ConnectOutput(taskBF, 3, coutBFS);
  if(kUsePID) mgr->ConnectOutput(taskBF, 4, coutQAPID);

  return taskBF;
}
