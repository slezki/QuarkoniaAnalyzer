# Quarkonia Analysis

This package has been crated to analyze Quarkonium particles decay di-muon final state.

* Setup: (it has being tested on 12_3_0_pre4 should run in any of the recent cmssw releases)

```
export SCRAM_ARCH=slc7_amd64_gcc10
scram p -n CMSSW_1230pre4_mumu CMSSW_12_3_0_pre4
cd CMSSW_1230pre4_mumu/src/
cmsenv
git clone git@github.com:slezki/QuarkoniaAnalyzer.git Onia/dimuonAnalyzer/
scram b

```

* Run: (use your favorite input sample)

```
cmsRun -n 4 Onia/dimuonAnalyzer/test/run-dimuon-miniaod.py.py nEvt=1000 type=ups MCTruth=True
```
