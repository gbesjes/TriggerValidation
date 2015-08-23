# TriggerValidation
Package to study the trigger chains used in the analysis of the Standard Model Higgs decaying in a pair of leptons tau performed by the ATLAS experiment

## Dependencies
- [rootpy](http://www.rootpy.org/) for the plotting scripts
- ATLAS analysis release 2.3.24,Base or higher
- TrigTauMatching and TrigDecisionTool
- TrigTauEmulationTool (for the emulation part only)

## Trigger Emulation
Two EventLoop algorithm are available in the package to run the tau trigger emulation tool.
- L1 emulation: `L1EmulationLoop`
- HLT emulation: `HLTEmulationLoop`

A python script is also available to pilot those two algorithms.
- emulation driver: `validator`

- Running the L1 on Ztautau MC
```
validator L1 Z 
```
- Running the HLT on Ztautau MC:
```
validator HLT Z
```

- Full list of options for the script
```
usage: validator [-h] [--path PATH] [--run-dir RUN_DIR] [--l1-trig L1_TRIG]
                 [--hlt-trig HLT_TRIG] [--hlt-ref-trig HLT_REF_TRIG]
                 [--verbose] [--num-events NUM_EVENTS]
                 {l1,hlt} {Z,EB}

positional arguments:
  {l1,hlt}              Choose l1 or hlt
  {Z,EB}                Choose Z or EB

optional arguments:
  -h, --help            show this help message and exit
  --path PATH           default =
                        /afs/cern.ch/user/q/qbuat/work/public/mc15_13TeV/test/
  --run-dir RUN_DIR     default = None
  --l1-trig L1_TRIG     default = None
  --hlt-trig HLT_TRIG   default = HLT_tau25_perf_tracktwo
  --hlt-ref-trig HLT_REF_TRIG
                        default = HLT_tau25_idperf_tracktwo
  --verbose             default = False
  --num-events NUM_EVENTS
                        default = -1
```  
